Texture2D texture0;
SamplerState sampler0;

cbuffer LiquidGlassConstants : register(b0)
{
    // x = border thickness, y = corner radius, z = artistic refraction scale, w = optical bezel width
    float4 MaterialParams0;
    // x = highlight strength, y = edge softness, z = chromatic dispersion, w = material opacity
    float4 MaterialParams1;
    // x = glass thickness, y = refractive index, z = tint opacity, w = base saturation
    float4 MaterialParams2;
    // x = light angle, y = surface profile, z = magnification strength, w = highlight sharpness
    float4 MaterialParams3;
    // xyz = tint color, w = inner shadow strength
    float4 MaterialParams4;
    // x = specular-only saturation, y = specular width in DIPs, z = contrast, w = exposure in stops
    float4 MaterialParams5;
};

float RoundedRectSdf(float2 p, float2 halfSize, float radius)
{
    float2 q = abs(p) - (halfSize - radius.xx);
    return length(max(q, 0.0f.xx)) + min(max(q.x, q.y), 0.0f) - radius;
}

float ConvexSquircleRaw(float t)
{
    float s = 1.0f - clamp(t, 0.0f, 2.0f);
    return pow(saturate(1.0f - s * s * s * s), 0.25f);
}

float ConvexSquircle(float t)
{
    return ConvexSquircleRaw(saturate(t));
}

float ConvexCircle(float t)
{
    float s = 1.0f - saturate(t);
    return sqrt(saturate(1.0f - s * s));
}

float ConcaveCircle(float t)
{
    return 1.0f - ConvexCircle(t);
}

float SmootherStep01(float t)
{
    t = saturate(t);
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float SurfaceHeight(float t, float profile)
{
    t = saturate(t);
    float result = 0.0f;

    if (profile < 0.5f)
    {
        result = ConvexSquircle(t);
    }
    else if (profile < 1.5f)
    {
        result = ConvexCircle(t);
    }
    else if (profile < 2.5f)
    {
        result = ConcaveCircle(t);
    }
    else
    {
        // Keep x * 2 unsaturated here. kube's Lip deliberately lets the squircle arc
        // travel through the second half of its domain before smootherstep blends it
        // into the concave surface.
        float convex = ConvexSquircleRaw(t * 2.0f);
        float concave = ConcaveCircle(t) + 0.1f;
        result = lerp(convex, concave, SmootherStep01(t));
    }

    return result;
}

float SurfaceDerivative(float t, float profile)
{
    const float delta = 0.0005f;
    const float step = t < 1.0f - delta ? delta : -delta;
    const float y = SurfaceHeight(t, profile);
    return (SurfaceHeight(t + step, profile) - y) / step;
}

float CalculateReferenceRefractionDistance(
    float height,
    float derivative,
    float bezelWidth,
    float glassThickness,
    float refractiveIndex)
{
    // kube's reduced 2-D Snell model: incoming ray is [0, 1], ambient IOR is 1,
    // and the ray refracts once through the curved top surface.
    const float inverseLength = rsqrt(max(derivative * derivative + 1.0f, 1e-6f));
    const float2 surfaceNormal = float2(-derivative * inverseLength, -inverseLength);
    const float eta = 1.0f / max(refractiveIndex, 1.0001f);
    const float normalDotIncident = surfaceNormal.y;
    const float k = 1.0f - eta * eta * (1.0f - normalDotIncident * normalDotIncident);
    float result = 0.0f;

    if (k > 0.0f)
    {
        const float q = eta * normalDotIncident + sqrt(k);
        const float2 refracted = float2(
            -q * surfaceNormal.x,
            eta - q * surfaceNormal.y);

        if (abs(refracted.y) > 1e-5f)
        {
            const float remainingHeight = height * bezelWidth + max(glassThickness, 0.0f);
            result = refracted.x * (remainingHeight / refracted.y);
        }
    }

    return result;
}

float2 RoundedRectNormal(float2 local, float2 halfRect, float radius, float centerSdf)
{
    const float epsilon = 0.5f;
    float2 gradient = float2(
        RoundedRectSdf(local + float2(epsilon, 0.0f), halfRect, radius) - centerSdf,
        RoundedRectSdf(local + float2(0.0f, epsilon), halfRect, radius) - centerSdf);
    float gradientLength = length(gradient);
    return gradientLength > 1e-5f ? gradient / gradientLength : float2(0.0f, -1.0f);
}

float3 ApplySaturation(float3 color, float saturation)
{
    const float luminance = dot(color, float3(0.2126f, 0.7152f, 0.0722f));
    return lerp(luminance.xxx, color, max(saturation, 0.0f));
}

float3 ApplyExposureContrast(float3 color, float exposure, float contrast)
{
    // Exposure is expressed in photographic stops. Contrast is centered around
    // middle gray so 1.0 is neutral and 0.0 collapses to 50% gray.
    color *= exp2(exposure);
    return (color - 0.5f.xxx) * max(contrast, 0.0f) + 0.5f.xxx;
}

float2 ClampSampleUv(float2 uv, float2 contentMin, float2 contentMax, float2 texelSize, bool hasContentRect)
{
    float2 result = saturate(uv);

    if (hasContentRect)
    {
        float2 padding = texelSize * 0.5f;
        float2 minimumUv = min(contentMin + padding, contentMax - padding);
        float2 maximumUv = max(contentMin + padding, contentMax - padding);
        result = clamp(uv, minimumUv, maximumUv);
    }

    return result;
}

float4 SampleTransmission(float2 uv, float2 contentMin, float2 contentMax, float2 texelSize, bool hasContentRect)
{
    return texture0.Sample(sampler0, ClampSampleUv(uv, contentMin, contentMax, texelSize, hasContentRect));
}

float ReferenceSpecularCoefficient(
    float distanceFromEdge,
    float specularWidth,
    float feather,
    float2 normal,
    float2 lightDirection,
    float highlightSharpness)
{
    const float width = max(specularWidth, 0.25f);
    const float edgeT = saturate(distanceFromEdge / width);
    const float arc = sqrt(saturate(1.0f - (1.0f - edgeT) * (1.0f - edgeT)));
    const float band = 1.0f - smoothstep(width, width + max(feather, 0.5f), distanceFromEdge);
    const float orientation = pow(saturate(abs(dot(normal, lightDirection))), max(highlightSharpness, 0.25f));
    return saturate(orientation * arc * band);
}

float4 LiquidGlassCore(float2 uv, float4 samplerDataExt, float4 samplerData)
{
    const float borderThickness = MaterialParams0.x;
    const float cornerRadius = MaterialParams0.y;
    const float refractionStrength = MaterialParams0.z;
    const float bezelWidth = MaterialParams0.w;
    const float highlightStrength = MaterialParams1.x;
    const float edgeSoftness = MaterialParams1.y;
    const float dispersionStrength = MaterialParams1.z;
    const float materialOpacity = MaterialParams1.w;
    const float glassThickness = MaterialParams2.x;
    const float refractiveIndex = max(MaterialParams2.y, 1.0001f);
    const float tintOpacity = saturate(MaterialParams2.z);
    const float saturation = max(MaterialParams2.w, 0.0f);
    const float lightAngle = MaterialParams3.x;
    const float surfaceProfile = clamp(MaterialParams3.y, 0.0f, 3.0f);
    const float magnificationStrength = max(MaterialParams3.z, 0.0f);
    const float highlightSharpness = max(MaterialParams3.w, 0.25f);
    const float3 tintColor = saturate(MaterialParams4.xyz);
    const float innerShadowStrength = saturate(MaterialParams4.w);
    const float specularSaturation = max(MaterialParams5.x, 0.0f);
    const float specularWidth = max(MaterialParams5.y, 0.25f);
    const float contrast = max(MaterialParams5.z, 0.0f);
    const float exposure = clamp(MaterialParams5.w, -4.0f, 4.0f);

    const float2 contentMin = min(samplerData.xy, samplerData.zw);
    const float2 contentMax = max(samplerData.xy, samplerData.zw);
    const float2 contentUvSizeRaw = contentMax - contentMin;
    const bool hasContentRect = all(contentUvSizeRaw > 1e-6f.xx);
    const float2 contentUvSize = hasContentRect ? contentUvSizeRaw : 1.0f.xx;
    const float2 localUv = hasContentRect ? ((uv - contentMin) / contentUvSize) : uv;

    const float2 localUvPixelStep = max(abs(ddx(localUv)) + abs(ddy(localUv)), 1e-6f.xx);
    const float2 rectSize = max(1.0f.xx / localUvPixelStep, 1.0f.xx);
    const float2 texelSize = max(samplerDataExt.zw, 1e-6f.xx);
    const float2 localPosition = localUv * rectSize;
    const float2 halfRect = rectSize * 0.5f;
    const float2 local = localPosition - halfRect;
    const float halfMinSize = max(min(halfRect.x, halfRect.y), 1.0f);
    const float radius = clamp(cornerRadius, 0.0f, halfMinSize);
    const float sdf = RoundedRectSdf(local, halfRect, radius);

    // Coverage is evaluated after the materialized Gaussian blur. Blur therefore affects
    // transmitted content without changing the rounded-rectangle silhouette.
    const float feather = max(edgeSoftness, 0.25f);
    const float coverage = saturate(0.5f - sdf / feather);
    const float alpha = coverage * saturate(materialOpacity);

    float4 result = 0.0f.xxxx;
    if (alpha > 0.0f)
    {
        const float distanceFromEdge = max(-sdf, 0.0f);
        // Border radius only changes the SDF geometry. The optical bezel is an independent
        // physical width exactly like kube's distanceFromSide / bezelWidth model.
        const float maximumBezel = max(halfMinSize - 0.5f, 1.0f);
        const float bezel = clamp(bezelWidth, 1.0f, maximumBezel);
        const float bezelT = saturate(distanceFromEdge / bezel);
        const float height = SurfaceHeight(bezelT, surfaceProfile);
        const float derivative = SurfaceDerivative(bezelT, surfaceProfile);
        const float referenceDisplacement = CalculateReferenceRefractionDistance(
            height,
            derivative,
            bezel,
            glassThickness,
            refractiveIndex);
        const float artisticScale = max(refractionStrength, 0.0f) / 24.0f;
        const float displacementPixels = referenceDisplacement * artisticScale;

        const float2 normal = RoundedRectNormal(local, halfRect, radius, sdf);
        const float2 refractionPixelOffset = -normal * displacementPixels;
        const float2 refractedUv = uv + refractionPixelOffset * texelSize;

        // kube's magnifier is a first displacement pass whose output is then sampled by the
        // refraction pass. Because its field is linear/radial, evaluating the magnification
        // at the already-refracted coordinate reproduces that two-stage composition exactly:
        // Source(x + R(x) + M(x + R(x))).
        const float maximumHalfExtent = max(max(halfRect.x, halfRect.y), 1.0f);
        const float2 refractedLocal = local + refractionPixelOffset;
        const float2 normalizedMagnification = refractedLocal / maximumHalfExtent;
        const float2 magnificationOffset = -normalizedMagnification * texelSize * magnificationStrength;
        const float2 sampleUv = refractedUv + magnificationOffset;

        const float bezelWeight = 1.0f - smoothstep(0.18f, 1.0f, bezelT);
        const float dispersionPixels = dispersionStrength * bezelWeight *
            (0.35f + min(abs(displacementPixels) * 0.04f, 1.5f));
        const float2 dispersionOffset = normal * texelSize * dispersionPixels;

        float3 color = float3(
            SampleTransmission(sampleUv - dispersionOffset, contentMin, contentMax, texelSize, hasContentRect).r,
            SampleTransmission(sampleUv, contentMin, contentMax, texelSize, hasContentRect).g,
            SampleTransmission(sampleUv + dispersionOffset, contentMin, contentMax, texelSize, hasContentRect).b);
        color = ApplySaturation(color, saturation);
        color = ApplyExposureContrast(color, exposure, contrast);
        color = lerp(color, tintColor, tintOpacity);

        const float innerShadow = 1.0f - smoothstep(0.0f, max(bezel * 0.65f, 1.0f), distanceFromEdge);
        color *= 1.0f - innerShadow * innerShadowStrength;

        const float borderMask = 1.0f - smoothstep(
            max(borderThickness, 0.0f),
            max(borderThickness, 0.0f) + feather,
            distanceFromEdge);
        color = lerp(color, tintColor, borderMask * 0.20f * highlightStrength);

        const float2 lightDirection = normalize(float2(cos(lightAngle), sin(lightAngle)));
        const float specularCoefficient = ReferenceSpecularCoefficient(
            distanceFromEdge,
            specularWidth,
            feather,
            normal,
            lightDirection,
            highlightSharpness);

        // kube first saturates the refracted image only inside the specular image alpha,
        // then blends a faded grayscale specular image over it. The generated specular map's
        // alpha is coefficient^2, while its RGB is coefficient, hence the two terms below.
        const float specularMask = specularCoefficient * specularCoefficient;
        const float3 saturatedSpecularColor = ApplySaturation(color, specularSaturation);
        color = lerp(color, saturatedSpecularColor, specularMask);
        color += (specularCoefficient * specularMask * highlightStrength).xxx;

        const float innerRim = smoothstep(specularWidth, specularWidth + 1.0f, distanceFromEdge) *
            (1.0f - smoothstep(specularWidth + 1.0f, specularWidth + 3.0f + feather, distanceFromEdge));
        color += (innerRim * 0.10f * highlightStrength).xxx;
        color = saturate(color);

        result = float4(color * alpha, alpha);
    }

    return result;
}

export float4 MaterializeColor(float4 color) { return color; }

export float4 PSBody(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyCC(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyCW(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyCM(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyWC(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyWW(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyWM(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyMC(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyMW(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyMM(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyC(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyW(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyM(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
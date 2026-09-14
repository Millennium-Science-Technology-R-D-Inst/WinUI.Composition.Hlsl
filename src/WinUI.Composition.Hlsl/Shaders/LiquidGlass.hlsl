Texture2D texture0;
SamplerState sampler0;

cbuffer LiquidGlassConstants : register(b0)
{
    // x = border thickness, y = corner radius, z = artistic refraction scale, w = optical bezel width
    float4 MaterialParams0;
    // x = highlight strength, y = edge softness, z = chromatic dispersion, w = material opacity
    float4 MaterialParams1;
    // x = glass thickness, y = refractive index, z = tint opacity, w = saturation
    float4 MaterialParams2;
    // x = light angle, y = surface profile, z = magnification strength, w = highlight sharpness
    float4 MaterialParams3;
    // xyz = tint color, w = inner shadow strength
    float4 MaterialParams4;
};

float RoundedRectSdf(float2 p, float2 halfSize, float radius)
{
    float2 q = abs(p) - (halfSize - radius.xx);
    return length(max(q, 0.0f.xx)) + min(max(q.x, q.y), 0.0f) - radius;
}

float ConvexSquircle(float t)
{
    float s = 1.0f - saturate(t);
    return pow(saturate(1.0f - s * s * s * s), 0.25f);
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
    // Profiles intentionally mirror the reference implementation: convex squircle,
    // convex circle, concave circle, and a lip that blends convex and concave surfaces.
    t = saturate(t);
    if (profile < 0.5f)
    {
        return ConvexSquircle(t);
    }
    if (profile < 1.5f)
    {
        return ConvexCircle(t);
    }
    if (profile < 2.5f)
    {
        return ConcaveCircle(t);
    }

    float convex = ConvexSquircle(saturate(t * 2.0f));
    float concave = ConcaveCircle(t) + 0.1f;
    return lerp(convex, concave, SmootherStep01(t));
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

float2 ClampSampleUv(float2 uv, float2 contentMin, float2 contentMax, float2 texelSize, bool hasContentRect)
{
    if (!hasContentRect)
    {
        return saturate(uv);
    }

    float2 padding = texelSize * 0.5f;
    float2 minimumUv = min(contentMin + padding, contentMax - padding);
    float2 maximumUv = max(contentMin + padding, contentMax - padding);
    return clamp(uv, minimumUv, maximumUv);
}

float4 SampleTransmission(float2 uv, float2 contentMin, float2 contentMax, float2 texelSize, bool hasContentRect)
{
    return texture0.Sample(sampler0, ClampSampleUv(uv, contentMin, contentMax, texelSize, hasContentRect));
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

    // Coverage is evaluated independently from the upstream Gaussian blur. Blur changes
    // transmitted content but cannot soften or square off the material silhouette.
    const float feather = max(edgeSoftness, 0.5f);
    const float coverage = saturate(0.5f - sdf / feather);
    const float alpha = coverage * saturate(materialOpacity);

    float4 result = 0.0f.xxxx;
    if (alpha > 0.0f)
    {
        const float distanceFromEdge = max(-sdf, 0.0f);
        const float maximumBezel = max(min(radius > 0.0f ? radius : halfMinSize, halfMinSize) - 0.5f, 1.0f);
        const float bezel = clamp(bezelWidth, 1.0f, maximumBezel);
        const float bezelT = saturate(distanceFromEdge / bezel);

        // Central difference preserves derivative sign for concave/lip profiles. Clamp the
        // slope angle before tan() so pathological edge derivatives cannot explode the UVs.
        const float derivativeStep = 0.0015f;
        const float t0 = max(bezelT - derivativeStep, 0.0f);
        const float t1 = min(bezelT + derivativeStep, 1.0f);
        const float height = SurfaceHeight(bezelT, surfaceProfile);
        const float derivative = (SurfaceHeight(t1, surfaceProfile) - SurfaceHeight(t0, surfaceProfile)) /
            max(t1 - t0, 1e-5f);
        const float slopeAngle = clamp(atan(derivative * (glassThickness / max(bezel, 1.0f))), -1.35f, 1.35f);
        const float refractedSin = clamp(sin(slopeAngle) / refractiveIndex, -1.0f, 1.0f);
        const float refractedAngle = asin(refractedSin);
        const float physicalDisplacement = height * glassThickness * (tan(slopeAngle) - tan(refractedAngle));
        const float artisticScale = max(refractionStrength, 0.0f) / 24.0f;
        const float displacementPixels = physicalDisplacement * artisticScale;

        const float2 normal = RoundedRectNormal(local, halfRect, radius, sdf);

        // The SVG reference's optional magnification is a radial pre-displacement stage.
        // Expressing it in pixels keeps the control independent of its pixel dimensions.
        const float2 normalizedLocal = local / max(halfRect, 1.0f.xx);
        const float2 magnificationOffset = -normalizedLocal * texelSize * magnificationStrength;
        const float2 refractUv = uv + magnificationOffset - normal * texelSize * displacementPixels;

        const float bezelWeight = 1.0f - smoothstep(0.20f, 1.0f, bezelT);
        const float dispersionPixels = dispersionStrength * bezelWeight * (0.35f + min(abs(displacementPixels) * 0.04f, 1.5f));
        const float2 dispersionOffset = normal * texelSize * dispersionPixels;

        float3 color = float3(
            SampleTransmission(refractUv - dispersionOffset, contentMin, contentMax, texelSize, hasContentRect).r,
            SampleTransmission(refractUv, contentMin, contentMax, texelSize, hasContentRect).g,
            SampleTransmission(refractUv + dispersionOffset, contentMin, contentMax, texelSize, hasContentRect).b);
        color = ApplySaturation(color, saturation);
        color = lerp(color, tintColor, tintOpacity);

        const float2 lightDirection = normalize(float2(cos(lightAngle), sin(lightAngle)));
        const float rimDot = abs(dot(normal, lightDirection));
        const float rimFalloff = 1.0f - smoothstep(0.0f, max(bezel * 0.45f, 1.0f), distanceFromEdge);
        const float specular = pow(saturate(rimDot * rimFalloff), highlightSharpness) * highlightStrength;

        const float innerShadow = 1.0f - smoothstep(0.0f, max(bezel * 0.65f, 1.0f), distanceFromEdge);
        color *= 1.0f - innerShadow * innerShadowStrength;

        const float innerRim = smoothstep(0.0f, 2.0f, distanceFromEdge) *
            (1.0f - smoothstep(2.0f, 5.0f + feather, distanceFromEdge));
        color += (specular + innerRim * 0.15f * highlightStrength).xxx;

        const float borderMask = 1.0f - smoothstep(
            max(borderThickness, 0.0f),
            max(borderThickness, 0.0f) + feather,
            distanceFromEdge);
        color = lerp(color, tintColor, borderMask * 0.20f * highlightStrength);
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

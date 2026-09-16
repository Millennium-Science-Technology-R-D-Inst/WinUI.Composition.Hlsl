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
    // xy = normalized pointer position in local control space, z = normalized interaction radius, w = strength
    float4 MaterialParams6;
    // xy = normalized pointer velocity/second, z = normalized outside hover range, w = pointer active
    float4 MaterialParams7;
    // x = pointer refraction strength, y = pointer highlight strength, z = motion refraction strength,
    // w = displacement-map normalization for the current optical surface
    float4 MaterialParams8;
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
        // Lip intentionally lets the convex arc travel through the second half of its
        // domain before smootherstep blends it into the concave surface.
        float convex = ConvexSquircleRaw(t * 2.0f);
        float concave = ConcaveCircle(t) + 0.1f;
        result = lerp(convex, concave, SmootherStep01(t));
    }
    return result;
}

float SurfaceDerivative(float t, float profile)
{
    // Match the 128-sample reference displacement generator.
    const float delta = 0.0001f;
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
    const float2 signs = float2(local.x < 0.0f ? -1.0f : 1.0f, local.y < 0.0f ? -1.0f : 1.0f);
    const float2 q = abs(local) - (halfRect - radius.xx);
    const float2 outside = max(q, 0.0f.xx);
    const float outsideLength = length(outside);
    float2 result = float2(0.0f, -1.0f);

    if (outsideLength > 1e-5f)
    {
        result = (outside / outsideLength) * signs;
    }
    else if (q.x > q.y)
    {
        result = float2(signs.x, 0.0f);
    }
    else
    {
        result = float2(0.0f, signs.y);
    }
    return result;
}

float CalculatePointerInteraction(
    float2 pixelPosition,
    float2 pointerPosition,
    float2 halfRect,
    float radius,
    float hoverRange,
    float interactionRadius)
{
    const float pointerSdf = RoundedRectSdf(pointerPosition - halfRect, halfRect, radius);
    float shapeActivation = pointerSdf <= 0.0f ? 1.0f : 0.0f;
    if (pointerSdf > 0.0f && hoverRange > 1e-4f)
        shapeActivation = 1.0f - smoothstep(0.0f, hoverRange, pointerSdf);

    const float pointerDistance = length(pixelPosition - pointerPosition);
    float localInfluence = 0.0f;
    if (interactionRadius > 1e-4f)
        localInfluence = 1.0f - smoothstep(0.0f, interactionRadius, pointerDistance);
    return saturate(shapeActivation * localInfluence);
}

float3 ApplySaturation(float3 color, float saturation)
{
    const float luminance = dot(color, float3(0.2126f, 0.7152f, 0.0722f));
    return lerp(luminance.xxx, color, max(saturation, 0.0f));
}

float3 ApplyExposureContrast(float3 color, float exposure, float contrast)
{
    color *= exp2(exposure);
    return (color - 0.5f.xxx) * max(contrast, 0.0f) + 0.5f.xxx;
}

float4 CalculateTransmissionBounds(
    float2 texelSize,
    float2 contentMin,
    float2 contentMax,
    bool hasContentRect)
{
    const float2 halfTexel = max(texelSize * 0.5f, 1e-6f.xx);
    const float2 textureMin = min(halfTexel, 1.0f.xx - halfTexel);
    const float2 textureMax = max(halfTexel, 1.0f.xx - halfTexel);
    float2 safeMin = textureMin;
    float2 safeMax = textureMax;

    if (hasContentRect)
    {
        const float2 paddingBefore = max(contentMin - textureMin, 0.0f.xx);
        const float2 paddingAfter = max(textureMax - contentMax, 0.0f.xx);

        // D2D SOFT Gaussian blur grows the output by 6 sigma in total, or about
        // 3 sigma on each side of the logical content. The outer part of that
        // allocation is the transparent-black kernel tail rather than useful
        // backdrop data. Keep the inner one-sigma region available to refraction.
        const float gaussianSupportFraction = 1.0f / 3.0f;
        safeMin = max(textureMin, contentMin - paddingBefore * gaussianSupportFraction);
        safeMax = min(textureMax, contentMax + paddingAfter * gaussianSupportFraction);
    }

    return float4(safeMin, safeMax);
}

float OffsetScaleToBounds(
    float2 origin,
    float2 offset,
    float2 safeMin,
    float2 safeMax)
{
    float result = 1.0f;
    const float epsilon = 1e-7f;

    if (offset.x > epsilon)
        result = min(result, (safeMax.x - origin.x) / offset.x);
    else if (offset.x < -epsilon)
        result = min(result, (safeMin.x - origin.x) / offset.x);

    if (offset.y > epsilon)
        result = min(result, (safeMax.y - origin.y) / offset.y);
    else if (offset.y < -epsilon)
        result = min(result, (safeMin.y - origin.y) / offset.y);

    return saturate(result);
}

float CalculateTransmissionOffsetScale(
    float2 origin,
    float2 baseOffset,
    float2 dispersionOffset,
    float2 safeMin,
    float2 safeMax)
{
    const float redScale = OffsetScaleToBounds(
        origin, baseOffset - dispersionOffset, safeMin, safeMax);
    const float greenScale = OffsetScaleToBounds(
        origin, baseOffset, safeMin, safeMax);
    const float blueScale = OffsetScaleToBounds(
        origin, baseOffset + dispersionOffset, safeMin, safeMax);
    return min(redScale, min(greenScale, blueScale));
}

float4 SampleTransmission(float2 uv, float2 texelSize)
{
    const float2 halfTexel = max(texelSize * 0.5f, 1e-6f.xx);
    const float2 textureMin = min(halfTexel, 1.0f.xx - halfTexel);
    const float2 textureMax = max(halfTexel, 1.0f.xx - halfTexel);
    const float2 sampleUv = clamp(uv, textureMin, textureMax);
    float4 result = texture0.Sample(sampler0, sampleUv);

    // D2D's SOFT Gaussian-blur border is materialized as premultiplied transparent padding.
    // Refraction/dispersion need the transmitted color, not that padding alpha folded into RGB.
    if (result.a > 1e-5f)
        result.rgb /= result.a;

    return result;
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
    const float normalizedDistance = max(distanceFromEdge, 0.0f) / width;
    const float arcTerm = 1.0f -
        (1.0f - normalizedDistance) * (1.0f - normalizedDistance);
    const float arc = sqrt(saturate(arcTerm));
    const float orientation = pow(
        saturate(abs(dot(normal, lightDirection))),
        max(highlightSharpness, 0.25f));
    const float support = 1.0f - smoothstep(
        2.0f,
        2.0f + max(feather / width, 0.25f),
        normalizedDistance);
    return saturate(orientation * arc * support);
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
    const float2 pointerNormalized = MaterialParams6.xy;
    const float pointerInteractionRadiusNormalized = max(MaterialParams6.z, 0.0f);
    const float pointerInteractionStrength = max(MaterialParams6.w, 0.0f);
    const float2 pointerVelocityNormalized = MaterialParams7.xy;
    const float pointerHoverRangeNormalized = max(MaterialParams7.z, 0.0f);
    const float pointerActive = saturate(MaterialParams7.w);
    const float pointerRefractionStrength = max(MaterialParams8.x, 0.0f);
    const float pointerHighlightStrength = max(MaterialParams8.y, 0.0f);
    const float pointerMotionRefractionStrength = max(MaterialParams8.z, 0.0f);
    const float refractionNormalization = max(MaterialParams8.w, 0.0f);

    const float2 contentMin = min(samplerData.xy, samplerData.zw);
    const float2 contentMax = max(samplerData.xy, samplerData.zw);
    const float2 contentUvSizeRaw = contentMax - contentMin;
    const bool hasContentRect = all(contentUvSizeRaw > 1e-6f.xx);
    const float2 contentUvSize = hasContentRect ? contentUvSizeRaw : 1.0f.xx;
    const float2 localUv = hasContentRect ? ((uv - contentMin) / contentUvSize) : uv;

    const float2 localUvDx = ddx(localUv);
    const float2 localUvDy = ddy(localUv);
    const float2 localUvPixelStep = max(
        float2(
            length(float2(localUvDx.x, localUvDy.x)),
            length(float2(localUvDx.y, localUvDy.y))),
        1e-6f.xx);
    const float2 rectSize = max(1.0f.xx / localUvPixelStep, 1.0f.xx);
    const float maximumExtent = max(max(rectSize.x, rectSize.y), 1.0f);
    const float2 texelSize = max(samplerDataExt.zw, 1e-6f.xx);
    const float2 localPosition = localUv * rectSize;
    const float2 pointerPosition = pointerNormalized * rectSize;
    const float2 pointerVelocity = pointerVelocityNormalized * rectSize;
    const float pointerInteractionRadius = pointerInteractionRadiusNormalized * maximumExtent;
    const float pointerHoverRange = pointerHoverRangeNormalized * maximumExtent;
    const float2 halfRect = rectSize * 0.5f;
    const float2 local = localPosition - halfRect;
    const float halfMinSize = max(min(halfRect.x, halfRect.y), 1.0f);
    const float radius = clamp(cornerRadius, 0.0f, halfMinSize);
    const float sdf = RoundedRectSdf(local, halfRect, radius);

    const float sdfPixelFootprint = max(
        length(float2(ddx(sdf), ddy(sdf))),
        0.5f);
    const float feather = max(edgeSoftness, sdfPixelFootprint * 0.5f);
    const float coverage = 1.0f - smoothstep(-feather, feather, sdf);
    const float alpha = coverage * saturate(materialOpacity);

    float4 result = 0.0f.xxxx;
    if (alpha > 0.0f)
    {
        const float distanceFromEdge = max(-sdf, 0.0f);
        const float maximumBezel = max(halfMinSize - 0.5f, 1.0f);
        const float bezel = clamp(bezelWidth, 1.0f, maximumBezel);
        const float bezelT = saturate(distanceFromEdge / bezel);
        const float height = SurfaceHeight(bezelT, surfaceProfile);
        const float derivative = SurfaceDerivative(bezelT, surfaceProfile);
        const float referenceDisplacement = CalculateReferenceRefractionDistance(
            height, derivative, bezel, glassThickness, refractiveIndex);
        const float artisticScale = max(refractionStrength, 0.0f) / 24.0f;

        // Keep optics and final coverage as separate fields, but let the optical surface cross
        // the same subpixel SDF neighborhood instead of forcing displacement to zero exactly at
        // the silhouette. The independent coverage field still owns final alpha.
        const float opticalFeather = max(max(feather, sdfPixelFootprint), 0.75f);
        const float opticalInterior = smoothstep(-opticalFeather, opticalFeather, -sdf);
        const float rawDisplacementPixels =
            referenceDisplacement * artisticScale * refractionNormalization;
        const float displacementLimit = max(maximumExtent * 0.48f, 1.0f);
        const float displacementPixels = clamp(
            rawDisplacementPixels,
            -displacementLimit,
            displacementLimit) * opticalInterior;

        const float pointerInteraction = CalculatePointerInteraction(
            localPosition,
            pointerPosition,
            halfRect,
            radius,
            pointerHoverRange,
            pointerInteractionRadius) * pointerInteractionStrength * pointerActive;

        float2 pointerDirection = 0.0f.xx;
        const float pointerDistance = length(localPosition - pointerPosition);
        if (pointerDistance > 1e-4f)
            pointerDirection = (localPosition - pointerPosition) / pointerDistance;

        float2 motionDirection = 0.0f.xx;
        const float pointerSpeed = length(pointerVelocity);
        if (pointerSpeed > 1e-4f)
            motionDirection = -pointerVelocity / pointerSpeed;

        const float pointerSpeedWeight = saturate(pointerSpeed / max(maximumExtent * 7.0f, 1.0f));
        const float2 pointerRefractionOffset =
            pointerDirection * pointerInteraction * pointerRefractionStrength * opticalInterior;
        const float2 pointerMotionOffset =
            motionDirection * pointerSpeedWeight * pointerInteraction * pointerMotionRefractionStrength * opticalInterior;

        const float2 normal = RoundedRectNormal(local, halfRect, radius, sdf);
        const float2 refractionPixelOffset =
            -normal * displacementPixels + pointerRefractionOffset + pointerMotionOffset;
        const float2 refractedUv = uv + refractionPixelOffset * texelSize;

        const float maximumHalfExtent = max(max(halfRect.x, halfRect.y), 1.0f);
        const float2 refractedLocal = local + refractionPixelOffset;
        const float2 normalizedMagnification = refractedLocal / maximumHalfExtent;
        const float magnificationEncodingScale = 127.0f / 255.0f;
        const float2 magnificationOffset =
            -normalizedMagnification * texelSize * magnificationStrength * magnificationEncodingScale * opticalInterior;
        const float2 sampleUv = refractedUv + magnificationOffset;

        const float bezelWeight = 1.0f - smoothstep(0.18f, 1.0f, bezelT);
        const float dispersionPixels = dispersionStrength * bezelWeight *
            (0.35f + min(abs(displacementPixels) * 0.04f, 1.5f)) * opticalInterior;
        const float2 dispersionOffset = normal * texelSize * dispersionPixels;

        // Apply one common boundary scale to the completed R/G/B sample offsets instead of
        // clamping each channel independently. This preserves the required ordering
        // Source(x + Refraction(x) + Magnification(x + Refraction(x))) and keeps dispersion
        // coherent while smoothly reducing only the part of the field that cannot be backed
        // by the materialized source texture.
        const float4 transmissionBounds = CalculateTransmissionBounds(
            texelSize, contentMin, contentMax, hasContentRect);
        const float2 transmissionOrigin = clamp(uv, transmissionBounds.xy, transmissionBounds.zw);
        const float2 baseSampleOffset = sampleUv - uv;
        const float transmissionScale = CalculateTransmissionOffsetScale(
            transmissionOrigin,
            baseSampleOffset,
            dispersionOffset,
            transmissionBounds.xy,
            transmissionBounds.zw);
        const float2 redSampleUv = transmissionOrigin +
            (baseSampleOffset - dispersionOffset) * transmissionScale;
        const float2 greenSampleUv = transmissionOrigin +
            baseSampleOffset * transmissionScale;
        const float2 blueSampleUv = transmissionOrigin +
            (baseSampleOffset + dispersionOffset) * transmissionScale;

        float3 color = float3(
            SampleTransmission(redSampleUv, texelSize).r,
            SampleTransmission(greenSampleUv, texelSize).g,
            SampleTransmission(blueSampleUv, texelSize).b);
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
            highlightSharpness) * opticalInterior;

        float pointerSpecular = 0.0f;
        const float pointerLightDistance = length(pointerPosition - localPosition);
        if (pointerLightDistance > 1e-4f && pointerInteraction > 0.0f)
        {
            const float2 pointerLightDirection = (pointerPosition - localPosition) / pointerLightDistance;
            pointerSpecular = ReferenceSpecularCoefficient(
                distanceFromEdge,
                specularWidth,
                feather,
                normal,
                pointerLightDirection,
                highlightSharpness) * pointerInteraction * pointerHighlightStrength * opticalInterior;
        }

        const float specularMask = specularCoefficient * specularCoefficient;
        const float3 saturatedSpecularColor = ApplySaturation(color, specularSaturation);
        color = lerp(color, saturatedSpecularColor, specularMask);
        const float specularAlpha = saturate(specularMask * highlightStrength);
        color = lerp(color, specularCoefficient.xxx, specularAlpha);
        color = lerp(color, 1.0f.xxx, saturate(pointerSpecular));
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

Texture2D texture0;
SamplerState sampler0;

cbuffer LiquidGlassConstants : register(b0)
{
    float4 MaterialParams0;
    float4 MaterialParams1;
    float4 MaterialParams2;
    float4 MaterialParams3;
    float4 MaterialParams4;
    float4 MaterialParams5;
    float4 MaterialParams6;
    float4 MaterialParams7;
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

float ConvexSquircle(float t) { return ConvexSquircleRaw(saturate(t)); }

float ConvexCircle(float t)
{
    float s = 1.0f - saturate(t);
    return sqrt(saturate(1.0f - s * s));
}

float ConcaveCircle(float t) { return 1.0f - ConvexCircle(t); }

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
        result = ConvexSquircle(t);
    else if (profile < 1.5f)
        result = ConvexCircle(t);
    else if (profile < 2.5f)
        result = ConcaveCircle(t);
    else
    {
        float convex = ConvexSquircleRaw(t * 2.0f);
        float concave = ConcaveCircle(t) + 0.1f;
        result = lerp(convex, concave, SmootherStep01(t));
    }
    return result;
}

float SurfaceDerivative(float t, float profile)
{
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
        const float2 refracted = float2(-q * surfaceNormal.x, eta - q * surfaceNormal.y);
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
        result = (outside / outsideLength) * signs;
    else if (q.x > q.y)
        result = float2(signs.x, 0.0f);
    else
        result = float2(0.0f, signs.y);
    return result;
}

float CalculatePointerInteraction(
    float2 pixelPosition,
    float2 pointerPosition,
    float2 center,
    float2 shapeHalfRect,
    float radius,
    float hoverRange,
    float interactionRadius)
{
    const float pointerSdf = RoundedRectSdf(pointerPosition - center, shapeHalfRect, radius);
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

    // The separable blur chain materializes full-color intermediates and performs
    // its own mirrored edge sampling. There is no D2D Gaussian transparent-padding
    // region to estimate here: samplerData is the authoritative logical content rect.
    if (hasContentRect)
    {
        // Stay half a texel inside the logical content rectangle. Custom-sampler
        // materialization can place transparent allocation texels immediately
        // outside samplerData's content rect; bilinear filtering exactly on that
        // boundary otherwise mixes premultiplied black into strong refraction.
        safeMin = max(textureMin, contentMin + halfTexel);
        safeMax = min(textureMax, contentMax - halfTexel);
    }

    const float2 center = (safeMin + safeMax) * 0.5f;
    safeMin = min(safeMin, center);
    safeMax = max(safeMax, center);
    return float4(safeMin, safeMax);
}

float OffsetScaleToBounds(float2 origin, float2 offset, float2 safeMin, float2 safeMax)
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

float4 SampleTransmission(float2 uv, float2 safeMin, float2 safeMax)
{
    float4 result = texture0.Sample(sampler0, clamp(uv, safeMin, safeMax));

    // The materialized backdrop is premultiplied-alpha. Strong Concave/Lip
    // refraction reaches the finite intermediate boundary first; if RGB is used
    // directly there, its allocation alpha darkens the transmitted color into a
    // black rim. Recover straight transmission color before applying glass alpha.
    if (result.a > 1e-5f)
        result.rgb /= result.a;

    return result;
}

float SpecularEdgeProfile(
    float distanceFromEdge,
    float specularWidth,
    float feather)
{
    const float width = max(specularWidth, 0.25f);
    const float normalizedDistance = max(distanceFromEdge, 0.0f) / width;
    const float arcTerm = 1.0f - (1.0f - normalizedDistance) * (1.0f - normalizedDistance);
    const float arc = sqrt(saturate(arcTerm));
    const float support = 1.0f - smoothstep(
        2.0f,
        2.0f + max(feather / width, 0.25f),
        normalizedDistance);
    return saturate(arc * support);
}

float ReferenceSpecularCoefficient(
    float distanceFromEdge,
    float specularWidth,
    float feather,
    float2 normal,
    float2 lightDirection,
    float highlightSharpness)
{
    const float edgeProfile = SpecularEdgeProfile(distanceFromEdge, specularWidth, feather);
    const float orientation = pow(
        saturate(abs(dot(normal, lightDirection))),
        max(highlightSharpness, 0.25f));
    return saturate(edgeProfile * orientation);
}

float PointerSpecularCoefficient(
    float distanceFromEdge,
    float specularWidth,
    float feather,
    float2 normal,
    float2 pointerLightDirection,
    bool pointerInside,
    float pointerInteraction,
    float highlightSharpness)
{
    const float edgeProfile = SpecularEdgeProfile(distanceFromEdge, specularWidth, feather);

    // Treat the pointer as a local light source. When it is inside the glass the
    // inward-facing rim should light; when it is outside, the outward-facing rim
    // should light. Keep a faint angular floor so the radial field reads as one
    // coherent reveal instead of disappearing abruptly around rounded corners.
    const float signedFacing = dot(normal, pointerLightDirection);
    const float facing = pointerInside ? saturate(-signedFacing) : saturate(signedFacing);
    const float angular = pow(
        saturate(0.15f + 0.85f * facing),
        max(highlightSharpness * 0.85f, 0.5f));

    // PointerInteraction already contains the rounded-rect hover gate and radial
    // distance falloff. Reshape it into a bright core with a soft reveal shoulder.
    const float radialCore = SmootherStep01(saturate(pointerInteraction));
    const float radialShoulder = sqrt(radialCore);
    const float radialField = lerp(radialCore, radialShoulder, 0.28f);
    return saturate(edgeProfile * angular * radialField);
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
        float2(length(float2(localUvDx.x, localUvDy.x)), length(float2(localUvDx.y, localUvDy.y))),
        1e-6f.xx);
    const float2 rectSize = max(1.0f.xx / localUvPixelStep, 1.0f.xx);
    const float maximumExtent = max(max(rectSize.x, rectSize.y), 1.0f);
    const float2 texelSize = max(abs(samplerDataExt.zw), 1e-6f.xx);
    const float2 localPosition = localUv * rectSize;
    const float2 pointerPosition = pointerNormalized * rectSize;
    const float2 pointerVelocity = pointerVelocityNormalized * rectSize;
    const float pointerInteractionRadius = pointerInteractionRadiusNormalized * maximumExtent;
    const float pointerHoverRange = pointerHoverRangeNormalized * maximumExtent;
    const float2 halfRect = rectSize * 0.5f;
    const float2 local = localPosition - halfRect;

    // Keep the SDF one raster pixel inside the brush bounds. LiquidGlassWinUI uses
    // the same invariant: if the geometric edge lies exactly on the brush edge,
    // antialiasing/refraction/specular coverage has nowhere to extend and rounded
    // corners get visibly cut during resize or strong refraction.
    const float shapeMargin = 1.0f;
    const float2 shapeHalfRect = max(halfRect - shapeMargin.xx, 1.0f.xx);
    const float halfMinSize = max(min(shapeHalfRect.x, shapeHalfRect.y), 1.0f);
    const float radius = clamp(cornerRadius, 0.0f, halfMinSize);
    const float sdf = RoundedRectSdf(local, shapeHalfRect, radius);

    const float sdfPixelFootprint = max(length(float2(ddx(sdf), ddy(sdf))), 0.5f);
    const float feather = max(edgeSoftness, sdfPixelFootprint * 0.5f);

    // One-sided AA: the entire geometric interior remains fully covered and only
    // the outside feather fades to transparent. The previous symmetric smoothstep
    // made the actual SDF edge 50% alpha, washing out and effectively clipping the
    // Fresnel/specular rim at all four corners.
    const float coverage = 1.0f - smoothstep(0.0f, feather, sdf);
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

        // Kube's standalone Concave profile produces a negative ray displacement:
        // at the silhouette that means sampling outside the element. CSS backdrop
        // filters can access surrounding backdrop pixels, but this WinUI custom
        // sampler receives a finite materialized texture clipped to the brush.
        //
        // LiquidGlassStudio/LiquidGlassWinUI avoid that unavailable-source problem
        // by always bending their edge field inward. Use that composition-safe
        // direction for the *pure* Concave profile while retaining its Kube surface
        // magnitude curve. Lip must stay signed: its convex outer lobe + concave
        // interior are what make the Switch optics work.
        const bool pureConcave = surfaceProfile >= 1.5f && surfaceProfile < 2.5f;
        const float transmissionDisplacement =
            pureConcave ? abs(referenceDisplacement) : referenceDisplacement;

        const float opticalFeather = max(max(feather, sdfPixelFootprint), 0.75f);
        const float opticalInterior = 1.0f - smoothstep(0.0f, opticalFeather, sdf);
        const float rawDisplacementPixels =
            transmissionDisplacement * artisticScale * refractionNormalization;
        const float displacementLimit = max(maximumExtent * 0.48f, 1.0f);
        const float displacementPixels = clamp(rawDisplacementPixels, -displacementLimit, displacementLimit) * opticalInterior;

        const float pointerInteraction = CalculatePointerInteraction(
            localPosition,
            pointerPosition,
            halfRect,
            shapeHalfRect,
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

        const float2 normal = RoundedRectNormal(local, shapeHalfRect, radius, sdf);
        const float2 refractionPixelOffset =
            -normal * displacementPixels + pointerRefractionOffset + pointerMotionOffset;
        const float2 refractedUv = uv + refractionPixelOffset * texelSize;

        // Required ordering: Source(x + Refraction(x) + Magnification(x + Refraction(x))).
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

        const float4 transmissionBounds = CalculateTransmissionBounds(
            texelSize, contentMin, contentMax, hasContentRect);
        const float2 transmissionOrigin = clamp(uv, transmissionBounds.xy, transmissionBounds.zw);
        const float2 baseSampleOffset = sampleUv - uv;

        // The source texture is finite even though a real backdrop is not. Constrain
        // each wavelength independently before sampling instead of hard-clamping the
        // completed UV (which pins a large Concave field to one border texel) or using
        // one shared RGB scale (which lets one channel collapse all three).
        const float2 redOffset = baseSampleOffset - dispersionOffset;
        const float2 greenOffset = baseSampleOffset;
        const float2 blueOffset = baseSampleOffset + dispersionOffset;
        const float redScale = OffsetScaleToBounds(
            transmissionOrigin, redOffset, transmissionBounds.xy, transmissionBounds.zw);
        const float greenScale = OffsetScaleToBounds(
            transmissionOrigin, greenOffset, transmissionBounds.xy, transmissionBounds.zw);
        const float blueScale = OffsetScaleToBounds(
            transmissionOrigin, blueOffset, transmissionBounds.xy, transmissionBounds.zw);

        float4 baseSample = SampleTransmission(
            transmissionOrigin, transmissionBounds.xy, transmissionBounds.zw);
        float4 redSample = SampleTransmission(
            transmissionOrigin + redOffset * redScale, transmissionBounds.xy, transmissionBounds.zw);
        float4 greenSample = SampleTransmission(
            transmissionOrigin + greenOffset * greenScale, transmissionBounds.xy, transmissionBounds.zw);
        float4 blueSample = SampleTransmission(
            transmissionOrigin + blueOffset * blueScale, transmissionBounds.xy, transmissionBounds.zw);

        // A zero-alpha allocation texel contains no recoverable straight color.
        // Fall back to the undisplaced backdrop at this output pixel rather than
        // manufacturing black. This path only activates at the materialized edge.
        if (redSample.a <= 1e-5f) redSample = baseSample;
        if (greenSample.a <= 1e-5f) greenSample = baseSample;
        if (blueSample.a <= 1e-5f) blueSample = baseSample;

        float3 color = float3(redSample.r, greenSample.g, blueSample.b);
        color = ApplySaturation(color, saturation);
        color = ApplyExposureContrast(color, exposure, contrast);
        color = lerp(color, tintColor, tintOpacity);

        const float innerShadow = 1.0f - smoothstep(0.0f, max(bezel * 0.65f, 1.0f), distanceFromEdge);
        color *= 1.0f - innerShadow * innerShadowStrength;

        float borderMask = 0.0f;
        if (borderThickness > 1e-4f)
        {
            borderMask = 1.0f - smoothstep(
                borderThickness,
                borderThickness + feather,
                distanceFromEdge);
        }
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
            const float pointerSdf = RoundedRectSdf(pointerPosition - halfRect, shapeHalfRect, radius);
            pointerSpecular = PointerSpecularCoefficient(
                distanceFromEdge,
                specularWidth,
                feather,
                normal,
                pointerLightDirection,
                pointerSdf <= 0.0f,
                pointerInteraction,
                highlightSharpness) *
                pointerHighlightStrength *
                (1.0f + pointerSpeedWeight * 0.18f) *
                opticalInterior;
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

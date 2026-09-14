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
    // x = light angle in radians, remaining components reserved for future material parameters
    float4 MaterialParams3;
};

float RoundedRectSdf(float2 p, float2 halfSize, float radius)
{
    float2 q = abs(p) - (halfSize - radius.xx);
    return length(max(q, 0.0f.xx)) + min(max(q.x, q.y), 0.0f) - radius;
}

float SurfaceHeight(float t)
{
    // Convex quarter-superellipse profile used by the reference implementations.
    // t = 0 is the outside edge of the bezel and t = 1 is the flat interior.
    float s = 1.0f - saturate(t);
    return pow(saturate(1.0f - s * s * s * s), 0.25f);
}

float2 RoundedRectNormal(float2 local, float2 halfRect, float radius, float centerSdf)
{
    // A half-pixel numerical derivative is stable across straight edges and rounded corners,
    // and unlike a radial approximation it produces the correct normal for non-square glass.
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
    // The source is materialized by the upstream native GaussianBlur graph stage.
    // Clamp only the sampling coordinates; the final rounded-rectangle coverage is evaluated
    // independently below so blur/refraction can never soften or square-off the glass corners.
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

    // Coverage is intentionally calculated after the blurred/materialized source is sampled.
    // That separation is what preserves the requested CornerRadius regardless of BlurRadius.
    const float feather = max(edgeSoftness, 0.5f);
    const float coverage = saturate(0.5f - sdf / feather);
    const float alpha = coverage * saturate(materialOpacity);

    // FXC's SM4 library compiler can emit a false-positive X4000 for helper functions with a
    // mid-function return. Keep one initialized return value and one final return.
    float4 result = 0.0f.xxxx;
    if (alpha > 0.0f)
    {
        const float distanceFromEdge = max(-sdf, 0.0f);
        const float maximumBezel = max(min(radius > 0.0f ? radius : halfMinSize, halfMinSize) - 0.5f, 1.0f);
        const float bezel = clamp(bezelWidth, 1.0f, maximumBezel);
        const float bezelT = saturate(distanceFromEdge / bezel);

        // Convert the convex profile into an optical displacement. This follows the physical
        // chain used by the WebGL/SVG references: profile height -> slope -> Snell refraction
        // angle -> horizontal travel through the remaining glass thickness.
        const float height = SurfaceHeight(bezelT);
        const float derivativeStep = 0.001f;
        const float nextHeight = SurfaceHeight(min(bezelT + derivativeStep, 1.0f));
        const float derivative = (nextHeight - height) / derivativeStep;
        const float slopeAngle = atan(derivative * (glassThickness / max(bezel, 1.0f)));
        const float refractedSin = clamp(sin(slopeAngle) / refractiveIndex, -1.0f, 1.0f);
        const float refractedAngle = asin(refractedSin);
        const float physicalDisplacement = height * glassThickness * (tan(slopeAngle) - tan(refractedAngle));
        const float artisticScale = max(refractionStrength, 0.0f) / 24.0f;
        const float displacementPixels = physicalDisplacement * artisticScale;

        const float2 normal = RoundedRectNormal(local, halfRect, radius, sdf);
        const float2 refractUv = uv - normal * texelSize * displacementPixels;

        // Dispersion is strongest in the optical bezel and fades to zero on the flat interior.
        const float bezelWeight = 1.0f - smoothstep(0.20f, 1.0f, bezelT);
        const float dispersionPixels = dispersionStrength * bezelWeight * (0.35f + min(abs(displacementPixels) * 0.04f, 1.5f));
        const float2 dispersionOffset = normal * texelSize * dispersionPixels;

        float3 color = float3(
            SampleTransmission(refractUv - dispersionOffset, contentMin, contentMax, texelSize, hasContentRect).r,
            SampleTransmission(refractUv, contentMin, contentMax, texelSize, hasContentRect).g,
            SampleTransmission(refractUv + dispersionOffset, contentMin, contentMax, texelSize, hasContentRect).b);
        color = ApplySaturation(color, saturation);
        color = lerp(color, 1.0f.xxx, tintOpacity);

        // Directional rim/specular response. LightAngle is animatable so the host can make the
        // highlight follow the pointer without recreating the effect graph.
        const float2 lightDirection = normalize(float2(cos(lightAngle), sin(lightAngle)));
        const float rimDot = abs(dot(normal, lightDirection));
        const float rimFalloff = 1.0f - smoothstep(0.0f, max(bezel * 0.45f, 1.0f), distanceFromEdge);
        const float specular = pow(saturate(rimDot * rimFalloff), 1.5f) * highlightStrength;

        const float innerShadow = 1.0f - smoothstep(0.0f, max(bezel * 0.65f, 1.0f), distanceFromEdge);
        color *= 1.0f - innerShadow * 0.09f;

        const float innerRim = smoothstep(0.0f, 2.0f, distanceFromEdge) *
            (1.0f - smoothstep(2.0f, 5.0f + feather, distanceFromEdge));
        color += (specular + innerRim * 0.15f * highlightStrength).xxx;

        const float borderMask = 1.0f - smoothstep(
            max(borderThickness, 0.0f),
            max(borderThickness, 0.0f) + feather,
            distanceFromEdge);
        color = lerp(color, 1.0f.xxx, borderMask * 0.20f * highlightStrength);
        color = saturate(color);

        // Composition expects premultiplied-alpha output from this material.
        result = float4(color * alpha, alpha);
    }

    return result;
}

// MaterializedTexture lowering uses this color passthrough for the source and final wrapper
// subgraphs. The custom sampler itself remains linked into the final consumer fragment so its
// SDF is evaluated at destination resolution.
export float4 MaterializeColor(float4 color) { return color; }

// DWM appends sampler edge-mode suffixes for custom sampler bodies.
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

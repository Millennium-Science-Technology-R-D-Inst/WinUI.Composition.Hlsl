Texture2D texture0;
SamplerState sampler0;

cbuffer LiquidGlassConstants : register(b0)
{
    float4 MaterialParams0;
    float4 MaterialParams1;
};

float RoundedRectSdf(float2 p, float2 halfSize, float radius)
{
    float2 q = abs(p) - (halfSize - radius.xx);
    return length(max(q, 0.0f.xx)) + min(max(q.x, q.y), 0.0f) - radius;
}

float4 SampleTransmission(float2 uv)
{
    // The source is already blurred by a separate native GaussianBlur
    // CompositionEffectBrush. This pass only performs glass sampling.
    return texture0.Sample(sampler0, uv);
}

float4 LiquidGlassCore(float2 uv, float4 samplerDataExt, float4 samplerData)
{
    const float borderThickness = MaterialParams0.y;
    const float cornerRadius = MaterialParams0.z;
    const float refractionStrength = MaterialParams0.w;
    const float highlightStrength = MaterialParams1.x;
    const float edgeSoftness = MaterialParams1.y;
    const float dispersionStrength = MaterialParams1.z;
    const float materialOpacity = MaterialParams1.w;

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
    const float clampedCornerRadius = min(cornerRadius, min(halfRect.x, halfRect.y));
    const float radius = max(clampedCornerRadius, 0.0f);
    const float sdf = RoundedRectSdf(local, halfRect, radius);
    const float feather = max(edgeSoftness, 1.0f);
    const float alpha = saturate((feather - sdf) / feather) * materialOpacity;
    if (alpha <= 0.0f)
    {
        return 0.0f.xxxx;
    }

    const float innerDistance = max(-sdf, 0.0f);
    const float halfMinSize = max(min(halfRect.x, halfRect.y), 1.0f);
    const float2 domeCoord = local / max(halfRect, 1.0f.xx);
    const float domeRadius = saturate(length(domeCoord));
    const float domeDepth = (1.0f - domeRadius) * halfMinSize;
    const float2 domeNormal = normalize(domeCoord + 1e-5f.xx);
    const float edgeFactor = 1.0f - saturate(innerDistance / max(radius, 1.0f));
    const float interiorFactor = saturate(domeDepth / halfMinSize);
    const float edgeDistance = max(borderThickness * 4.0f + feather * 2.0f, 1.0f);
    const float rimDistance = max(borderThickness * 2.0f + 1.0f, 1.0f);
    const float edgeIntensity = exp(-innerDistance / edgeDistance) * 0.85f;
    const float rimIntensity = exp(-innerDistance / rimDistance) * 0.25f;
    const float centerFade = 1.0f - smoothstep(
        halfMinSize * 0.08f,
        halfMinSize * 0.55f,
        domeDepth);
    const float refractionWeight = (edgeIntensity + rimIntensity) * centerFade;
    const float dispersionWeight = edgeIntensity * centerFade;

    const float2 refractUv = uv - domeNormal * texelSize * refractionStrength * refractionWeight;
    const float2 dispersionOffset = domeNormal * texelSize * dispersionStrength * dispersionWeight;

    float3 color = float3(
        SampleTransmission(refractUv - dispersionOffset).r,
        SampleTransmission(refractUv).g,
        SampleTransmission(refractUv + dispersionOffset).b);
    color = lerp(color, 1.0f.xxx, 0.08f + interiorFactor * 0.06f);

    const float borderMask = 1.0f - smoothstep(borderThickness, borderThickness + feather, innerDistance);
    const float innerGlow = 1.0f - smoothstep(borderThickness * 2.0f, borderThickness * 6.0f + feather, innerDistance);
    const float domeHeight = sqrt(saturate(1.0f - dot(domeCoord, domeCoord)));
    const float3 surfaceNormal = normalize(float3(-domeCoord * 0.35f, 0.45f + domeHeight * 0.75f));
    const float3 lightDir = normalize(float3(-0.35f, -0.45f, 0.82f));
    const float specular = pow(saturate(dot(surfaceNormal, lightDir)), 18.0f) * (0.20f + edgeFactor * 0.50f);
    const float topSweep = pow(saturate(1.0f - localPosition.y / rectSize.y), 2.5f) * (0.15f + edgeFactor * 0.20f);

    color += (specular * 0.28f + topSweep * 0.12f + innerGlow * 0.10f) * highlightStrength;
    color = lerp(color, 1.0f.xxx, borderMask * 0.22f * highlightStrength);
    color = saturate(color);

    return float4(color * alpha, alpha);
}

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

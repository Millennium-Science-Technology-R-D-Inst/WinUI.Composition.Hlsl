float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData)
{
    float2 contentMin = min(samplerData.xy, samplerData.zw);
    float2 contentMax = max(samplerData.xy, samplerData.zw);
    float2 size = max(contentMax - contentMin, 1e-6f.xx);
    float2 localUv = saturate((uv - contentMin) / size);
    float2 offset = samplerDataExt.zw * (localUv - 0.5f.xx);
    return texture0.Sample(sampler0, uv + offset);
}

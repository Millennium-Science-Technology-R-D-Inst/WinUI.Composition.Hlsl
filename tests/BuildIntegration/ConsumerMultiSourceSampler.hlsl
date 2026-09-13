float4 Shade(
    float2 uv0,
    float4 samplerDataExt0,
    float2 uv1,
    float4 samplerDataExt1)
{
    float4 first = texture0.Sample(sampler0, uv0);
    float4 second = texture1.Sample(sampler1, uv1);
    return lerp(first, second, 0.5f);
}

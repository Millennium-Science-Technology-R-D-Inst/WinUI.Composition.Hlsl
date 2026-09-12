float4 Shade(float2 uv, float4 samplerDataExt)
{
    return float4(uv, samplerDataExt.x, 1.0f);
}

#include "Common.hlsli"

float4 Shade(float2 uv, float4 samplerDataExt)
{
    return MakeConsumerColor(uv, samplerDataExt.x);
}

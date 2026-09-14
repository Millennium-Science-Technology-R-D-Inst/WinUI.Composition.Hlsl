#include "Common.hlsli"

#ifndef WINUI_COMPOSITION_HLSL_TEST
#error WINUI_COMPOSITION_HLSL_TEST must be supplied by HlslCompositionShader.Defines.
#endif

float4 Shade(float2 uv, float4 samplerDataExt)
{
    // Exercise the generated texture0/sampler0 binding so library reflection can
    // verify the single-source sampler resource ABI in build integration tests.
    float4 sampled = texture0.Sample(sampler0, uv);
    float4 tint = MakeConsumerColor(uv, samplerDataExt.x + WINUI_COMPOSITION_HLSL_TEST * 0.0f);
    return sampled * tint;
}

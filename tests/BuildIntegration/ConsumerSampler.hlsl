#include "Common.hlsli"

#ifndef WINUI_COMPOSITION_HLSL_TEST
#error WINUI_COMPOSITION_HLSL_TEST must be supplied by HlslCompositionShader.Defines.
#endif

float4 Shade(float2 uv, float4 samplerDataExt)
{
    return MakeConsumerColor(uv, samplerDataExt.x + WINUI_COMPOSITION_HLSL_TEST * 0.0f);
}

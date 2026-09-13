#ifndef WINUI_COMPOSITION_MANAGED_TEST
#error WINUI_COMPOSITION_MANAGED_TEST must be supplied by HlslCompositionShader.Defines.
#endif

export float4 PSBody(float4 color)
{
    return color + WINUI_COMPOSITION_MANAGED_TEST * 0.0f;
}

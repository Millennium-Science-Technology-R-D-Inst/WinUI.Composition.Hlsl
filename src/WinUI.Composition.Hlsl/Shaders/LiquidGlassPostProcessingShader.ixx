module;
#include <Windows.h>
#include "LiquidGlassPostProcessingShader.g.h"

export module WinUI.Composition.Hlsl.Shaders.LiquidGlassPostProcessing;

import std;

export namespace WinUI::Composition::Hlsl::Shaders
{
    struct PostProcessingShaderView { void const* data; std::size_t size; };
    PostProcessingShaderView LiquidGlassPostProcessingShader() noexcept
    {
        return { g_LiquidGlassPostProcessingShader, sizeof(g_LiquidGlassPostProcessingShader) };
    }
}

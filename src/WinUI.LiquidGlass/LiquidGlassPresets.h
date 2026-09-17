#pragma once

#include "winrt_module_imports.h"
#include "LiquidGlassPresets.g.h"

namespace winrt::WinUI::LiquidGlass::implementation
{
    struct LiquidGlassPresets
    {
        static WinUI::Composition::Hlsl::LiquidGlassBrush CreateBrush(WinUI::LiquidGlass::LiquidGlassPreset preset);
    };
}

namespace winrt::WinUI::LiquidGlass::factory_implementation
{
    struct LiquidGlassPresets : LiquidGlassPresetsT<LiquidGlassPresets, implementation::LiquidGlassPresets>
    {
    };
}

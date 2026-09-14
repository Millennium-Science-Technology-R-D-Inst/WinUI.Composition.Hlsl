#pragma once

#include "LiquidGlassToggleButton.g.h"

namespace winrt::WinUI::Composition::Hlsl::Controls::implementation
{
    struct LiquidGlassToggleButton : LiquidGlassToggleButtonT<LiquidGlassToggleButton>
    {
        LiquidGlassToggleButton();
    };
}

namespace winrt::WinUI::Composition::Hlsl::Controls::factory_implementation
{
    struct LiquidGlassToggleButton : LiquidGlassToggleButtonT<LiquidGlassToggleButton, implementation::LiquidGlassToggleButton>
    {
    };
}

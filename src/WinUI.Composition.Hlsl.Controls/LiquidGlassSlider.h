#pragma once

#include "LiquidGlassSlider.g.h"

namespace winrt::WinUI::Composition::Hlsl::Controls::implementation
{
    struct LiquidGlassSlider : LiquidGlassSliderT<LiquidGlassSlider>
    {
        LiquidGlassSlider();
    };
}

namespace winrt::WinUI::Composition::Hlsl::Controls::factory_implementation
{
    struct LiquidGlassSlider : LiquidGlassSliderT<LiquidGlassSlider, implementation::LiquidGlassSlider>
    {
    };
}

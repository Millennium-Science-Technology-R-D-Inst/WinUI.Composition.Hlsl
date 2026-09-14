#pragma once

#include "LiquidGlassButton.g.h"

namespace winrt::WinUI::Composition::Hlsl::Controls::implementation
{
    struct LiquidGlassButton : LiquidGlassButtonT<LiquidGlassButton>
    {
        LiquidGlassButton();
    };
}

namespace winrt::WinUI::Composition::Hlsl::Controls::factory_implementation
{
    struct LiquidGlassButton : LiquidGlassButtonT<LiquidGlassButton, implementation::LiquidGlassButton>
    {
    };
}

#pragma once

#include "LiquidGlassCard.g.h"

namespace winrt::WinUI::Composition::Hlsl::Controls::implementation
{
    struct LiquidGlassCard : LiquidGlassCardT<LiquidGlassCard>
    {
        LiquidGlassCard();
    };
}

namespace winrt::WinUI::Composition::Hlsl::Controls::factory_implementation
{
    struct LiquidGlassCard : LiquidGlassCardT<LiquidGlassCard, implementation::LiquidGlassCard>
    {
    };
}

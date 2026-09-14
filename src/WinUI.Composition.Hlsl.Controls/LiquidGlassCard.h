#pragma once

#include "LiquidGlassCard.g.h"
#include <winrt/WinUI.Composition.Hlsl.h>

namespace winrt::WinUI::Composition::Hlsl::Controls::implementation
{
    struct LiquidGlassCard : LiquidGlassCardT<LiquidGlassCard>
    {
        LiquidGlassCard();
        winrt::WinUI::Composition::Hlsl::LiquidGlassBrush GlassBrush() const noexcept { return m_glassBrush; }

    private:
        winrt::WinUI::Composition::Hlsl::LiquidGlassBrush m_glassBrush{ nullptr };
    };
}

namespace winrt::WinUI::Composition::Hlsl::Controls::factory_implementation
{
    struct LiquidGlassCard : LiquidGlassCardT<LiquidGlassCard, implementation::LiquidGlassCard>
    {
    };
}

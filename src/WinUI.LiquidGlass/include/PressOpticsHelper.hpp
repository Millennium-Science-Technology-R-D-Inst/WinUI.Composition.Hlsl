#pragma once

namespace winrt::WinUI::LiquidGlass::detail
{
    // Optical half of the press interaction for ButtonBase-derived LiquidGlass controls.
    // Geometry remains in XAML VisualStates; the exact pre-press material values are
    // restored on release so custom application brushes are not permanently modified.
    template<typename Self>
    class PressOpticsHelper
    {
    public:
        PressOpticsHelper()
        {
            auto self = static_cast<Self*>(this);
            self->PointerPressed([this](auto const&, auto const&) { EnterPressedState(); });
            self->PointerReleased([this](auto const&, auto const&) { LeavePressedState(); });
            self->PointerCaptureLost([this](auto const&, auto const&) { LeavePressedState(); });
        }

    private:
        void EnterPressedState()
        {
            if (m_pressed) return;
            auto self = static_cast<Self*>(this);
            auto brush = self->GlassBrush();
            if (!brush) return;

            m_pressedBrush = brush;
            m_refraction = brush.RefractionStrength();
            m_tintOpacity = brush.TintOpacity();
            m_highlight = brush.HighlightStrength();
            m_innerShadow = brush.InnerShadowStrength();
            m_pressed = true;

            brush.RefractionStrength(std::min(128.0, m_refraction * 1.18 + 1.5));
            brush.TintOpacity(std::min(1.0, m_tintOpacity + 0.08));
            brush.HighlightStrength(std::min(4.0, m_highlight * 1.12 + 0.03));
            brush.InnerShadowStrength(std::min(1.0, m_innerShadow + 0.04));
        }

        void LeavePressedState()
        {
            if (!m_pressed) return;
            if (m_pressedBrush)
            {
                m_pressedBrush.RefractionStrength(m_refraction);
                m_pressedBrush.TintOpacity(m_tintOpacity);
                m_pressedBrush.HighlightStrength(m_highlight);
                m_pressedBrush.InnerShadowStrength(m_innerShadow);
            }
            m_pressedBrush = nullptr;
            m_pressed = false;
        }

        WinUI::Composition::Hlsl::LiquidGlassBrush m_pressedBrush{ nullptr };
        double m_refraction{};
        double m_tintOpacity{};
        double m_highlight{};
        double m_innerShadow{};
        bool m_pressed{};
    };
}

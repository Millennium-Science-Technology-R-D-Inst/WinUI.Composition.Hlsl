#pragma once

namespace winrt::WinUI::LiquidGlass::detail
{
    struct OpticsSnapshot
    {
        WinUI::Composition::Hlsl::LiquidGlassBrush brush{ nullptr };
        double refraction{};
        double tintOpacity{};
        double highlight{};
        double innerShadow{};
        bool active{};
    };

    inline void EnterPressedOptics(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        WinUI::Composition::Hlsl::LiquidGlassBrush const& brush,
        OpticsSnapshot& state)
    {
        if (state.active || !owner || !brush) return;

        state.brush = brush;
        state.refraction = brush.RefractionStrength();
        state.tintOpacity = brush.TintOpacity();
        state.highlight = brush.HighlightStrength();
        state.innerShadow = brush.InnerShadowStrength();
        state.active = true;

        auto const refractionMultiplier = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedRefractionMultiplier(owner), 0.0, 4.0);
        auto const refractionBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedRefractionBoost(owner), -128.0, 128.0);
        auto const tintBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedTintBoost(owner), -1.0, 1.0);
        auto const highlightMultiplier = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedHighlightMultiplier(owner), 0.0, 4.0);
        auto const highlightBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedHighlightBoost(owner), -4.0, 4.0);
        auto const shadowBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedInnerShadowBoost(owner), -1.0, 1.0);

        brush.RefractionStrength(std::clamp(state.refraction * refractionMultiplier + refractionBoost, 0.0, 128.0));
        brush.TintOpacity(std::clamp(state.tintOpacity + tintBoost, 0.0, 1.0));
        brush.HighlightStrength(std::clamp(state.highlight * highlightMultiplier + highlightBoost, 0.0, 4.0));
        brush.InnerShadowStrength(std::clamp(state.innerShadow + shadowBoost, 0.0, 1.0));
    }

    inline void LeavePressedOptics(OpticsSnapshot& state)
    {
        if (!state.active) return;
        if (state.brush)
        {
            state.brush.RefractionStrength(state.refraction);
            state.brush.TintOpacity(state.tintOpacity);
            state.brush.HighlightStrength(state.highlight);
            state.brush.InnerShadowStrength(state.innerShadow);
        }
        state.brush = nullptr;
        state.active = false;
    }

    template<typename Self>
    class PressOpticsHelper
    {
    public:
        PressOpticsHelper()
        {
            auto self = static_cast<Self*>(this);
            self->PointerPressed([this](auto const& sender, auto const&)
            {
                auto object = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
                if (!object) return;
                auto owner = static_cast<Self*>(this);
                EnterPressedOptics(object, owner->GlassBrush(), m_state);
            });
            self->PointerReleased([this](auto const&, auto const&) { LeavePressedOptics(m_state); });
            self->PointerCaptureLost([this](auto const&, auto const&) { LeavePressedOptics(m_state); });
            self->PointerCanceled([this](auto const&, auto const&) { LeavePressedOptics(m_state); });
        }

    private:
        OpticsSnapshot m_state;
    };
}

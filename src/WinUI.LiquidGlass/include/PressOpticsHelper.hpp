#pragma once

namespace winrt::WinUI::LiquidGlass::detail
{
    struct OpticsSnapshot
    {
        WinUI::Composition::Hlsl::LiquidGlassBrush brush{ nullptr };
        double blur{};
        double refraction{};
        double dispersion{};
        double saturation{};
        double contrast{};
        double exposure{};
        double tintOpacity{};
        double highlight{};
        double innerShadow{};
        bool active{};
    };

    inline void CaptureOptics(
        WinUI::Composition::Hlsl::LiquidGlassBrush const& brush,
        OpticsSnapshot& state)
    {
        state.brush = brush;
        state.blur = brush.BlurRadius();
        state.refraction = brush.RefractionStrength();
        state.dispersion = brush.DispersionStrength();
        state.saturation = brush.Saturation();
        state.contrast = brush.Contrast();
        state.exposure = brush.Exposure();
        state.tintOpacity = brush.TintOpacity();
        state.highlight = brush.HighlightStrength();
        state.innerShadow = brush.InnerShadowStrength();
        state.active = true;
    }

    inline void RestoreOptics(OpticsSnapshot& state)
    {
        if (!state.active) return;
        if (state.brush)
        {
            state.brush.BlurRadius(state.blur);
            state.brush.RefractionStrength(state.refraction);
            state.brush.DispersionStrength(state.dispersion);
            state.brush.Saturation(state.saturation);
            state.brush.Contrast(state.contrast);
            state.brush.Exposure(state.exposure);
            state.brush.TintOpacity(state.tintOpacity);
            state.brush.HighlightStrength(state.highlight);
            state.brush.InnerShadowStrength(state.innerShadow);
        }
        state.brush = nullptr;
        state.active = false;
    }

    inline void EnterPointerOverOptics(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        WinUI::Composition::Hlsl::LiquidGlassBrush const& brush,
        OpticsSnapshot& state)
    {
        if (state.active || !owner || !brush) return;
        CaptureOptics(brush, state);

        auto const blurBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPointerOverBlurBoost(owner), -64.0, 64.0);
        auto const refractionMultiplier = std::clamp(
            implementation::LiquidGlassInteraction::GetPointerOverRefractionMultiplier(owner), 0.0, 4.0);
        auto const refractionBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPointerOverRefractionBoost(owner), -128.0, 128.0);
        auto const dispersionMultiplier = std::clamp(
            implementation::LiquidGlassInteraction::GetPointerOverDispersionMultiplier(owner), 0.0, 8.0);
        auto const saturationMultiplier = std::clamp(
            implementation::LiquidGlassInteraction::GetPointerOverSaturationMultiplier(owner), 0.0, 8.0);
        auto const contrastMultiplier = std::clamp(
            implementation::LiquidGlassInteraction::GetPointerOverContrastMultiplier(owner), 0.0, 8.0);
        auto const exposureBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPointerOverExposureBoost(owner), -8.0, 8.0);
        auto const tintBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPointerOverTintBoost(owner), -1.0, 1.0);
        auto const highlightMultiplier = std::clamp(
            implementation::LiquidGlassInteraction::GetPointerOverHighlightMultiplier(owner), 0.0, 4.0);
        auto const highlightBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPointerOverHighlightBoost(owner), -4.0, 4.0);
        auto const shadowBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPointerOverInnerShadowBoost(owner), -1.0, 1.0);

        brush.BlurRadius(std::clamp(state.blur + blurBoost, 0.0, 64.0));
        brush.RefractionStrength(std::clamp(state.refraction * refractionMultiplier + refractionBoost, 0.0, 128.0));
        brush.DispersionStrength(std::clamp(state.dispersion * dispersionMultiplier, 0.0, 16.0));
        brush.Saturation(std::clamp(state.saturation * saturationMultiplier, 0.0, 4.0));
        brush.Contrast(std::clamp(state.contrast * contrastMultiplier, 0.0, 4.0));
        brush.Exposure(std::clamp(state.exposure + exposureBoost, -4.0, 4.0));
        brush.TintOpacity(std::clamp(state.tintOpacity + tintBoost, 0.0, 1.0));
        brush.HighlightStrength(std::clamp(state.highlight * highlightMultiplier + highlightBoost, 0.0, 4.0));
        brush.InnerShadowStrength(std::clamp(state.innerShadow + shadowBoost, 0.0, 1.0));
    }

    inline void LeavePointerOverOptics(OpticsSnapshot& state)
    {
        RestoreOptics(state);
    }

    inline void EnterPressedOptics(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        WinUI::Composition::Hlsl::LiquidGlassBrush const& brush,
        OpticsSnapshot& state)
    {
        if (state.active || !owner || !brush) return;
        CaptureOptics(brush, state);

        auto const blurBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedBlurBoost(owner), -64.0, 64.0);
        auto const refractionMultiplier = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedRefractionMultiplier(owner), 0.0, 4.0);
        auto const refractionBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedRefractionBoost(owner), -128.0, 128.0);
        auto const dispersionMultiplier = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedDispersionMultiplier(owner), 0.0, 8.0);
        auto const saturationMultiplier = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedSaturationMultiplier(owner), 0.0, 8.0);
        auto const contrastMultiplier = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedContrastMultiplier(owner), 0.0, 8.0);
        auto const exposureBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedExposureBoost(owner), -8.0, 8.0);
        auto const tintBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedTintBoost(owner), -1.0, 1.0);
        auto const highlightMultiplier = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedHighlightMultiplier(owner), 0.0, 4.0);
        auto const highlightBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedHighlightBoost(owner), -4.0, 4.0);
        auto const shadowBoost = std::clamp(
            implementation::LiquidGlassInteraction::GetPressedInnerShadowBoost(owner), -1.0, 1.0);

        brush.BlurRadius(std::clamp(state.blur + blurBoost, 0.0, 64.0));
        brush.RefractionStrength(std::clamp(state.refraction * refractionMultiplier + refractionBoost, 0.0, 128.0));
        brush.DispersionStrength(std::clamp(state.dispersion * dispersionMultiplier, 0.0, 16.0));
        brush.Saturation(std::clamp(state.saturation * saturationMultiplier, 0.0, 4.0));
        brush.Contrast(std::clamp(state.contrast * contrastMultiplier, 0.0, 4.0));
        brush.Exposure(std::clamp(state.exposure + exposureBoost, -4.0, 4.0));
        brush.TintOpacity(std::clamp(state.tintOpacity + tintBoost, 0.0, 1.0));
        brush.HighlightStrength(std::clamp(state.highlight * highlightMultiplier + highlightBoost, 0.0, 4.0));
        brush.InnerShadowStrength(std::clamp(state.innerShadow + shadowBoost, 0.0, 1.0));
    }

    inline void LeavePressedOptics(OpticsSnapshot& state)
    {
        RestoreOptics(state);
    }

    template<typename Self>
    class PressOpticsHelper
    {
    public:
        PressOpticsHelper()
        {
            auto self = static_cast<Self*>(this);
            self->PointerEntered([this](auto const& sender, auto const&)
            {
                m_pointerOver = true;
                auto object = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
                auto owner = static_cast<Self*>(this);
                if (object) EnterPointerOverOptics(object, owner->GlassBrush(), m_pointerOverState);
            });
            self->PointerExited([this](auto const&, auto const&)
            {
                m_pointerOver = false;
                if (!m_pressed) LeavePointerOverOptics(m_pointerOverState);
            });
            self->PointerPressed([this](auto const& sender, auto const&)
            {
                m_pressed = true;
                auto object = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
                auto owner = static_cast<Self*>(this);
                if (object) EnterPressedOptics(object, owner->GlassBrush(), m_pressedState);
            });
            self->PointerReleased([this](auto const&, auto const&)
            {
                FinishPress();
            });
            self->PointerCaptureLost([this](auto const&, auto const&)
            {
                FinishPress();
            });
            self->PointerCanceled([this](auto const&, auto const&)
            {
                FinishPress();
            });
        }

    private:
        void FinishPress()
        {
            LeavePressedOptics(m_pressedState);
            m_pressed = false;
            if (!m_pointerOver) LeavePointerOverOptics(m_pointerOverState);
        }

        OpticsSnapshot m_pointerOverState;
        OpticsSnapshot m_pressedState;
        bool m_pointerOver{};
        bool m_pressed{};
    };
}

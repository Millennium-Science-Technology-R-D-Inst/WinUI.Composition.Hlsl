#pragma once

#include "PressOpticsHelper.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self>
    class FocusOpticsHelper
    {
    public:
        FocusOpticsHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const& sender, auto const&) { ApplyRest(sender); });
            self->GotFocus([this](auto const& sender, auto const&) { EnterFocus(sender); });
            self->LostFocus([this](auto const& sender, auto const&) { LeaveFocus(sender); });
        }

    private:
        template<typename Sender>
        static Microsoft::UI::Xaml::DependencyObject Owner(Sender const& sender)
        {
            return sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
        }

        template<typename Sender>
        void ApplyRest(Sender const& sender)
        {
            auto owner = Owner(sender);
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element) return;
            auto const scale = std::clamp(
                implementation::LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
            SetElementScale(element, scale, scale);
        }

        template<typename Sender>
        void EnterFocus(Sender const& sender)
        {
            auto owner = Owner(sender);
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element) return;

            auto self = static_cast<Self*>(this);
            if (!m_state.active)
            {
                if (auto brush = self->GlassBrush())
                {
                    CaptureOptics(brush, m_state);
                    brush.BlurRadius(std::clamp(
                        m_state.blur + implementation::LiquidGlassInteraction::GetFocusedBlurBoost(owner), 0.0, 64.0));
                    brush.RefractionStrength(std::clamp(
                        m_state.refraction * implementation::LiquidGlassInteraction::GetFocusedRefractionMultiplier(owner), 0.0, 128.0));
                    brush.DispersionStrength(std::clamp(
                        m_state.dispersion * implementation::LiquidGlassInteraction::GetFocusedDispersionMultiplier(owner), 0.0, 16.0));
                    brush.Saturation(std::clamp(
                        m_state.saturation * implementation::LiquidGlassInteraction::GetFocusedSaturationMultiplier(owner), 0.0, 4.0));
                    brush.Contrast(std::clamp(
                        m_state.contrast * implementation::LiquidGlassInteraction::GetFocusedContrastMultiplier(owner), 0.0, 4.0));
                    brush.Exposure(std::clamp(
                        m_state.exposure + implementation::LiquidGlassInteraction::GetFocusedExposureBoost(owner), -4.0, 4.0));
                    brush.TintOpacity(std::clamp(
                        m_state.tintOpacity + implementation::LiquidGlassInteraction::GetFocusedTintBoost(owner), 0.0, 1.0));
                    brush.HighlightStrength(std::clamp(
                        m_state.highlight + implementation::LiquidGlassInteraction::GetFocusedHighlightBoost(owner), 0.0, 4.0));
                    brush.InnerShadowStrength(std::clamp(
                        m_state.innerShadow + implementation::LiquidGlassInteraction::GetFocusedInnerShadowBoost(owner), 0.0, 1.0));
                    AnimateOpticsTransition(owner, brush, m_state);
                }
            }

            auto const scale = std::clamp(
                implementation::LiquidGlassInteraction::GetFocusedScale(owner), .25, 4.0);
            AnimateElementScale(
                owner,
                element,
                scale,
                implementation::LiquidGlassInteraction::GetMotionDuration(owner));
        }

        template<typename Sender>
        void LeaveFocus(Sender const& sender)
        {
            auto owner = Owner(sender);
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();

            if (m_state.active && m_state.brush)
            {
                auto brush = m_state.brush;
                OpticsSnapshot from;
                CaptureOptics(brush, from);
                RestoreOptics(m_state);
                if (owner) AnimateOpticsTransition(owner, brush, from);
            }

            if (!owner || !element) return;
            auto const scale = std::clamp(
                implementation::LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
            AnimateElementScale(
                owner,
                element,
                scale,
                implementation::LiquidGlassInteraction::GetMotionDuration(owner));
        }

        OpticsSnapshot m_state;
    };
}

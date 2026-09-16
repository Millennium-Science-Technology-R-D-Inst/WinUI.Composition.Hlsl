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
            self->GotFocus([this](auto const& sender, auto const&)
            {
                m_focused = true;
                Recompute(sender);
            });
            self->LostFocus([this](auto const& sender, auto const&)
            {
                m_focused = false;
                Recompute(sender);
            });
            self->RegisterPropertyChangedCallback(
                Self::GlassBrushProperty(),
                [this](Microsoft::UI::Xaml::DependencyObject const& sender,
                       Microsoft::UI::Xaml::DependencyProperty const&)
                {
                    Recompute(sender);
                });
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

        static void ApplyFocusedOptics(
            Microsoft::UI::Xaml::DependencyObject const& owner,
            WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            ApplyOpticsDelta(
                brush,
                std::clamp(implementation::LiquidGlassInteraction::GetFocusedBlurBoost(owner), -64.0, 64.0),
                std::clamp(implementation::LiquidGlassInteraction::GetFocusedRefractionMultiplier(owner), 0.0, 4.0),
                0.0,
                std::clamp(implementation::LiquidGlassInteraction::GetFocusedDispersionMultiplier(owner), 0.0, 8.0),
                std::clamp(implementation::LiquidGlassInteraction::GetFocusedSaturationMultiplier(owner), 0.0, 8.0),
                std::clamp(implementation::LiquidGlassInteraction::GetFocusedContrastMultiplier(owner), 0.0, 8.0),
                std::clamp(implementation::LiquidGlassInteraction::GetFocusedExposureBoost(owner), -8.0, 8.0),
                std::clamp(implementation::LiquidGlassInteraction::GetFocusedTintBoost(owner), -1.0, 1.0),
                1.0,
                std::clamp(implementation::LiquidGlassInteraction::GetFocusedHighlightBoost(owner), -4.0, 4.0),
                std::clamp(implementation::LiquidGlassInteraction::GetFocusedInnerShadowBoost(owner), -1.0, 1.0));
        }

        template<typename Sender>
        void Recompute(Sender const& sender)
        {
            auto owner = Owner(sender);
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element) return;

            auto self = static_cast<Self*>(this);
            auto brush = self->GlassBrush();
            if (m_state.active && (!brush || get_abi(m_state.brush) != get_abi(brush)))
            {
                // Restore the previous brush before migrating the focused state.
                // A GlassBrush DP replacement must never leave transient optics
                // baked into a brush that is no longer attached to the control.
                RestoreOptics(m_state);
            }

            if (m_focused)
            {
                if (brush)
                {
                    OpticsSnapshot from;
                    CaptureOptics(brush, from);
                    if (!m_state.active)
                    {
                        CaptureOptics(brush, m_state);
                    }
                    else
                    {
                        ApplySnapshot(m_state);
                    }
                    ApplyFocusedOptics(owner, brush);
                    AnimateOpticsTransition(owner, brush, from);
                }

                auto const scale = std::clamp(
                    implementation::LiquidGlassInteraction::GetFocusedScale(owner), .25, 4.0);
                AnimateElementScale(
                    owner,
                    element,
                    scale,
                    implementation::LiquidGlassInteraction::GetMotionDuration(owner));
                return;
            }

            if (m_state.active && m_state.brush)
            {
                auto activeBrush = m_state.brush;
                OpticsSnapshot from;
                CaptureOptics(activeBrush, from);
                RestoreOptics(m_state);
                AnimateOpticsTransition(owner, activeBrush, from);
            }

            auto const scale = std::clamp(
                implementation::LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
            AnimateElementScale(
                owner,
                element,
                scale,
                implementation::LiquidGlassInteraction::GetMotionDuration(owner));
        }

        OpticsSnapshot m_state;
        bool m_focused{};
    };
}

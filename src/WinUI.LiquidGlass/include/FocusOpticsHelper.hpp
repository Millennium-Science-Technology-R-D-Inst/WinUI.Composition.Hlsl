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
            // Text-entry focus should acknowledge immediately. A keyframe duration is a
            // better contract than a spring whose settling time ignores MotionDuration.
            self->SetValue(
                implementation::LiquidGlassInteraction::UseSpringMotionProperty(),
                box_value(false));

            self->Loaded([this](auto const& sender, auto const&)
            {
                m_loaded = true;
                RestorePendingState();
                ApplyRest(sender);
            });
            self->Unloaded([this](auto const&, auto const&)
            {
                // Unloaded can precede XamlCompositionBrushBase::OnDisconnected. Keep the
                // authored baseline for the next Loaded, but do not mutate a live/closing
                // CompositionEffectBrush during teardown.
                m_loaded = false;
                m_focused = false;
            });
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
                    if (!m_loaded)
                    {
                        // A replacement while detached retires the old brush. Do not write
                        // its saved baseline into a compositor that may already be closed.
                        m_state = {};
                        return;
                    }
                    Recompute(sender);
                });
        }

    private:
        template<typename Sender>
        static Microsoft::UI::Xaml::DependencyObject Owner(Sender const& sender)
        {
            return sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
        }

        void RestorePendingState()
        {
            if (!m_state.active) return;
            auto self = static_cast<Self*>(this);
            auto brush = self->GlassBrush();
            if (brush && m_state.brush && get_abi(brush) == get_abi(m_state.brush))
                RestoreOptics(m_state);
            else
                m_state = {};
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
            if (!m_loaded) return;
            auto owner = Owner(sender);
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element || !element.XamlRoot()) return;

            auto self = static_cast<Self*>(this);
            auto brush = self->GlassBrush();
            if (m_state.active && (!brush || get_abi(m_state.brush) != get_abi(brush)))
            {
                // Runtime brush replacement is still a live transition; restore the old
                // brush before migrating the focused state to the replacement.
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
        bool m_loaded{};
        bool m_focused{};
    };
}

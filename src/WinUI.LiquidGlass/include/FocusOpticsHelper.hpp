#pragma once

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

        static void SetScale(Microsoft::UI::Xaml::FrameworkElement const& element, double value, bool animate, double durationMs)
        {
            if (!element) return;
            auto visual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(element);
            visual.CenterPoint({ float(element.ActualWidth() * .5), float(element.ActualHeight() * .5), 0 });
            if (!animate)
            {
                visual.Scale({ static_cast<float>(value), static_cast<float>(value), 1 });
                return;
            }
            auto animation = visual.Compositor().CreateVector3KeyFrameAnimation();
            animation.InsertKeyFrame(1.0f, { static_cast<float>(value), static_cast<float>(value), 1 });
            animation.Duration(std::chrono::milliseconds{ static_cast<int64_t>(std::lround(std::clamp(durationMs, 0.0, 2000.0))) });
            visual.StartAnimation(L"Scale", animation);
        }

        template<typename Sender>
        void ApplyRest(Sender const& sender)
        {
            auto owner = Owner(sender);
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element) return;
            SetScale(element, std::clamp(implementation::LiquidGlassInteraction::GetRestScale(owner), .25, 4.0), false, 0);
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
                }
            }

            SetScale(element,
                std::clamp(implementation::LiquidGlassInteraction::GetFocusedScale(owner), .25, 4.0),
                true,
                implementation::LiquidGlassInteraction::GetMotionDuration(owner));
        }

        template<typename Sender>
        void LeaveFocus(Sender const& sender)
        {
            auto owner = Owner(sender);
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            RestoreOptics(m_state);
            if (!owner || !element) return;
            SetScale(element,
                std::clamp(implementation::LiquidGlassInteraction::GetRestScale(owner), .25, 4.0),
                true,
                implementation::LiquidGlassInteraction::GetMotionDuration(owner));
        }

        OpticsSnapshot m_state;
    };
}

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
            if (!m_focused)
            {
                if (auto brush = self->GlassBrush())
                {
                    m_brush = brush;
                    m_tintOpacity = brush.TintOpacity();
                    brush.TintOpacity(std::clamp(
                        m_tintOpacity + implementation::LiquidGlassInteraction::GetFocusedTintBoost(owner), 0.0, 1.0));
                }
                m_focused = true;
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
            if (m_focused && m_brush) m_brush.TintOpacity(m_tintOpacity);
            m_brush = nullptr;
            m_focused = false;
            if (!owner || !element) return;
            SetScale(element,
                std::clamp(implementation::LiquidGlassInteraction::GetRestScale(owner), .25, 4.0),
                true,
                implementation::LiquidGlassInteraction::GetMotionDuration(owner));
        }

        WinUI::Composition::Hlsl::LiquidGlassBrush m_brush{ nullptr };
        double m_tintOpacity{};
        bool m_focused{};
    };
}

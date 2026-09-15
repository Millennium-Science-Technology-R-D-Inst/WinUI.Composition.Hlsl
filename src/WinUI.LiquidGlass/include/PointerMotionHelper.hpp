#pragma once

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self>
    class PointerMotionHelper
    {
    public:
        PointerMotionHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const& sender, auto const&) { ApplyRest(sender); });
            self->PointerEntered([this](auto const& sender, auto const&)
            {
                CaptureTranslation(sender);
                m_pointerOver = true;
                AnimateState(sender);
            });
            self->PointerMoved([this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
            {
                ApplyPointerResponse(sender, args);
            });
            self->PointerExited([this](auto const& sender, auto const&)
            {
                m_pointerOver = false;
                if (!m_pressed)
                {
                    AnimateState(sender);
                    RestoreTranslation(sender);
                }
            });
            self->PointerPressed([this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
            {
                CaptureTranslation(sender);
                m_pressed = true;
                AnimateState(sender);
                ApplyPointerDisplacement(sender, args);
            });
            self->PointerReleased([this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
            {
                m_pressed = false;
                AnimateState(sender);
                if (m_pointerOver) ApplyPointerDisplacement(sender, args);
                else RestoreTranslation(sender);
            });
            self->PointerCaptureLost([this](auto const& sender, auto const&)
            {
                m_pressed = false;
                AnimateState(sender);
                RestoreTranslation(sender);
            });
            self->PointerCanceled([this](auto const& sender, auto const&)
            {
                m_pressed = false;
                AnimateState(sender);
                RestoreTranslation(sender);
            });
        }

    private:
        static std::chrono::milliseconds Duration(Microsoft::UI::Xaml::DependencyObject const& owner)
        {
            auto const value = std::clamp(
                implementation::LiquidGlassInteraction::GetMotionDuration(owner), 0.0, 2000.0);
            return std::chrono::milliseconds{ static_cast<int64_t>(std::lround(value)) };
        }

        static void SetScale(Microsoft::UI::Xaml::FrameworkElement const& element, double x, double y)
        {
            if (!element) return;
            auto visual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(element);
            visual.CenterPoint({ float(element.ActualWidth() * .5), float(element.ActualHeight() * .5), 0 });
            visual.Scale({ static_cast<float>(x), static_cast<float>(y), 1 });
        }

        static void AnimateScale(
            Microsoft::UI::Xaml::FrameworkElement const& element,
            double x,
            double y,
            std::chrono::milliseconds duration)
        {
            if (!element) return;
            auto visual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(element);
            visual.CenterPoint({ float(element.ActualWidth() * .5), float(element.ActualHeight() * .5), 0 });
            auto animation = visual.Compositor().CreateVector3KeyFrameAnimation();
            animation.InsertKeyFrame(1.0f, { static_cast<float>(x), static_cast<float>(y), 1 });
            animation.Duration(duration);
            visual.StartAnimation(L"Scale", animation);
        }

        template<typename Sender>
        void CaptureTranslation(Sender const& sender)
        {
            if (m_translationCaptured) return;
            auto element = sender.template try_as<Microsoft::UI::Xaml::UIElement>();
            if (!element) return;
            m_restTranslation = element.Translation();
            m_translationCaptured = true;
        }

        template<typename Sender>
        void RestoreTranslation(Sender const& sender)
        {
            if (!m_translationCaptured) return;
            if (auto element = sender.template try_as<Microsoft::UI::Xaml::UIElement>())
                element.Translation(m_restTranslation);
            m_translationCaptured = false;
        }

        template<typename Sender>
        void ApplyRest(Sender const& sender)
        {
            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element) return;
            auto const scale = std::clamp(implementation::LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
            SetScale(element, scale, scale);
        }

        template<typename Sender>
        void AnimateState(Sender const& sender)
        {
            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element) return;

            double scale = implementation::LiquidGlassInteraction::GetRestScale(owner);
            if (m_pressed) scale = implementation::LiquidGlassInteraction::GetPressedScale(owner);
            else if (m_pointerOver) scale = implementation::LiquidGlassInteraction::GetPointerOverScale(owner);
            scale = std::clamp(scale, .25, 4.0);
            AnimateScale(element, scale, scale, Duration(owner));
        }

        template<typename Sender>
        void ApplyPointerResponse(Sender const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_pointerOver) return;
            ApplyElasticity(sender, args);
            ApplyPointerDisplacement(sender, args);
        }

        template<typename Sender>
        void ApplyElasticity(Sender const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            auto relativeTo = sender.template try_as<Microsoft::UI::Xaml::UIElement>();
            if (!owner || !element || !relativeTo) return;

            auto const elasticity = std::clamp(
                implementation::LiquidGlassInteraction::GetElasticity(owner), 0.0, 1.0);
            if (elasticity <= 1e-5) return;

            auto const width = element.ActualWidth();
            auto const height = element.ActualHeight();
            if (width <= 0.0 || height <= 0.0) return;

            auto const point = args.GetCurrentPoint(relativeTo).Position();
            auto const nx = std::clamp((point.X / width) * 2.0 - 1.0, -1.0, 1.0);
            auto const ny = std::clamp((point.Y / height) * 2.0 - 1.0, -1.0, 1.0);
            auto const radial = std::min(1.0, std::sqrt(nx * nx + ny * ny));
            auto const amount = elasticity * radial * (m_pressed ? .060 : .032);
            auto const base = std::clamp(
                m_pressed ? implementation::LiquidGlassInteraction::GetPressedScale(owner)
                          : implementation::LiquidGlassInteraction::GetPointerOverScale(owner),
                .25, 4.0);

            auto sx = base * (1.0 + amount * std::abs(nx));
            auto sy = base * (1.0 + amount * std::abs(ny));
            if (std::abs(nx) > std::abs(ny)) sy *= 1.0 - amount * .35;
            else sx *= 1.0 - amount * .35;
            SetScale(element, sx, sy);
        }

        template<typename Sender>
        void ApplyPointerDisplacement(Sender const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            auto relativeTo = sender.template try_as<Microsoft::UI::Xaml::UIElement>();
            if (!owner || !element || !relativeTo) return;

            CaptureTranslation(sender);
            if (!m_translationCaptured) return;

            auto const maxDisplacement = std::clamp(
                implementation::LiquidGlassInteraction::GetPointerDisplacement(owner), 0.0, 64.0);
            if (maxDisplacement <= 1e-5)
            {
                element.Translation(m_restTranslation);
                return;
            }

            auto const width = element.ActualWidth();
            auto const height = element.ActualHeight();
            if (width <= 0.0 || height <= 0.0) return;

            auto const point = args.GetCurrentPoint(relativeTo).Position();
            auto const nx = std::clamp((point.X / width) * 2.0 - 1.0, -1.0, 1.0);
            auto const ny = std::clamp((point.Y / height) * 2.0 - 1.0, -1.0, 1.0);
            auto const normalizer = std::tanh(1.35);
            auto const curveX = std::tanh(nx * 1.35) / normalizer;
            auto const curveY = std::tanh(ny * 1.35) / normalizer;
            auto const pressMultiplier = m_pressed
                ? std::clamp(implementation::LiquidGlassInteraction::GetPressedDisplacementMultiplier(owner), 0.0, 4.0)
                : 1.0;

            element.Translation({
                m_restTranslation.x + static_cast<float>(curveX * maxDisplacement * pressMultiplier),
                m_restTranslation.y + static_cast<float>(curveY * maxDisplacement * pressMultiplier),
                m_restTranslation.z });
        }

        Windows::Foundation::Numerics::float3 m_restTranslation{};
        bool m_translationCaptured{};
        bool m_pointerOver{};
        bool m_pressed{};
    };
}

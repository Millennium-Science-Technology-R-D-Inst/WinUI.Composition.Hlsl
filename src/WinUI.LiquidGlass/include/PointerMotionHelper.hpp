#pragma once

#include "MotionAnimation.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self>
    class PointerMotionHelper
    {
    public:
        PointerMotionHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const& sender, auto const&)
            {
                m_loaded = true;
                m_pointerOver = false;
                m_pressed = false;
                EndDrag();
                ApplyRest(sender);
            });
            self->Unloaded([this](auto const&, auto const&)
            {
                // Pointer-up can be swallowed by navigation/window teardown. Drop transient
                // interaction state without touching Composition; Loaded restores the stable
                // translation/scale once the visual is valid again.
                m_loaded = false;
                m_pointerOver = false;
                m_pressed = false;
                EndDrag();
            });

            auto bindPointerHandler = [self](
                auto routedEvent,
                Windows::Foundation::IInspectable& storage,
                auto&& callback)
            {
                using Handler = Microsoft::UI::Xaml::Input::PointerEventHandler;
                storage = winrt::box_value<Handler>({ std::forward<decltype(callback)>(callback) });
                self->AddHandler(routedEvent, storage, true);
            };

            // ButtonBase handles PointerPressed/Released before ordinary instance handlers.
            // Listen with handledEventsToo so native Button/Toggle/CheckBox descendants keep
            // the same liquid motion contract as plain content controls.
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerEnteredEvent(),
                m_pointerEnteredHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    CaptureTranslation(sender);
                    m_pointerOver = true;
                    AnimateState(sender);
                });
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerMovedEvent(),
                m_pointerMovedHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
                {
                    if (m_pressed) ApplyDragResponse(sender, args);
                    else ApplyPointerResponse(sender, args);
                });
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerExitedEvent(),
                m_pointerExitedHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    m_pointerOver = false;
                    if (!m_pressed)
                    {
                        AnimateState(sender);
                        RestoreTranslation(sender);
                    }
                });
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerPressedEvent(),
                m_pointerPressedHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
                {
                    CaptureTranslation(sender);
                    BeginDrag(sender, args);
                    m_pressed = true;
                    AnimateState(sender);
                    ApplyDragResponse(sender, args);
                });
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerReleasedEvent(),
                m_pointerReleasedHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    m_pressed = false;
                    EndDrag();
                    AnimateState(sender);
                    RestoreTranslation(sender);
                });
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerCaptureLostEvent(),
                m_pointerCaptureLostHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    m_pressed = false;
                    EndDrag();
                    AnimateState(sender);
                    RestoreTranslation(sender);
                });
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(),
                m_pointerCanceledHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    m_pressed = false;
                    EndDrag();
                    AnimateState(sender);
                    RestoreTranslation(sender);
                });
        }

    private:
        static double SaturatingDistance(double value, double limit)
        {
            value = std::abs(value);
            if (value <= 1e-6 || limit <= 0.0) return 0.0;

            // SuGarToolkit's liquid-glass interaction uses k*x/(x+618) for both
            // offset and stretch. It keeps short drags precise while asymptotically
            // bounding extreme drags instead of hard-clamping them.
            constexpr double growth = 618.0;
            return limit * value / (value + growth);
        }

        template<typename Sender>
        void CaptureTranslation(Sender const& sender)
        {
            if (!m_loaded || m_translationCaptured) return;
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!element) return;

            Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetIsTranslationEnabled(element, true);
            m_restTranslation = element.Translation();
            m_translationCaptured = true;
        }

        template<typename Sender>
        void RestoreTranslation(Sender const& sender)
        {
            if (!m_loaded || !m_translationCaptured) return;
            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element) return;

            AnimateElementTranslation(
                owner,
                element,
                m_restTranslation,
                implementation::LiquidGlassInteraction::GetMotionDuration(owner));
        }

        template<typename Sender>
        void BeginDrag(Sender const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_loaded) return;
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            auto uiElement = sender.template try_as<Microsoft::UI::Xaml::UIElement>();
            if (!element || !uiElement) return;

            auto xamlRoot = element.XamlRoot();
            m_dragCoordinateRoot = xamlRoot ? xamlRoot.Content() : Microsoft::UI::Xaml::UIElement{ nullptr };
            if (!m_dragCoordinateRoot) m_dragCoordinateRoot = uiElement;

            auto const point = args.GetCurrentPoint(m_dragCoordinateRoot);
            m_activePointerId = point.PointerId();
            m_dragStart = point.Position();
        }

        void EndDrag()
        {
            m_activePointerId = 0;
            m_dragCoordinateRoot = nullptr;
        }

        template<typename Sender>
        void ApplyRest(Sender const& sender)
        {
            if (!m_loaded) return;
            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element) return;
            CaptureTranslation(sender);
            if (m_translationCaptured)
                SetElementTranslation(element, m_restTranslation);
            auto const scale = std::clamp(implementation::LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
            SetElementScale(element, scale, scale);
        }

        template<typename Sender>
        void AnimateState(Sender const& sender)
        {
            if (!m_loaded) return;
            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element) return;

            double scale = implementation::LiquidGlassInteraction::GetRestScale(owner);
            if (m_pressed) scale = implementation::LiquidGlassInteraction::GetPressedScale(owner);
            else if (m_pointerOver) scale = implementation::LiquidGlassInteraction::GetPointerOverScale(owner);
            scale = std::clamp(scale, .25, 4.0);
            AnimateElementScale(
                owner,
                element,
                scale,
                scale,
                implementation::LiquidGlassInteraction::GetMotionDuration(owner));
        }

        template<typename Sender>
        void ApplyPointerResponse(Sender const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_loaded || !m_pointerOver) return;
            ApplyElasticity(sender, args);
            ApplyPointerDisplacement(sender, args);
        }

        template<typename Sender>
        void ApplyElasticity(Sender const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_loaded) return;
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
            auto const amount = elasticity * radial * .032;
            auto const base = std::clamp(
                implementation::LiquidGlassInteraction::GetPointerOverScale(owner),
                .25, 4.0);

            auto sx = base * (1.0 + amount * std::abs(nx));
            auto sy = base * (1.0 + amount * std::abs(ny));
            if (std::abs(nx) > std::abs(ny)) sy *= 1.0 - amount * .35;
            else sx *= 1.0 - amount * .35;
            SetElementScale(element, sx, sy);
        }

        template<typename Sender>
        void ApplyDragResponse(Sender const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_loaded || !m_dragCoordinateRoot || m_activePointerId == 0) return;

            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element) return;

            auto const point = args.GetCurrentPoint(m_dragCoordinateRoot);
            if (point.PointerId() != m_activePointerId) return;

            auto const width = element.ActualWidth();
            auto const height = element.ActualHeight();
            if (width <= 0.0 || height <= 0.0) return;

            auto const deltaX = static_cast<double>(point.Position().X - m_dragStart.X);
            auto const deltaY = static_cast<double>(point.Position().Y - m_dragStart.Y);
            auto const absX = std::abs(deltaX);
            auto const absY = std::abs(deltaY);

            auto const elasticity = std::clamp(
                implementation::LiquidGlassInteraction::GetElasticity(owner), 0.0, 1.0);
            auto const responseGain = elasticity <= 1e-5
                ? 0.0
                : std::clamp(elasticity / .18, 0.0, 3.0);

            // Keep the same three bounded response families used by SuGarToolkit:
            // 16 DIP axial stretch, 0.382 relative cross-axis compression and
            // 24 DIP drag offset. Elasticity scales their amplitude without changing
            // the asymptotic response curve.
            auto const stretchX = SaturatingDistance(absX, 16.0 * responseGain) -
                SaturatingDistance(absY, .382 * responseGain) * width;
            auto const stretchY = SaturatingDistance(absY, 16.0 * responseGain) -
                SaturatingDistance(absX, .382 * responseGain) * height;
            auto const scaleBase = std::clamp(
                implementation::LiquidGlassInteraction::GetPressedScale(owner), .25, 4.0);
            auto const relativeScaleX = std::clamp(1.0 + stretchX / width, .62, 1.45);
            auto const relativeScaleY = std::clamp(1.0 + stretchY / height, .62, 1.45);
            SetElementScale(element, scaleBase * relativeScaleX, scaleBase * relativeScaleY);

            auto const maxDisplacement = std::clamp(
                implementation::LiquidGlassInteraction::GetPointerDisplacement(owner), 0.0, 64.0);
            auto const configuredGain = maxDisplacement > 1e-5
                ? std::clamp(maxDisplacement / 2.5, 0.0, 8.0)
                : 0.0;
            auto const offsetLimit = 24.0 * responseGain * configuredGain;
            auto const offsetX = std::copysign(SaturatingDistance(absX, offsetLimit), deltaX);
            auto const offsetY = std::copysign(SaturatingDistance(absY, offsetLimit), deltaY);
            SetElementTranslation(element, {
                m_restTranslation.x + static_cast<float>(offsetX),
                m_restTranslation.y + static_cast<float>(offsetY),
                m_restTranslation.z });
        }

        template<typename Sender>
        void ApplyPointerDisplacement(Sender const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_loaded) return;
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
                SetElementTranslation(element, m_restTranslation);
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

            SetElementTranslation(element, {
                m_restTranslation.x + static_cast<float>(curveX * maxDisplacement),
                m_restTranslation.y + static_cast<float>(curveY * maxDisplacement),
                m_restTranslation.z });
        }

        Windows::Foundation::Numerics::float3 m_restTranslation{};
        Microsoft::UI::Xaml::UIElement m_dragCoordinateRoot{ nullptr };
        Windows::Foundation::IInspectable m_pointerEnteredHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerMovedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerExitedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerPressedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerReleasedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCaptureLostHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCanceledHandler{ nullptr };
        Windows::Foundation::Point m_dragStart{};
        uint32_t m_activePointerId{};
        bool m_loaded{};
        bool m_translationCaptured{};
        bool m_pointerOver{};
        bool m_pressed{};
    };
}

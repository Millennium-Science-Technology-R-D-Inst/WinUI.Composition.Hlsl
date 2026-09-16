#pragma once

#include "ChildSurfaceInteraction.hpp"
#include "MotionAnimation.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self>
    class ChoiceGlyphMotionHelper
    {
    public:
        ChoiceGlyphMotionHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const&, auto const&)
            {
                RefreshTarget();
                ApplyState(false);
            });
            self->Unloaded([this](auto const&, auto const&)
            {
                m_target = nullptr;
                m_pointerOver = false;
                m_pressed = false;
            });

            auto bind = [self](auto routedEvent, Windows::Foundation::IInspectable& storage, auto&& callback)
            {
                using Handler = Microsoft::UI::Xaml::Input::PointerEventHandler;
                storage = winrt::box_value<Handler>({ std::forward<decltype(callback)>(callback) });
                self->AddHandler(routedEvent, storage, true);
            };
            bind(Microsoft::UI::Xaml::UIElement::PointerEnteredEvent(), m_pointerEnteredHandler,
                [this](auto const&, auto const&) { m_pointerOver = true; ApplyState(true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerExitedEvent(), m_pointerExitedHandler,
                [this](auto const&, auto const&) { m_pointerOver = false; if (!m_pressed) ApplyState(true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerPressedEvent(), m_pointerPressedHandler,
                [this](auto const&, auto const&) { m_pressed = true; ApplyState(true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerReleasedEvent(), m_pointerReleasedHandler,
                [this](auto const&, auto const&) { m_pressed = false; ApplyState(true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCaptureLostEvent(), m_pointerCaptureLostHandler,
                [this](auto const&, auto const&) { m_pressed = false; ApplyState(true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(), m_pointerCanceledHandler,
                [this](auto const&, auto const&) { m_pressed = false; ApplyState(true); });
        }

        void RefreshTarget()
        {
            auto self = static_cast<Self*>(this);
            auto root = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!root) return;
            m_target = FindNamedDescendant(root, L"ChoiceGlyphHost").try_as<Microsoft::UI::Xaml::FrameworkElement>();
        }

    private:
        void ApplyState(bool animate)
        {
            if (!m_target) RefreshTarget();
            if (!m_target) return;
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!owner) return;

            double scale = implementation::LiquidGlassInteraction::GetRestScale(owner);
            if (m_pressed) scale = implementation::LiquidGlassInteraction::GetPressedScale(owner);
            else if (m_pointerOver) scale = implementation::LiquidGlassInteraction::GetPointerOverScale(owner);
            scale = std::clamp(scale, .25, 4.0);

            if (animate)
                AnimateElementScale(owner, m_target, scale, implementation::LiquidGlassInteraction::GetMotionDuration(owner));
            else
                SetElementScale(m_target, scale, scale);
        }

        Microsoft::UI::Xaml::FrameworkElement m_target{ nullptr };
        Windows::Foundation::IInspectable m_pointerEnteredHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerExitedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerPressedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerReleasedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCaptureLostHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCanceledHandler{ nullptr };
        bool m_pointerOver{};
        bool m_pressed{};
    };
}

#pragma once

#include "ChildSurfaceInteraction.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self>
    class SwitchElevationHelper
    {
    public:
        SwitchElevationHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const&, auto const&)
            {
                m_loaded = true;
                RefreshSurface();
                ApplyElevation();
            });
            self->Unloaded([this](auto const&, auto const&)
            {
                // XAML/compositor teardown is deliberately no-write. The next Loaded
                // resolves the new template surface and restores authored elevation.
                m_loaded = false;
                m_pointerOver = false;
                m_pressed = false;
                m_surface = nullptr;
            });

            auto bind = [self](auto routedEvent, Windows::Foundation::IInspectable& storage, auto&& callback)
            {
                using Handler = Microsoft::UI::Xaml::Input::PointerEventHandler;
                storage = winrt::box_value(Handler{ std::forward<decltype(callback)>(callback) });
                self->AddHandler(routedEvent, storage, true);
            };

            bind(Microsoft::UI::Xaml::UIElement::PointerEnteredEvent(), m_pointerEnteredHandler,
                [this](auto const&, auto const&)
                {
                    m_pointerOver = true;
                    ApplyElevation();
                });
            bind(Microsoft::UI::Xaml::UIElement::PointerExitedEvent(), m_pointerExitedHandler,
                [this](auto const&, auto const&)
                {
                    m_pointerOver = false;
                    ApplyElevation();
                });
            bind(Microsoft::UI::Xaml::UIElement::PointerPressedEvent(), m_pointerPressedHandler,
                [this](auto const&, auto const&)
                {
                    m_pressed = true;
                    ApplyElevation();
                });
            bind(Microsoft::UI::Xaml::UIElement::PointerReleasedEvent(), m_pointerReleasedHandler,
                [this](auto const&, auto const&)
                {
                    m_pressed = false;
                    ApplyElevation();
                });
            bind(Microsoft::UI::Xaml::UIElement::PointerCaptureLostEvent(), m_pointerCaptureLostHandler,
                [this](auto const&, auto const&)
                {
                    m_pressed = false;
                    ApplyElevation();
                });
            bind(Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(), m_pointerCanceledHandler,
                [this](auto const&, auto const&)
                {
                    m_pressed = false;
                    ApplyElevation();
                });
        }

        void RefreshElevationSurface()
        {
            if (!m_loaded) return;
            // OnApplyTemplate may replace the Border while this helper is still loaded.
            // Drop the old reference before resolving the new template part.
            m_surface = nullptr;
            RefreshSurface();
            ApplyElevation();
        }

    private:
        static constexpr float kRestElevation = 8.0f;
        static constexpr float kPointerOverElevation = 9.0f;
        static constexpr float kPressedElevation = 12.0f;

        void RefreshSurface()
        {
            auto self = static_cast<Self*>(this);
            auto root = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!root) return;

            auto surface = FindNamedDescendant(root, L"SwitchKnobSurface")
                .try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!surface)
            {
                m_surface = nullptr;
                return;
            }

            if (!m_surface || get_abi(m_surface) != get_abi(surface))
            {
                m_surface = surface;
                Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetIsTranslationEnabled(m_surface, true);
            }
        }

        void ApplyElevation()
        {
            if (!m_loaded) return;
            if (!m_surface) RefreshSurface();
            if (!m_surface) return;

            auto const z = m_pressed
                ? kPressedElevation
                : (m_pointerOver ? kPointerOverElevation : kRestElevation);
            auto current = m_surface.Translation();
            current.z = z;
            m_surface.Translation(current);
        }

        Microsoft::UI::Xaml::FrameworkElement m_surface{ nullptr };
        Windows::Foundation::IInspectable m_pointerEnteredHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerExitedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerPressedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerReleasedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCaptureLostHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCanceledHandler{ nullptr };
        bool m_loaded{};
        bool m_pointerOver{};
        bool m_pressed{};
    };
}

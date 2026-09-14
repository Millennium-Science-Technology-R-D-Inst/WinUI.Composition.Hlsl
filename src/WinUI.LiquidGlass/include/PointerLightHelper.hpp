#pragma once

namespace winrt::WinUI::LiquidGlass::detail
{
    // Drives LiquidGlassBrush::LightAngle from local pointer position. The angle that
    // was active when tracking began is restored on exit, so application-provided
    // resting lighting remains authoritative.
    template<typename Self>
    class PointerLightHelper
    {
    public:
        PointerLightHelper()
        {
            auto self = static_cast<Self*>(this);
            self->PointerEntered([this](auto const&, auto const&) { BeginTracking(); });
            self->PointerMoved([this](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
            {
                UpdateLight(args);
            });
            self->PointerExited([this](auto const&, auto const&) { EndTracking(); });
        }

    private:
        void BeginTracking()
        {
            auto self = static_cast<Self*>(this);
            if (auto brush = self->GlassBrush())
            {
                m_restingLightAngle = brush.LightAngle();
                m_tracking = true;
            }
        }

        void UpdateLight(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            auto self = static_cast<Self*>(this);
            auto brush = self->GlassBrush();
            if (!brush) return;

            if (!m_tracking)
            {
                m_restingLightAngle = brush.LightAngle();
                m_tracking = true;
            }

            auto element = self->template as<Microsoft::UI::Xaml::FrameworkElement>();
            auto relativeTo = self->template as<Microsoft::UI::Xaml::UIElement>();
            auto const width = element.ActualWidth();
            auto const height = element.ActualHeight();
            if (width <= 0.0 || height <= 0.0) return;

            auto const position = args.GetCurrentPoint(relativeTo).Position();
            auto const dx = position.X - width * 0.5;
            auto const dy = position.Y - height * 0.5;
            if (std::abs(dx) + std::abs(dy) > 1e-4)
            {
                brush.LightAngle(std::atan2(dy, dx));
            }
        }

        void EndTracking()
        {
            if (!m_tracking) return;
            auto self = static_cast<Self*>(this);
            if (auto brush = self->GlassBrush()) brush.LightAngle(m_restingLightAngle);
            m_tracking = false;
        }

        double m_restingLightAngle{ -0.95 };
        bool m_tracking{};
    };
}

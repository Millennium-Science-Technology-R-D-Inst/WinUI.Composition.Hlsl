#pragma once

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self>
    class PointerLightHelper
    {
    public:
        PointerLightHelper()
        {
            auto self = static_cast<Self*>(this);
            self->PointerEntered([this](auto const& sender, auto const&) { BeginTracking(sender); });
            self->PointerMoved([this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
            {
                UpdateLight(sender, args);
            });
            self->PointerExited([this](auto const&, auto const&) { EndTracking(); });
            self->PointerCanceled([this](auto const&, auto const&) { EndTracking(); });
            self->PointerCaptureLost([this](auto const&, auto const&) { EndTracking(); });
            self->Unloaded([this](auto const&, auto const&) { EndTracking(); });
        }

    private:
        using LightBrush = WinUI::Composition::Hlsl::LiquidGlassBrush;

        static void SetMaterialLightAngle(LightBrush const& brush, double value)
        {
            if (!brush) return;
            if (auto material = brush.Material())
            {
                material.LightAngle(static_cast<float>(value));
            }
        }

        void TrackBrush(LightBrush const& brush)
        {
            if (!brush)
            {
                EndTracking();
                return;
            }

            if (m_trackingBrush && get_abi(m_trackingBrush) != get_abi(brush))
            {
                SetMaterialLightAngle(m_trackingBrush, m_trackingBrush.LightAngle());
            }

            m_trackingBrush = brush;
            m_tracking = true;
        }

        template<typename Sender>
        void BeginTracking(Sender const& sender)
        {
            auto object = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!object || !implementation::LiquidGlassInteraction::GetPointerLightingEnabled(object)) return;

            auto self = static_cast<Self*>(this);
            TrackBrush(self->GlassBrush());
        }

        template<typename Sender>
        void UpdateLight(Sender const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            auto object = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!object || !implementation::LiquidGlassInteraction::GetPointerLightingEnabled(object))
            {
                EndTracking();
                return;
            }

            auto self = static_cast<Self*>(this);
            auto brush = self->GlassBrush();
            if (!brush)
            {
                EndTracking();
                return;
            }

            if (!m_tracking || !m_trackingBrush || get_abi(m_trackingBrush) != get_abi(brush))
            {
                TrackBrush(brush);
            }

            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            auto relativeTo = sender.template try_as<Microsoft::UI::Xaml::UIElement>();
            if (!element || !relativeTo) return;

            auto const width = element.ActualWidth();
            auto const height = element.ActualHeight();
            if (width <= 0.0 || height <= 0.0) return;

            auto const position = args.GetCurrentPoint(relativeTo).Position();
            auto const dx = position.X - width * 0.5;
            auto const dy = position.Y - height * 0.5;
            if (std::abs(dx) + std::abs(dy) <= 1e-4) return;

            auto const pointerAngle = std::atan2(dy, dx);
            auto const influence = std::clamp(
                implementation::LiquidGlassInteraction::GetPointerLightInfluence(object), 0.0, 1.0);
            auto const restingLightAngle = brush.LightAngle();

            auto const x = std::cos(restingLightAngle) * (1.0 - influence) + std::cos(pointerAngle) * influence;
            auto const y = std::sin(restingLightAngle) * (1.0 - influence) + std::sin(pointerAngle) * influence;
            if (std::abs(x) + std::abs(y) > 1e-5)
            {
                SetMaterialLightAngle(brush, std::atan2(y, x));
            }
        }

        void EndTracking()
        {
            if (m_trackingBrush)
            {
                // Pointer lighting is transient. Restore the authored DP value on
                // the exact brush whose live material was modified, including when
                // GlassBrush was replaced while the pointer was still inside.
                SetMaterialLightAngle(m_trackingBrush, m_trackingBrush.LightAngle());
            }
            m_trackingBrush = nullptr;
            m_tracking = false;
        }

        LightBrush m_trackingBrush{ nullptr };
        bool m_tracking{};
    };
}

#pragma once

#include <cmath>

#include "MotionAnimation.hpp"
#include "PressOpticsHelper.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self>
    class MagnifierMotionHelper
    {
    public:
        MagnifierMotionHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const&, auto const&)
            {
                m_loaded = true;
                RestorePendingOptics();
            });
            self->Unloaded([this](auto const&, auto const&) { ClearForTeardown(); });
            self->RegisterPropertyChangedCallback(Self::GlassBrushProperty(), [this](auto const&, auto const&)
            {
                if (!m_loaded)
                {
                    // Detached brush replacement retires the old material. Never write the
                    // saved active state into a brush whose compositor may already be closed.
                    m_dragOptics = {};
                    m_restMagnification = 0.0;
                }
            });

            auto bind = [self](auto routedEvent, Windows::Foundation::IInspectable& storage, auto&& callback)
            {
                using Handler = Microsoft::UI::Xaml::Input::PointerEventHandler;
                storage = winrt::box_value<Handler>({ std::forward<decltype(callback)>(callback) });
                self->AddHandler(routedEvent, storage, true);
            };

            // Register before the legacy derived handlers. Once this helper successfully
            // captures the pointer it marks the routed event handled, leaving one owner for
            // position, deformation and active optics.
            bind(Microsoft::UI::Xaml::UIElement::PointerPressedEvent(), m_pointerPressedHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
                {
                    BeginInteraction(sender, args);
                });
            bind(Microsoft::UI::Xaml::UIElement::PointerMovedEvent(), m_pointerMovedHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
                {
                    UpdateInteraction(sender, args);
                });
            bind(Microsoft::UI::Xaml::UIElement::PointerReleasedEvent(), m_pointerReleasedHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
                {
                    EndFromRelease(sender, args);
                });
            bind(Microsoft::UI::Xaml::UIElement::PointerCaptureLostEvent(), m_pointerCaptureLostHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
                {
                    EndInteraction(sender, true);
                    if (m_consumedInteraction) args.Handled(true);
                });
            bind(Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(), m_pointerCanceledHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
                {
                    EndInteraction(sender, true);
                    if (m_consumedInteraction) args.Handled(true);
                });
        }

    private:
        using Clock = std::chrono::steady_clock;
        static constexpr double kVelocityTimeConstantSeconds = 0.060;
        static constexpr double kDeformationTimeConstantSeconds = 0.120;
        static constexpr double kMaximumDeformation = 0.11;
        static constexpr double kDeformationHalfSpeed = 2800.0;
        static constexpr double kCrossAxisCompression = 0.55;
        static constexpr int64_t kOpticsDurationMs = 210;
        static constexpr int64_t kMagnificationDurationMs = 240;

        static double SmoothingAlpha(double elapsedSeconds, double timeConstantSeconds)
        {
            if (elapsedSeconds <= 0.0) return 0.0;
            return 1.0 - std::exp(-elapsedSeconds / timeConstantSeconds);
        }

        static void AnimateOpticsToCurrent(
            WinUI::Composition::Hlsl::LiquidGlassBrush const& brush,
            OpticsSnapshot const& from,
            std::chrono::milliseconds duration)
        {
            if (!brush || !from.active) return;
            auto material = brush.Material();
            if (!material) return;
            auto effect = material.EffectBrush();
            if (!effect) return;
            auto compositionBrush = effect.EffectBrush();
            if (!compositionBrush) return;

            auto easing = compositionBrush.Compositor().CreateCubicBezierEasingFunction(
                { 0.20f, 0.0f }, { 0.0f, 1.0f });
            AnimateOpticsScalar(effect, compositionBrush, easing, duration,
                L"RefractionStrength", from.refraction, brush.RefractionStrength());
            AnimateOpticsScalar(effect, compositionBrush, easing, duration,
                L"DispersionStrength", from.dispersion, brush.DispersionStrength());
            AnimateOpticsScalar(effect, compositionBrush, easing, duration,
                L"Saturation", from.saturation, brush.Saturation());
            AnimateOpticsScalar(effect, compositionBrush, easing, duration,
                L"Contrast", from.contrast, brush.Contrast());
            AnimateOpticsScalar(effect, compositionBrush, easing, duration,
                L"Exposure", from.exposure, brush.Exposure());
            AnimateOpticsScalar(effect, compositionBrush, easing, duration,
                L"TintOpacity", from.tintOpacity, brush.TintOpacity());
            AnimateOpticsScalar(effect, compositionBrush, easing, duration,
                L"HighlightStrength", from.highlight, brush.HighlightStrength());
            AnimateOpticsScalar(effect, compositionBrush, easing, duration,
                L"InnerShadowStrength", from.innerShadow, brush.InnerShadowStrength());
        }

        static void AnimateMagnification(
            WinUI::Composition::Hlsl::LiquidGlassBrush const& brush,
            double from,
            double to)
        {
            if (!brush || std::abs(from - to) <= 1e-5) return;
            auto material = brush.Material();
            if (!material) return;
            auto effect = material.EffectBrush();
            if (!effect) return;
            auto compositionBrush = effect.EffectBrush();
            if (!compositionBrush) return;
            auto easing = compositionBrush.Compositor().CreateCubicBezierEasingFunction(
                { 0.20f, 0.0f }, { 0.0f, 1.0f });
            AnimateOpticsScalar(
                effect,
                compositionBrush,
                easing,
                std::chrono::milliseconds{ kMagnificationDurationMs },
                L"MagnificationStrength",
                from,
                to);
        }

        void RestorePendingOptics()
        {
            if (!m_dragOptics.active) return;
            auto self = static_cast<Self*>(this);
            auto brush = self->GlassBrush();
            if (brush && m_dragOptics.brush && get_abi(brush) == get_abi(m_dragOptics.brush))
            {
                ApplySnapshot(m_dragOptics);
                brush.MagnificationStrength(m_restMagnification);
            }
            m_dragOptics = {};
        }

        template<typename Sender>
        void BeginInteraction(
            Sender const& sender,
            Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_loaded || m_dragging) return;
            auto self = static_cast<Self*>(this);
            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = sender.template try_as<Microsoft::UI::Xaml::UIElement>();
            auto frameworkElement = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element || !frameworkElement) return;

            auto xamlRoot = frameworkElement.XamlRoot();
            auto coordinateRoot = xamlRoot ? xamlRoot.Content() : Microsoft::UI::Xaml::UIElement{ nullptr };
            if (!coordinateRoot) coordinateRoot = element;
            auto const point = args.GetCurrentPoint(coordinateRoot);
            if (!element.CapturePointer(args.Pointer())) return;

            Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetIsTranslationEnabled(frameworkElement, true);
            m_dragStartTranslation = frameworkElement.Translation();
            m_coordinateRoot = coordinateRoot;
            m_dragStartPointer = point.Position();
            m_lastPointer = point.Position();
            m_lastPointerTime = Clock::now();
            m_filteredVelocityX = 0.0;
            m_filteredVelocityY = 0.0;
            m_deformation = 0.0;
            m_axisX = 1.0;
            m_axisY = 0.0;
            m_pointerId = point.PointerId();
            m_dragging = true;
            m_consumedInteraction = true;

            if (auto brush = self->GlassBrush())
            {
                m_restMagnification = brush.MagnificationStrength();
                CaptureOptics(brush, m_dragOptics);
                m_dragOptics.owner = owner;
                auto from = m_dragOptics;
                ApplyPressedOptics(owner, brush);
                AnimateOpticsToCurrent(
                    brush,
                    from,
                    std::chrono::milliseconds{ kOpticsDurationMs });

                auto const activeMagnification = std::clamp(
                    m_restMagnification * implementation::LiquidGlassInteraction::GetActiveMagnificationMultiplier(owner),
                    0.0,
                    128.0);
                brush.MagnificationStrength(activeMagnification);
                AnimateMagnification(brush, m_restMagnification, activeMagnification);
            }

            auto const scale = std::clamp(
                implementation::LiquidGlassInteraction::GetPressedScale(owner), .25, 4.0);
            AnimateElementScale(
                owner,
                frameworkElement,
                scale,
                scale,
                implementation::LiquidGlassInteraction::GetMotionDuration(owner));
            args.Handled(true);
        }

        template<typename Sender>
        void UpdateInteraction(
            Sender const& sender,
            Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_loaded || !m_dragging || !m_coordinateRoot) return;
            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto frameworkElement = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !frameworkElement) return;

            auto const point = args.GetCurrentPoint(m_coordinateRoot);
            if (point.PointerId() != m_pointerId) return;
            auto const position = point.Position();

            // Position is intentionally unfiltered. Pointer-to-position latency must remain
            // one event; only the secondary liquid deformation is smoothed.
            SetElementTranslation(frameworkElement, {
                m_dragStartTranslation.x + position.X - m_dragStartPointer.X,
                m_dragStartTranslation.y + position.Y - m_dragStartPointer.Y,
                m_dragStartTranslation.z });

            auto const now = Clock::now();
            auto const elapsed = std::chrono::duration<double>(now - m_lastPointerTime).count();
            if (elapsed > 1e-4 && elapsed < 0.15)
            {
                auto const instantaneousX = static_cast<double>(position.X - m_lastPointer.X) / elapsed;
                auto const instantaneousY = static_cast<double>(position.Y - m_lastPointer.Y) / elapsed;
                auto const velocityAlpha = SmoothingAlpha(elapsed, kVelocityTimeConstantSeconds);
                m_filteredVelocityX += (instantaneousX - m_filteredVelocityX) * velocityAlpha;
                m_filteredVelocityY += (instantaneousY - m_filteredVelocityY) * velocityAlpha;

                auto const speed = std::hypot(m_filteredVelocityX, m_filteredVelocityY);
                if (speed > 1.0)
                {
                    m_axisX = std::abs(m_filteredVelocityX) / speed;
                    m_axisY = std::abs(m_filteredVelocityY) / speed;
                }
                auto const targetDeformation = kMaximumDeformation *
                    speed / (speed + kDeformationHalfSpeed);
                auto const deformationAlpha = SmoothingAlpha(elapsed, kDeformationTimeConstantSeconds);
                m_deformation += (targetDeformation - m_deformation) * deformationAlpha;
            }
            else if (elapsed >= 0.15)
            {
                m_filteredVelocityX = 0.0;
                m_filteredVelocityY = 0.0;
                m_deformation *= 0.5;
            }

            auto const axisX2 = m_axisX * m_axisX;
            auto const axisY2 = m_axisY * m_axisY;
            auto const base = std::clamp(
                implementation::LiquidGlassInteraction::GetPressedScale(owner), .25, 4.0);
            auto const scaleX = base * (1.0 + m_deformation * axisX2 -
                m_deformation * kCrossAxisCompression * axisY2);
            auto const scaleY = base * (1.0 + m_deformation * axisY2 -
                m_deformation * kCrossAxisCompression * axisX2);
            SetElementScale(frameworkElement, scaleX, scaleY);

            m_lastPointer = position;
            m_lastPointerTime = now;
            args.Handled(true);
        }

        template<typename Sender>
        void EndFromRelease(
            Sender const& sender,
            Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_loaded || !m_dragging) return;
            auto element = sender.template try_as<Microsoft::UI::Xaml::UIElement>();
            m_dragging = false;
            if (element) element.ReleasePointerCapture(args.Pointer());
            FinishVisualState(sender, true);
            args.Handled(true);
        }

        template<typename Sender>
        void EndInteraction(Sender const& sender, bool animate)
        {
            if (!m_loaded || !m_dragging) return;
            m_dragging = false;
            FinishVisualState(sender, animate);
        }

        template<typename Sender>
        void FinishVisualState(Sender const& sender, bool animate)
        {
            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto frameworkElement = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (owner && m_dragOptics.active && m_dragOptics.brush)
            {
                auto brush = m_dragOptics.brush;
                OpticsSnapshot from;
                CaptureOptics(brush, from);
                auto const currentMagnification = brush.MagnificationStrength();
                ApplySnapshot(m_dragOptics);
                brush.MagnificationStrength(m_restMagnification);
                if (animate && MotionAnimationsEnabled(owner))
                {
                    AnimateOpticsToCurrent(
                        brush,
                        from,
                        std::chrono::milliseconds{ kOpticsDurationMs });
                    AnimateMagnification(brush, currentMagnification, m_restMagnification);
                }
                m_dragOptics = {};
            }
            else
            {
                m_dragOptics = {};
            }

            if (owner && frameworkElement)
            {
                auto const scale = std::clamp(
                    implementation::LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
                if (animate)
                {
                    AnimateElementScale(
                        owner,
                        frameworkElement,
                        scale,
                        scale,
                        implementation::LiquidGlassInteraction::GetMotionDuration(owner));
                }
                else
                {
                    SetElementScale(frameworkElement, scale, scale);
                }
            }

            m_coordinateRoot = nullptr;
            m_pointerId = 0;
            m_filteredVelocityX = 0.0;
            m_filteredVelocityY = 0.0;
            m_deformation = 0.0;
        }

        void ClearForTeardown()
        {
            // No effect/visual restoration here: Unloaded can race compositor shutdown.
            // Keep the optics and magnification baselines so Loaded can restore the logical
            // brush state once the composition surface is valid again.
            m_loaded = false;
            m_coordinateRoot = nullptr;
            m_pointerId = 0;
            m_filteredVelocityX = 0.0;
            m_filteredVelocityY = 0.0;
            m_deformation = 0.0;
            m_dragging = false;
            m_consumedInteraction = false;
        }

        Microsoft::UI::Xaml::UIElement m_coordinateRoot{ nullptr };
        Windows::Foundation::IInspectable m_pointerPressedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerMovedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerReleasedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCaptureLostHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCanceledHandler{ nullptr };
        Windows::Foundation::Point m_dragStartPointer{};
        Windows::Foundation::Point m_lastPointer{};
        Windows::Foundation::Numerics::float3 m_dragStartTranslation{};
        Clock::time_point m_lastPointerTime{};
        OpticsSnapshot m_dragOptics;
        double m_restMagnification{};
        double m_filteredVelocityX{};
        double m_filteredVelocityY{};
        double m_deformation{};
        double m_axisX{ 1.0 };
        double m_axisY{};
        std::uint32_t m_pointerId{};
        bool m_loaded{};
        bool m_dragging{};
        bool m_consumedInteraction{};
    };
}

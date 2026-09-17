#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>

#include "ChildSurfaceInteraction.hpp"
#include "MotionAnimation.hpp"
#include "PressOpticsHelper.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self>
    class KubeMagnifierMotionHelper
    {
    public:
        KubeMagnifierMotionHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const& sender, auto const&)
            {
                m_loaded = true;
                RefreshSurface(sender);
                RestorePendingOptics();
                ApplyRestVisual(false);
            });
            self->Unloaded([this](auto const&, auto const&) { ClearForTeardown(); });
            self->RegisterPropertyChangedCallback(Self::GlassBrushProperty(), [this](auto const&, auto const&)
            {
                if (!m_loaded)
                {
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
                [this](auto const& sender, auto const&) { EndInteraction(sender, true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(), m_pointerCanceledHandler,
                [this](auto const& sender, auto const&) { EndInteraction(sender, true); });
        }

    private:
        using Clock = std::chrono::steady_clock;
        static constexpr double kRestObjectScale = .8;
        static constexpr double kActiveObjectScale = 1.0;
        static constexpr double kBodyStiffness = 340.0;
        static constexpr double kBodyDamping = 20.0;
        static constexpr double kSquashStiffness = 340.0;
        static constexpr double kSquashDamping = 30.0;
        static constexpr double kVelocityFloorScaleY = .7;
        static constexpr double kVelocityDivisor = 5000.0;
        static constexpr double kRestElevation = 8.0;
        static constexpr double kActiveElevation = 16.0;
        static constexpr int64_t kOpticsDurationMs = 180;
        static constexpr int64_t kMagnificationDurationMs = 180;
        static constexpr auto kDynamicsInterval = std::chrono::milliseconds{ 16 };

        static void StepSpring(
            double target,
            double stiffness,
            double damping,
            double dt,
            double& value,
            double& velocity)
        {
            // Framer/Motion uses a unit-mass damped spring. Semi-implicit Euler keeps
            // velocity continuous when the target changes, unlike restarting a new
            // Composition spring for every PointerMoved event.
            auto const acceleration = stiffness * (target - value) - damping * velocity;
            velocity += acceleration * dt;
            value += velocity * dt;
        }

        template<typename Sender>
        void RefreshSurface(Sender const& sender)
        {
            auto root = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!root)
            {
                m_surface = nullptr;
                return;
            }
            m_surface = FindNamedDescendant(root, L"MagnifierSurface")
                .try_as<Microsoft::UI::Xaml::Controls::Border>();
            if (m_surface)
            {
                Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetIsTranslationEnabled(m_surface, true);
                if (!m_surface.Shadow()) m_surface.Shadow(Microsoft::UI::Xaml::Media::ThemeShadow{});
            }
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
                { .20f, 0.0f }, { 0.0f, 1.0f });
            AnimateOpticsScalar(effect, compositionBrush, easing, duration,
                L"RefractionStrength", from.refraction, brush.RefractionStrength());
            AnimateOpticsScalar(effect, compositionBrush, easing, duration,
                L"DispersionStrength", from.dispersion, brush.DispersionStrength());
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
                { .20f, 0.0f }, { 0.0f, 1.0f });
            AnimateOpticsScalar(
                effect,
                compositionBrush,
                easing,
                std::chrono::milliseconds{ kMagnificationDurationMs },
                L"MagnificationStrength",
                from,
                to);
        }

        void SetSurfaceElevation(double elevation, bool animate)
        {
            if (!m_surface) return;
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!owner) return;
            auto translation = m_surface.Translation();
            translation.z = static_cast<float>(elevation);
            if (animate)
                AnimateElementTranslation(owner, m_surface, translation,
                    implementation::LiquidGlassInteraction::GetMotionDuration(owner));
            else
                SetElementTranslation(m_surface, translation);
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

        void EnsureDynamicsTimer()
        {
            if (!m_loaded) return;
            auto self = static_cast<Self*>(this);
            if (!m_dynamicsTimer)
            {
                m_dynamicsTimer = self->DispatcherQueue().CreateTimer();
                m_dynamicsTimer.Interval(kDynamicsInterval);
                m_dynamicsTimer.IsRepeating(true);
                m_dynamicsTimer.Tick([this](auto const&, auto const&) { TickDynamics(); });
            }
            m_lastDynamicsTick = Clock::now();
            if (!m_dynamicsTimer.IsRunning()) m_dynamicsTimer.Start();
        }

        void TickDynamics()
        {
            if (!m_loaded)
            {
                if (m_dynamicsTimer) m_dynamicsTimer.Stop();
                return;
            }

            auto self = static_cast<Self*>(this);
            auto element = self->template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!element) return;

            auto const now = Clock::now();
            auto dt = std::chrono::duration<double>(now - m_lastDynamicsTick).count();
            m_lastDynamicsTick = now;
            dt = std::clamp(dt, 1.0 / 240.0, 1.0 / 30.0);

            auto const objectTarget = m_dragging ? kActiveObjectScale : kRestObjectScale;
            StepSpring(objectTarget, kBodyStiffness, kBodyDamping, dt,
                m_objectScale, m_objectScaleVelocity);

            auto const velocityFactor = std::max(
                kVelocityFloorScaleY,
                1.0 - std::abs(m_pointerVelocityX) / kVelocityDivisor);
            auto const targetScaleY = m_objectScale * velocityFactor;
            StepSpring(targetScaleY, kSquashStiffness, kSquashDamping, dt,
                m_scaleY, m_scaleYVelocity);

            // This intentionally uses the *current* Y spring, matching Kube's
            // objectScaleX = objectScale + (1 - objectScaleY) dependency chain.
            auto const targetScaleX = m_objectScale + (1.0 - m_scaleY);
            StepSpring(targetScaleX, kSquashStiffness, kSquashDamping, dt,
                m_scaleX, m_scaleXVelocity);

            SetElementScale(element, m_scaleX, m_scaleY);

            if (!m_dragging)
            {
                auto const settled =
                    std::abs(m_objectScale - kRestObjectScale) < .001 &&
                    std::abs(m_scaleX - 1.0) < .001 &&
                    std::abs(m_scaleY - kRestObjectScale) < .001 &&
                    std::abs(m_objectScaleVelocity) < .01 &&
                    std::abs(m_scaleXVelocity) < .01 &&
                    std::abs(m_scaleYVelocity) < .01;
                if (settled)
                {
                    m_objectScale = kRestObjectScale;
                    m_scaleX = 1.0;
                    m_scaleY = kRestObjectScale;
                    m_objectScaleVelocity = 0.0;
                    m_scaleXVelocity = 0.0;
                    m_scaleYVelocity = 0.0;
                    SetElementScale(element, m_scaleX, m_scaleY);
                    if (m_dynamicsTimer) m_dynamicsTimer.Stop();
                }
            }
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
            if (!m_surface) RefreshSurface(sender);

            auto xamlRoot = frameworkElement.XamlRoot();
            m_coordinateRoot = xamlRoot ? xamlRoot.Content() : Microsoft::UI::Xaml::UIElement{ nullptr };
            if (!m_coordinateRoot) m_coordinateRoot = element;
            auto const point = args.GetCurrentPoint(m_coordinateRoot);
            if (!element.CapturePointer(args.Pointer()))
            {
                m_coordinateRoot = nullptr;
                return;
            }

            Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetIsTranslationEnabled(frameworkElement, true);
            m_dragStartTranslation = frameworkElement.Translation();
            m_dragStartPointer = point.Position();
            m_lastPointer = point.Position();
            m_lastPointerTime = Clock::now();
            m_pointerVelocityX = 0.0;
            m_pointerId = point.PointerId();
            m_dragging = true;
            EnsureDynamicsTimer();
            SetSurfaceElevation(kActiveElevation, true);

            if (auto brush = self->GlassBrush())
            {
                m_restMagnification = brush.MagnificationStrength();
                CaptureOptics(brush, m_dragOptics);
                m_dragOptics.owner = owner;
                auto from = m_dragOptics;
                ApplyPressedOptics(owner, brush);
                AnimateOpticsToCurrent(brush, from, std::chrono::milliseconds{ kOpticsDurationMs });

                auto const activeMagnification = std::clamp(
                    m_restMagnification * implementation::LiquidGlassInteraction::GetActiveMagnificationMultiplier(owner),
                    0.0,
                    128.0);
                brush.MagnificationStrength(activeMagnification);
                AnimateMagnification(brush, m_restMagnification, activeMagnification);
            }
            args.Handled(true);
        }

        template<typename Sender>
        void UpdateInteraction(
            Sender const& sender,
            Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_loaded || !m_dragging || !m_coordinateRoot) return;
            auto frameworkElement = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!frameworkElement) return;

            auto const point = args.GetCurrentPoint(m_coordinateRoot);
            if (point.PointerId() != m_pointerId) return;
            auto const position = point.Position();

            // dragMomentum=false: position follows the pointer directly with no filtering.
            SetElementTranslation(frameworkElement, {
                m_dragStartTranslation.x + position.X - m_dragStartPointer.X,
                m_dragStartTranslation.y + position.Y - m_dragStartPointer.Y,
                m_dragStartTranslation.z });

            auto const now = Clock::now();
            auto const elapsed = std::chrono::duration<double>(now - m_lastPointerTime).count();
            if (elapsed > 1e-4 && elapsed < .16)
            {
                // Kube feeds Framer's drag info.velocity.x directly into the derived
                // squash target. Keep the raw event velocity; the continuous spring timer
                // supplies the smoothing instead of pre-filtering the velocity itself.
                m_pointerVelocityX = static_cast<double>(position.X - m_lastPointer.X) / elapsed;
            }
            else if (elapsed >= .16)
            {
                m_pointerVelocityX = 0.0;
            }

            m_lastPointer = position;
            m_lastPointerTime = now;
            EnsureDynamicsTimer();
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
            m_pointerVelocityX = 0.0; // Kube onDragEnd -> velocityX.set(0)
            if (element) element.ReleasePointerCapture(args.Pointer());
            FinishVisualState(sender, true);
            EnsureDynamicsTimer();
            args.Handled(true);
        }

        template<typename Sender>
        void EndInteraction(Sender const& sender, bool animate)
        {
            if (!m_loaded || !m_dragging) return;
            m_dragging = false;
            m_pointerVelocityX = 0.0;
            FinishVisualState(sender, animate);
            EnsureDynamicsTimer();
        }

        template<typename Sender>
        void FinishVisualState(Sender const& sender, bool animate)
        {
            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
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
                    AnimateOpticsToCurrent(brush, from, std::chrono::milliseconds{ kOpticsDurationMs });
                    AnimateMagnification(brush, currentMagnification, m_restMagnification);
                }
                m_dragOptics = {};
            }
            else
            {
                m_dragOptics = {};
            }

            SetSurfaceElevation(kRestElevation, animate);
            m_coordinateRoot = nullptr;
            m_pointerId = 0;
        }

        void ApplyRestVisual(bool animate)
        {
            auto self = static_cast<Self*>(this);
            auto element = self->template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!element) return;

            if (!animate)
            {
                m_objectScale = kRestObjectScale;
                m_scaleX = 1.0;
                m_scaleY = kRestObjectScale;
                m_objectScaleVelocity = 0.0;
                m_scaleXVelocity = 0.0;
                m_scaleYVelocity = 0.0;
                SetElementScale(element, m_scaleX, m_scaleY);
            }
            else
            {
                EnsureDynamicsTimer();
            }
            SetSurfaceElevation(kRestElevation, false);
        }

        void ClearForTeardown()
        {
            m_loaded = false;
            m_dragging = false;
            m_coordinateRoot = nullptr;
            m_surface = nullptr;
            m_pointerId = 0;
            m_pointerVelocityX = 0.0;
            if (m_dynamicsTimer)
            {
                m_dynamicsTimer.Stop();
                m_dynamicsTimer = nullptr;
            }
        }

        Microsoft::UI::Xaml::UIElement m_coordinateRoot{ nullptr };
        Microsoft::UI::Xaml::Controls::Border m_surface{ nullptr };
        Microsoft::UI::Dispatching::DispatcherQueueTimer m_dynamicsTimer{ nullptr };
        Windows::Foundation::IInspectable m_pointerPressedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerMovedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerReleasedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCaptureLostHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCanceledHandler{ nullptr };
        Windows::Foundation::Point m_dragStartPointer{};
        Windows::Foundation::Point m_lastPointer{};
        Windows::Foundation::Numerics::float3 m_dragStartTranslation{};
        Clock::time_point m_lastPointerTime{};
        Clock::time_point m_lastDynamicsTick{};
        OpticsSnapshot m_dragOptics;
        double m_restMagnification{};
        double m_pointerVelocityX{};
        double m_objectScale{ kRestObjectScale };
        double m_scaleX{ 1.0 };
        double m_scaleY{ kRestObjectScale };
        double m_objectScaleVelocity{};
        double m_scaleXVelocity{};
        double m_scaleYVelocity{};
        std::uint32_t m_pointerId{};
        bool m_loaded{};
        bool m_dragging{};
    };
}

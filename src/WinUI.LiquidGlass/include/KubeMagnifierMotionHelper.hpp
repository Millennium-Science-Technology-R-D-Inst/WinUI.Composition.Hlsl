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
        static constexpr double kRestScaleX = 1.0;
        static constexpr double kRestScaleY = .8;
        static constexpr double kDragScale = 1.0;
        static constexpr double kBodyDampingRatio = .5423261445466404; // k=340,d=20
        static constexpr double kBodyPeriodMs = 340.7636269136104;
        static constexpr double kSquashDampingRatio = .8134892168199607; // k=340,d=30
        static constexpr double kSquashPeriodMs = 340.7636269136104;
        static constexpr double kVelocityFloorScaleY = .7;
        static constexpr double kVelocityDivisor = 5000.0;
        static constexpr double kVelocityTimeConstantSeconds = .025;
        static constexpr double kRestElevation = 8.0;
        static constexpr double kActiveElevation = 16.0;
        static constexpr int64_t kOpticsDurationMs = 180;
        static constexpr int64_t kMagnificationDurationMs = 180;

        static double SmoothingAlpha(double dt)
        {
            if (dt <= 0.0) return 0.0;
            return 1.0 - std::exp(-dt / kVelocityTimeConstantSeconds);
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
            m_filteredVelocityX = 0.0;
            m_pointerId = point.PointerId();
            m_dragging = true;
            SetSurfaceElevation(kActiveElevation, true);

            // Kube objectScale: rest .8 -> drag 1, but the derived X scale stays at 1
            // while the Y scale grows from .8 to 1. This is the visible pickup pulse.
            AnimateElementScaleSpring(
                owner,
                frameworkElement,
                kDragScale,
                kDragScale,
                kBodyDampingRatio,
                kBodyPeriodMs);

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
            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto frameworkElement = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !frameworkElement) return;

            auto const point = args.GetCurrentPoint(m_coordinateRoot);
            if (point.PointerId() != m_pointerId) return;
            auto const position = point.Position();

            // Pointer position remains one-event direct, just like Framer Motion drag without momentum.
            SetElementTranslation(frameworkElement, {
                m_dragStartTranslation.x + position.X - m_dragStartPointer.X,
                m_dragStartTranslation.y + position.Y - m_dragStartPointer.Y,
                m_dragStartTranslation.z });

            auto const now = Clock::now();
            auto const elapsed = std::chrono::duration<double>(now - m_lastPointerTime).count();
            if (elapsed > 1e-4 && elapsed < .16)
            {
                auto const instantaneousVelocityX = static_cast<double>(position.X - m_lastPointer.X) / elapsed;
                auto const alpha = SmoothingAlpha(elapsed);
                m_filteredVelocityX += (instantaneousVelocityX - m_filteredVelocityX) * alpha;
            }
            else if (elapsed >= .16)
            {
                m_filteredVelocityX = 0.0;
            }

            // Exact Kube target relation while dragging:
            // scaleY=max(.7,1-|vx|/5000), scaleX=1+(1-scaleY).
            auto const scaleY = std::max(kVelocityFloorScaleY,
                1.0 - std::abs(m_filteredVelocityX) / kVelocityDivisor);
            auto const scaleX = 1.0 + (1.0 - scaleY);
            AnimateElementScaleSpring(
                owner,
                frameworkElement,
                scaleX,
                scaleY,
                kSquashDampingRatio,
                kSquashPeriodMs);

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
                    AnimateOpticsToCurrent(brush, from, std::chrono::milliseconds{ kOpticsDurationMs });
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
                if (animate)
                {
                    AnimateElementScaleSpring(
                        owner,
                        frameworkElement,
                        kRestScaleX,
                        kRestScaleY,
                        kBodyDampingRatio,
                        kBodyPeriodMs);
                }
                else
                {
                    SetElementScale(frameworkElement, kRestScaleX, kRestScaleY);
                }
            }
            SetSurfaceElevation(kRestElevation, animate);

            m_coordinateRoot = nullptr;
            m_pointerId = 0;
            m_filteredVelocityX = 0.0;
        }

        void ApplyRestVisual(bool animate)
        {
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = self->template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element) return;
            if (animate)
                AnimateElementScaleSpring(owner, element, kRestScaleX, kRestScaleY, kBodyDampingRatio, kBodyPeriodMs);
            else
                SetElementScale(element, kRestScaleX, kRestScaleY);
            SetSurfaceElevation(kRestElevation, false);
        }

        void ClearForTeardown()
        {
            m_loaded = false;
            m_dragging = false;
            m_coordinateRoot = nullptr;
            m_surface = nullptr;
            m_pointerId = 0;
            m_filteredVelocityX = 0.0;
        }

        Microsoft::UI::Xaml::UIElement m_coordinateRoot{ nullptr };
        Microsoft::UI::Xaml::Controls::Border m_surface{ nullptr };
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
        std::uint32_t m_pointerId{};
        bool m_loaded{};
        bool m_dragging{};
    };
}

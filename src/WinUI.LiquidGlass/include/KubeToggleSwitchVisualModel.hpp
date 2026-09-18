#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>

#include "ChildSurfaceInteraction.hpp"
#include "MotionAnimation.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self>
    class KubeToggleSwitchVisualModel
    {
    public:
        KubeToggleSwitchVisualModel()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const&, auto const&)
            {
                m_loaded = true;
                RefreshVisualModel();
            });
            self->Unloaded([this](auto const&, auto const&) { ClearForTeardown(); });
            self->RegisterPropertyChangedCallback(Self::GlassBrushProperty(), [this](auto const&, auto const&)
            {
                m_pointerField.InvalidateBrush();
                if (m_loaded) RefreshVisualModel();
            });
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Primitives::ToggleButton::IsCheckedProperty(),
                [this](auto const&, auto const&)
                {
                    // ToggleButton may commit IsChecked from its class handler before our
                    // routed PointerReleased handler runs. Always observe the semantic state
                    // change, even while a pointer gesture is still active, otherwise the
                    // visual ratio remains one click behind and teleports on the next press.
                    if (m_loaded) SyncSemanticState(true);
                });

            auto bind = [self](auto routedEvent, Windows::Foundation::IInspectable& storage, auto&& callback)
            {
                using Handler = Microsoft::UI::Xaml::Input::PointerEventHandler;
                storage = winrt::box_value<Handler>({ std::forward<decltype(callback)>(callback) });
                self->AddHandler(routedEvent, storage, true);
            };
            bind(Microsoft::UI::Xaml::UIElement::PointerPressedEvent(), m_pointerPressedHandler,
                [this](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args) { BeginDrag(args); });
            bind(Microsoft::UI::Xaml::UIElement::PointerMovedEvent(), m_pointerMovedHandler,
                [this](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args) { UpdateDrag(args); });
            bind(Microsoft::UI::Xaml::UIElement::PointerReleasedEvent(), m_pointerReleasedHandler,
                [this](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args) { EndDragFromRelease(args); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCaptureLostEvent(), m_pointerCaptureLostHandler,
                [this](auto const&, auto const&) { CancelDrag(true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(), m_pointerCanceledHandler,
                [this](auto const&, auto const&) { CancelDrag(true); });
        }

        void RefreshVisualModel()
        {
            if (!m_loaded) return;
            auto self = static_cast<Self*>(this);
            auto root = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!root) return;

            m_knob = FindNamedDescendant(root, L"SwitchKnob").try_as<Microsoft::UI::Xaml::FrameworkElement>();
            m_surface = FindNamedDescendant(root, L"SwitchKnobSurface").try_as<Microsoft::UI::Xaml::FrameworkElement>();
            auto track = FindNamedDescendant(root, L"Track").try_as<Microsoft::UI::Xaml::Controls::Border>();
            if (!m_knob || !m_surface || !track)
            {
                m_pointerField.Detach(false);
                ClearTrackVisual(false);
                return;
            }

            // Bind the authored material directly to the rendered knob surface. Do not
            // rely on the ToggleButton Background -> TemplateBinding chain here: control
            // state/template precedence can otherwise leave only the rim/shadow visible.
            if (auto surfaceBorder = m_surface.try_as<Microsoft::UI::Xaml::Controls::Border>())
            {
                auto glass = self->GlassBrush();
                surfaceBorder.Background(glass
                    ? glass.as<Microsoft::UI::Xaml::Media::Brush>()
                    : Microsoft::UI::Xaml::Media::Brush{ nullptr });
            }

            if (!m_trackHost || get_abi(m_trackHost) != get_abi(track))
            {
                ClearTrackVisual(true);
                m_trackHost = track;
                BuildTrackVisual();
            }
            m_trackHost.Background(nullptr);
            Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetIsTranslationEnabled(m_knob, true);

            if (!m_initialized)
            {
                m_currentRatio = SemanticRatio(self->IsChecked());
                m_targetRatio = m_currentRatio;
                m_visualRatio = m_currentRatio;
                m_currentScale = kRestScale;
                m_targetScale = kRestScale;
                m_ratioVelocity = 0.0;
                m_scaleVelocity = 0.0;
                m_initialized = true;
                ApplyDynamicVisual();
            }
            else if (!m_dragging)
            {
                SyncSemanticState(false);
            }
            RefreshPointerField();
        }

        bool TryHandleToggle()
        {
            // If native ToggleButton tries to toggle while a real drag is in progress,
            // suppress that click. The drag release will commit the semantic state from
            // the visual ratio. This handles either class-handler ordering (before or
            // after our PointerReleased handler) without a second toggle or a teleport.
            if (m_dragging && m_dragOverrideArmed)
            {
                m_nativeToggleConsumedThisGesture = true;
                return true;
            }
            if (!m_consumeNextToggle) return false;
            m_consumeNextToggle = false;
            return true;
        }

    private:
        static constexpr double kTrackWidth = 160.0;
        static constexpr double kTrackHeight = 67.0;
        static constexpr double kTrackRadius = 33.5;
        static constexpr double kTravelDips = 57.9;
        static constexpr double kOverscrollDamping = 22.0;
        static constexpr double kDragThresholdDips = 4.0;
        using Clock = std::chrono::steady_clock;
        static constexpr double kRestScale = .65;
        static constexpr double kPressedScale = .9;
        // Kube / Motion uses unit-mass springs with these literal coefficients.
        static constexpr double kPositionStiffness = 1000.0;
        static constexpr double kPositionDamping = 80.0;
        static constexpr double kScaleStiffness = 2000.0;
        static constexpr double kScaleDamping = 80.0;
        static constexpr auto kDynamicsInterval = std::chrono::milliseconds{ 16 };

        static double SemanticRatio(Windows::Foundation::IReference<bool> const& value)
        {
            if (!value) return .5;
            return value.Value() ? 1.0 : 0.0;
        }

        double PhysicalRatio(double semanticRatio) const
        {
            auto self = static_cast<Self const*>(this);
            return self->FlowDirection() == Microsoft::UI::Xaml::FlowDirection::RightToLeft
                ? 1.0 - semanticRatio
                : semanticRatio;
        }

        Windows::Foundation::Numerics::float3 TranslationForRatio(double semanticRatio) const
        {
            return { static_cast<float>(PhysicalRatio(semanticRatio) * kTravelDips), 0.0f, 0.0f };
        }

        static uint8_t LerpByte(uint8_t from, uint8_t to, double t)
        {
            return static_cast<uint8_t>(std::lround(
                static_cast<double>(from) +
                (static_cast<double>(to) - static_cast<double>(from)) * std::clamp(t, 0.0, 1.0)));
        }

        static Windows::UI::Color TrackColor(double ratio)
        {
            auto const t = std::clamp(ratio, 0.0, 1.0);
            // CSS #94949F77 -> #3BBF4EEE from Kube Switch.tsx.
            return {
                LerpByte(0x77, 0xee, t),
                LerpByte(0x94, 0x3b, t),
                LerpByte(0x94, 0xbf, t),
                LerpByte(0x9f, 0x4e, t)
            };
        }

        static void StepSpring(
            double target,
            double stiffness,
            double damping,
            double dt,
            double& value,
            double& velocity)
        {
            // Preserve spring velocity across rapid target changes. Substep at 120 Hz
            // so k=2000 remains stable even if the UI thread briefly misses a frame.
            auto const steps = std::max(1, static_cast<int>(std::ceil(dt / (1.0 / 120.0))));
            auto const h = dt / static_cast<double>(steps);
            for (int i = 0; i < steps; ++i)
            {
                auto const acceleration = stiffness * (target - value) - damping * velocity;
                velocity += acceleration * h;
                value += velocity * h;
            }
        }

        void BuildTrackVisual()
        {
            if (!m_trackHost) return;
            auto hostVisual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(m_trackHost);
            auto compositor = hostVisual.Compositor();

            auto geometry = compositor.CreateRoundedRectangleGeometry();
            geometry.Size({ static_cast<float>(kTrackWidth), static_cast<float>(kTrackHeight) });
            geometry.CornerRadius({ static_cast<float>(kTrackRadius), static_cast<float>(kTrackRadius) });

            m_trackBrush = compositor.CreateColorBrush(TrackColor(m_currentRatio));
            auto shape = compositor.CreateSpriteShape(geometry);
            shape.FillBrush(m_trackBrush);

            m_trackVisual = compositor.CreateShapeVisual();
            m_trackVisual.Size({ static_cast<float>(kTrackWidth), static_cast<float>(kTrackHeight) });
            m_trackVisual.Shapes().Append(shape);
            Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetElementChildVisual(m_trackHost, m_trackVisual);
        }

        void ClearTrackVisual(bool detachHost)
        {
            if (detachHost && m_trackHost)
                Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetElementChildVisual(m_trackHost, nullptr);
            m_trackHost = nullptr;
            m_trackVisual = nullptr;
            m_trackBrush = nullptr;
        }

        void ApplyDynamicVisual()
        {
            if (m_knob)
            {
                SetElementTranslation(m_knob, TranslationForRatio(m_currentRatio));
                SetElementScale(m_knob, m_currentScale, m_currentScale);
            }
            if (m_trackBrush)
                m_trackBrush.Color(TrackColor(m_currentRatio));
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
            if (!m_dynamicsTimer.IsRunning())
            {
                m_lastDynamicsTick = Clock::now();
                m_dynamicsTimer.Start();
            }
        }

        void TickDynamics()
        {
            if (!m_loaded)
            {
                if (m_dynamicsTimer) m_dynamicsTimer.Stop();
                return;
            }

            auto const now = Clock::now();
            auto dt = std::chrono::duration<double>(now - m_lastDynamicsTick).count();
            m_lastDynamicsTick = now;
            dt = std::clamp(dt, 1.0 / 240.0, 1.0 / 30.0);

            StepSpring(
                m_targetRatio,
                kPositionStiffness,
                kPositionDamping,
                dt,
                m_currentRatio,
                m_ratioVelocity);
            StepSpring(
                m_targetScale,
                kScaleStiffness,
                kScaleDamping,
                dt,
                m_currentScale,
                m_scaleVelocity);

            ApplyDynamicVisual();

            auto const settled =
                std::abs(m_currentRatio - m_targetRatio) < .0005 &&
                std::abs(m_ratioVelocity) < .005 &&
                std::abs(m_currentScale - m_targetScale) < .0005 &&
                std::abs(m_scaleVelocity) < .005;
            if (settled)
            {
                m_currentRatio = m_targetRatio;
                m_ratioVelocity = 0.0;
                m_currentScale = m_targetScale;
                m_scaleVelocity = 0.0;
                ApplyDynamicVisual();
                if (m_dynamicsTimer) m_dynamicsTimer.Stop();
            }
        }

        void SetRatioTarget(double ratio, bool animate)
        {
            m_visualRatio = ratio;
            m_targetRatio = ratio;

            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!animate || !owner || !MotionAnimationsEnabled(owner))
            {
                m_currentRatio = ratio;
                m_ratioVelocity = 0.0;
                ApplyDynamicVisual();
                return;
            }
            EnsureDynamicsTimer();
        }

        void SetScaleTarget(double scale, bool animate)
        {
            m_targetScale = scale;

            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!animate || !owner || !MotionAnimationsEnabled(owner))
            {
                m_currentScale = scale;
                m_scaleVelocity = 0.0;
                ApplyDynamicVisual();
                return;
            }
            EnsureDynamicsTimer();
        }

        void SyncSemanticState(bool animate)
        {
            if (!m_knob || !m_trackBrush) return;
            auto self = static_cast<Self*>(this);
            SetRatioTarget(SemanticRatio(self->IsChecked()), animate);
            if (!m_dragging)
                SetScaleTarget(kRestScale, animate);
        }

        void BeginDrag(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_loaded || m_dragging) return;
            auto self = static_cast<Self*>(this);
            auto element = self->template try_as<Microsoft::UI::Xaml::UIElement>();
            auto frameworkElement = self->template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!element || !frameworkElement) return;
            if (!m_knob || !m_trackBrush) RefreshVisualModel();
            if (!m_knob || !m_trackBrush) return;

            auto xamlRoot = frameworkElement.XamlRoot();
            m_coordinateRoot = xamlRoot ? xamlRoot.Content() : Microsoft::UI::Xaml::UIElement{ nullptr };
            if (!m_coordinateRoot) m_coordinateRoot = element;
            auto const point = args.GetCurrentPoint(m_coordinateRoot);

            // ButtonBase commonly owns the pointer capture already. A second CapturePointer
            // may return false even though routed move/release events continue to arrive.
            // Do not make visual press/drag activation depend on owning that capture.
            m_ownsCapture = element.CapturePointer(args.Pointer());
            m_pointerId = point.PointerId();
            m_dragStart = point.Position();
            m_baseRatio = SemanticRatio(self->IsChecked());
            m_visualRatio = m_baseRatio;
            m_dragOverrideArmed = false;
            m_nativeToggleConsumedThisGesture = false;
            m_consumeNextToggle = false;
            m_dragging = true;
            SetScaleTarget(kPressedScale, true);
        }

        void UpdateDrag(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_loaded || !m_dragging || !m_coordinateRoot || !m_knob) return;
            auto const point = args.GetCurrentPoint(m_coordinateRoot);
            if (point.PointerId() != m_pointerId) return;
            auto self = static_cast<Self*>(this);
            auto const direction = self->FlowDirection() == Microsoft::UI::Xaml::FlowDirection::RightToLeft ? -1.0 : 1.0;
            auto const delta = static_cast<double>(point.Position().X - m_dragStart.X) * direction;
            if (std::abs(delta) >= kDragThresholdDips) m_dragOverrideArmed = true;

            auto ratio = m_baseRatio + delta / kTravelDips;
            if (ratio < 0.0) ratio /= kOverscrollDamping;
            else if (ratio > 1.0) ratio = 1.0 + (ratio - 1.0) / kOverscrollDamping;
            SetRatioTarget(ratio, true);
        }

        void EndDragFromRelease(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_loaded || !m_dragging) return;
            auto self = static_cast<Self*>(this);
            auto element = self->template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            bool releaseInside = false;
            if (element)
            {
                auto const point = args.GetCurrentPoint(element).Position();
                releaseInside = point.X >= 0.0 && point.X <= element.ActualWidth() &&
                    point.Y >= 0.0 && point.Y <= element.ActualHeight();
            }

            if (m_dragOverrideArmed)
            {
                auto const targetChecked = std::clamp(m_visualRatio, 0.0, 1.0) >= .5;
                auto current = self->IsChecked();
                bool const changed = !current || current.Value() != targetChecked;
                m_dragOverrideArmed = false;
                m_consumeNextToggle = releaseInside && !m_nativeToggleConsumedThisGesture;

                if (changed)
                {
                    // The IsChecked callback owns the settle animation when semantic state changes.
                    self->IsChecked(box_value(targetChecked).as<Windows::Foundation::IReference<bool>>());
                }
                else
                {
                    // No semantic callback will fire; settle the overscrolled ratio explicitly.
                    SetRatioTarget(targetChecked ? 1.0 : 0.0, true);
                }
                FinishPointer(true, false);
                return;
            }

            m_dragOverrideArmed = false;
            m_nativeToggleConsumedThisGesture = false;
            // For a normal click, ToggleButton may toggle before or after this handler.
            // Never settle toward the old state while the release is inside; IsChecked is
            // the sole owner of the position transition. Outside release gets no click, so
            // it must settle back to the existing semantic state here.
            FinishPointer(true, !releaseInside);
        }

        void CancelDrag(bool animate)
        {
            if (!m_loaded || !m_dragging)
            {
                m_dragOverrideArmed = false;
                m_nativeToggleConsumedThisGesture = false;
                return;
            }
            m_dragOverrideArmed = false;
            m_nativeToggleConsumedThisGesture = false;
            m_consumeNextToggle = false;
            m_ownsCapture = false; // capture is already gone on this path
            FinishPointer(animate, true);
        }

        void FinishPointer(bool animate, bool settlePosition)
        {
            if (!m_dragging) return;
            auto self = static_cast<Self*>(this);
            auto element = self->template try_as<Microsoft::UI::Xaml::UIElement>();

            m_dragging = false;
            m_coordinateRoot = nullptr;
            m_pointerId = 0;

            if (settlePosition)
                SetRatioTarget(SemanticRatio(self->IsChecked()), animate);
            SetScaleTarget(kRestScale, animate);

            if (element && m_ownsCapture) element.ReleasePointerCaptures();
            m_ownsCapture = false;
        }

        void RefreshPointerField()
        {
            auto self = static_cast<Self*>(this);
            auto target = m_surface ? m_surface : m_knob;
            auto xamlRoot = self->XamlRoot();
            if (!target || !xamlRoot)
            {
                m_pointerField.Detach(false);
                return;
            }
            auto weak = self->get_weak();
            m_pointerField.Attach(xamlRoot, target, [weak]() -> WinUI::Composition::Hlsl::LiquidGlassBrush
            {
                if (auto owner = weak.get()) return owner->GlassBrush();
                return nullptr;
            });
        }

        void ClearForTeardown()
        {
            m_loaded = false;
            m_dragging = false;
            m_dragOverrideArmed = false;
            m_nativeToggleConsumedThisGesture = false;
            m_consumeNextToggle = false;
            auto self = static_cast<Self*>(this);
            if (m_ownsCapture)
            {
                if (auto element = self->template try_as<Microsoft::UI::Xaml::UIElement>())
                    element.ReleasePointerCaptures();
            }
            m_ownsCapture = false;
            m_pointerField.Detach(false);
            m_coordinateRoot = nullptr;
            m_pointerId = 0;
            if (m_dynamicsTimer)
            {
                m_dynamicsTimer.Stop();
                m_dynamicsTimer = nullptr;
            }
            m_knob = nullptr;
            m_surface = nullptr;
            m_initialized = false;
            ClearTrackVisual(false);
        }

        PointerFieldSurface m_pointerField;
        Microsoft::UI::Xaml::FrameworkElement m_knob{ nullptr };
        Microsoft::UI::Xaml::FrameworkElement m_surface{ nullptr };
        Microsoft::UI::Xaml::Controls::Border m_trackHost{ nullptr };
        Microsoft::UI::Xaml::UIElement m_coordinateRoot{ nullptr };
        Microsoft::UI::Composition::ShapeVisual m_trackVisual{ nullptr };
        Microsoft::UI::Composition::CompositionColorBrush m_trackBrush{ nullptr };
        Microsoft::UI::Dispatching::DispatcherQueueTimer m_dynamicsTimer{ nullptr };
        Windows::Foundation::IInspectable m_pointerPressedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerMovedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerReleasedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCaptureLostHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCanceledHandler{ nullptr };
        Windows::Foundation::Point m_dragStart{};
        Clock::time_point m_lastDynamicsTick{};
        double m_baseRatio{};
        double m_visualRatio{};
        double m_currentRatio{};
        double m_targetRatio{};
        double m_ratioVelocity{};
        double m_currentScale{ kRestScale };
        double m_targetScale{ kRestScale };
        double m_scaleVelocity{};
        std::uint32_t m_pointerId{};
        bool m_initialized{};
        bool m_loaded{};
        bool m_dragging{};
        bool m_dragOverrideArmed{};
        bool m_nativeToggleConsumedThisGesture{};
        bool m_consumeNextToggle{};
        bool m_ownsCapture{};
    };
}

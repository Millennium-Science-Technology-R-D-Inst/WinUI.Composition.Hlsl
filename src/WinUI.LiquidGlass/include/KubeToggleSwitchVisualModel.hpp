#pragma once

#include <algorithm>
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
                    if (m_loaded && !m_dragging) SyncSemanticState(true);
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
            if (!m_knob || !track)
            {
                m_pointerField.Detach(false);
                ClearTrackVisual(false);
                return;
            }

            if (!m_trackHost || get_abi(m_trackHost) != get_abi(track))
            {
                ClearTrackVisual(true);
                m_trackHost = track;
                BuildTrackVisual();
            }
            m_trackHost.Background(nullptr);
            Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetIsTranslationEnabled(m_knob, true);

            if (!m_dragging) SyncSemanticState(false);
            RefreshPointerField();
        }

        bool TryHandleToggle()
        {
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
        static constexpr double kRestScale = .65;
        static constexpr double kPressedScale = .9;
        static constexpr double kPositionDampingRatio = 1.2649110640673518; // k=1000,d=80
        static constexpr double kPositionPeriodMs = 198.69176531592203;
        static constexpr double kScaleDampingRatio = .8944271909999159; // k=2000,d=80
        static constexpr double kScalePeriodMs = 140.49629462081452;

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

        static Microsoft::UI::Composition::ShapeVisual CreateTrackLayer(
            Microsoft::UI::Composition::Compositor const& compositor,
            Windows::UI::Color const& color)
        {
            auto geometry = compositor.CreateRoundedRectangleGeometry();
            geometry.Size({ static_cast<float>(kTrackWidth), static_cast<float>(kTrackHeight) });
            geometry.CornerRadius({ static_cast<float>(kTrackRadius), static_cast<float>(kTrackRadius) });
            auto shape = compositor.CreateSpriteShape(geometry);
            shape.FillBrush(compositor.CreateColorBrush(color));
            auto visual = compositor.CreateShapeVisual();
            visual.Size({ static_cast<float>(kTrackWidth), static_cast<float>(kTrackHeight) });
            visual.Shapes().Append(shape);
            return visual;
        }

        void BuildTrackVisual()
        {
            if (!m_trackHost) return;
            auto hostVisual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(m_trackHost);
            auto compositor = hostVisual.Compositor();
            m_ratio = compositor.CreatePropertySet();
            m_ratio.InsertScalar(L"Value", 0.0f);
            m_rootVisual = compositor.CreateContainerVisual();
            m_rootVisual.Size({ static_cast<float>(kTrackWidth), static_cast<float>(kTrackHeight) });
            m_baseVisual = CreateTrackLayer(compositor, { 0x77, 0x94, 0x94, 0x9f });
            m_checkedVisual = CreateTrackLayer(compositor, { 0xee, 0x3b, 0xbf, 0x4e });
            m_rootVisual.Children().InsertAtBottom(m_baseVisual);
            m_rootVisual.Children().InsertAtTop(m_checkedVisual);

            auto checkedOpacity = compositor.CreateExpressionAnimation(L"ratio.Value");
            checkedOpacity.SetReferenceParameter(L"ratio", m_ratio);
            m_checkedVisual.StartAnimation(L"Opacity", checkedOpacity);
            Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetElementChildVisual(m_trackHost, m_rootVisual);
        }

        void ClearTrackVisual(bool detachHost)
        {
            if (detachHost && m_trackHost)
                Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetElementChildVisual(m_trackHost, nullptr);
            m_trackHost = nullptr;
            m_ratio = nullptr;
            m_rootVisual = nullptr;
            m_baseVisual = nullptr;
            m_checkedVisual = nullptr;
        }

        void SetVisualRatio(double ratio, bool animate)
        {
            if (!m_knob || !m_ratio) return;
            m_visualRatio = ratio;
            auto const trackRatio = static_cast<float>(std::clamp(ratio, 0.0, 1.0));
            m_ratio.StopAnimation(L"Value");

            if (!animate)
            {
                m_ratio.InsertScalar(L"Value", trackRatio);
                SetElementTranslation(m_knob, TranslationForRatio(ratio));
                return;
            }

            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!owner || !MotionAnimationsEnabled(owner))
            {
                m_ratio.InsertScalar(L"Value", trackRatio);
                SetElementTranslation(m_knob, TranslationForRatio(ratio));
                return;
            }

            auto scalarSpring = m_ratio.Compositor().CreateSpringScalarAnimation();
            scalarSpring.FinalValue(box_value(trackRatio).as<Windows::Foundation::IReference<float>>());
            scalarSpring.DampingRatio(static_cast<float>(kPositionDampingRatio));
            scalarSpring.Period(std::chrono::milliseconds{ static_cast<int64_t>(std::lround(kPositionPeriodMs)) });
            m_ratio.StartAnimation(L"Value", scalarSpring);

            Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetIsTranslationEnabled(m_knob, true);
            auto visual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(m_knob);
            auto vectorSpring = visual.Compositor().CreateSpringVector3Animation();
            vectorSpring.FinalValue(box_value(TranslationForRatio(ratio)).as<
                Windows::Foundation::IReference<Windows::Foundation::Numerics::float3>>());
            vectorSpring.DampingRatio(static_cast<float>(kPositionDampingRatio));
            vectorSpring.Period(std::chrono::milliseconds{ static_cast<int64_t>(std::lround(kPositionPeriodMs)) });
            visual.StartAnimation(L"Translation", vectorSpring);
        }

        void AnimateKnobScale(bool pressed)
        {
            if (!m_knob) return;
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!owner) return;
            auto const value = pressed ? kPressedScale : kRestScale;
            AnimateElementScaleSpring(owner, m_knob, value, value, kScaleDampingRatio, kScalePeriodMs);
        }

        void SyncSemanticState(bool animate)
        {
            if (!m_knob || !m_ratio) return;
            auto self = static_cast<Self*>(this);
            SetVisualRatio(SemanticRatio(self->IsChecked()), animate);
            if (!m_dragging)
            {
                if (animate) AnimateKnobScale(false);
                else SetElementScale(m_knob, kRestScale, kRestScale);
            }
        }

        void BeginDrag(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_loaded || m_dragging) return;
            auto self = static_cast<Self*>(this);
            auto element = self->template try_as<Microsoft::UI::Xaml::UIElement>();
            auto frameworkElement = self->template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!element || !frameworkElement) return;
            if (!m_knob || !m_ratio) RefreshVisualModel();
            if (!m_knob || !m_ratio) return;

            auto xamlRoot = frameworkElement.XamlRoot();
            m_coordinateRoot = xamlRoot ? xamlRoot.Content() : Microsoft::UI::Xaml::UIElement{ nullptr };
            if (!m_coordinateRoot) m_coordinateRoot = element;
            auto const point = args.GetCurrentPoint(m_coordinateRoot);
            if (!element.CapturePointer(args.Pointer()))
            {
                m_coordinateRoot = nullptr;
                return;
            }

            m_pointerId = point.PointerId();
            m_dragStart = point.Position();
            m_baseRatio = SemanticRatio(self->IsChecked());
            m_visualRatio = m_baseRatio;
            m_dragOverrideArmed = false;
            m_consumeNextToggle = false;
            m_dragging = true;
            SetVisualRatio(m_baseRatio, false);
            AnimateKnobScale(true);
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
            SetVisualRatio(ratio, false);
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
                if (!current || current.Value() != targetChecked)
                    self->IsChecked(box_value(targetChecked).as<Windows::Foundation::IReference<bool>>());
                m_consumeNextToggle = releaseInside;
                m_dragOverrideArmed = false;
                FinishPointer(true, true);
                return;
            }

            m_dragOverrideArmed = false;
            // A normal click is committed by ToggleButton::OnToggle after release. Do not
            // spring toward the old semantic state first; that old-state settle caused the
            // visible ON -> OFF teleport/restart.
            FinishPointer(true, !releaseInside);
        }

        void CancelDrag(bool animate)
        {
            if (!m_loaded || !m_dragging)
            {
                m_dragOverrideArmed = false;
                return;
            }
            m_dragOverrideArmed = false;
            m_consumeNextToggle = false;
            FinishPointer(animate, true);
        }

        void FinishPointer(bool animate, bool settlePosition)
        {
            if (!m_dragging) return;
            auto self = static_cast<Self*>(this);
            auto element = self->template try_as<Microsoft::UI::Xaml::UIElement>();

            // Mark inactive before releasing capture. CaptureLost can fire synchronously.
            m_dragging = false;
            m_coordinateRoot = nullptr;
            m_pointerId = 0;

            if (settlePosition)
                SetVisualRatio(SemanticRatio(self->IsChecked()), animate);
            if (animate) AnimateKnobScale(false);
            else if (m_knob) SetElementScale(m_knob, kRestScale, kRestScale);

            if (element) element.ReleasePointerCaptures();
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
            m_consumeNextToggle = false;
            auto self = static_cast<Self*>(this);
            if (auto element = self->template try_as<Microsoft::UI::Xaml::UIElement>())
                element.ReleasePointerCaptures();
            m_pointerField.Detach(false);
            m_coordinateRoot = nullptr;
            m_pointerId = 0;
            m_knob = nullptr;
            m_surface = nullptr;
            ClearTrackVisual(false);
        }

        PointerFieldSurface m_pointerField;
        Microsoft::UI::Xaml::FrameworkElement m_knob{ nullptr };
        Microsoft::UI::Xaml::FrameworkElement m_surface{ nullptr };
        Microsoft::UI::Xaml::Controls::Border m_trackHost{ nullptr };
        Microsoft::UI::Xaml::UIElement m_coordinateRoot{ nullptr };
        Microsoft::UI::Composition::CompositionPropertySet m_ratio{ nullptr };
        Microsoft::UI::Composition::ContainerVisual m_rootVisual{ nullptr };
        Microsoft::UI::Composition::ShapeVisual m_baseVisual{ nullptr };
        Microsoft::UI::Composition::ShapeVisual m_checkedVisual{ nullptr };
        Windows::Foundation::IInspectable m_pointerPressedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerMovedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerReleasedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCaptureLostHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCanceledHandler{ nullptr };
        Windows::Foundation::Point m_dragStart{};
        double m_baseRatio{};
        double m_visualRatio{};
        std::uint32_t m_pointerId{};
        bool m_loaded{};
        bool m_dragging{};
        bool m_dragOverrideArmed{};
        bool m_consumeNextToggle{};
    };
}

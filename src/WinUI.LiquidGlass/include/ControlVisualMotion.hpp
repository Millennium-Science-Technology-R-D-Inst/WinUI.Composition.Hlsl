#pragma once

#include "ChildSurfaceInteraction.hpp"
#include "MotionAnimation.hpp"
#include "PressOpticsHelper.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self>
    class SliderSurfaceVisualHelper
    {
    public:
        SliderSurfaceVisualHelper()
        {
            auto self = static_cast<Self*>(this);
            ConfigureResources();

            self->Loaded([this](auto const&, auto const&)
            {
                RefreshVisual();
                QueuePostLoadedRefresh();
            });
            self->Unloaded([this](auto const&, auto const&)
            {
                m_pointerField.Detach(false);
                m_surface = nullptr;
                m_thumb = nullptr;
            });
            self->RegisterPropertyChangedCallback(Self::GlassBrushProperty(), [this](auto const&, auto const&)
            {
                ConfigureResources();
                m_pointerField.InvalidateBrush();
                RefreshVisual();
            });
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Slider::OrientationProperty(),
                [this](auto const&, auto const&) { RefreshVisual(); });
        }

        void RefreshVisual()
        {
            auto self = static_cast<Self*>(this);
            auto root = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!root) return;

            auto const horizontal = self->Orientation() == Microsoft::UI::Xaml::Controls::Orientation::Horizontal;
            auto const thumbName = horizontal ? L"HorizontalThumb" : L"VerticalThumb";
            auto thumb = FindNamedDescendant(root, thumbName).try_as<Microsoft::UI::Xaml::Controls::Primitives::Thumb>();
            if (!thumb) thumb = FindFirstDescendant<Microsoft::UI::Xaml::Controls::Primitives::Thumb>(root);
            if (!thumb) return;

            // The native Slider uses Thumb.ActualWidth/ActualHeight in both UpdateTrackLayout
            // and pointer delta -> value conversion. Keep that semantic footprint at the
            // WinUI contract size; the 90x60 glass lens is a visual child only.
            thumb.Width(18.0);
            thumb.Height(18.0);
            thumb.Margin({ 0.0, 0.0, 0.0, 0.0 });
            m_thumb = thumb;

            auto glass = self->GlassBrush();
            if (glass)
            {
                glass.CornerRadius(30.0);
                glass.BezelWidth(16.0);
            }

            auto surface = FindFirstDescendant<Microsoft::UI::Xaml::Controls::Border>(thumb);
            if (surface)
            {
                surface.Width(horizontal ? 90.0 : 60.0);
                surface.Height(horizontal ? 60.0 : 90.0);
                surface.Margin({ 0.0, 0.0, 0.0, 0.0 });
                surface.HorizontalAlignment(Microsoft::UI::Xaml::HorizontalAlignment::Center);
                surface.VerticalAlignment(Microsoft::UI::Xaml::VerticalAlignment::Center);
                surface.CornerRadius({ 30.0, 30.0, 30.0, 30.0 });
                surface.Background(glass
                    ? glass.as<Microsoft::UI::Xaml::Media::Brush>()
                    : Microsoft::UI::Xaml::Media::Brush{ nullptr });
                surface.BorderBrush(SolidBrush(0x33, 0xff, 0xff, 0xff));
                surface.BorderThickness({ 1.0, 1.0, 1.0, 1.0 });
                m_surface = surface;
            }
            else
            {
                m_surface = nullptr;
            }

            if (auto inner = FindNamedDescendant(thumb, L"SliderInnerThumb").try_as<Microsoft::UI::Xaml::Shapes::Ellipse>())
                inner.Visibility(Microsoft::UI::Xaml::Visibility::Collapsed);

            // Do not draw a second progress model. WinUI already sizes DecreaseRect from the
            // same RangeBase/value geometry used by keyboard, pointer and UIA interactions.
            auto const trackName = horizontal ? L"HorizontalTrackRect" : L"VerticalTrackRect";
            auto const decreaseName = horizontal ? L"HorizontalDecreaseRect" : L"VerticalDecreaseRect";
            if (auto track = FindNamedDescendant(root, trackName).try_as<Microsoft::UI::Xaml::Shapes::Rectangle>())
                track.Fill(SolidBrush(0x66, 0x89, 0x89, 0x8f));
            if (auto decrease = FindNamedDescendant(root, decreaseName).try_as<Microsoft::UI::Xaml::Shapes::Rectangle>())
            {
                decrease.Opacity(1.0);
                decrease.Fill(SolidBrush(0xff, 0x03, 0x77, 0xf7));
            }

            RefreshPointerFieldTarget();
        }

    private:
        static Microsoft::UI::Xaml::Media::SolidColorBrush SolidBrush(
            uint8_t alpha,
            uint8_t red,
            uint8_t green,
            uint8_t blue)
        {
            Microsoft::UI::Xaml::Media::SolidColorBrush brush;
            brush.Color({ alpha, red, green, blue });
            return brush;
        }

        void QueuePostLoadedRefresh()
        {
            auto self = static_cast<Self*>(this);
            auto queue = self->DispatcherQueue();
            if (!queue) return;
            auto weak = self->get_weak();
            queue.TryEnqueue([weak]
            {
                if (auto owner = weak.get())
                {
                    auto helper = static_cast<SliderSurfaceVisualHelper<Self>*>(owner.get());
                    helper->RefreshVisual();
                }
            });
        }

        void ConfigureResources()
        {
            auto self = static_cast<Self*>(this);
            auto const track = SolidBrush(0x66, 0x89, 0x89, 0x8f);
            auto const value = SolidBrush(0xff, 0x03, 0x77, 0xf7);
            auto const transparent = SolidBrush(0x00, 0x00, 0x00, 0x00);
            auto const stroke = SolidBrush(0x33, 0xff, 0xff, 0xff);

            self->Background(track);
            self->Foreground(value);
            self->CornerRadius({ 7.0, 7.0, 7.0, 7.0 });

            auto resources = self->Resources();
            auto insert = [&resources](wchar_t const* key, Windows::Foundation::IInspectable const& resourceValue)
            {
                if (resourceValue) resources.Insert(box_value(hstring{ key }), resourceValue);
            };

            insert(L"SliderThumbBackground", transparent);
            insert(L"SliderThumbBackgroundPointerOver", transparent);
            insert(L"SliderThumbBackgroundPressed", transparent);
            insert(L"SliderThumbBackgroundDisabled", transparent);
            insert(L"SliderOuterThumbBackground", transparent);
            insert(L"SliderThumbBorderBrush", stroke);
            insert(L"SliderTrackFill", track);
            insert(L"SliderTrackFillPointerOver", track);
            insert(L"SliderTrackFillPressed", track);
            insert(L"SliderTrackFillDisabled", track);
            insert(L"SliderTrackValueFill", value);
            insert(L"SliderTrackValueFillPointerOver", value);
            insert(L"SliderTrackValueFillPressed", value);
            insert(L"SliderTrackValueFillDisabled", value);
            resources.Insert(box_value(L"SliderTrackThemeHeight"), box_value(14.0));
            resources.Insert(box_value(L"SliderHorizontalHeight"), box_value(60.0));
            resources.Insert(box_value(L"SliderVerticalWidth"), box_value(60.0));
            resources.Insert(box_value(L"SliderHorizontalThumbWidth"), box_value(18.0));
            resources.Insert(box_value(L"SliderHorizontalThumbHeight"), box_value(18.0));
            resources.Insert(box_value(L"SliderVerticalThumbWidth"), box_value(18.0));
            resources.Insert(box_value(L"SliderVerticalThumbHeight"), box_value(18.0));
        }

        void RefreshPointerFieldTarget()
        {
            auto self = static_cast<Self*>(this);
            auto target = m_surface.try_as<Microsoft::UI::Xaml::FrameworkElement>();
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

        PointerFieldSurface m_pointerField;
        Microsoft::UI::Xaml::Controls::Primitives::Thumb m_thumb{ nullptr };
        Microsoft::UI::Xaml::Controls::Border m_surface{ nullptr };
    };

    template<typename Self>
    class SearchMotionHelper
    {
    public:
        SearchMotionHelper()
        {
            auto self = static_cast<Self*>(this);
            // Search needs immediate input acknowledgement. A fixed-duration keyframe honors
            // MotionDuration; the generic spring path intentionally ignores that duration.
            self->SetValue(implementation::LiquidGlassInteraction::UseSpringMotionProperty(), box_value(false));

            self->Loaded([this](auto const& sender, auto const&) { Recompute(sender, false); });
            self->Unloaded([this](auto const&, auto const&)
            {
                // Teardown is no-write: the compositor may already have closed the effect.
                m_baseline = {};
                m_focused = false;
                m_pressed = false;
            });
            self->GotFocus([this](auto const& sender, auto const&) { m_focused = true; Recompute(sender, true); });
            self->LostFocus([this](auto const& sender, auto const&) { m_focused = false; Recompute(sender, true); });

            auto bind = [self](auto routedEvent, Windows::Foundation::IInspectable& storage, auto&& callback)
            {
                using Handler = Microsoft::UI::Xaml::Input::PointerEventHandler;
                storage = winrt::box_value<Handler>({ std::forward<decltype(callback)>(callback) });
                self->AddHandler(routedEvent, storage, true);
            };
            bind(Microsoft::UI::Xaml::UIElement::PointerPressedEvent(), m_pointerPressedHandler,
                [this](auto const& sender, auto const&) { m_pressed = true; Recompute(sender, true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerReleasedEvent(), m_pointerReleasedHandler,
                [this](auto const& sender, auto const&) { m_pressed = false; Recompute(sender, true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCaptureLostEvent(), m_pointerCaptureLostHandler,
                [this](auto const& sender, auto const&) { m_pressed = false; Recompute(sender, true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(), m_pointerCanceledHandler,
                [this](auto const& sender, auto const&) { m_pressed = false; Recompute(sender, true); });

            self->RegisterPropertyChangedCallback(Self::GlassBrushProperty(),
                [this](Microsoft::UI::Xaml::DependencyObject const& sender, auto const&)
                {
                    if (m_baseline.active) RestoreOptics(m_baseline);
                    m_baseline = {};
                    Recompute(sender, false);
                });
        }

    private:
        template<typename Sender>
        void Recompute(Sender const& sender, bool animate)
        {
            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element) return;
            auto self = static_cast<Self*>(this);
            auto brush = self->GlassBrush();
            if (!brush) return;

            if (m_baseline.active && get_abi(m_baseline.brush) != get_abi(brush))
            {
                RestoreOptics(m_baseline);
                m_baseline = {};
            }

            auto const active = m_focused || m_pressed;
            OpticsSnapshot from;
            CaptureOptics(brush, from);
            if (!active)
            {
                if (m_baseline.active)
                {
                    RestoreOptics(m_baseline);
                    if (animate) AnimateOpticsTransition(owner, brush, from);
                    m_baseline = {};
                }
            }
            else
            {
                if (!m_baseline.active) CaptureOptics(brush, m_baseline);
                else ApplySnapshot(m_baseline);
                auto const focusedBoost = m_focused
                    ? implementation::LiquidGlassInteraction::GetFocusedTintBoost(owner)
                    : 0.0;
                auto const pressedBoost = m_pressed
                    ? implementation::LiquidGlassInteraction::GetPressedTintBoost(owner)
                    : 0.0;
                brush.TintOpacity(std::clamp(
                    m_baseline.tintOpacity + std::max(focusedBoost, pressedBoost),
                    0.0,
                    1.0));
                if (animate) AnimateOpticsTransition(owner, brush, from);
            }

            auto scale = m_focused
                ? implementation::LiquidGlassInteraction::GetFocusedScale(owner)
                : implementation::LiquidGlassInteraction::GetRestScale(owner);
            if (m_pressed) scale *= .99;
            scale = std::clamp(scale, .25, 4.0);
            if (animate)
            {
                AnimateElementScale(
                    owner,
                    element,
                    scale,
                    scale,
                    implementation::LiquidGlassInteraction::GetMotionDuration(owner));
            }
            else
            {
                SetElementScale(element, scale, scale);
            }
        }

        OpticsSnapshot m_baseline;
        Windows::Foundation::IInspectable m_pointerPressedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerReleasedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCaptureLostHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCanceledHandler{ nullptr };
        bool m_focused{};
        bool m_pressed{};
    };

    template<typename Self>
    class ToggleSwitchVisualModel
    {
    public:
        ToggleSwitchVisualModel()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const&, auto const&) { RefreshVisualModel(); });
            self->Unloaded([this](auto const&, auto const&) { ClearForTeardown(); });
            self->RegisterPropertyChangedCallback(Self::GlassBrushProperty(), [this](auto const&, auto const&)
            {
                m_pointerField.InvalidateBrush();
                RefreshVisualModel();
            });
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Primitives::ToggleButton::IsCheckedProperty(),
                [this](auto const&, auto const&) { if (!m_dragging) SyncSemanticState(true); });

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
            auto self = static_cast<Self*>(this);
            auto root = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!root) return;

            m_knob = FindNamedDescendant(root, L"SwitchKnob").try_as<Microsoft::UI::Xaml::FrameworkElement>();
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

            auto xamlRoot = self->XamlRoot();
            if (!xamlRoot)
            {
                m_pointerField.Detach(false);
                return;
            }
            auto weak = self->get_weak();
            m_pointerField.Attach(xamlRoot, m_knob, [weak]() -> WinUI::Composition::Hlsl::LiquidGlassBrush
            {
                if (auto owner = weak.get()) return owner->GlassBrush();
                return nullptr;
            });
        }

        bool TryHandleToggle()
        {
            if (!m_dragOverrideArmed) return false;
            auto self = static_cast<Self*>(this);
            auto const targetChecked = std::clamp(m_visualRatio, 0.0, 1.0) >= 0.5;
            auto current = self->IsChecked();
            m_dragOverrideArmed = false;
            if (!current || current.Value() != targetChecked)
                self->IsChecked(box_value(targetChecked).as<Windows::Foundation::IReference<bool>>());
            return true;
        }

    private:
        static constexpr double kTrackWidth = 160.0;
        static constexpr double kTrackHeight = 67.0;
        static constexpr double kTrackRadius = 33.5;
        static constexpr double kTravelDips = 57.9;
        static constexpr double kOverscrollDamping = 22.0;
        static constexpr double kDragThresholdDips = 4.0;

        static double SemanticRatio(Windows::Foundation::IReference<bool> const& value)
        {
            if (!value) return 0.5;
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
            m_progress = compositor.CreatePropertySet();
            m_progress.InsertScalar(L"Ratio", 0.0f);
            m_rootVisual = compositor.CreateContainerVisual();
            m_rootVisual.Size({ static_cast<float>(kTrackWidth), static_cast<float>(kTrackHeight) });
            m_baseVisual = CreateTrackLayer(compositor, { 0xff, 0x94, 0x94, 0x9f });
            m_checkedVisual = CreateTrackLayer(compositor, { 0xff, 0x3b, 0xbf, 0x4e });
            m_rootVisual.Children().InsertAtBottom(m_baseVisual);
            m_rootVisual.Children().InsertAtTop(m_checkedVisual);

            auto alphaExpression = compositor.CreateExpressionAnimation(
                L"0.466666667 + (0.933333333 - 0.466666667) * progress.Ratio");
            alphaExpression.SetReferenceParameter(L"progress", m_progress);
            m_rootVisual.StartAnimation(L"Opacity", alphaExpression);
            auto checkedExpression = compositor.CreateExpressionAnimation(L"progress.Ratio");
            checkedExpression.SetReferenceParameter(L"progress", m_progress);
            m_checkedVisual.StartAnimation(L"Opacity", checkedExpression);
            Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetElementChildVisual(m_trackHost, m_rootVisual);
        }

        void ClearTrackVisual(bool detachHost)
        {
            if (detachHost && m_trackHost)
                Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetElementChildVisual(m_trackHost, nullptr);
            m_trackHost = nullptr;
            m_progress = nullptr;
            m_rootVisual = nullptr;
            m_baseVisual = nullptr;
            m_checkedVisual = nullptr;
        }

        void SetTrackRatio(double ratio, bool animate)
        {
            if (!m_progress) return;
            ratio = std::clamp(ratio, 0.0, 1.0);
            m_progress.StopAnimation(L"Ratio");
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!animate || !owner || !MotionAnimationsEnabled(owner))
            {
                m_progress.InsertScalar(L"Ratio", static_cast<float>(ratio));
                return;
            }

            if (implementation::LiquidGlassInteraction::GetUseSpringMotion(owner))
            {
                auto animation = m_progress.Compositor().CreateSpringScalarAnimation();
                animation.FinalValue(box_value(static_cast<float>(ratio)).as<Windows::Foundation::IReference<float>>());
                animation.DampingRatio(static_cast<float>(std::clamp(
                    implementation::LiquidGlassInteraction::GetSpringDampingRatio(owner), .05, 3.0)));
                animation.Period(std::chrono::milliseconds{
                    static_cast<int64_t>(std::lround(std::clamp(
                        implementation::LiquidGlassInteraction::GetSpringPeriod(owner), 16.0, 2000.0))) });
                m_progress.StartAnimation(L"Ratio", animation);
                return;
            }

            auto animation = m_progress.Compositor().CreateScalarKeyFrameAnimation();
            animation.InsertKeyFrame(1.0f, static_cast<float>(ratio));
            animation.Duration(std::chrono::milliseconds{
                static_cast<int64_t>(std::lround(std::clamp(
                    implementation::LiquidGlassInteraction::GetMotionDuration(owner), 0.0, 2000.0))) });
            m_progress.StartAnimation(L"Ratio", animation);
        }

        void SyncSemanticState(bool animate)
        {
            if (!m_knob || !m_progress) return;
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!owner) return;
            auto const ratio = SemanticRatio(self->IsChecked());
            auto const target = TranslationForRatio(ratio);
            if (animate)
            {
                AnimateElementTranslation(
                    owner,
                    m_knob,
                    target,
                    implementation::LiquidGlassInteraction::GetMotionDuration(owner));
                SetTrackRatio(ratio, true);
            }
            else
            {
                SetElementTranslation(m_knob, target);
                SetTrackRatio(ratio, false);
            }

            auto const restScale = std::clamp(
                implementation::LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
            if (animate)
                AnimateElementScale(owner, m_knob, restScale, implementation::LiquidGlassInteraction::GetMotionDuration(owner));
            else
                SetElementScale(m_knob, restScale, restScale);
        }

        void BeginDrag(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (m_dragging) return;
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = self->template try_as<Microsoft::UI::Xaml::UIElement>();
            auto frameworkElement = self->template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element || !frameworkElement) return;
            if (!m_knob || !m_progress) RefreshVisualModel();
            if (!m_knob || !m_progress) return;

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
            m_dragging = true;
            SetElementTranslation(m_knob, TranslationForRatio(m_baseRatio));
            SetTrackRatio(m_baseRatio, false);

            auto const pressedScale = std::clamp(
                implementation::LiquidGlassInteraction::GetPressedScale(owner), .25, 4.0);
            AnimateElementScale(owner, m_knob, pressedScale, implementation::LiquidGlassInteraction::GetMotionDuration(owner));
        }

        void UpdateDrag(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_dragging || !m_coordinateRoot || !m_knob) return;
            auto const point = args.GetCurrentPoint(m_coordinateRoot);
            if (point.PointerId() != m_pointerId) return;
            auto self = static_cast<Self*>(this);
            auto const direction = self->FlowDirection() == Microsoft::UI::Xaml::FlowDirection::RightToLeft ? -1.0 : 1.0;
            auto const delta = static_cast<double>(point.Position().X - m_dragStart.X) * direction;
            if (std::abs(delta) >= kDragThresholdDips) m_dragOverrideArmed = true;

            auto ratio = m_baseRatio + delta / kTravelDips;
            if (ratio < 0.0) ratio /= kOverscrollDamping;
            else if (ratio > 1.0) ratio = 1.0 + (ratio - 1.0) / kOverscrollDamping;
            m_visualRatio = ratio;
            SetElementTranslation(m_knob, TranslationForRatio(ratio));
            SetTrackRatio(ratio, false);
        }

        void EndDragFromRelease(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_dragging) return;
            auto self = static_cast<Self*>(this);
            auto element = self->template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            bool releaseInside = false;
            if (element)
            {
                auto const point = args.GetCurrentPoint(element).Position();
                releaseInside = point.X >= 0.0 && point.X <= element.ActualWidth() &&
                    point.Y >= 0.0 && point.Y <= element.ActualHeight();
            }

            // Inside releases are left to ButtonBase::OnClick -> OnToggle -> Click. Only a
            // captured release outside has to commit manually because no native click follows.
            if (m_dragOverrideArmed && !releaseInside)
            {
                auto const targetChecked = std::clamp(m_visualRatio, 0.0, 1.0) >= 0.5;
                auto current = self->IsChecked();
                m_dragOverrideArmed = false;
                if (!current || current.Value() != targetChecked)
                    self->IsChecked(box_value(targetChecked).as<Windows::Foundation::IReference<bool>>());
            }
            EndDrag(true, !releaseInside);
        }

        void CancelDrag(bool animate)
        {
            if (!m_dragging)
            {
                m_dragOverrideArmed = false;
                return;
            }
            m_dragOverrideArmed = false;
            EndDrag(animate, true);
        }

        void EndDrag(bool animate, bool clearOverride)
        {
            if (!m_dragging) return;
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto const semanticRatio = SemanticRatio(self->IsChecked());
            auto targetRatio = m_dragOverrideArmed
                ? (std::clamp(m_visualRatio, 0.0, 1.0) >= 0.5 ? 1.0 : 0.0)
                : semanticRatio;
            if (clearOverride)
            {
                m_dragOverrideArmed = false;
                targetRatio = semanticRatio;
            }

            if (m_knob && owner)
            {
                auto const target = TranslationForRatio(targetRatio);
                if (animate)
                    AnimateElementTranslation(owner, m_knob, target, implementation::LiquidGlassInteraction::GetMotionDuration(owner));
                else
                    SetElementTranslation(m_knob, target);

                auto const restScale = std::clamp(
                    implementation::LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
                if (animate)
                    AnimateElementScale(owner, m_knob, restScale, implementation::LiquidGlassInteraction::GetMotionDuration(owner));
                else
                    SetElementScale(m_knob, restScale, restScale);
            }
            SetTrackRatio(targetRatio, animate);

            if (auto element = self->template try_as<Microsoft::UI::Xaml::UIElement>())
                element.ReleasePointerCaptures();
            m_coordinateRoot = nullptr;
            m_pointerId = 0;
            m_dragging = false;
        }

        void ClearForTeardown()
        {
            auto self = static_cast<Self*>(this);
            if (auto element = self->template try_as<Microsoft::UI::Xaml::UIElement>())
                element.ReleasePointerCaptures();
            m_pointerField.Detach(false);
            m_coordinateRoot = nullptr;
            m_pointerId = 0;
            m_dragging = false;
            m_dragOverrideArmed = false;
            m_knob = nullptr;
            // Do not call SetElementChildVisual during XAML/compositor teardown.
            ClearTrackVisual(false);
        }

        PointerFieldSurface m_pointerField;
        Microsoft::UI::Xaml::FrameworkElement m_knob{ nullptr };
        Microsoft::UI::Xaml::Controls::Border m_trackHost{ nullptr };
        Microsoft::UI::Xaml::UIElement m_coordinateRoot{ nullptr };
        Microsoft::UI::Composition::CompositionPropertySet m_progress{ nullptr };
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
        bool m_dragging{};
        bool m_dragOverrideArmed{};
    };
}

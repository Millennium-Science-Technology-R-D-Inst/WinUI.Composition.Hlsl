#pragma once

#include "ChildSurfaceInteraction.hpp"
#include "MotionAnimation.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    // Composition renderer for the 160x67 switch track. The semantic toggle state remains
    // owned by ToggleSwitchInteractionHelper/ToggleButton; this helper only predicts and
    // animates the visual color state while a pointer is down.
    template<typename Self>
    class SwitchTrackVisualHelper
    {
    public:
        SwitchTrackVisualHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const&, auto const&) { RefreshTrackTarget(); });
            self->Unloaded([this](auto const&, auto const&) { ResetPointerTracking(); DetachTrackVisual(); });
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Primitives::ToggleButton::IsCheckedProperty(),
                [this](auto const&, auto const&) { if (!m_pointerDown) ApplySemanticState(true); });

            auto bind = [self](auto routedEvent, Windows::Foundation::IInspectable& storage, auto&& callback)
            {
                using Handler = Microsoft::UI::Xaml::Input::PointerEventHandler;
                storage = winrt::box_value<Handler>({ std::forward<decltype(callback)>(callback) });
                self->AddHandler(routedEvent, storage, true);
            };
            bind(Microsoft::UI::Xaml::UIElement::PointerPressedEvent(), m_pointerPressedHandler,
                [this](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args) { BeginPointerInteraction(args); });
            bind(Microsoft::UI::Xaml::UIElement::PointerMovedEvent(), m_pointerMovedHandler,
                [this](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args) { UpdatePointerInteraction(args); });
            bind(Microsoft::UI::Xaml::UIElement::PointerReleasedEvent(), m_pointerReleasedHandler,
                [this](auto const&, auto const&) { EndPointerInteraction(true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCaptureLostEvent(), m_pointerCaptureLostHandler,
                [this](auto const&, auto const&) { EndPointerInteraction(true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(), m_pointerCanceledHandler,
                [this](auto const&, auto const&) { EndPointerInteraction(true); });
        }

        void RefreshTrackTarget()
        {
            auto self = static_cast<Self*>(this);
            auto root = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto track = root
                ? FindNamedDescendant(root, L"Track").try_as<Microsoft::UI::Xaml::Controls::Border>()
                : Microsoft::UI::Xaml::Controls::Border{ nullptr };
            if (!track)
            {
                DetachTrackVisual();
                return;
            }

            if (!m_trackHost || get_abi(m_trackHost) != get_abi(track))
            {
                DetachTrackVisual();
                m_trackHost = track;
                BuildTrackVisual();
            }
            m_trackHost.Background(nullptr);
            if (!m_pointerDown) SetTrackRatio(SemanticRatio(self->IsChecked()), false);
        }

    private:
        static constexpr double kTrackWidth = 160.0;
        static constexpr double kTrackHeight = 67.0;
        static constexpr double kTrackRadius = 33.5;
        static constexpr double kTravelDips = 57.9;
        static constexpr double kOverscrollDamping = 22.0;

        static double SemanticRatio(Windows::Foundation::IReference<bool> const& value)
        {
            if (!value) return 0.5;
            return value.Value() ? 1.0 : 0.0;
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

        void DetachTrackVisual()
        {
            if (m_trackHost)
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

        void ApplySemanticState(bool animate)
        {
            if (!m_progress) RefreshTrackTarget();
            auto self = static_cast<Self*>(this);
            SetTrackRatio(SemanticRatio(self->IsChecked()), animate);
        }

        void BeginPointerInteraction(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (m_pointerDown) return;
            auto self = static_cast<Self*>(this);
            auto element = self->template try_as<Microsoft::UI::Xaml::UIElement>();
            auto frameworkElement = self->template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!element || !frameworkElement) return;
            if (!m_progress) RefreshTrackTarget();
            if (!m_progress) return;

            auto xamlRoot = frameworkElement.XamlRoot();
            m_coordinateRoot = xamlRoot ? xamlRoot.Content() : Microsoft::UI::Xaml::UIElement{ nullptr };
            if (!m_coordinateRoot) m_coordinateRoot = element;
            auto const point = args.GetCurrentPoint(m_coordinateRoot);
            m_pointerId = point.PointerId();
            m_start = point.Position();
            m_baseRatio = SemanticRatio(self->IsChecked());
            m_consideredChecked = m_baseRatio >= 0.5;
            m_pointerDown = true;
            SetTrackRatio(m_consideredChecked ? 1.0 : 0.0, false);
        }

        void UpdatePointerInteraction(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_pointerDown || !m_coordinateRoot) return;
            auto const point = args.GetCurrentPoint(m_coordinateRoot);
            if (point.PointerId() != m_pointerId) return;
            auto self = static_cast<Self*>(this);
            auto const direction = self->FlowDirection() == Microsoft::UI::Xaml::FlowDirection::RightToLeft ? -1.0 : 1.0;
            auto const delta = static_cast<double>(point.Position().X - m_start.X) * direction;
            auto ratio = m_baseRatio + delta / kTravelDips;
            if (ratio < 0.0) ratio /= kOverscrollDamping;
            else if (ratio > 1.0) ratio = 1.0 + (ratio - 1.0) / kOverscrollDamping;
            auto const consideredChecked = ratio > 0.5;
            if (consideredChecked != m_consideredChecked)
            {
                m_consideredChecked = consideredChecked;
                SetTrackRatio(consideredChecked ? 1.0 : 0.0, true);
            }
        }

        void EndPointerInteraction(bool animate)
        {
            if (!m_pointerDown)
            {
                if (animate) ApplySemanticState(true);
                return;
            }
            ResetPointerTracking();
            ApplySemanticState(animate);
        }

        void ResetPointerTracking()
        {
            m_pointerDown = false;
            m_pointerId = 0;
            m_coordinateRoot = nullptr;
        }

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
        Windows::Foundation::Point m_start{};
        double m_baseRatio{};
        uint32_t m_pointerId{};
        bool m_pointerDown{};
        bool m_consideredChecked{};
    };
}

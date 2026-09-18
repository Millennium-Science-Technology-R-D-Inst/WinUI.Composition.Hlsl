#pragma once

namespace winrt::WinUI::LiquidGlass::detail
{
    inline bool MotionAnimationsEnabled(
        Microsoft::UI::Xaml::DependencyObject const& owner)
    {
        if (!owner) return false;
        if (!implementation::LiquidGlassInteraction::GetRespectSystemAnimations(owner))
            return true;

        try
        {
            return Windows::UI::ViewManagement::UISettings{}.AnimationsEnabled();
        }
        catch (...)
        {
            // Do not make interaction dependent on UISettings activation. If the
            // platform query is unavailable, preserve the app-requested motion.
            return true;
        }
    }

    inline Microsoft::UI::Composition::CubicBezierEasingFunction CreateMotionEasing(
        Microsoft::UI::Composition::Compositor const& compositor)
    {
        return compositor.CreateCubicBezierEasingFunction(
            Windows::Foundation::Numerics::float2{ .20f, 0.0f },
            Windows::Foundation::Numerics::float2{ 0.0f, 1.0f });
    }

    inline Windows::Foundation::Numerics::float3 ResolveScaleCenter(
        Microsoft::UI::Xaml::FrameworkElement const& element)
    {
        // Runtime-created visuals (for example the Slider's sibling glass lens) can be
        // scaled before their first arrange pass. In that window ActualWidth/Height are
        // still zero even when explicit Width/Height are already authored. Falling back
        // to the authored dimensions keeps the initial scale centered instead of scaling
        // around the top-left corner and visibly shifting the control.
        auto width = element.ActualWidth();
        auto height = element.ActualHeight();

        if (!(width > 0.0))
        {
            auto const authoredWidth = element.Width();
            if (authoredWidth > 0.0) width = authoredWidth;
        }
        if (!(height > 0.0))
        {
            auto const authoredHeight = element.Height();
            if (authoredHeight > 0.0) height = authoredHeight;
        }

        return {
            static_cast<float>(std::max(0.0, width) * .5),
            static_cast<float>(std::max(0.0, height) * .5),
            0.0f
        };
    }

    inline void SetElementScale(
        Microsoft::UI::Xaml::FrameworkElement const& element,
        double x,
        double y)
    {
        if (!element) return;

        auto visual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(element);
        visual.CenterPoint(ResolveScaleCenter(element));
        visual.StopAnimation(L"Scale");
        visual.Scale({
            static_cast<float>(std::clamp(x, .25, 4.0)),
            static_cast<float>(std::clamp(y, .25, 4.0)),
            1.0f });
    }

    inline void AnimateElementScaleSpring(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        Microsoft::UI::Xaml::FrameworkElement const& element,
        double x,
        double y,
        double dampingRatio,
        double periodMs)
    {
        if (!owner || !element) return;

        x = std::clamp(x, .25, 4.0);
        y = std::clamp(y, .25, 4.0);
        if (!MotionAnimationsEnabled(owner))
        {
            SetElementScale(element, x, y);
            return;
        }

        auto visual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(element);
        visual.CenterPoint(ResolveScaleCenter(element));
        auto animation = visual.Compositor().CreateSpringVector3Animation();
        auto const finalValue = Windows::Foundation::Numerics::float3{
            static_cast<float>(x), static_cast<float>(y), 1.0f };
        animation.FinalValue(box_value(finalValue).as<
            Windows::Foundation::IReference<Windows::Foundation::Numerics::float3>>());
        animation.DampingRatio(static_cast<float>(std::clamp(dampingRatio, .05, 3.0)));
        animation.Period(std::chrono::milliseconds{
            static_cast<int64_t>(std::lround(std::clamp(periodMs, 16.0, 2000.0))) });
        visual.StartAnimation(L"Scale", animation);
    }

    inline void AnimateElementScale(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        Microsoft::UI::Xaml::FrameworkElement const& element,
        double x,
        double y,
        double durationMs)
    {
        if (!owner || !element) return;

        x = std::clamp(x, .25, 4.0);
        y = std::clamp(y, .25, 4.0);

        auto visual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(element);
        visual.CenterPoint(ResolveScaleCenter(element));

        auto const duration = std::clamp(durationMs, 0.0, 2000.0);
        if (!MotionAnimationsEnabled(owner) || duration <= 0.0)
        {
            visual.StopAnimation(L"Scale");
            visual.Scale({ static_cast<float>(x), static_cast<float>(y), 1.0f });
            return;
        }

        if (implementation::LiquidGlassInteraction::GetUseSpringMotion(owner))
        {
            // Spring shape belongs to the visual being animated. Attached-property lookup
            // walks up the tree, so a child surface can override the owner's spring while
            // retaining the control-level defaults when no local value is present.
            AnimateElementScaleSpring(
                owner,
                element,
                x,
                y,
                implementation::LiquidGlassInteraction::GetSpringDampingRatio(element),
                implementation::LiquidGlassInteraction::GetSpringPeriod(element));
            return;
        }

        auto animation = visual.Compositor().CreateVector3KeyFrameAnimation();
        animation.InsertKeyFrame(1.0f, {
            static_cast<float>(x),
            static_cast<float>(y),
            1.0f }, CreateMotionEasing(visual.Compositor()));
        animation.Duration(std::chrono::milliseconds{
            static_cast<int64_t>(std::lround(duration)) });
        visual.StartAnimation(L"Scale", animation);
    }

    inline void AnimateElementScale(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        Microsoft::UI::Xaml::FrameworkElement const& element,
        double value,
        double durationMs)
    {
        AnimateElementScale(owner, element, value, value, durationMs);
    }

    inline void SetElementTranslation(
        Microsoft::UI::Xaml::FrameworkElement const& element,
        Windows::Foundation::Numerics::float3 const& value)
    {
        if (!element) return;

        Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetIsTranslationEnabled(element, true);
        auto visual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(element);
        visual.StopAnimation(L"Translation");
        // Translation is a XAML facade property. The backing Visual accepts animations
        // targeting "Translation", but the immediate value is written through UIElement.
        element.Translation(value);
    }

    inline void AnimateElementTranslation(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        Microsoft::UI::Xaml::FrameworkElement const& element,
        Windows::Foundation::Numerics::float3 const& value,
        double durationMs)
    {
        if (!owner || !element) return;

        Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetIsTranslationEnabled(element, true);
        auto visual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(element);
        auto const duration = std::clamp(durationMs, 0.0, 2000.0);

        if (!MotionAnimationsEnabled(owner) || duration <= 0.0)
        {
            visual.StopAnimation(L"Translation");
            element.Translation(value);
            return;
        }

        if (implementation::LiquidGlassInteraction::GetUseSpringMotion(owner))
        {
            auto animation = visual.Compositor().CreateSpringVector3Animation();
            animation.FinalValue(box_value(value).as<
                Windows::Foundation::IReference<Windows::Foundation::Numerics::float3>>());
            animation.DampingRatio(static_cast<float>(std::clamp(
                implementation::LiquidGlassInteraction::GetSpringDampingRatio(owner), .05, 3.0)));
            animation.Period(std::chrono::milliseconds{
                static_cast<int64_t>(std::lround(std::clamp(
                    implementation::LiquidGlassInteraction::GetSpringPeriod(owner), 16.0, 2000.0))) });
            visual.StartAnimation(L"Translation", animation);
            return;
        }

        auto animation = visual.Compositor().CreateVector3KeyFrameAnimation();
        animation.InsertKeyFrame(1.0f, value, CreateMotionEasing(visual.Compositor()));
        animation.Duration(std::chrono::milliseconds{
            static_cast<int64_t>(std::lround(duration)) });
        visual.StartAnimation(L"Translation", animation);
    }
}

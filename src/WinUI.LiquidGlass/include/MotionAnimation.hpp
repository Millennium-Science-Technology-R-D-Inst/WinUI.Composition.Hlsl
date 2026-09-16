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

    inline void SetElementScale(
        Microsoft::UI::Xaml::FrameworkElement const& element,
        double x,
        double y)
    {
        if (!element) return;

        auto visual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(element);
        visual.CenterPoint({
            static_cast<float>(element.ActualWidth() * .5),
            static_cast<float>(element.ActualHeight() * .5),
            0.0f });
        visual.StopAnimation(L"Scale");
        visual.Scale({
            static_cast<float>(std::clamp(x, .25, 4.0)),
            static_cast<float>(std::clamp(y, .25, 4.0)),
            1.0f });
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
        visual.CenterPoint({
            static_cast<float>(element.ActualWidth() * .5),
            static_cast<float>(element.ActualHeight() * .5),
            0.0f });

        if (!MotionAnimationsEnabled(owner))
        {
            visual.StopAnimation(L"Scale");
            visual.Scale({ static_cast<float>(x), static_cast<float>(y), 1.0f });
            return;
        }

        if (implementation::LiquidGlassInteraction::GetUseSpringMotion(owner))
        {
            auto animation = visual.Compositor().CreateSpringVector3Animation();
            auto const finalValue = Windows::Foundation::Numerics::float3{
                static_cast<float>(x), static_cast<float>(y), 1.0f };
            animation.FinalValue(box_value(finalValue).as<
                Windows::Foundation::IReference<Windows::Foundation::Numerics::float3>>());
            animation.DampingRatio(static_cast<float>(std::clamp(
                implementation::LiquidGlassInteraction::GetSpringDampingRatio(owner), .05, 3.0)));
            animation.Period(std::chrono::milliseconds{
                static_cast<int64_t>(std::lround(std::clamp(
                    implementation::LiquidGlassInteraction::GetSpringPeriod(owner), 16.0, 2000.0))) });
            visual.StartAnimation(L"Scale", animation);
            return;
        }

        auto animation = visual.Compositor().CreateVector3KeyFrameAnimation();
        animation.InsertKeyFrame(1.0f, {
            static_cast<float>(x),
            static_cast<float>(y),
            1.0f });
        animation.Duration(std::chrono::milliseconds{
            static_cast<int64_t>(std::lround(std::clamp(durationMs, 0.0, 2000.0))) });
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

        if (!MotionAnimationsEnabled(owner))
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
        animation.InsertKeyFrame(1.0f, value);
        animation.Duration(std::chrono::milliseconds{
            static_cast<int64_t>(std::lround(std::clamp(durationMs, 0.0, 2000.0))) });
        visual.StartAnimation(L"Translation", animation);
    }
}

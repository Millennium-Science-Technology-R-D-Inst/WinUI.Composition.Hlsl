#include <algorithm>
#include <chrono>

#include <winrt/Microsoft.UI.Composition.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Hosting.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Windows.Foundation.Numerics.h>

#include "LiquidGlassControls.h"

#include "LiquidGlassCard.g.cpp"
#include "LiquidGlassButton.g.cpp"
#include "LiquidGlassToggleButton.g.cpp"
#include "LiquidGlassHyperlinkButton.g.cpp"
#include "LiquidGlassCheckBox.g.cpp"
#include "LiquidGlassRadioButton.g.cpp"
#include "LiquidGlassSlider.g.cpp"
#include "LiquidGlassTextBox.g.cpp"
#include "LiquidGlassPasswordBox.g.cpp"
#include "LiquidGlassComboBox.g.cpp"
#include "LiquidGlassToggleSwitch.g.cpp"

namespace winrt::WinUI::LiquidGlass::implementation
{
    namespace
    {
        namespace Composition = Microsoft::UI::Composition;
        namespace Controls = Microsoft::UI::Xaml::Controls;
        namespace Hosting = Microsoft::UI::Xaml::Hosting;
        namespace Media = Microsoft::UI::Xaml::Media;
        namespace Primitives = Microsoft::UI::Xaml::Controls::Primitives;
        namespace Shapes = Microsoft::UI::Xaml::Shapes;
        using Microsoft::UI::Xaml::DependencyObject;
        using Microsoft::UI::Xaml::FrameworkElement;

        constexpr wchar_t const* kThemeUri = L"ms-appx:///WinUI.LiquidGlass/Themes/Generic.xaml";

        void ConfigureCardBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            brush.CornerRadius(20.0);
            brush.BlurRadius(14.0);
            brush.RefractionStrength(18.0);
            brush.DispersionStrength(0.65);
            brush.BezelWidth(24.0);
            brush.GlassThickness(36.0);
            brush.RefractiveIndex(1.48);
            brush.HighlightStrength(0.65);
            brush.HighlightSharpness(1.7);
            brush.SpecularSaturation(3.5);
            brush.SpecularWidth(1.2);
            brush.TintOpacity(0.09);
            brush.Saturation(1.12);
            brush.InnerShadowStrength(0.08);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0x30, 0xff, 0xff, 0xff });
        }

        void ConfigureButtonBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            brush.CornerRadius(8.0);
            brush.BlurRadius(8.0);
            brush.RefractionStrength(14.0);
            brush.DispersionStrength(0.7);
            brush.BezelWidth(12.0);
            brush.GlassThickness(28.0);
            brush.RefractiveIndex(1.45);
            brush.HighlightStrength(0.62);
            brush.HighlightSharpness(1.8);
            brush.SpecularSaturation(4.0);
            brush.SpecularWidth(1.0);
            brush.TintOpacity(0.065);
            brush.Saturation(1.12);
            brush.InnerShadowStrength(0.07);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0x34, 0xff, 0xff, 0xff });
        }

        void ConfigureInputBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            brush.CornerRadius(8.0);
            brush.BlurRadius(10.0);
            brush.RefractionStrength(11.0);
            brush.DispersionStrength(0.5);
            brush.BezelWidth(12.0);
            brush.GlassThickness(28.0);
            brush.RefractiveIndex(1.45);
            brush.HighlightStrength(0.45);
            brush.HighlightSharpness(2.0);
            brush.SpecularSaturation(4.0);
            brush.SpecularWidth(1.0);
            brush.TintOpacity(0.08);
            brush.Saturation(1.08);
            brush.InnerShadowStrength(0.06);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0x32, 0xff, 0xff, 0xff });
        }

        void ConfigureSliderBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            // kube.io slider: convex surface, thick optical body and strongly saturated rim.
            brush.SurfaceProfile(WinUI::Composition::Hlsl::LiquidGlassSurfaceProfile::ConvexSquircle);
            brush.CornerRadius(12.0);
            brush.BlurRadius(1.0);
            brush.RefractionStrength(10.0);
            brush.DispersionStrength(0.7);
            brush.BezelWidth(10.0);
            brush.GlassThickness(80.0);
            brush.RefractiveIndex(1.45);
            brush.HighlightStrength(0.45);
            brush.HighlightSharpness(1.7);
            brush.SpecularSaturation(7.0);
            brush.SpecularWidth(1.0);
            brush.TintOpacity(0.08);
            brush.Saturation(1.0);
            brush.InnerShadowStrength(0.05);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0x80, 0xff, 0xff, 0xff });
        }

        void ConfigureSwitchBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            // kube.io switch deliberately uses the Lip profile: convex outer rim + concave center.
            brush.SurfaceProfile(WinUI::Composition::Hlsl::LiquidGlassSurfaceProfile::Lip);
            brush.CornerRadius(20.0);
            brush.BlurRadius(1.0);
            brush.RefractionStrength(12.0);
            brush.DispersionStrength(0.75);
            brush.BezelWidth(19.0);
            brush.GlassThickness(47.0);
            brush.RefractiveIndex(1.5);
            brush.HighlightStrength(0.52);
            brush.HighlightSharpness(1.6);
            brush.SpecularSaturation(6.0);
            brush.SpecularWidth(1.0);
            brush.TintOpacity(0.08);
            brush.Saturation(1.0);
            brush.InnerShadowStrength(0.08);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0x86, 0xff, 0xff, 0xff });
        }

        void ConfigureChoiceBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            ConfigureButtonBrush(brush);
            brush.CornerRadius(10.0);
            brush.BezelWidth(9.0);
            brush.RefractionStrength(10.0);
            brush.GlassThickness(24.0);
            brush.HighlightStrength(0.5);
        }

        DependencyObject FindNamedDescendant(DependencyObject const& root, std::wstring_view name)
        {
            if (!root)
            {
                return nullptr;
            }

            if (auto element = root.try_as<FrameworkElement>(); element && element.Name() == name)
            {
                return root;
            }

            auto const count = Media::VisualTreeHelper::GetChildrenCount(root);
            for (int32_t index = 0; index < count; ++index)
            {
                if (auto result = FindNamedDescendant(Media::VisualTreeHelper::GetChild(root, index), name))
                {
                    return result;
                }
            }
            return nullptr;
        }

        template<typename T>
        T FindDescendant(DependencyObject const& root)
        {
            if (!root)
            {
                return nullptr;
            }

            auto const count = Media::VisualTreeHelper::GetChildrenCount(root);
            for (int32_t index = 0; index < count; ++index)
            {
                auto child = Media::VisualTreeHelper::GetChild(root, index);
                if (auto match = child.try_as<T>())
                {
                    return match;
                }
                if (auto nested = FindDescendant<T>(child))
                {
                    return nested;
                }
            }
            return nullptr;
        }

        DependencyObject FindPreferredBrushSurface(DependencyObject const& root)
        {
            if (!root)
            {
                return nullptr;
            }

            auto const count = Media::VisualTreeHelper::GetChildrenCount(root);
            for (int32_t index = 0; index < count; ++index)
            {
                auto child = Media::VisualTreeHelper::GetChild(root, index);
                if (auto nested = FindPreferredBrushSurface(child))
                {
                    return nested;
                }
            }

            if (root.try_as<Shapes::Shape>() ||
                root.try_as<Controls::Border>() ||
                root.try_as<Primitives::Thumb>() ||
                root.try_as<Controls::Control>())
            {
                return root;
            }
            return nullptr;
        }

        bool SetSurfaceBrush(DependencyObject const& target, Media::Brush const& brush)
        {
            if (auto shape = target.try_as<Shapes::Shape>())
            {
                shape.Fill(brush);
                return true;
            }
            if (auto border = target.try_as<Controls::Border>())
            {
                border.Background(brush);
                return true;
            }
            if (auto control = target.try_as<Controls::Control>())
            {
                control.Background(brush);
                return true;
            }
            return false;
        }

        void AnimateScale(FrameworkElement const& target, float scale, std::chrono::milliseconds duration)
        {
            if (!target)
            {
                return;
            }

            auto visual = Hosting::ElementCompositionPreview::GetElementVisual(target);
            visual.CenterPoint({
                static_cast<float>(target.ActualWidth() * 0.5),
                static_cast<float>(target.ActualHeight() * 0.5),
                0.0f });
            auto animation = visual.Compositor().CreateVector3KeyFrameAnimation();
            animation.InsertKeyFrame(1.0f, { scale, scale, 1.0f });
            animation.Duration(duration);
            visual.StartAnimation(L"Scale", animation);
        }

        void AnimateTogglePulse(FrameworkElement const& target)
        {
            if (!target)
            {
                return;
            }

            auto visual = Hosting::ElementCompositionPreview::GetElementVisual(target);
            visual.CenterPoint({
                static_cast<float>(target.ActualWidth() * 0.5),
                static_cast<float>(target.ActualHeight() * 0.5),
                0.0f });
            auto animation = visual.Compositor().CreateVector3KeyFrameAnimation();
            animation.InsertKeyFrame(0.0f, { 1.0f, 1.0f, 1.0f });
            animation.InsertKeyFrame(0.42f, { 1.12f, 1.12f, 1.0f });
            animation.InsertKeyFrame(1.0f, { 1.0f, 1.0f, 1.0f });
            animation.Duration(std::chrono::milliseconds{ 220 });
            visual.StartAnimation(L"Scale", animation);
        }

        void SetSliderOpticalState(
            WinUI::Composition::Hlsl::LiquidGlassBrush const& brush,
            bool active)
        {
            brush.RefractionStrength(active ? 22.0 : 10.0);
            brush.TintOpacity(active ? 0.015 : 0.08);
            brush.HighlightStrength(active ? 0.68 : 0.45);
            brush.SpecularSaturation(active ? 9.0 : 7.0);
        }

        void WireSubtleFocusMotion(
            Controls::Control const& control,
            WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            control.GotFocus([brush](auto const& sender, auto const&)
                {
                    auto element = sender.template try_as<FrameworkElement>();
                    AnimateScale(element, 1.0f, std::chrono::milliseconds{ 150 });
                    brush.RefractionStrength(14.0);
                    brush.TintOpacity(0.10);
                });
            control.LostFocus([brush](auto const& sender, auto const&)
                {
                    auto element = sender.template try_as<FrameworkElement>();
                    AnimateScale(element, 0.99f, std::chrono::milliseconds{ 180 });
                    brush.RefractionStrength(11.0);
                    brush.TintOpacity(0.08);
                });
            control.PointerPressed([brush](auto const& sender, auto const&)
                {
                    auto element = sender.template try_as<FrameworkElement>();
                    AnimateScale(element, 0.985f, std::chrono::milliseconds{ 70 });
                    brush.TintOpacity(0.13);
                });
            control.PointerReleased([brush](auto const& sender, auto const&)
                {
                    auto element = sender.template try_as<FrameworkElement>();
                    AnimateScale(element, 1.0f, std::chrono::milliseconds{ 130 });
                    brush.TintOpacity(0.10);
                });
        }
    }

#define WINUI_LIQUID_GLASS_STYLED_BUTTON(Type) \
    Type::Type() \
    { \
        DefaultStyleKey(box_value(xaml_typename<WinUI::LiquidGlass::Type>())); \
        DefaultStyleResourceUri(Windows::Foundation::Uri{ kThemeUri }); \
        m_glassBrush = WinUI::Composition::Hlsl::LiquidGlassBrush{}; \
        ConfigureButtonBrush(m_glassBrush); \
        Background(m_glassBrush.as<Media::Brush>()); \
    } \
    WinUI::Composition::Hlsl::LiquidGlassBrush Type::GlassBrush() const { return m_glassBrush; }

#define WINUI_LIQUID_GLASS_INPUT_CONTROL(Type) \
    Type::Type() \
    { \
        m_glassBrush = WinUI::Composition::Hlsl::LiquidGlassBrush{}; \
        ConfigureInputBrush(m_glassBrush); \
        Background(m_glassBrush.as<Media::Brush>()); \
        WireSubtleFocusMotion(*this, m_glassBrush); \
    } \
    WinUI::Composition::Hlsl::LiquidGlassBrush Type::GlassBrush() const { return m_glassBrush; }

#define WINUI_LIQUID_GLASS_CHOICE_CONTROL(Type) \
    Type::Type() \
    { \
        m_glassBrush = WinUI::Composition::Hlsl::LiquidGlassBrush{}; \
        ConfigureChoiceBrush(m_glassBrush); \
        Background(m_glassBrush.as<Media::Brush>()); \
    } \
    WinUI::Composition::Hlsl::LiquidGlassBrush Type::GlassBrush() const { return m_glassBrush; }

    LiquidGlassCard::LiquidGlassCard()
    {
        DefaultStyleKey(box_value(xaml_typename<WinUI::LiquidGlass::LiquidGlassCard>()));
        DefaultStyleResourceUri(Windows::Foundation::Uri{ kThemeUri });
        m_glassBrush = WinUI::Composition::Hlsl::LiquidGlassBrush{};
        ConfigureCardBrush(m_glassBrush);
        Background(m_glassBrush.as<Media::Brush>());
    }

    WinUI::Composition::Hlsl::LiquidGlassBrush LiquidGlassCard::GlassBrush() const
    {
        return m_glassBrush;
    }

    WINUI_LIQUID_GLASS_STYLED_BUTTON(LiquidGlassButton)
    WINUI_LIQUID_GLASS_STYLED_BUTTON(LiquidGlassToggleButton)
    WINUI_LIQUID_GLASS_STYLED_BUTTON(LiquidGlassHyperlinkButton)
    WINUI_LIQUID_GLASS_CHOICE_CONTROL(LiquidGlassCheckBox)
    WINUI_LIQUID_GLASS_CHOICE_CONTROL(LiquidGlassRadioButton)
    WINUI_LIQUID_GLASS_INPUT_CONTROL(LiquidGlassTextBox)
    WINUI_LIQUID_GLASS_INPUT_CONTROL(LiquidGlassPasswordBox)
    WINUI_LIQUID_GLASS_INPUT_CONTROL(LiquidGlassComboBox)

    LiquidGlassSlider::LiquidGlassSlider()
    {
        m_glassBrush = WinUI::Composition::Hlsl::LiquidGlassBrush{};
        ConfigureSliderBrush(m_glassBrush);
        auto weak = get_weak();
        Loaded([weak](auto const& sender, auto const&)
            {
                auto self = weak.get();
                if (!self || self->m_interactionsWired)
                {
                    return;
                }

                auto slider = sender.template try_as<Controls::Slider>();
                if (!slider)
                {
                    return;
                }

                auto thumbObject = FindNamedDescendant(
                    slider,
                    slider.Orientation() == Controls::Orientation::Horizontal ? L"HorizontalThumb" : L"VerticalThumb");
                auto thumb = thumbObject.try_as<Primitives::Thumb>();
                if (!thumb)
                {
                    thumb = FindDescendant<Primitives::Thumb>(slider);
                }
                if (!thumb)
                {
                    return;
                }

                auto const thumbRadius = std::max(1.0, std::min(thumb.ActualWidth(), thumb.ActualHeight()) * 0.5);
                self->m_glassBrush.CornerRadius(thumbRadius);
                self->m_glassBrush.BezelWidth(std::max(1.0, std::min(16.0, thumbRadius - 0.5)));
                if (auto surface = FindPreferredBrushSurface(thumb))
                {
                    SetSurfaceBrush(surface, self->m_glassBrush.as<Media::Brush>());
                }

                auto brush = self->m_glassBrush;
                thumb.DragStarted([brush](auto const& dragSender, auto const&)
                    {
                        AnimateScale(dragSender.template try_as<FrameworkElement>(), 1.28f, std::chrono::milliseconds{ 90 });
                        SetSliderOpticalState(brush, true);
                    });
                thumb.DragCompleted([brush](auto const& dragSender, auto const&)
                    {
                        AnimateScale(dragSender.template try_as<FrameworkElement>(), 1.0f, std::chrono::milliseconds{ 180 });
                        SetSliderOpticalState(brush, false);
                    });
                self->m_interactionsWired = true;
            });
    }

    WinUI::Composition::Hlsl::LiquidGlassBrush LiquidGlassSlider::GlassBrush() const
    {
        return m_glassBrush;
    }

    LiquidGlassToggleSwitch::LiquidGlassToggleSwitch()
    {
        m_glassBrush = WinUI::Composition::Hlsl::LiquidGlassBrush{};
        ConfigureSwitchBrush(m_glassBrush);
        Background(m_glassBrush.as<Media::Brush>());

        auto weak = get_weak();
        Loaded([weak](auto const& sender, auto const&)
            {
                auto self = weak.get();
                if (!self || self->m_interactionsWired)
                {
                    return;
                }

                auto toggle = sender.template try_as<Controls::ToggleSwitch>();
                if (!toggle)
                {
                    return;
                }

                auto knob = FindNamedDescendant(toggle, L"SwitchKnob").try_as<FrameworkElement>();
                auto switchThumb = FindNamedDescendant(toggle, L"SwitchThumb").try_as<Primitives::Thumb>();
                if (!switchThumb)
                {
                    switchThumb = FindDescendant<Primitives::Thumb>(toggle);
                }

                if (switchThumb && knob)
                {
                    auto brush = self->m_glassBrush;
                    switchThumb.DragStarted([brush, knob](auto const&, auto const&)
                        {
                            AnimateScale(knob, 1.18f, std::chrono::milliseconds{ 90 });
                            brush.RefractionStrength(24.0);
                            brush.TintOpacity(0.02);
                            brush.HighlightStrength(0.72);
                        });
                    switchThumb.DragCompleted([brush, knob](auto const&, auto const&)
                        {
                            AnimateScale(knob, 1.0f, std::chrono::milliseconds{ 190 });
                            brush.RefractionStrength(12.0);
                            brush.TintOpacity(0.08);
                            brush.HighlightStrength(0.52);
                        });
                }

                toggle.Toggled([weak](auto const& toggledSender, auto const&)
                    {
                        if (!weak.get())
                        {
                            return;
                        }
                        auto current = toggledSender.template try_as<Controls::ToggleSwitch>();
                        if (current)
                        {
                            AnimateTogglePulse(FindNamedDescendant(current, L"SwitchKnob").try_as<FrameworkElement>());
                        }
                    });
                self->m_interactionsWired = true;
            });
    }

    WinUI::Composition::Hlsl::LiquidGlassBrush LiquidGlassToggleSwitch::GlassBrush() const
    {
        return m_glassBrush;
    }

#undef WINUI_LIQUID_GLASS_CHOICE_CONTROL
#undef WINUI_LIQUID_GLASS_INPUT_CONTROL
#undef WINUI_LIQUID_GLASS_STYLED_BUTTON
}

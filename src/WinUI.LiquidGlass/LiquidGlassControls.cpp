#include <algorithm>
#include <chrono>
#include <cmath>

#include <winrt/Microsoft.UI.Composition.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Hosting.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Windows.Foundation.Numerics.h>

#include "LiquidGlassControls.h"

#include "LiquidGlassCard.g.cpp"
#include "LiquidGlassMagnifier.g.cpp"
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
        namespace Controls = Microsoft::UI::Xaml::Controls;
        namespace Hosting = Microsoft::UI::Xaml::Hosting;
        namespace Media = Microsoft::UI::Xaml::Media;
        namespace Primitives = Microsoft::UI::Xaml::Controls::Primitives;
        namespace Shapes = Microsoft::UI::Xaml::Shapes;
        using Microsoft::UI::Xaml::DependencyObject;
        using Microsoft::UI::Xaml::FrameworkElement;

        constexpr wchar_t const* kThemeUri = L"ms-appx:///WinUI.LiquidGlass/Themes/Generic.xaml";

        void ConfigurePanelBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            // kube MixedUI floating-player filter: convex squircle, 29 DIP bezel,
            // 90 DIP glass thickness, IOR 1.3, subtle specular and 1 DIP blur.
            brush.SurfaceProfile(WinUI::Composition::Hlsl::LiquidGlassSurfaceProfile::ConvexSquircle);
            brush.CornerRadius(31.0);
            brush.BlurRadius(1.0);
            brush.RefractionStrength(24.0);
            brush.DispersionStrength(0.45);
            brush.BezelWidth(29.0);
            brush.GlassThickness(90.0);
            brush.RefractiveIndex(1.3);
            brush.HighlightStrength(0.4);
            brush.HighlightSharpness(1.6);
            brush.SpecularSaturation(6.0);
            brush.SpecularWidth(1.0);
            brush.TintOpacity(0.12);
            brush.Saturation(1.0);
            brush.InnerShadowStrength(0.05);
            brush.EdgeSoftness(1.0);
            brush.MaterialOpacity(1.0);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0x55, 0xff, 0xff, 0xff });
        }

        void ConfigureButtonBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            brush.SurfaceProfile(WinUI::Composition::Hlsl::LiquidGlassSurfaceProfile::ConvexSquircle);
            brush.CornerRadius(8.0);
            brush.BlurRadius(1.0);
            brush.RefractionStrength(18.0);
            brush.DispersionStrength(0.55);
            brush.BezelWidth(12.0);
            brush.GlassThickness(48.0);
            brush.RefractiveIndex(1.45);
            brush.HighlightStrength(0.4);
            brush.HighlightSharpness(1.8);
            brush.SpecularSaturation(5.0);
            brush.SpecularWidth(1.0);
            brush.TintOpacity(0.12);
            brush.Saturation(1.0);
            brush.InnerShadowStrength(0.06);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0x44, 0xff, 0xff, 0xff });
        }

        void ConfigureSearchBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            // kube Searchbox: 420x56, radius 28, bezel 27, thickness 70, IOR 1.5,
            // refraction level .7, specular opacity .2 and saturation 4.
            brush.SurfaceProfile(WinUI::Composition::Hlsl::LiquidGlassSurfaceProfile::ConvexSquircle);
            brush.CornerRadius(28.0);
            brush.BlurRadius(1.0);
            brush.RefractionStrength(16.8); // 24 * .7
            brush.DispersionStrength(0.35);
            brush.BezelWidth(27.0);
            brush.GlassThickness(70.0);
            brush.RefractiveIndex(1.5);
            brush.HighlightStrength(0.2);
            brush.HighlightSharpness(1.7);
            brush.SpecularSaturation(4.0);
            brush.SpecularWidth(1.0);
            brush.TintOpacity(0.05);
            brush.Saturation(1.0);
            brush.InnerShadowStrength(0.04);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0x30, 0xff, 0xff, 0xff });
        }

        void ConfigureInputBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            brush.SurfaceProfile(WinUI::Composition::Hlsl::LiquidGlassSurfaceProfile::ConvexSquircle);
            brush.CornerRadius(8.0);
            brush.BlurRadius(1.0);
            brush.RefractionStrength(16.0);
            brush.DispersionStrength(0.4);
            brush.BezelWidth(14.0);
            brush.GlassThickness(60.0);
            brush.RefractiveIndex(1.45);
            brush.HighlightStrength(0.3);
            brush.HighlightSharpness(1.8);
            brush.SpecularSaturation(4.0);
            brush.SpecularWidth(1.0);
            brush.TintOpacity(0.10);
            brush.Saturation(1.0);
            brush.InnerShadowStrength(0.05);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0x36, 0xff, 0xff, 0xff });
        }

        void ConfigureSliderBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            // kube Slider thumb: convex squircle, bezel 16, thickness 80, IOR 1.45.
            // Effective scaleRatio is .4 at rest and .9 while dragging.
            brush.SurfaceProfile(WinUI::Composition::Hlsl::LiquidGlassSurfaceProfile::ConvexSquircle);
            brush.CornerRadius(30.0);
            brush.BlurRadius(0.0);
            brush.RefractionStrength(9.6); // 24 * .4
            brush.DispersionStrength(0.45);
            brush.BezelWidth(16.0);
            brush.GlassThickness(80.0);
            brush.RefractiveIndex(1.45);
            brush.HighlightStrength(0.4);
            brush.HighlightSharpness(1.7);
            brush.SpecularSaturation(7.0);
            brush.SpecularWidth(1.0);
            brush.TintOpacity(1.0);
            brush.Saturation(1.0);
            brush.InnerShadowStrength(0.05);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0xff, 0xff, 0xff, 0xff });
        }

        void ConfigureSwitchBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            // kube Switch thumb: Lip, 146x92/r46, bezel 19, thickness 47, IOR 1.5.
            brush.SurfaceProfile(WinUI::Composition::Hlsl::LiquidGlassSurfaceProfile::Lip);
            brush.CornerRadius(46.0);
            brush.BlurRadius(0.2);
            brush.RefractionStrength(9.6); // 24 * .4
            brush.DispersionStrength(0.45);
            brush.BezelWidth(19.0);
            brush.GlassThickness(47.0);
            brush.RefractiveIndex(1.5);
            brush.HighlightStrength(0.5);
            brush.HighlightSharpness(1.6);
            brush.SpecularSaturation(6.0);
            brush.SpecularWidth(1.0);
            brush.TintOpacity(1.0);
            brush.Saturation(1.0);
            brush.InnerShadowStrength(0.02);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0xff, 0xff, 0xff, 0xff });
        }

        void ConfigureMagnifierBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            // kube MagnifyingGlass: 210x150/r75, bezel 25, thickness 110, IOR 1.5.
            // Rest: refraction .8 and magnification 24. Drag: 1.0 and magnification 48.
            brush.SurfaceProfile(WinUI::Composition::Hlsl::LiquidGlassSurfaceProfile::ConvexSquircle);
            brush.CornerRadius(75.0);
            brush.BlurRadius(0.0);
            brush.RefractionStrength(19.2); // 24 * .8
            brush.DispersionStrength(0.55);
            brush.BezelWidth(25.0);
            brush.GlassThickness(110.0);
            brush.RefractiveIndex(1.5);
            brush.MagnificationStrength(24.0);
            brush.HighlightStrength(0.5);
            brush.HighlightSharpness(1.6);
            brush.SpecularSaturation(9.0);
            brush.SpecularWidth(1.0);
            brush.TintOpacity(0.02);
            brush.Saturation(1.0);
            brush.InnerShadowStrength(0.20);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0x28, 0xff, 0xff, 0xff });
        }

        void ConfigureChoiceBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            ConfigureButtonBrush(brush);
            brush.CornerRadius(10.0);
            brush.BezelWidth(9.0);
            brush.GlassThickness(32.0);
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
            if (root.try_as<Shapes::Shape>() || root.try_as<Controls::Border>() ||
                root.try_as<Primitives::Thumb>() || root.try_as<Controls::Control>())
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

        void SetScale(FrameworkElement const& target, float scaleX, float scaleY)
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
            visual.Scale({ scaleX, scaleY, 1.0f });
        }

        void AnimateScale(FrameworkElement const& target, float scaleX, float scaleY, std::chrono::milliseconds duration)
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
            animation.InsertKeyFrame(1.0f, { scaleX, scaleY, 1.0f });
            animation.Duration(duration);
            visual.StartAnimation(L"Scale", animation);
        }

        void AnimateScale(FrameworkElement const& target, float scale, std::chrono::milliseconds duration)
        {
            AnimateScale(target, scale, scale, duration);
        }

        void AnimateSwitchPulse(FrameworkElement const& target)
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
            animation.InsertKeyFrame(0.0f, { 0.65f, 0.65f, 1.0f });
            animation.InsertKeyFrame(0.42f, { 0.75f, 0.75f, 1.0f });
            animation.InsertKeyFrame(1.0f, { 0.65f, 0.65f, 1.0f });
            animation.Duration(std::chrono::milliseconds{ 220 });
            visual.StartAnimation(L"Scale", animation);
        }

        void SetSliderOpticalState(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush, bool active)
        {
            brush.RefractionStrength(active ? 21.6 : 9.6); // .9 / .4 scale ratio
            brush.TintOpacity(active ? 0.10 : 1.0);        // kube background opacity .1 / 1
        }

        void SetSwitchOpticalState(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush, bool active)
        {
            brush.RefractionStrength(active ? 21.6 : 9.6);
            brush.TintOpacity(active ? 0.10 : 1.0);
            brush.InnerShadowStrength(active ? 0.27 : 0.02);
        }

        void WireSearchMotion(Controls::TextBox const& control, WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            control.Loaded([](auto const& sender, auto const&)
                {
                    SetScale(sender.template try_as<FrameworkElement>(), 0.8f, 0.8f);
                });
            control.GotFocus([brush](auto const& sender, auto const&)
                {
                    AnimateScale(sender.template try_as<FrameworkElement>(), 1.0f, std::chrono::milliseconds{ 120 });
                    brush.TintOpacity(0.20);
                });
            control.LostFocus([brush](auto const& sender, auto const&)
                {
                    AnimateScale(sender.template try_as<FrameworkElement>(), 0.8f, std::chrono::milliseconds{ 170 });
                    brush.TintOpacity(0.05);
                });
            control.PointerPressed([brush](auto const& sender, auto const&)
                {
                    auto current = sender.template try_as<Controls::Control>();
                    auto const base = current && current.FocusState() != Microsoft::UI::Xaml::FocusState::Unfocused ? 1.0f : 0.8f;
                    AnimateScale(sender.template try_as<FrameworkElement>(), base * 0.99f, std::chrono::milliseconds{ 55 });
                    brush.TintOpacity(0.30);
                });
            control.PointerReleased([brush](auto const& sender, auto const&)
                {
                    auto current = sender.template try_as<Controls::Control>();
                    auto const focused = current && current.FocusState() != Microsoft::UI::Xaml::FocusState::Unfocused;
                    AnimateScale(sender.template try_as<FrameworkElement>(), focused ? 1.0f : 0.8f, std::chrono::milliseconds{ 110 });
                    brush.TintOpacity(focused ? 0.20 : 0.05);
                });
        }

        void WireSubtleInputMotion(Controls::Control const& control, WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            control.GotFocus([brush](auto const& sender, auto const&)
                {
                    AnimateScale(sender.template try_as<FrameworkElement>(), 1.0f, std::chrono::milliseconds{ 120 });
                    brush.TintOpacity(0.14);
                });
            control.LostFocus([brush](auto const& sender, auto const&)
                {
                    AnimateScale(sender.template try_as<FrameworkElement>(), 0.99f, std::chrono::milliseconds{ 160 });
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
        WireSubtleInputMotion(*this, m_glassBrush); \
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
        ConfigurePanelBrush(m_glassBrush);
        Background(m_glassBrush.as<Media::Brush>());
    }

    WinUI::Composition::Hlsl::LiquidGlassBrush LiquidGlassCard::GlassBrush() const
    {
        return m_glassBrush;
    }

    LiquidGlassMagnifier::LiquidGlassMagnifier()
    {
        DefaultStyleKey(box_value(xaml_typename<WinUI::LiquidGlass::LiquidGlassMagnifier>()));
        DefaultStyleResourceUri(Windows::Foundation::Uri{ kThemeUri });
        m_glassBrush = WinUI::Composition::Hlsl::LiquidGlassBrush{};
        ConfigureMagnifierBrush(m_glassBrush);
        Background(m_glassBrush.as<Media::Brush>());
        RenderTransformOrigin({ 0.5f, 0.5f });
        m_dragTransform = Media::CompositeTransform{};
        RenderTransform(m_dragTransform);

        Loaded([](auto const& sender, auto const&)
            {
                SetScale(sender.template try_as<FrameworkElement>(), 0.8f, 0.8f);
            });

        auto weak = get_weak();
        PointerPressed([weak](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
            {
                auto self = weak.get();
                if (!self)
                {
                    return;
                }
                auto point = args.GetCurrentPoint(*self);
                self->m_activePointerId = point.PointerId();
                self->m_lastPointer = point.Position();
                self->m_dragging = self->CapturePointer(args.Pointer());
                if (!self->m_dragging)
                {
                    return;
                }
                self->m_glassBrush.RefractionStrength(24.0);
                self->m_glassBrush.MagnificationStrength(48.0);
                self->m_glassBrush.InnerShadowStrength(0.27);
                AnimateScale(*self, 1.0f, std::chrono::milliseconds{ 110 });
                args.Handled(true);
            });

        PointerMoved([weak](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
            {
                auto self = weak.get();
                if (!self || !self->m_dragging)
                {
                    return;
                }
                auto point = args.GetCurrentPoint(*self);
                if (point.PointerId() != self->m_activePointerId)
                {
                    return;
                }
                auto const position = point.Position();
                auto const dx = position.X - self->m_lastPointer.X;
                auto const dy = position.Y - self->m_lastPointer.Y;
                self->m_dragTransform.TranslateX(self->m_dragTransform.TranslateX() + dx);
                self->m_dragTransform.TranslateY(self->m_dragTransform.TranslateY() + dy);

                // Same visual idea as kube: horizontal drag velocity squashes Y and stretches X.
                auto const pseudoVelocityX = static_cast<float>(dx * 60.0);
                auto const scaleY = std::max(0.7f, 1.0f - std::abs(pseudoVelocityX) / 5000.0f);
                auto const scaleX = 1.0f + (1.0f - scaleY);
                SetScale(*self, scaleX, scaleY);
                self->m_lastPointer = position;
                args.Handled(true);
            });

        PointerReleased([weak](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
            {
                auto self = weak.get();
                if (!self || !self->m_dragging)
                {
                    return;
                }
                self->ReleasePointerCapture(args.Pointer());
                self->m_dragging = false;
                self->m_activePointerId = 0;
                self->m_glassBrush.RefractionStrength(19.2);
                self->m_glassBrush.MagnificationStrength(24.0);
                self->m_glassBrush.InnerShadowStrength(0.20);
                AnimateScale(*self, 0.8f, std::chrono::milliseconds{ 180 });
                args.Handled(true);
            });

        PointerCanceled([weak](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
            {
                auto self = weak.get();
                if (!self)
                {
                    return;
                }
                if (self->m_dragging)
                {
                    self->ReleasePointerCapture(args.Pointer());
                }
                self->m_dragging = false;
                self->m_activePointerId = 0;
                self->m_glassBrush.RefractionStrength(19.2);
                self->m_glassBrush.MagnificationStrength(24.0);
                self->m_glassBrush.InnerShadowStrength(0.20);
                AnimateScale(*self, 0.8f, std::chrono::milliseconds{ 180 });
            });

        PointerCaptureLost([weak](auto const&, auto const&)
            {
                auto self = weak.get();
                if (!self)
                {
                    return;
                }
                self->m_dragging = false;
                self->m_activePointerId = 0;
                self->m_glassBrush.RefractionStrength(19.2);
                self->m_glassBrush.MagnificationStrength(24.0);
                self->m_glassBrush.InnerShadowStrength(0.20);
                AnimateScale(*self, 0.8f, std::chrono::milliseconds{ 180 });
            });
    }

    WinUI::Composition::Hlsl::LiquidGlassBrush LiquidGlassMagnifier::GlassBrush() const
    {
        return m_glassBrush;
    }

    WINUI_LIQUID_GLASS_STYLED_BUTTON(LiquidGlassButton)
    WINUI_LIQUID_GLASS_STYLED_BUTTON(LiquidGlassToggleButton)
    WINUI_LIQUID_GLASS_STYLED_BUTTON(LiquidGlassHyperlinkButton)
    WINUI_LIQUID_GLASS_CHOICE_CONTROL(LiquidGlassCheckBox)
    WINUI_LIQUID_GLASS_CHOICE_CONTROL(LiquidGlassRadioButton)
    WINUI_LIQUID_GLASS_INPUT_CONTROL(LiquidGlassPasswordBox)
    WINUI_LIQUID_GLASS_INPUT_CONTROL(LiquidGlassComboBox)

    LiquidGlassTextBox::LiquidGlassTextBox()
    {
        m_glassBrush = WinUI::Composition::Hlsl::LiquidGlassBrush{};
        ConfigureSearchBrush(m_glassBrush);
        Background(m_glassBrush.as<Media::Brush>());
        WireSearchMotion(*this, m_glassBrush);
    }

    WinUI::Composition::Hlsl::LiquidGlassBrush LiquidGlassTextBox::GlassBrush() const
    {
        return m_glassBrush;
    }

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
                SetScale(thumb, 0.6f, 0.6f);

                auto brush = self->m_glassBrush;
                thumb.DragStarted([brush](auto const& dragSender, auto const&)
                    {
                        AnimateScale(dragSender.template try_as<FrameworkElement>(), 1.0f, std::chrono::milliseconds{ 90 });
                        SetSliderOpticalState(brush, true);
                    });
                thumb.DragCompleted([brush](auto const& dragSender, auto const&)
                    {
                        AnimateScale(dragSender.template try_as<FrameworkElement>(), 0.6f, std::chrono::milliseconds{ 180 });
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
                if (!knob)
                {
                    knob = switchThumb;
                }
                if (knob)
                {
                    auto const radius = std::max(1.0, std::min(knob.ActualWidth(), knob.ActualHeight()) * 0.5);
                    self->m_glassBrush.CornerRadius(radius);
                    self->m_glassBrush.BezelWidth(std::max(1.0, std::min(19.0, radius - 0.5)));
                    if (auto surface = FindPreferredBrushSurface(knob))
                    {
                        SetSurfaceBrush(surface, self->m_glassBrush.as<Media::Brush>());
                    }
                    SetScale(knob, 0.65f, 0.65f);
                }

                if (switchThumb && knob)
                {
                    auto brush = self->m_glassBrush;
                    switchThumb.DragStarted([brush, knob](auto const&, auto const&)
                        {
                            AnimateScale(knob, 0.9f, std::chrono::milliseconds{ 90 });
                            SetSwitchOpticalState(brush, true);
                        });
                    switchThumb.DragCompleted([brush, knob](auto const&, auto const&)
                        {
                            AnimateScale(knob, 0.65f, std::chrono::milliseconds{ 190 });
                            SetSwitchOpticalState(brush, false);
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
                            AnimateSwitchPulse(FindNamedDescendant(current, L"SwitchKnob").try_as<FrameworkElement>());
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

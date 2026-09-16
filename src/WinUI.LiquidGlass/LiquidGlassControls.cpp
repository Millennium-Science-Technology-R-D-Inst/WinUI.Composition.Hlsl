#include "pch.h"
#include "winrt_module_imports.h"
#include "LiquidGlassControls.h"

#if __has_include("LiquidGlassCard.g.cpp")
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
#endif

namespace winrt::WinUI::LiquidGlass::implementation
{
    using namespace std::chrono_literals;
    namespace
    {
        namespace Xaml = Microsoft::UI::Xaml;
        namespace Controls = Xaml::Controls;
        namespace Primitives = Controls::Primitives;
        namespace Media = Xaml::Media;
        namespace Shapes = Xaml::Shapes;

        using Brush = WinUI::Composition::Hlsl::LiquidGlassBrush;
        using Profile = WinUI::Composition::Hlsl::LiquidGlassSurfaceProfile;
        using Xaml::DependencyObject;
        using Xaml::FrameworkElement;

        enum class Preset { Panel, Button, Choice, Search, Input, Slider, Switch, Magnifier };

        Media::Brush AsBrush(Brush const& value)
        {
            return value ? value.as<Media::Brush>() : Media::Brush{ nullptr };
        }

        Brush CreateBrush(Preset preset)
        {
            Brush b;
            b.SurfaceProfile(Profile::ConvexSquircle);
            b.BlurRadius(1.0);
            b.Saturation(1.0);
            b.Contrast(1.0);
            b.Exposure(0.0);
            b.MaterialOpacity(1.0);
            b.EdgeSoftness(1.0);

            switch (preset)
            {
            case Preset::Panel:
                b.CornerRadius(31); b.RefractionStrength(24); b.DispersionStrength(.45);
                b.BezelWidth(29); b.GlassThickness(90); b.RefractiveIndex(1.3);
                b.HighlightStrength(.4); b.HighlightSharpness(1.6); b.SpecularSaturation(6);
                b.SpecularWidth(1); b.TintOpacity(.12); b.InnerShadowStrength(.05);
                b.FallbackColor({ 0x55, 0xff, 0xff, 0xff }); break;
            case Preset::Button:
            case Preset::Choice:
                b.CornerRadius(preset == Preset::Choice ? 10 : 8);
                b.RefractionStrength(18); b.DispersionStrength(.55);
                b.BezelWidth(preset == Preset::Choice ? 9 : 12);
                b.GlassThickness(preset == Preset::Choice ? 32 : 48);
                b.RefractiveIndex(1.45); b.HighlightStrength(.4); b.HighlightSharpness(1.8);
                b.SpecularSaturation(5); b.SpecularWidth(1); b.TintOpacity(.12);
                b.InnerShadowStrength(.06); b.FallbackColor({ 0x44, 0xff, 0xff, 0xff }); break;
            case Preset::Search:
                b.CornerRadius(28); b.RefractionStrength(16.8); b.DispersionStrength(.35);
                b.BezelWidth(27); b.GlassThickness(70); b.RefractiveIndex(1.5);
                b.HighlightStrength(.2); b.HighlightSharpness(1.7); b.SpecularSaturation(4);
                b.SpecularWidth(1); b.TintOpacity(.05); b.InnerShadowStrength(.04);
                b.FallbackColor({ 0x30, 0xff, 0xff, 0xff }); break;
            case Preset::Input:
                b.CornerRadius(8); b.RefractionStrength(16); b.DispersionStrength(.4);
                b.BezelWidth(14); b.GlassThickness(60); b.RefractiveIndex(1.45);
                b.HighlightStrength(.3); b.HighlightSharpness(1.8); b.SpecularSaturation(4);
                b.SpecularWidth(1); b.TintOpacity(.10); b.InnerShadowStrength(.05);
                b.FallbackColor({ 0x36, 0xff, 0xff, 0xff }); break;
            case Preset::Slider:
                b.CornerRadius(30); b.BlurRadius(0); b.RefractionStrength(9.6);
                b.DispersionStrength(.45); b.BezelWidth(16); b.GlassThickness(80);
                b.RefractiveIndex(1.45); b.HighlightStrength(.4); b.HighlightSharpness(1.7);
                b.SpecularSaturation(7); b.SpecularWidth(1); b.TintOpacity(1);
                b.InnerShadowStrength(.05); b.FallbackColor({ 0xff, 0xff, 0xff, 0xff }); break;
            case Preset::Switch:
                b.SurfaceProfile(Profile::Lip); b.CornerRadius(46); b.BlurRadius(.2);
                b.RefractionStrength(9.6); b.DispersionStrength(.45); b.BezelWidth(19);
                b.GlassThickness(47); b.RefractiveIndex(1.5); b.HighlightStrength(.5);
                b.HighlightSharpness(1.6); b.SpecularSaturation(6); b.SpecularWidth(1);
                b.TintOpacity(1); b.InnerShadowStrength(.02);
                b.FallbackColor({ 0xff, 0xff, 0xff, 0xff }); break;
            case Preset::Magnifier:
                b.CornerRadius(75); b.BlurRadius(0); b.RefractionStrength(19.2);
                b.DispersionStrength(.55); b.BezelWidth(25); b.GlassThickness(110);
                b.RefractiveIndex(1.5); b.MagnificationStrength(24); b.HighlightStrength(.5);
                b.HighlightSharpness(1.6); b.SpecularSaturation(9); b.SpecularWidth(1);
                b.TintOpacity(.02); b.InnerShadowStrength(.20);
                b.FallbackColor({ 0x28, 0xff, 0xff, 0xff }); break;
            }
            return b;
        }
    }

    LiquidGlassCard::LiquidGlassCard()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Panel));
    }

    LiquidGlassButton::LiquidGlassButton()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Button));
    }

    LiquidGlassToggleButton::LiquidGlassToggleButton()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Button));
    }

    LiquidGlassHyperlinkButton::LiquidGlassHyperlinkButton()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Button));
    }

    LiquidGlassCheckBox::LiquidGlassCheckBox()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        auto brush = CreateBrush(Preset::Choice);
        brush.CornerRadius(5.0);
        GlassBrush(brush);
    }

    LiquidGlassRadioButton::LiquidGlassRadioButton()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Choice));
    }

    LiquidGlassComboBox::LiquidGlassComboBox()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Choice));
        SetValue(LiquidGlassInteraction::FocusedScaleProperty(), box_value(1.01));
        SetValue(LiquidGlassInteraction::FocusedTintBoostProperty(), box_value(.04));
    }

    LiquidGlassTextBox::LiquidGlassTextBox()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Input));
        SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.98));
        SetValue(LiquidGlassInteraction::FocusedScaleProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::FocusedTintBoostProperty(), box_value(.15));
        SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(140.0));
    }

    void LiquidGlassPasswordBox::ApplyGlassBrush(Brush const& value)
    {
        m_glassBrush = value;
        if (m_passwordBox) m_passwordBox.Background(AsBrush(value));
    }

    LiquidGlassPasswordBox::LiquidGlassPasswordBox()
    {
        m_passwordBox = Controls::PasswordBox{};
        m_passwordBox.HorizontalAlignment(Xaml::HorizontalAlignment::Stretch);
        HorizontalContentAlignment(Xaml::HorizontalAlignment::Stretch);
        Content(m_passwordBox);
        GlassBrush(CreateBrush(Preset::Input));
        SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.99));
        SetValue(LiquidGlassInteraction::FocusedScaleProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::FocusedTintBoostProperty(), box_value(.04));
        SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(140.0));
    }

    hstring LiquidGlassPasswordBox::PlaceholderText() const { return m_passwordBox ? m_passwordBox.PlaceholderText() : hstring{}; }
    void LiquidGlassPasswordBox::PlaceholderText(hstring const& value) { if (m_passwordBox) m_passwordBox.PlaceholderText(value); }
    hstring LiquidGlassPasswordBox::Password() const { return m_passwordBox ? m_passwordBox.Password() : hstring{}; }
    void LiquidGlassPasswordBox::Password(hstring const& value) { if (m_passwordBox) m_passwordBox.Password(value); }

    LiquidGlassMagnifier::LiquidGlassMagnifier()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Magnifier));
        SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.8));
        SetValue(LiquidGlassInteraction::PressedScaleProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(175.0));
        SetValue(LiquidGlassInteraction::OpticsTransitionDurationProperty(), box_value(170.0));
        SetValue(LiquidGlassInteraction::ElasticityProperty(), box_value(.70));
        SetValue(LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(1.25));
        SetValue(LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(0.0));
        SetValue(LiquidGlassInteraction::PressedTintBoostProperty(), box_value(0.0));
        SetValue(LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(0.0));
        SetValue(LiquidGlassInteraction::PressedInnerShadowBoostProperty(), box_value(.07));
        SetValue(LiquidGlassInteraction::ActiveMagnificationMultiplierProperty(), box_value(2.0));
    }

    void LiquidGlassSlider::ApplyGlassBrush(Brush const& value)
    {
        m_glassBrush = value;
        detail::SliderSurfaceVisualHelper<LiquidGlassSlider>::RefreshVisual();
        detail::SliderDragMotionHelper<LiquidGlassSlider>::RefreshInteractionTarget();
    }

    LiquidGlassSlider::LiquidGlassSlider()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Slider));
        SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.6));
        SetValue(LiquidGlassInteraction::PressedScaleProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(120.0));
        SetValue(LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(2.25));
        SetValue(LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(0.0));
        SetValue(LiquidGlassInteraction::PressedTintBoostProperty(), box_value(-.9));
        SetValue(LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(0.0));
        SetValue(LiquidGlassInteraction::PressedInnerShadowBoostProperty(), box_value(0.0));
    }

    LiquidGlassToggleSwitch::LiquidGlassToggleSwitch()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Switch));
        SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.65));
        SetValue(LiquidGlassInteraction::PressedScaleProperty(), box_value(.9));
        SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(130.0));
        SetValue(LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(2.25));
        SetValue(LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(0.0));
        SetValue(LiquidGlassInteraction::PressedTintBoostProperty(), box_value(-.9));
        SetValue(LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(0.0));
        SetValue(LiquidGlassInteraction::PressedInnerShadowBoostProperty(), box_value(.07));
    }

    Windows::Foundation::IInspectable LiquidGlassToggleSwitch::Header() const { return m_header; }
    void LiquidGlassToggleSwitch::Header(Windows::Foundation::IInspectable const& value) { m_header = value; Content(value); }
    bool LiquidGlassToggleSwitch::IsOn() const { auto v = IsChecked(); return v && v.Value(); }
    void LiquidGlassToggleSwitch::IsOn(bool value) { IsChecked(box_value(value).as<Windows::Foundation::IReference<bool>>()); }
}

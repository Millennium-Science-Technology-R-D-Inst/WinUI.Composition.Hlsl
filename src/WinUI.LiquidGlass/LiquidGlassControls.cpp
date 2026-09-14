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
            brush.TintOpacity(0.09);
            brush.Saturation(1.12);
            brush.InnerShadowStrength(0.08);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0x30, 0xff, 0xff, 0xff });
        }

        void ConfigureButtonBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            brush.CornerRadius(8.0);
            brush.BlurRadius(10.0);
            brush.RefractionStrength(14.0);
            brush.DispersionStrength(0.7);
            brush.BezelWidth(14.0);
            brush.GlassThickness(26.0);
            brush.RefractiveIndex(1.45);
            brush.HighlightStrength(0.72);
            brush.HighlightSharpness(1.8);
            brush.TintOpacity(0.07);
            brush.Saturation(1.14);
            brush.InnerShadowStrength(0.08);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0x38, 0xff, 0xff, 0xff });
        }

        void ConfigureNativeBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            brush.CornerRadius(8.0);
            brush.BlurRadius(8.0);
            brush.RefractionStrength(10.0);
            brush.DispersionStrength(0.5);
            brush.BezelWidth(12.0);
            brush.GlassThickness(22.0);
            brush.RefractiveIndex(1.45);
            brush.HighlightStrength(0.55);
            brush.HighlightSharpness(1.8);
            brush.TintOpacity(0.05);
            brush.Saturation(1.08);
            brush.InnerShadowStrength(0.06);
            brush.FallbackColor(winrt::Windows::UI::Color{ 0x30, 0xff, 0xff, 0xff });
        }

        template <typename TControl>
        void ApplyGlassBackground(TControl const& control, WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            control.Background(brush.as<Microsoft::UI::Xaml::Media::Brush>());
        }
    }

#define WINUI_LIQUID_GLASS_NATIVE_CONTROL(Type) \
    Type::Type() \
    { \
        m_glassBrush = WinUI::Composition::Hlsl::LiquidGlassBrush{}; \
        ConfigureNativeBrush(m_glassBrush); \
        ApplyGlassBackground(*this, m_glassBrush); \
    } \
    WinUI::Composition::Hlsl::LiquidGlassBrush Type::GlassBrush() const { return m_glassBrush; }

#define WINUI_LIQUID_GLASS_STYLED_BUTTON(Type) \
    Type::Type() \
    { \
        DefaultStyleKey(box_value(xaml_typename<WinUI::LiquidGlass::Type>())); \
        DefaultStyleResourceUri(Windows::Foundation::Uri{ kThemeUri }); \
        m_glassBrush = WinUI::Composition::Hlsl::LiquidGlassBrush{}; \
        ConfigureButtonBrush(m_glassBrush); \
        ApplyGlassBackground(*this, m_glassBrush); \
    } \
    WinUI::Composition::Hlsl::LiquidGlassBrush Type::GlassBrush() const { return m_glassBrush; }

    LiquidGlassCard::LiquidGlassCard()
    {
        DefaultStyleKey(box_value(xaml_typename<WinUI::LiquidGlass::LiquidGlassCard>()));
        DefaultStyleResourceUri(Windows::Foundation::Uri{ kThemeUri });
        m_glassBrush = WinUI::Composition::Hlsl::LiquidGlassBrush{};
        ConfigureCardBrush(m_glassBrush);
        ApplyGlassBackground(*this, m_glassBrush);
    }

    WinUI::Composition::Hlsl::LiquidGlassBrush LiquidGlassCard::GlassBrush() const
    {
        return m_glassBrush;
    }

    WINUI_LIQUID_GLASS_STYLED_BUTTON(LiquidGlassButton)
    WINUI_LIQUID_GLASS_STYLED_BUTTON(LiquidGlassToggleButton)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassHyperlinkButton)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassCheckBox)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassRadioButton)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassSlider)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassTextBox)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassPasswordBox)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassComboBox)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassToggleSwitch)

#undef WINUI_LIQUID_GLASS_STYLED_BUTTON
#undef WINUI_LIQUID_GLASS_NATIVE_CONTROL
}

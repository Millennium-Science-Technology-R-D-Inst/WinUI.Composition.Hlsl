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
    }

#define WINUI_LIQUID_GLASS_NATIVE_CONTROL(Type) \
    Type::Type() \
    { \
        m_glassBrush = WinUI::Composition::Hlsl::LiquidGlassBrush{}; \
        Background(m_glassBrush); \
    } \
    WinUI::Composition::Hlsl::LiquidGlassBrush Type::GlassBrush() const { return m_glassBrush; }

#define WINUI_LIQUID_GLASS_STYLED_CONTROL(Type) \
    Type::Type() \
    { \
        DefaultStyleKey(box_value(xaml_typename<WinUI::LiquidGlass::Type>())); \
        DefaultStyleResourceUri(Windows::Foundation::Uri{ kThemeUri }); \
        m_glassBrush = WinUI::Composition::Hlsl::LiquidGlassBrush{}; \
        Background(m_glassBrush); \
    } \
    WinUI::Composition::Hlsl::LiquidGlassBrush Type::GlassBrush() const { return m_glassBrush; }

    WINUI_LIQUID_GLASS_STYLED_CONTROL(LiquidGlassCard)
    WINUI_LIQUID_GLASS_STYLED_CONTROL(LiquidGlassButton)
    WINUI_LIQUID_GLASS_STYLED_CONTROL(LiquidGlassToggleButton)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassHyperlinkButton)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassCheckBox)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassRadioButton)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassSlider)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassTextBox)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassPasswordBox)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassComboBox)
    WINUI_LIQUID_GLASS_NATIVE_CONTROL(LiquidGlassToggleSwitch)

#undef WINUI_LIQUID_GLASS_STYLED_CONTROL
#undef WINUI_LIQUID_GLASS_NATIVE_CONTROL
}

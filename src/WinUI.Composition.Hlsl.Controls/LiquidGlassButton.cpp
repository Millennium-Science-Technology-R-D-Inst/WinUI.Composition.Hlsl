#include "LiquidGlassButton.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>

#if __has_include("LiquidGlassButton.g.cpp")
#include "LiquidGlassButton.g.cpp"
#endif

namespace winrt::WinUI::Composition::Hlsl::Controls::implementation
{
    LiquidGlassButton::LiquidGlassButton()
    {
        m_glassBrush = winrt::WinUI::Composition::Hlsl::LiquidGlassBrush();
        m_glassBrush.BezelWidth(18.0);
        m_glassBrush.BlurRadius(10.0);
        m_glassBrush.BorderThickness(1.2);
        m_glassBrush.CornerRadius(18.0);
        m_glassBrush.DispersionStrength(0.8);
        m_glassBrush.GlassThickness(28.0);
        m_glassBrush.HighlightStrength(0.7);
        m_glassBrush.RefractiveIndex(1.45);
        m_glassBrush.RefractionStrength(16.0);
        m_glassBrush.Saturation(1.15);
        m_glassBrush.TintOpacity(0.07);
        Background(m_glassBrush);

        DefaultStyleKey(winrt::box_value(winrt::xaml_typename<class_type>()));
        DefaultStyleResourceUri(winrt::Windows::Foundation::Uri(L"ms-appx:///WinUI.Composition.Hlsl.Controls/Themes/Generic.xaml"));
    }
}

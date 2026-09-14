#include "LiquidGlassSlider.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>

#if __has_include("LiquidGlassSlider.g.cpp")
#include "LiquidGlassSlider.g.cpp"
#endif

namespace winrt::WinUI::Composition::Hlsl::Controls::implementation
{
    LiquidGlassSlider::LiquidGlassSlider()
    {
        m_glassBrush = winrt::WinUI::Composition::Hlsl::LiquidGlassBrush();
        m_glassBrush.BezelWidth(14.0);
        m_glassBrush.BlurRadius(8.0);
        m_glassBrush.BorderThickness(1.0);
        m_glassBrush.CornerRadius(8.0);
        m_glassBrush.DispersionStrength(0.55);
        m_glassBrush.GlassThickness(22.0);
        m_glassBrush.HighlightStrength(0.55);
        m_glassBrush.RefractiveIndex(1.45);
        m_glassBrush.RefractionStrength(12.0);
        m_glassBrush.Saturation(1.10);
        m_glassBrush.TintOpacity(0.06);
        Background(m_glassBrush);

        DefaultStyleKey(winrt::box_value(winrt::xaml_typename<class_type>()));
        DefaultStyleResourceUri(winrt::Windows::Foundation::Uri(L"ms-appx:///WinUI.Composition.Hlsl.Controls/Themes/Generic.xaml"));
    }
}

#include "LiquidGlassCard.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>

#if __has_include("LiquidGlassCard.g.cpp")
#include "LiquidGlassCard.g.cpp"
#endif

namespace winrt::WinUI::Composition::Hlsl::Controls::implementation
{
    LiquidGlassCard::LiquidGlassCard()
    {
        m_glassBrush = winrt::WinUI::Composition::Hlsl::LiquidGlassBrush();
        m_glassBrush.BezelWidth(24.0);
        m_glassBrush.BlurRadius(14.0);
        m_glassBrush.BorderThickness(1.0);
        m_glassBrush.CornerRadius(24.0);
        m_glassBrush.DispersionStrength(0.65);
        m_glassBrush.GlassThickness(36.0);
        m_glassBrush.HighlightStrength(0.65);
        m_glassBrush.RefractiveIndex(1.48);
        m_glassBrush.RefractionStrength(18.0);
        m_glassBrush.Saturation(1.12);
        m_glassBrush.TintOpacity(0.09);
        Background(m_glassBrush);

        DefaultStyleKey(winrt::box_value(winrt::xaml_typename<class_type>()));
        DefaultStyleResourceUri(winrt::Windows::Foundation::Uri(L"ms-appx:///WinUI.Composition.Hlsl.Controls/Themes/Generic.xaml"));
    }
}

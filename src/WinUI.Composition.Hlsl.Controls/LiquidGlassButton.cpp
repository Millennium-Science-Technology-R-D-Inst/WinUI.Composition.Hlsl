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
        DefaultStyleKey(winrt::box_value(winrt::xaml_typename<class_type>()));
        DefaultStyleResourceUri(winrt::Windows::Foundation::Uri(L"ms-appx:///WinUI.Composition.Hlsl.Controls/Themes/Generic.xaml"));
    }
}

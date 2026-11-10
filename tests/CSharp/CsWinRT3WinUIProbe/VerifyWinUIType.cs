// Minimal standalone test: does WindowsAppSDK 2.6.5 expose Microsoft.UI.Xaml
// to a CsWinRT 3 Windows .1 project, without any HLSL or LiquidGlass dependency?
namespace WinUIProjectionProbe;

internal static class VerifyWinUIType
{
    public static System.Type FrameworkElementType => typeof(Microsoft.UI.Xaml.FrameworkElement);
}

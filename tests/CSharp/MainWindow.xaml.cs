using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using WinUI.Composition.Hlsl;

namespace HlslCSharpConsumer;

public sealed partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        RootGrid.Loaded += (_, _) => UpdateMaterial();
    }

    private void OnThemeChanged(object sender, SelectionChangedEventArgs e)
    {
        if (RootGrid is null) return;
        RootGrid.RequestedTheme = ThemeSelector.SelectedIndex switch
        {
            1 => ElementTheme.Light,
            2 => ElementTheme.Dark,
            _ => ElementTheme.Default,
        };
        UpdateMaterial();
    }

    private void OnMaterialToggled(object sender, RoutedEventArgs e) => UpdateMaterial();
    private void OnMaterialValueChanged(object sender, Microsoft.UI.Xaml.Controls.Primitives.RangeBaseValueChangedEventArgs e) => UpdateMaterial();

    private void UpdateMaterial()
    {
        if (GlassPanel?.Background is not LiquidGlassBrush brush)
        {
            if (StatusText is not null) StatusText.Text = "High contrast uses the system solid-color brush.";
            return;
        }
        brush.IsEnabled = MaterialSwitch?.IsOn ?? true;
        brush.BlurRadius = BlurSlider?.Value ?? 10;
        brush.RefractionStrength = RefractionSlider?.Value ?? 18;
        brush.DispersionStrength = DispersionSlider?.Value ?? 1.2;
        if (StatusText is not null) StatusText.Text = brush.IsEnabled ? "Native HLSL material active" : "Fallback color active";
    }
}

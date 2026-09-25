using WinUI.LiquidGlass;

namespace LiquidGlassCSharpConsumer;

internal static class ApiSurfaceCompile
{
    // Compile-only coverage for the packaged C#/WinRT projection. This method is
    // never called; CI only verifies type resolution and cross-package projection.
    private static void Validate()
    {
        _ = typeof(LiquidGlassButton);
        _ = typeof(LiquidGlassSlider);
        _ = typeof(LiquidGlassToggleSwitch);
        _ = LiquidGlassToggleSwitch.IsOnProperty;
        _ = LiquidGlassToggleSwitch.HeaderProperty;
        _ = LiquidGlassToggleSwitch.HeaderTemplateProperty;
        _ = LiquidGlassToggleSwitch.OnContentProperty;
        _ = LiquidGlassToggleSwitch.OnContentTemplateProperty;
        _ = LiquidGlassToggleSwitch.OffContentProperty;
        _ = LiquidGlassToggleSwitch.OffContentTemplateProperty;
        _ = typeof(LiquidGlassPasswordBox);
        _ = LiquidGlassPasswordBox.PasswordProperty;
        _ = LiquidGlassPasswordBox.PlaceholderTextProperty;

        var password = new LiquidGlassPasswordBox();
        _ = password.InnerPasswordBox;

        _ = typeof(LiquidGlassSearchBox);
        _ = LiquidGlassSearchBox.MaxSuggestionListHeightProperty;
        _ = LiquidGlassSearchBox.IsSuggestionListOpenProperty;
        _ = LiquidGlassSearchBox.TextMemberPathProperty;
        _ = LiquidGlassSearchBox.TextProperty;
        _ = LiquidGlassSearchBox.UpdateTextOnSelectProperty;
        _ = LiquidGlassSearchBox.PlaceholderTextProperty;
        _ = LiquidGlassSearchBox.HeaderProperty;
        _ = LiquidGlassSearchBox.AutoMaximizeSuggestionAreaProperty;
        _ = LiquidGlassSearchBox.TextBoxStyleProperty;
        _ = LiquidGlassSearchBox.QueryIconProperty;
        _ = LiquidGlassSearchBox.LightDismissOverlayModeProperty;
        _ = LiquidGlassSearchBox.DescriptionProperty;
        _ = LiquidGlassSearchBox.ItemsSourceProperty;
        _ = LiquidGlassSearchBox.ItemTemplateProperty;
        _ = LiquidGlassInteraction.PointerLightingEnabledProperty;
        _ = LiquidGlassPresets.CreateBrush(LiquidGlassPreset.Button);
    }
}

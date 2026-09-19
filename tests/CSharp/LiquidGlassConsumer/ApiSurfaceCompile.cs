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
        _ = typeof(LiquidGlassSearchBox);
        _ = LiquidGlassInteraction.PointerLightingEnabledProperty;
        _ = LiquidGlassPresets.CreateBrush(LiquidGlassPreset.Button);
    }
}

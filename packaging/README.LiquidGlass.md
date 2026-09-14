# WinUI.LiquidGlass

Native C++/WinRT Liquid Glass controls for WinUI 3. The package depends on the matching `WinUI.Composition.Hlsl` runtime and keeps the control layer separate from the lower-level HLSL/Composition engine.

```xml
<PackageReference Include="WinUI.LiquidGlass" Version="1.0.x" />
```

Controls: `LiquidGlassCard`, `LiquidGlassMagnifier`, `LiquidGlassButton`, `LiquidGlassToggleButton`, `LiquidGlassHyperlinkButton`, `LiquidGlassCheckBox`, `LiquidGlassRadioButton`, `LiquidGlassSlider`, `LiquidGlassTextBox`, `LiquidGlassPasswordBox`, `LiquidGlassComboBox`, and `LiquidGlassToggleSwitch`.

Each control exposes `GlassBrush` as a dependency property. The default value is a per-instance `WinUI.Composition.Hlsl.LiquidGlassBrush`, while applications can replace it from XAML or C++/WinRT:

```xml
<glass:LiquidGlassButton Content="Run">
    <glass:LiquidGlassButton.GlassBrush>
        <hlsl:LiquidGlassBrush
            BlurRadius="2"
            RefractionStrength="24"
            DispersionStrength="0.7"
            CornerRadius="12" />
    </glass:LiquidGlassButton.GlassBrush>
</glass:LiquidGlassButton>
```

Templated controls load `ms-appx:///WinUI.LiquidGlass/Themes/Generic.xaml` through their default-style resource URI. Complex input controls intentionally retain the WinUI base templates so keyboard, gamepad, focus, and UI Automation behavior remains platform-native; the glass material is applied to the appropriate native surface at runtime.

The implementation uses C++/WinRT optimized namespace modules. Generated XAML translation units receive the same module-import preamble as the authored control implementation, while the class-named headers required by `module.g.cpp` are thin forwarding headers into the centralized `LiquidGlassControls` module.

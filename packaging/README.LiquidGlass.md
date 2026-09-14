# WinUI.LiquidGlass

Native C++/WinRT Liquid Glass controls for WinUI 3. This package depends on the matching `WinUI.Composition.Hlsl` runtime and exposes per-control `GlassBrush` instances for optical customization.

```xml
<PackageReference Include="WinUI.LiquidGlass" Version="1.0.x" />
```

Controls: `LiquidGlassCard`, `LiquidGlassButton`, `LiquidGlassToggleButton`, `LiquidGlassHyperlinkButton`, `LiquidGlassCheckBox`, `LiquidGlassRadioButton`, `LiquidGlassSlider`, `LiquidGlassTextBox`, `LiquidGlassPasswordBox`, `LiquidGlassComboBox`, and `LiquidGlassToggleSwitch`.

Button/Toggle/Card use liquid-glass-aware templates. Complex input controls intentionally retain the WinUI base templates so keyboard, gamepad, focus, and UI Automation semantics remain platform-native.

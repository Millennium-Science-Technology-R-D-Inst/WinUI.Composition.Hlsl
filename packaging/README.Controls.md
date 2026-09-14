# WinUI.Composition.Hlsl.Controls

Native C++/WinRT Liquid Glass controls for WinUI 3. The package depends on `WinUI.Composition.Hlsl`, so installing the controls package also installs the matching shader/material runtime.

```xml
<PackageReference Include="WinUI.Composition.Hlsl.Controls" Version="1.0.x" />
```

Available controls:

- `LiquidGlassButton`
- `LiquidGlassToggleButton`
- `LiquidGlassCard`
- `LiquidGlassSlider`

The controls use `Themes/Generic.xaml` and `DefaultStyleResourceUri`, following the native XAML control-library pattern. The dictionary also exposes keyed `LiquidGlass*Style` resources that can be applied to stock WinUI controls.

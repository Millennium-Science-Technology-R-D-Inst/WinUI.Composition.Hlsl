# LiquidGlassBrush class

An XAML brush that renders the built-in Liquid Glass material.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v0.1.0-preview.7  
**Assembly:** `WinUI.Composition.Hlsl.dll`


```idl
[default_interface]
runtimeclass LiquidGlassBrush : Microsoft.UI.Xaml.Media.XamlCompositionBrushBase
```

## Properties

| Property | Type | Default | Description |
| --- | --- | --- | --- |
| `IsEnabled` | `Boolean` | `true` | Enables the effect. When false, the brush displays `FallbackColor`. |
| `BlurRadius` | `Double` | `12` | Upstream native Gaussian transmission blur amount. |
| `RefractionStrength` | `Double` | `24` | Edge displacement strength. |
| `DispersionStrength` | `Double` | `1.2` | RGB dispersion strength. |
| `CornerRadius` | `Double` | `12` | Rounded material radius. |
| `BorderThickness` | `Double` | `1` | Border thickness. |
| `HighlightStrength` | `Double` | `0.8` | Highlight intensity. |

The numeric dependency properties use `Double`, matching WinUI XAML text conversion. Values are validated and converted to GPU `float` values internally.

The material uses a native GaussianBlur `CompositionEffectBrush` as the source of the custom HLSL glass brush. The two effects intentionally use separate factories so the private HLSL runtime never has to lower a mixed native/custom factory graph.

## Theme resources

Keep material style separate from `RequestedTheme`. Define the same semantic key for Light and Dark and use a system brush for High Contrast.

```xml
<ResourceDictionary.ThemeDictionaries>
    <ResourceDictionary x:Key="Light">
        <hlsl:LiquidGlassBrush x:Key="CardMaterialBrush" FallbackColor="#CCFFFFFF" />
    </ResourceDictionary>
    <ResourceDictionary x:Key="Dark">
        <hlsl:LiquidGlassBrush x:Key="CardMaterialBrush" FallbackColor="#CC202020" />
    </ResourceDictionary>
    <ResourceDictionary x:Key="HighContrast">
        <SolidColorBrush x:Key="CardMaterialBrush" Color="{ThemeResource SystemColorWindowColor}" />
    </ResourceDictionary>
</ResourceDictionary.ThemeDictionaries>
```

Use `{ThemeResource CardMaterialBrush}` from controls. To switch visual material without changing Light/Dark, update `LiquidGlassBrush.IsEnabled` on the active resource.

## Remarks

The brush checks `CompositionCapabilities.AreEffectsSupported()` when connected. If effects are disabled, initialization fails, or `IsEnabled` is false, it uses `FallbackColor`. It closes replaced Composition brushes when disconnected or rebuilt.



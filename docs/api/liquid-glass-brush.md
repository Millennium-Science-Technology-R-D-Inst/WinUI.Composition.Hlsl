# LiquidGlassBrush class

A XAML brush that renders the built-in Liquid Glass material.

**Namespace:** `WinUI.Composition.Hlsl`  
**Assembly:** `WinUI.Composition.Hlsl.dll`

```idl
[default_interface]
runtimeclass LiquidGlassBrush : Microsoft.UI.Xaml.Media.XamlCompositionBrushBase
```

## Properties

| Property | Type | Default | Description |
| --- | --- | --- | --- |
| `IsEnabled` | `Boolean` | `true` | Enables the effect. When false, the brush displays `FallbackColor`. |
| `BlurRadius` | `Double` | `12` | Gaussian radius in DIPs. Blur affects transmitted content, not the final rounded silhouette. |
| `CornerRadius` | `Double` | `36` | Rounded material radius. |
| `BezelWidth` | `Double` | `32` | Width of the curved optical bezel. |
| `GlassThickness` | `Double` | `50` | Optical thickness used to turn the refracted ray angle into displacement. |
| `RefractiveIndex` | `Double` | `1.5` | IOR (index of refraction) used by Snell's law. |
| `RefractionStrength` | `Double` | `24` | Artistic multiplier over the physically-derived displacement. |
| `DispersionStrength` | `Double` | `1.2` | RGB dispersion strength. |
| `BorderThickness` | `Double` | `1.5` | Bright edge-rim width. |
| `HighlightStrength` | `Double` | `0.85` | Directional specular intensity. |
| `TintOpacity` | `Double` | `0.08` | White tint contribution. |
| `Saturation` | `Double` | `1.25` | Transmitted-color saturation multiplier. |
| `LightAngle` | `Double` | `-0.95` | Directional highlight angle in radians. |

The numeric dependency properties use `Double`, matching WinUI XAML text conversion. Values are validated and converted to GPU `float` values internally.

The material is compiled as one effect graph containing the native Gaussian stage and the custom sampler. The private backend lowers the native upstream graph through a `MaterializedTexture` intermediate, then links the custom sampler into the final consumer. It does not pass a `CompositionEffectBrush` as the source of another `CompositionEffectBrush`.

## Theme resources

Keep material style separate from `RequestedTheme`. Define the same semantic key for Light and Dark and use a system brush for High Contrast.

```xml
<ResourceDictionary.ThemeDictionaries>
    <ResourceDictionary x:Key="Light">
        <hlsl:LiquidGlassBrush
            x:Key="CardMaterialBrush"
            FallbackColor="#CCFFFFFF"
            TintOpacity="0.09" />
    </ResourceDictionary>
    <ResourceDictionary x:Key="Dark">
        <hlsl:LiquidGlassBrush
            x:Key="CardMaterialBrush"
            FallbackColor="#CC202020"
            TintOpacity="0.05"
            Saturation="1.1" />
    </ResourceDictionary>
    <ResourceDictionary x:Key="HighContrast">
        <SolidColorBrush x:Key="CardMaterialBrush" Color="{ThemeResource SystemColorWindowColor}" />
    </ResourceDictionary>
</ResourceDictionary.ThemeDictionaries>
```

Use `{ThemeResource CardMaterialBrush}` from controls. To switch visual material without changing Light/Dark, update `LiquidGlassBrush.IsEnabled` on the active resource.

## Control styling

The C++ demo includes `LiquidGlassControls.xaml`, a deliberately separate prototype resource dictionary with reusable Button, ToggleButton, Slider and card styles. Button and ToggleButton templates expose explicit Normal, PointerOver, Pressed, Disabled, Focused and Checked visual states. Keeping these styles outside the core runtime avoids coupling an experimental control-template ABI to the shader/runtime package; the dictionary can later move to a dedicated `WinUI.Composition.Hlsl.Controls` package.

## Remarks

The brush checks `CompositionCapabilities.AreEffectsSupported()` when connected. If effects are disabled, initialization fails, or `IsEnabled` is false, it uses `FallbackColor`. It closes replaced Composition brushes when disconnected or rebuilt.

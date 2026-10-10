# LiquidGlassBrush class

A XAML brush that renders the built-in Liquid Glass material.

**Namespace:** `WinUI.Composition.Hlsl`  
**Assembly:** `WinUI.Composition.Hlsl.dll`

```idl
[default_interface]
runtimeclass LiquidGlassBrush : Microsoft.UI.Xaml.Media.XamlCompositionBrushBase
```

## Properties

Every author-facing material parameter is exposed as a XAML dependency property, so it can be set from XAML, bound, or animated. Numeric dependency properties use `Double` for normal WinUI XAML conversion and are validated before being converted to GPU `float` values.

| Property | Type | Default | Description |
| --- | --- | --- | --- |
| `IsEnabled` | `Boolean` | `true` | Enables the effect. When false, the brush displays inherited `FallbackColor`. |
| `Material` | `LiquidGlassMaterial` | — | Gets the live Composition material instance while connected. Read-only. |
| `BlurRadius` | `Double` | `12` | Gaussian transmission blur radius in DIPs. |
| `RefractionStrength` | `Double` | `24` | Artistic multiplier over physically-derived refraction displacement. |
| `DispersionStrength` | `Double` | `1.2` | RGB chromatic-separation strength. |
| `CornerRadius` | `Double` | `36` | WinUI-compatible rounded material radius in DIPs. |
| `ShapeRoundness` | `Double` | `4` | Optical corner roundness in the range 2–8. `2` gives a circular corner-normal field, `4` is the default squircle-like response, and larger values make corner refraction progressively squarer. It does not replace the final WinUI rounded-rectangle coverage silhouette. |
| `RasterizationScale` | `Double` | `1` | Logical-DIP to raster-pixel scale used by the blur and custom sampler. Set it to the target `XamlRoot.RasterizationScale` when using a raw brush. `WinUI.LiquidGlass` controls synchronize it automatically, except the Slider visual model which already applies its exact Kube raster scaling directly. |
| `BorderThickness` | `Double` | `1.5` | Bright edge-rim width. |
| `HighlightStrength` | `Double` | `0.85` | Directional specular intensity. |
| `EdgeSoftness` | `Double` | `1` | SDF edge/antialias transition width. |
| `MaterialOpacity` | `Double` | `1` | Overall material opacity. |
| `BezelWidth` | `Double` | `32` | Width of the curved optical bezel. |
| `GlassThickness` | `Double` | `50` | Optical thickness used to convert refracted ray angle into displacement. |
| `RefractiveIndex` | `Double` | `1.5` | IOR (index of refraction) used by Snell's law. |
| `TintOpacity` | `Double` | `0.08` | Tint contribution. |
| `Saturation` | `Double` | `1.25` | Transmitted-color saturation multiplier. |
| `LightAngle` | `Double` | `-0.95` | Directional-light angle in radians. |
| `SurfaceProfile` | `LiquidGlassSurfaceProfile` | `ConvexSquircle` | Selects the 1D optical bezel height profile; this is independent from the 2D optical corner `ShapeRoundness`. |
| `MagnificationStrength` | `Double` | `0` | Additional magnification displacement. |
| `HighlightSharpness` | `Double` | `1.5` | Directional highlight concentration. |
| `TintRed` | `Double` | `1` | Tint red component, normalized 0–1. |
| `TintGreen` | `Double` | `1` | Tint green component, normalized 0–1. |
| `TintBlue` | `Double` | `1` | Tint blue component, normalized 0–1. |
| `InnerShadowStrength` | `Double` | `0.09` | Inner-bezel darkening strength. |
| `SpecularSaturation` | `Double` | `4` | Saturation applied to the specular color response. |
| `SpecularWidth` | `Double` | `1` | Specular edge-profile width in DIPs. |
| `Contrast` | `Double` | `1` | Contrast multiplier around mid-grey. |
| `Exposure` | `Double` | `0` | Exposure in stops; internally evaluated through `exp2`. |

For exact validation ranges and Composition-level semantics, see [LiquidGlassMaterial](liquid-glass-material.md).

The custom-sampler ABI exposes physical bitmap dimensions and reciprocal texel size, not the original XAML logical size, so the shader cannot infer display DPI on its own. A raw `LiquidGlassBrush` therefore defaults to `RasterizationScale="1"`; applications rendering it above 100% display scale should set or bind the scale explicitly. Avoid sharing one brush instance across simultaneous XAML roots with different rasterization scales.

The private pointer interaction field is intentionally not duplicated as XAML dependency properties. Pointer coordinates and velocity are high-frequency state and are written directly to the live Composition effect by `WinUI.LiquidGlass` interaction helpers.

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

## Remarks

The brush checks `CompositionCapabilities.AreEffectsSupported()` when connected. If effects are disabled, initialization fails, or `IsEnabled` is false, it uses `FallbackColor`. It closes replaced Composition resources when disconnected or rebuilt.

# LiquidGlassMaterial class

Provides a Composition-level instance of the built-in Liquid Glass effect.

The material is described as a Composition pipeline with a separable native-backed blur stage feeding the custom HLSL sampler. The runtime materializes the transmitted backdrop before arbitrary UV sampling so refraction, dispersion, and magnification can operate on a stable source texture.

The built-in shader models the glass edge as an optical bezel. For every destination pixel it evaluates a rounded-rectangle signed-distance field (SDF), derives an optical normal, evaluates the selected surface-height profile, obtains the surface slope, applies Snell refraction using `RefractiveIndex`, and converts the refracted ray through `GlassThickness` into a sampling displacement. `RefractionStrength` remains an artistic multiplier over that physically-derived displacement.

The final rounded-rectangle coverage mask is evaluated after the transmitted backdrop has been sampled. Consequently `BlurRadius` changes the transmitted image but does not blur the material's own `CornerRadius` silhouette.

The built-in LiquidGlass shader is compiled at build time with the Visual C++/Windows SDK FXC MSBuild task as an SM4 shader-linking library. The generated DXBC bytecode is embedded in the native DLL; runtime `D3DCompile` remains available only for dynamic source-based `HlslEffect` APIs.

**Namespace:** `WinUI.Composition.Hlsl`  
**Assembly:** `WinUI.Composition.Hlsl.dll`

```csharp
public LiquidGlassMaterial(Compositor compositor);
```

## Properties

`LiquidGlassMaterial` currently exposes 25 author-facing material parameters. They are also mirrored by `LiquidGlassBrush` as XAML dependency properties unless noted otherwise.

| Property | Type | Range | Default | Description |
| --- | --- | --- | --- | --- |
| `EffectBrush` | `HlslEffectBrush` | — | — | Gets the final live custom glass brush. Read-only. |
| `BlurRadius` | `float` | 0–64 | `12` | Gaussian transmission blur radius in DIPs. |
| `RefractionStrength` | `float` | 0–128 | `24` | Artistic multiplier applied after physically-derived Snell displacement. |
| `DispersionStrength` | `float` | 0–16 | `1.2` | RGB sampling separation around the optical bezel. |
| `CornerRadius` | `float` | 0–512 | `36` | Radius of the final WinUI-compatible rounded-rectangle silhouette. |
| `BorderThickness` | `float` | 0–32 | `1.5` | Width of the bright material rim in DIPs. |
| `HighlightStrength` | `float` | 0–4 | `0.85` | Directional specular intensity. |
| `EdgeSoftness` | `float` | 0.25–16 | `1` | Width of the final SDF antialias/edge transition in DIPs. |
| `MaterialOpacity` | `float` | 0–1 | `1` | Overall glass-surface opacity. |
| `BezelWidth` | `float` | 1–256 | `32` | Width of the curved optical edge before the interior surface becomes flat. |
| `GlassThickness` | `float` | 0–256 | `50` | Optical travel thickness used to turn refracted ray angle into screen displacement. |
| `RefractiveIndex` | `float` | 1–3.5 | `1.5` | IOR (index of refraction) used by Snell's law. `1` approaches no physical bending. |
| `TintOpacity` | `float` | 0–1 | `0.08` | Amount of tint mixed into transmitted light. |
| `Saturation` | `float` | 0–4 | `1.25` | Saturation multiplier applied to transmitted color. |
| `LightAngle` | `float` | -2π–2π | `-0.95` | Directional-light angle in radians. |
| `SurfaceProfile` | `LiquidGlassSurfaceProfile` | enum | `ConvexSquircle` | Selects `ConvexSquircle`, `ConvexCircle`, `Concave`, or `Lip` bezel height profile. |
| `MagnificationStrength` | `float` | 0–128 | `0` | Additional backdrop magnification displacement. |
| `HighlightSharpness` | `float` | 0.25–64 | `1.5` | Angular concentration exponent for the directional highlight. |
| `TintRed` | `float` | 0–1 | `1` | Red component of the tint color. |
| `TintGreen` | `float` | 0–1 | `1` | Green component of the tint color. |
| `TintBlue` | `float` | 0–1 | `1` | Blue component of the tint color. |
| `InnerShadowStrength` | `float` | 0–1 | `0.09` | Darkens the inner bezel to reinforce optical depth. |
| `SpecularSaturation` | `float` | 0–50 | `4` | Saturation applied to the color feeding the specular response. |
| `SpecularWidth` | `float` | 0.25–32 | `1` | Width of the specular edge profile in DIPs. |
| `Contrast` | `float` | 0–4 | `1` | Contrast multiplier around mid-grey. |
| `Exposure` | `float` | -4–4 | `0` | Exposure in stops; the shader applies `exp2(Exposure)`. |

The high-frequency pointer field and `RefractionNormalization` are private effect controls, not authoring properties. Interaction helpers update them directly on the existing Composition effect rather than invalidating XAML dependency properties on every pointer move.

All author-facing shader properties are registered as animatable Composition effect properties; changing them does not require rebuilding the effect graph. `BlurRadius` updates the existing separable blur brushes rather than recreating the graph.

## Example

```csharp
var material = new LiquidGlassMaterial(compositor)
{
    BlurRadius = 12,
    CornerRadius = 28,
    BezelWidth = 24,
    GlassThickness = 48,
    RefractiveIndex = 1.5f,
    RefractionStrength = 24,
    DispersionStrength = 1.0f,
    TintOpacity = 0.08f,
    Saturation = 1.2f,
};

spriteVisual.Brush = material.EffectBrush.Brush;
```

For interactive lighting, compute `atan2(pointerY - centerY, pointerX - centerX)` and assign the result to `LightAngle`. Only the constant-buffer value changes; the blur/custom-sampler graph remains intact.

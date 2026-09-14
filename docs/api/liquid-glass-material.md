# LiquidGlassMaterial class

Provides a Composition-level instance of the built-in Liquid Glass effect.

The material is described as one Composition effect graph: native D2D Gaussian blur -> custom HLSL sampler. The runtime's `MaterializedTexture` lowering emits an explicit intermediate texture boundary for the native upstream graph so the custom sampler can perform arbitrary UV sampling. The custom sampler remains linked into the final output subgraph instead of being rendered into the Gaussian blur's internal prescale target.

The built-in shader now models the glass edge as a convex optical bezel instead of deriving displacement only from an artistic edge falloff. For every destination pixel it evaluates the rounded-rectangle signed-distance field (SDF), derives a surface normal, evaluates a convex height profile, obtains the surface slope, applies Snell refraction using `RefractiveIndex`, and converts the angular difference into a sampling displacement through `GlassThickness`. `RefractionStrength` remains as an artistic multiplier so existing callers retain a direct strength control.

The final rounded-rectangle coverage mask is evaluated after the blurred backdrop has been sampled. Consequently `BlurRadius` changes the transmitted image but does not blur the material's own `CornerRadius` silhouette.

The built-in LiquidGlass shader is compiled at build time with the Visual C++/Windows SDK FXC MSBuild task as an SM4 shader-linking library. The generated DXBC bytecode is embedded in the native DLL; runtime `D3DCompile` remains available only for dynamic source-based `HlslEffect` APIs.

**Namespace:** `WinUI.Composition.Hlsl`  
**Assembly:** `WinUI.Composition.Hlsl.dll`

```csharp
public LiquidGlassMaterial(Compositor compositor);
```

## Properties

| Property | Type | Range | Default | Description |
| --- | --- | --- | --- | --- |
| `EffectBrush` | `HlslEffectBrush` | — | — | Gets the final custom glass brush used by Composition visuals. |
| `BlurRadius` | `float` | 0–64 | `12` | Gaussian radius in DIPs; converted to D2D standard deviation (`radius / 3`). |
| `CornerRadius` | `float` | 0–512 | `36` | Radius of the final rounded-rectangle material silhouette. |
| `BezelWidth` | `float` | 1–256 | `32` | Width of the curved optical edge before the surface becomes flat. |
| `GlassThickness` | `float` | 0–256 | `50` | Optical travel thickness used when converting refraction angle into displacement. |
| `RefractiveIndex` | `float` | 1–3.5 | `1.5` | IOR (index of refraction) used by Snell's law. `1` approaches no physical bending. |
| `RefractionStrength` | `float` | 0–128 | `24` | Artistic multiplier applied after physically-derived displacement. |
| `DispersionStrength` | `float` | 0–16 | `1.2` | RGB sampling separation, concentrated around the optical bezel. |
| `BorderThickness` | `float` | 0–32 | `1.5` | Width of the bright material rim. |
| `HighlightStrength` | `float` | 0–4 | `0.85` | Directional specular and inner-rim intensity. |
| `TintOpacity` | `float` | 0–1 | `0.08` | Amount of white tint mixed into transmitted light. |
| `Saturation` | `float` | 0–4 | `1.25` | Saturation multiplier applied to the transmitted color. |
| `LightAngle` | `float` | -2π–2π | `-0.95` | Direction, in radians, used by the bezel specular term. It is animatable and suitable for pointer-driven lighting. |

All properties above are animatable Composition effect properties; changing them does not require rebuilding the effect graph.

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

For interactive lighting, compute `atan2(pointerY - centerY, pointerX - centerX)` and assign the result to `LightAngle`. Only the constant-buffer value changes; the materialized blur/custom-sampler graph remains intact.

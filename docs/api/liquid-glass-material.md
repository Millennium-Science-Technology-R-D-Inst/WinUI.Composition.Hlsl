# LiquidGlassMaterial class

Provides a Composition-level instance of the built-in Liquid Glass effect.

The material is described as a Composition pipeline with a separable native-backed blur, a two-input transmission post-processing stage, and the custom HLSL sampler. The runtime materializes the transmitted backdrop before arbitrary UV sampling so refraction, dispersion, and magnification can operate on a stable source texture.

The built-in shader models the glass edge as an optical bezel. For every destination pixel it evaluates a rounded-rectangle signed-distance field (SDF), derives an optical normal, evaluates the selected surface-height profile, obtains the surface slope, applies Snell refraction using `RefractiveIndex`, and converts the refracted ray through `GlassThickness` into a sampling displacement. `RefractionStrength` remains an artistic multiplier over that physically-derived displacement.

`CornerRadius` continues to define the final WinUI-compatible rounded-rectangle coverage silhouette. `ShapeRoundness` independently controls the two-dimensional optical normal used in generic rounded corners: `2` is circular, `4` is the default squircle-like response, and values up to `8` make the optical corner progressively squarer. This is deliberately separate from `SurfaceProfile::ConvexSquircle`, which describes the one-dimensional bezel height cross-section rather than the XY footprint.

The custom-sampler coordinate system is raster-pixel based. `RasterizationScale` converts author-facing DIP geometry into that raster space, including the separable blur radius, rounded geometry, bezel width, glass thickness, edge softness, and specular width. `RefractionNormalization` intentionally remains in authored units so optical displacement scales exactly once. Composition-only callers should set the scale for the target surface; `LiquidGlassBrush` and the first-party controls provide the XAML-facing path.

The final rounded-rectangle coverage mask is evaluated after the transmitted backdrop has been sampled. Consequently `BlurRadius` changes the transmitted image but does not blur the material's own `CornerRadius` silhouette.

The built-in LiquidGlass shader is compiled at build time with the Visual C++/Windows SDK FXC MSBuild task as an SM4 shader-linking library. The generated DXBC bytecode is embedded in the native DLL; runtime `D3DCompile` remains available only for dynamic source-based `HlslEffect` APIs.

**Namespace:** `WinUI.Composition.Hlsl`  
**Assembly:** `WinUI.Composition.Hlsl.dll`

```csharp
public LiquidGlassMaterial(Compositor compositor);
```

## Properties

`LiquidGlassMaterial` currently exposes 43 author-facing material parameters. They are also mirrored by `LiquidGlassBrush` as XAML dependency properties unless noted otherwise.

| Property | Type | Range | Default | Description |
| --- | --- | --- | --- | --- |
| `EffectBrush` | `HlslEffectBrush` | — | — | Gets the final live custom glass brush. Read-only. |
| `PostProcessingEffectBrush` | `HlslEffectBrush` | — | — | Gets the preceding transmission stage for direct Composition color animations. Read-only. |
| `BlurRadius` | `float` | 0–64 | `12` | Gaussian transmission blur radius in DIPs. |
| `RefractionStrength` | `float` | 0–128 | `24` | Artistic multiplier applied after physically-derived Snell displacement. |
| `DispersionStrength` | `float` | 0–16 | `1.2` | RGB sampling separation around the optical bezel. |
| `CornerRadius` | `float` | 0–512 | `36` | Radius of the final WinUI-compatible rounded-rectangle silhouette in DIPs. |
| `ShapeRoundness` | `float` | 2–8 | `4` | Roundness of the generic corner optical-normal field. `2` is circular; `4` is the default squircle-like response; larger values produce a squarer optical corner without changing the final WinUI clip geometry. |
| `RasterizationScale` | `float` | 0.01–16 | `1` | Raster pixels per authored logical DIP. Set this for high-DPI targets so blur and optical geometry match the host's logical dimensions. |
| `BorderThickness` | `float` | 0–32 | `1.5` | Width of the bright material rim in DIPs. |
| `HighlightStrength` | `float` | 0–4 | `0.85` | Directional specular intensity. |
| `EdgeSoftness` | `float` | 0.25–16 | `1` | Width of the final SDF antialias/edge transition in DIPs. |
| `MaterialOpacity` | `float` | 0–1 | `1` | Overall glass-surface opacity. |
| `BezelWidth` | `float` | 1–256 | `32` | Width in DIPs of the curved optical edge before the interior surface becomes flat. |
| `GlassThickness` | `float` | 0–256 | `50` | Optical travel thickness in authored logical units used to turn refracted ray angle into screen displacement. |
| `RefractiveIndex` | `float` | 1–3.5 | `1.5` | IOR (index of refraction) used by Snell's law. `1` approaches no physical bending. |
| `TintOpacity` | `float` | 0–1 | `0.08` | Amount of tint mixed into transmitted light. |
| `Saturation` | `float` | 0–4 | `1.25` | Saturation multiplier applied to transmitted color. |
| `LightAngle` | `float` | -2π–2π | `-0.95` | Directional-light angle in radians. |
| `SurfaceProfile` | `LiquidGlassSurfaceProfile` | enum | `ConvexSquircle` | Selects `ConvexSquircle`, `ConvexCircle`, `Concave`, or `Lip` for the 1D bezel height profile; it is independent from the 2D optical `ShapeRoundness`. |
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
| `DispersionRange` | `float` | 0–1 | `1` | Fraction of the bezel occupied by chromatic separation. |
| `RefFresnelRange` | `float` | 0–100 | `30` | Fresnel rim falloff range in DIPs. Zero disables the band. |
| `RefFresnelHardness` | `float` | 0–100 | `20` | Spatial hardness of the Fresnel band. |
| `RefFresnelFactor` | `float` | 0–100 | `0` | Fresnel intensity. Zero preserves existing presets. |
| `GlareRange` | `float` | 0–100 | `30` | Glare band range in DIPs. |
| `GlareHardness` | `float` | 0–100 | `20` | Spatial hardness of the glare band. |
| `GlareFactor` | `float` | 0–120 | `0` | Directional glare intensity, independent from the Kube specular layer. |
| `GlareConvergence` | `float` | 0–100 | `50` | Angular concentration of glare. |
| `GlareOppositeFactor` | `float` | 0–100 | `80` | Opposite-side glare intensity relative to the front lobe. |
| `GlareAngle` | `float` | -180–180 | `-45` | Glare direction in degrees; `LightAngle` remains radians for the existing specular layer. |
| `ShapeRadius` | `float` | -1–1 | `-1` | Relative radius of the shorter inset half-side. A negative value uses the existing absolute `CornerRadius`; 0–1 overrides it. |
| `Magnification` | `float` | 1–3 | `1` | Uniform backdrop zoom about its content center, independent from localized `MagnificationStrength`. |
| `BloomAmount` | `float` | 0–1 | `1` | Blurred fraction: 0 selects raw backdrop, 1 selects the full blur. |
| `Brightness` | `float` | -1–1 | `0` | Additive straight-color brightness before contrast. |
| `Temperature` | `float` | -1–1 | `0` | Cool/warm channel gain. |
| `Vibrance` | `float` | 0–1 | `0` | Extra saturation weighted toward low-chroma colors. |

The high-frequency pointer field and `RefractionNormalization` are private effect controls, not authoring properties. Interaction helpers update them directly on the existing Composition effect rather than invalidating XAML dependency properties on every pointer move.

All author-facing shader properties are registered as animatable Composition effect properties on their owning stage; changing them does not require rebuilding the effect graph. `BlurRadius` and `RasterizationScale` update the existing separable blur brushes rather than recreating the graph.

Transmission color runs in the order raw/blur mix → EV exposure → temperature →
brightness → contrast → saturation/vibrance → clamp. It unpremultiplies before
color adjustment and restores premultiplication afterward. `EffectBrush` exposes
the final optics brush; use the Material properties or `PostProcessingEffectBrush` to control the preceding
post-processing stage. `Exposure` retains this library's EV convention: to match
a reference multiplicative exposure gain, use `log2(gain)`.

## Example

```csharp
var material = new LiquidGlassMaterial(compositor)
{
    BlurRadius = 12,
    CornerRadius = 28,
    ShapeRoundness = 5,
    RasterizationScale = 1.5f,
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

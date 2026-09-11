# LiquidGlassMaterial class

Provides a Composition-level instance of the built-in Liquid Glass effect.

The material uses two separate Composition effect-brush passes: a native Gaussian blur first materializes and blurs the backdrop, then the custom HLSL sampler performs refraction, dispersion, shape, and highlights. Keeping the native and custom effects in separate factories avoids unsupported mixed native/custom lowering and gives the custom sampler a real texture source.

The built-in LiquidGlass shader is compiled at build time with the Visual C++/Windows SDK FXC MSBuild task as an SM4 shader-linking library. The generated DXBC bytecode is embedded in the native DLL; runtime `D3DCompile` remains available only for dynamic source-based `HlslEffect` APIs.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v0.1.0-preview.7  
**Assembly:** `WinUI.Composition.Hlsl.dll`

```csharp
public LiquidGlassMaterial(Compositor compositor);
```

## Properties

| Property | Type | Range | Description |
| --- | --- | --- | --- |
| `EffectBrush` | `HlslEffectBrush` | — | Gets the final custom glass brush used by Composition visuals. |
| `BlurRadius` | `float` | 0–64 | Controls the upstream native Gaussian transmission blur. |
| `RefractionStrength` | `float` | 0–128 | Controls displacement near the material edge. |
| `DispersionStrength` | `float` | 0–16 | Controls RGB channel separation. |
| `CornerRadius` | `float` | 0–512 | Controls rounded-rectangle geometry. |
| `BorderThickness` | `float` | 0–32 | Controls material border width. |
| `HighlightStrength` | `float` | 0–4 | Controls highlight intensity. |

## Example

```csharp
var material = new LiquidGlassMaterial(compositor)
{
    BlurRadius = 10,
    RefractionStrength = 18,
};
spriteVisual.Brush = material.EffectBrush.Brush;
```

# LiquidGlassMaterial class

Provides a Composition-level instance of the built-in Liquid Glass effect.

The material uses a native D2D Gaussian brush pass followed by the custom HLSL sampler. The native stage uses balanced optimization and hard borders; the brush boundary materializes its output before refraction, dispersion, shape, and highlights.

The built-in LiquidGlass shader is compiled at build time with the Visual C++/Windows SDK FXC MSBuild task as an SM4 shader-linking library. The generated DXBC bytecode is embedded in the native DLL; runtime `D3DCompile` remains available only for dynamic source-based `HlslEffect` APIs.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v0.1.0-preview.8  
**Assembly:** `WinUI.Composition.Hlsl.dll`

```csharp
public LiquidGlassMaterial(Compositor compositor);
```

## Properties

| Property | Type | Range | Description |
| --- | --- | --- | --- |
| `EffectBrush` | `HlslEffectBrush` | — | Gets the final custom glass brush used by Composition visuals. |
| `BlurRadius` | `float` | 0–64 | Controls the Gaussian radius in DIPs; internally converted to D2D standard deviation (`radius / 3`). |
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

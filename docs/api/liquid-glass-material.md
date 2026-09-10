# LiquidGlassMaterial class

Provides a Composition-level instance of the built-in Liquid Glass effect.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v0.1.0-preview.7  
**Assembly:** `WinUI.Composition.Hlsl.dll`


```csharp
public LiquidGlassMaterial(Compositor compositor);
```

## Properties

| Property | Type | Range | Description |
| --- | --- | --- | --- |
| `EffectBrush` | `HlslEffectBrush` | — | Gets the brush used by Composition visuals. |
| `BlurRadius` | `float` | 0–64 | Controls local transmission blur. |
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
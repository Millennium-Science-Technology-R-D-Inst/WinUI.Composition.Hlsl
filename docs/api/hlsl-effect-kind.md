# HlslEffectKind enum

Identifies the shader contract represented by an [HlslEffect](hlsl-effect.md).

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v0.1.0-preview.8  
**Assembly:** `WinUI.Composition.Hlsl.dll`


```csharp
public enum HlslEffectKind
```

## Fields

| Name | Value | Description |
| --- | ---: | --- |
| `Color` | 0 | The shader exports `float4 PSBody(float4 color)`. |
| `Sampler` | 1 | The shader defines `float4 Shade(float2 uv, float4 samplerDataExt)`. |

## Applies to

Windows App SDK 2.4; WinUI.Composition.Hlsl 0.1.0-preview.8.



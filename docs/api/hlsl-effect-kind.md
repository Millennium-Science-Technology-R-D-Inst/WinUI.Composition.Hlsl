# HlslEffectKind enum

Identifies the shader and graph-lowering contract represented by an [HlslEffect](hlsl-effect.md).

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v1.0.0  
**Assembly:** `WinUI.Composition.Hlsl.dll`

```csharp
public enum HlslEffectKind
```

## Fields

| Name | Value | Public shader contract | Composition behavior |
| --- | ---: | --- | --- |
| `Color` | 0 | `float4 PSBody(float4 color)` | Linked color transform. |
| `Sampler` | 1 | `float4 Shade(float2 uv, float4 samplerDataExt)` | Custom sampler using the linked Composition sampler contract. |
| `MaterializedSampler` | 2 | `float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData)` | Materializes one upstream native effect graph to a texture before custom sampling. |

For source-based and `<HlslCompositionShader>` builds, the library generates the private `PSBody*` edge-mode wrappers. `MaterializedSampler` also generates the identity `MaterializeColor` helper required by the currently validated materialized graph lowering.

`MaterializedSampler` does not imply arbitrary multi-input textures or multiple custom HLSL nodes. The current backend accepts one named source and one isolated terminal custom sampler.

## Applies to

Windows App SDK 2.4 baseline; WinUI.Composition.Hlsl 1.0.0.

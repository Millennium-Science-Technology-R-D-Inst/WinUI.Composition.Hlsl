# HlslEffect class

Describes an immutable HLSL-backed Composition effect.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v1.0.0  
**Assembly:** `WinUI.Composition.Hlsl.dll`

## Properties

| Property | Type | Description |
| --- | --- | --- |
| `Id` | `Guid` | Deterministic or explicitly supplied effect identifier. |
| `Kind` | `HlslEffectKind` | `Color`, `Sampler`, or `MaterializedSampler`. |
| `SourceName` | `String` | The single public source-parameter name. |
| `IsPrecompiled` | `Boolean` | `true` when the effect owns DXBC instead of HLSL source. |
| `PropertyNames` | `IVectorView<String>` | Declared animatable scalar property names. |

## Creating effects

`CreateColorTransform` derives an ID and uses the color ABI:

```hlsl
export float4 PSBody(float4 color);
```

`CreateCustomSampler` derives an ID and uses the lightweight sampler ABI:

```hlsl
float4 Shade(float2 uv, float4 samplerDataExt);
```

`CreateMaterializedSampler` uses the materialized sampler ABI:

```hlsl
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData);
```

The materialized form is intended for `native Composition effect graph -> materialized texture -> HLSL sampler`. It exposes both sampler metadata structures used by the validated private linker path. It currently supports one upstream source and one isolated terminal custom shader node.

`CreateColor`, `CreateSampler`, and `CreateMaterializedSampler` accept an explicit GUID. Use explicit IDs only when an external contract owns the identifier; registering a different schema under the same private effect GUID is rejected.

The `*WithProperties` overloads add a named source and `HlslFloatProperty` descriptors. Properties become entries in `UserConstants` and can be animated through the underlying Composition property set.

## Precompiled effects

`CreateCompiledColor`, `CreateCompiledSampler`, `CreateCompiledMaterializedSampler`, and their `*WithProperties` forms consume an immutable [HlslShaderLibrary](hlsl-shader-library.md). They do not call `D3DCompile` during effect creation.

For arbitrary externally supplied DXBC, the library performs one-time defensive reflection to verify required exports and scalar constant-buffer layout before the private Composition runtime consumes the payload. This is not per-frame validation.

## Standard graph node methods

### CreateGraphicsEffect

```csharp
public IGraphicsEffect CreateGraphicsEffect();
```

Creates the standard Windows Graphics Effects node represented by this description. This makes HLSL usable through normal `Compositor.CreateEffectFactory` graph construction rather than through a separate swap-chain rendering path.

### CreateGraphicsEffectWithSource

```csharp
public IGraphicsEffect CreateGraphicsEffectWithSource(IGraphicsEffectSource source);
```

Creates the node with an explicit upstream graphics-effect source. This is the preferred API when inserting `MaterializedSampler` after an existing native Composition/D2D effect graph.

Example shape:

```text
CompositionEffectSourceParameter
        -> native blur/transform/etc.
        -> HlslEffect.CreateGraphicsEffectWithSource(...)
        -> Compositor.CreateEffectFactory(...)
        -> CompositionEffectBrush
        -> XamlCompositionBrushBase
```

No `SwapChainPanel`, app-owned swap chain, or separate overlay visual is required.

## Performance guidance

For production shaders, prefer the NuGet `<HlslCompositionShader>` build item or persist the `Bytecode` returned from [HlslCompiler](hlsl-compiler.md). Build-time FXC catches source/entry-point failures and avoids runtime compilation. Use direct Composition animations on [HlslEffectBrush.Properties](hlsl-effect-brush.md) for high-frequency updates rather than calling `SetFloat` every frame.

## Current scope

The public model intentionally remains narrow: one named source, scalar public properties, fixed linker contracts, and at most one custom HLSL node in the current lowered graph. Multiple custom texture sources, arbitrary custom-node chains, and guessed private vector/matrix metadata are not exposed until the corresponding private ABI is verified.

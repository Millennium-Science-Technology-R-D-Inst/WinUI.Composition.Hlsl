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

`CreateColorTransform` derives an ID and uses `float4 PSBody(float4 color)`.

`CreateCustomSampler` derives an ID and uses:

```hlsl
float4 Shade(float2 uv, float4 samplerDataExt);
```

`CreateCustomMaterializedSampler` derives an ID and uses:

```hlsl
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData);
```

The explicit-GUID counterparts are `CreateColor`, `CreateSampler`, and `CreateMaterializedSampler`. Passing `Guid.Empty` also derives a deterministic ID, but the `CreateCustom*` forms are clearer for application-owned source shaders.

`MaterializedSampler` is intended for `native Composition effect graph -> materialized texture -> HLSL sampler`. It currently supports one upstream source and one isolated terminal custom shader node.

The `*WithProperties` overloads add a named source and `HlslFloatProperty` descriptors. Properties become entries in `UserConstants` and can be animated through the underlying Composition property set.

## Precompiled effects

`CreateCompiledColor`, `CreateCompiledSampler`, `CreateCompiledMaterializedSampler`, and their `*WithProperties` forms consume an immutable [HlslShaderLibrary](hlsl-shader-library.md). They do not call `D3DCompile` during effect creation.

For arbitrary externally supplied DXBC, the library performs one-time defensive reflection to verify required exports and scalar constant-buffer layout before the private Composition runtime consumes the payload. This is not per-frame validation.

## Property paths for standard Composition factories

```csharp
public string GetPropertyPath(string name);
public IReadOnlyList<string> GetAnimatablePropertyPaths();
```

These methods expose the exact effect property paths required by standard `Compositor.CreateEffectFactory(graph, animatableProperties)`. They are especially useful when `CreateGraphicsEffectWithSource` is used to insert HLSL into a graph assembled directly with Windows Graphics Effects APIs.

## Standard graph node methods

```csharp
public IGraphicsEffect CreateGraphicsEffect();
public IGraphicsEffect CreateGraphicsEffectWithSource(IGraphicsEffectSource source);
```

These create standard Windows Graphics Effects nodes. The second form embeds an explicit upstream effect/source, enabling graphs such as:

```text
CompositionEffectSourceParameter
        -> native blur/transform/etc.
        -> HlslEffect.CreateGraphicsEffectWithSource(...)
        -> Compositor.CreateEffectFactory(graph, effect.GetAnimatablePropertyPaths())
        -> CompositionEffectBrush
        -> XamlCompositionBrushBase
```

No `SwapChainPanel`, app-owned swap chain, or separate overlay visual is required.

## Performance guidance

For production shaders, prefer the NuGet `<HlslCompositionShader>` build item or persist the `Bytecode` returned from [HlslCompiler](hlsl-compiler.md). Build-time FXC catches source/entry-point failures and avoids runtime compilation. Use direct Composition animations on [HlslEffectBrush](hlsl-effect-brush.md) for high-frequency updates rather than calling `SetFloat` every frame.

## Current scope

The public model intentionally remains narrow: one named source, scalar public properties, fixed linker contracts, and at most one custom HLSL node in the current lowered graph. Multiple custom texture sources, arbitrary custom-node chains, and guessed private vector/matrix metadata are not exposed until the corresponding private ABI is verified.

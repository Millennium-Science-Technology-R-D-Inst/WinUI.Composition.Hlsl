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

`MaterializedSampler` is intended for `native Composition effect graph -> materialized texture -> HLSL sampler`. Public materialized execution is currently enabled only on the validated x64 adapter. `HlslCompiler` can still precompile/cache the materialized DXBC contract on other architectures.

The `*WithProperties` overloads add a named source and `HlslFloatProperty` descriptors. Properties become entries in `UserConstants` and can be animated through the underlying Composition property set.

## Precompiled effects

`CreateCompiledColor`, `CreateCompiledSampler`, `CreateCompiledMaterializedSampler`, and their `*WithProperties` forms consume an immutable [HlslShaderLibrary](hlsl-shader-library.md). They do not call `D3DCompile` during effect creation.

For arbitrary externally supplied DXBC, the library performs one-time defensive reflection to verify required exports and scalar constant-buffer layout before the private Composition runtime consumes the payload. This is not per-frame validation.

## Property paths for standard Composition factories

```csharp
public string GetPropertyPath(string name);
public IReadOnlyList<string> GetAnimatablePropertyPaths();
```

These methods expose the exact property paths required by `Compositor.CreateEffectFactory(graph, animatableProperties)`, so standard Composition graph construction does not need to know the library's internal effect name.

## Standard graph node methods

```csharp
public IGraphicsEffect CreateGraphicsEffect();
public IGraphicsEffect CreateGraphicsEffectWithSource(IGraphicsEffectSource source);
```

`CreateGraphicsEffect` produces the standard Windows Graphics Effects node for the description.

`CreateGraphicsEffectWithSource` accepts an explicit source. `Color` and ordinary `Sampler` can use a source parameter/brush source, but the current `SingleCustom` backend deliberately rejects another `IGraphicsEffect` as their upstream node because mixed native/custom subgraph linking has not been verified. Use `MaterializedSampler` when the upstream source is a native effect graph:

```text
CompositionEffectSourceParameter
        -> native blur/transform/etc.
        -> MaterializedSampler.CreateGraphicsEffectWithSource(upstream)
        -> Compositor.CreateEffectFactory(graph, effect.GetAnimatablePropertyPaths())
        -> CompositionEffectBrush
        -> XamlCompositionBrushBase
```

This keeps the graph inside Composition/XAML without `SwapChainPanel`, an app-owned swap chain, or an overlay renderer.

## Performance guidance

For production shaders, prefer the NuGet `<HlslCompositionShader>` build item or persist the `Bytecode` returned from [HlslCompiler](hlsl-compiler.md). Build-time FXC catches source/entry-point failures and avoids runtime compilation. Use direct Composition animations on [HlslEffectBrush](hlsl-effect-brush.md) for high-frequency updates rather than calling `SetFloat` every frame.

## Current scope

The public model intentionally remains narrow: one named source, scalar public properties, fixed linker contracts, and at most one custom HLSL node in the current lowered graph. Multiple custom texture sources, arbitrary custom-node chains, ordinary `Color/Sampler` mixed with upstream native effect nodes, and guessed private vector/matrix metadata are not exposed until the corresponding private ABI is verified.

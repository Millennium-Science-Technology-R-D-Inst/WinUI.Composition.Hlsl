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
| `SourceName` | `String` | Compatibility accessor for the first source-parameter name. |
| `SourceNames` | `IVectorView<String>` | Ordered public source-parameter names. Linked `Color`/`Sampler` effects support 1-16 sources. |
| `IsPrecompiled` | `Boolean` | `true` when the effect owns DXBC instead of HLSL source. |
| `PropertyNames` | `IVectorView<String>` | Declared animatable scalar property names. |

## Creating single-source effects

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

`MaterializedSampler` is intended for `native Composition effect graph -> materialized texture -> HLSL sampler`. Public materialized execution is currently enabled only on the validated x64 adapter. `HlslCompiler` can still precompile/cache the materialized DXBC contract on other architectures. Materialized lowering currently supports exactly one source.

The `*WithProperties` overloads add a named source and `HlslFloatProperty` descriptors. Properties become entries in `UserConstants` and can be animated through the underlying Composition property set.

## Linked multi-source effects

`CreateAdvanced` exposes the linked-color path with 1-16 ordered source names. Multiple-source `Color` and ordinary `Sampler` effects remain one custom HLSL node; the difference is that the node receives multiple Composition inputs.

```csharp
public static HlslEffect CreateAdvanced(
    string shader,
    HlslEffectKind kind,
    IReadOnlyList<string> sourceNames,
    IReadOnlyList<HlslProperty> properties);
```

For a two-source color effect, the source shader uses the generated multi-source contract described by [HlslCompiler](hlsl-compiler.md):

```hlsl
float4 Shade(float4 color0, float4 color1)
{
    return lerp(color0, color1, 0.5f);
}
```

For a two-source sampler, the custom function receives one `(uv, samplerDataExt)` pair per source and can sample the generated `texture0/sampler0`, `texture1/sampler1`, ... declarations:

```hlsl
float4 Shade(
    float2 uv0, float4 samplerDataExt0,
    float2 uv1, float4 samplerDataExt1)
{
    float4 a = texture0.Sample(sampler0, uv0);
    float4 b = texture1.Sample(sampler1, uv1);
    return lerp(a, b, 0.5f);
}
```

The source-name order is ABI-significant: source 0 maps to the first `color0` or `(uv0, samplerDataExt0)` group, source 1 maps to the second group, and so on. Duplicate or invalid identifiers are rejected.

`MaterializedSampler` deliberately rejects more than one source. Multiple independently materialized native subgraphs require a different private lowering strategy and are not inferred from the linked multi-source implementation.

## Precompiled effects

The typed `CreateCompiledColor`, `CreateCompiledSampler`, `CreateCompiledMaterializedSampler`, and their `*WithProperties` forms are single-source convenience APIs consuming an immutable [HlslShaderLibrary](hlsl-shader-library.md). They remain useful for external/legacy DXBC when the caller deliberately controls the effect contract.

Generated single-source bytecode has a shorter path. `<HlslCompositionShader>` and `HlslCompiler` embed the selected kind/profile/source-count metadata in the compiled library, and `HlslShaderLibrary` independently verifies the actual `PSBody` ABI. `CreateCompiled` therefore does not require the caller to repeat the effect kind for a one-source library:

```cpp
auto library = HlslShaderLibrary::CreateFromGeneratedByteArray(
    g_Effects_Glass_Shader);

auto effect = HlslEffect::CreateCompiled({}, library);
```

Native build-generated single-source headers can use the direct convenience API:

```cpp
#include "Glass.g.h"
import winrt.WinUI.Composition.Hlsl;

using namespace winrt::WinUI::Composition::Hlsl;

auto effect = HlslEffect::CreateCompiledFromGeneratedByteArray(
    {},
    g_Effects_Glass_Shader);
```

For multi-source DXBC use `CreateCompiledAdvanced`. The number of source names must equal the `SourceCount` reflected by `HlslShaderLibrary`; a mismatch is rejected before the private Composition runtime receives the payload.

```cpp
auto library = HlslShaderLibrary::CreateFromGeneratedByteArray(
    g_Effects_Blend_Shader);

auto sourceNames = winrt::single_threaded_vector<winrt::hstring>();
sourceNames.Append(L"First");
sourceNames.Append(L"Second");

auto properties = winrt::single_threaded_vector<HlslProperty>();
auto effect = HlslEffect::CreateCompiledAdvanced(
    {},
    library,
    library.EffectKind(),
    sourceNames.GetView(),
    properties.GetView());
```

For arbitrary externally supplied DXBC, the explicit-profile `HlslShaderLibrary` APIs remain available. Defensive reflection verifies required exports, source count, and scalar constant-buffer layout before the private Composition runtime consumes the payload. This is not per-frame validation.

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
public IGraphicsEffect CreateGraphicsEffectWithSources(
    IReadOnlyList<IGraphicsEffectSource> sources);
```

`CreateGraphicsEffect` produces the standard Windows Graphics Effects node for the description. `CreateGraphicsEffectWithSources` supplies an ordered list of explicit inputs for a multi-source effect; the order must match `SourceNames` and the compiled shader source groups.

`Color` and ordinary `Sampler` use linked Composition inputs. The current `SingleCustom` backend deliberately does not reinterpret an arbitrary upstream native-effect graph as a sampler texture. When a custom sampler must consume the result of a native blur/transform graph, use the single-source `MaterializedSampler` path:

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

The public model supports 1-16 linked inputs for `Color` and ordinary `Sampler`, scalar public properties, fixed linker contracts, and at most one custom HLSL node in the current lowered graph. Still intentionally unsupported are multi-source `MaterializedSampler`, arbitrary custom-node chains/multiple custom HLSL nodes, treating an ordinary linked sampler input as an automatically materialized native-effect graph, and unverified private vector/matrix property-updater metadata.

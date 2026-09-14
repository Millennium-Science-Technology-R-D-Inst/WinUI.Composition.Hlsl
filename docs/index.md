# WinUI.Composition.Hlsl documentation

`WinUI.Composition.Hlsl` adds custom HLSL nodes to the normal Windows Graphics Effects / `Microsoft.UI.Composition` / WinUI 3 XAML pipeline. It supports C++/WinRT and C#, build-time or runtime compilation, linked multi-source effects, typed animatable properties, and packaged material brushes.

## Start here

If this is your first time using the package, read these in order:

1. [Get started](get-started.md) — install the package, compile a shader, and create a Composition brush.
2. [Concepts](concepts.md) — HLSL vs DXBC vs `.g.h`, shader profiles, effect kinds, linked sources, materialization, and typed properties.
3. [Architecture](architecture.md) — how the WinRT API, shader compiler, Graphics Effects graph, and private Composition adapter fit together.
4. [API reference](api/index.md) — class-by-class public WinRT reference.

## Installation and compatibility

The NuGet package requires **Windows App SDK 1.6 or later**. It ships native runtime assets for x86, x64, and ARM64; all three are public supported architectures.

x86/x64 adapter resolution is version-independent rather than pinned to a Windows App SDK version table. The package therefore does not impose a 2.4 requirement or an artificial upper Windows App SDK bound. Because the backend uses a private Composition implementation mechanism, a future Windows implementation that removes that mechanism entirely can still require adapter changes; unsupported private layouts fail closed.

## Recommended production path

Declare application-owned HLSL at build time:

```xml
<ItemGroup>
  <HlslCompositionShader Include="Effects\Glass.hlsl" />
</ItemGroup>
```

Defaults:

| Metadata | Default | Meaning |
| --- | --- | --- |
| `Kind` | `Auto` | Infer `Color`, `Sampler`, or `MaterializedSampler` from the public contract. |
| `Profile` | `Pixel40` | Compile a `lib_4_0` linkable library. |
| `SourceCount` | `1` | Number of ordered linked sources. |

Native C++ projects generate a self-contained `.g.h`; managed projects deploy self-describing DXBC under `Hlsl\...`.

## Shader contracts

### Color

```hlsl
export float4 PSBody(float4 color);
```

Multi-source color effects use generated `Shade(float4 color0, ...)` wrappers.

### Sampler

```hlsl
float4 Shade(float2 uv, float4 samplerDataExt);
```

Linked `Sampler` supports 1-16 ordered inputs. Source `i` maps to `texture{i}:register(t{i})` and `sampler{i}:register(s{i})`.

### MaterializedSampler

```hlsl
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData);
```

The current public contract supports one materialized source. Linked multi-source support should not be confused with multiple independently materialized upstream graphs.

## Typed properties

`HlslProperty` supports:

- `Scalar`
- `Vector2`
- `Vector3`
- `Vector4`
- `Matrix3x2`
- `Matrix4x4`

Property schema validation is a setup/build concern. Property updates use Composition updater/property paths and do not recompile shaders.

## Performance guidance

Prefer build-time compilation for static application assets. Reuse `HlslShaderLibrary`, `HlslEffect`, factories, and brushes rather than rebuilding them repeatedly. Generated-library reflection happens at library creation/loading, not during rendering. `GetRuntimeCapabilities()` is side-effect free and does not probe/patch the private runtime.

The package deliberately avoids moving errors that can be detected by FXC/MSBuild into a per-frame runtime validation path.

## API reference

- [API overview](api/index.md)
- [HlslComposition](api/hlsl-composition.md)
- [HlslCompiler](api/hlsl-compiler.md)
- [HlslEffect](api/hlsl-effect.md)
- [HlslEffectBrush](api/hlsl-effect-brush.md)
- [HlslProperty](api/hlsl-property.md)
- [HlslRuntimeCapabilities](api/hlsl-runtime-capabilities.md)
- [HlslShaderLibrary](api/hlsl-shader-library.md)
- [LiquidGlassMaterial](api/liquid-glass-material.md)
- [LiquidGlassBrush](api/liquid-glass-brush.md)

## Design reference

- [Sampler resource binding contract](design/resource-binding-contract.md)
- [Materialized graph compilation](design/materialized-graph-runtime.md)
- [Precompiled and asynchronous shader libraries](design/precompiled-shaders.md)
- [Runtime safety and lifetime contract](design/runtime-safety.md)

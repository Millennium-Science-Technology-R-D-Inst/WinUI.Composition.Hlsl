# Concepts for WinUI.Composition.Hlsl

This article defines the terms used by the package. Read it before the API reference if you are new to HLSL or Windows Composition effects.

## Composition effect pipeline

A WinUI 3 application normally gives `Microsoft.UI.Composition.Compositor` a Windows Graphics Effects graph. Composition compiles that graph and owns rendering, scheduling, intermediate surfaces, clipping, transforms, and presentation.

`WinUI.Composition.Hlsl` adds custom HLSL nodes to that model. It does **not** create an application-owned swap chain or render loop.

```text
IGraphicsEffect graph
        |
        v
Composition effect compilation
        |
        +--- native effect nodes
        |
        +--- custom HLSL node
        |
        v
CompositionEffectFactory
        |
        v
CompositionEffectBrush
        |
        v
XAML / Composition visual tree
```

## HLSL source, DXBC, and `.g.h`

These are three representations of the same shader program at different stages.

**HLSL** is source code. The package uses Windows SDK FXC because the private Composition backend consumes Shader Model 4 linkable libraries.

**DXBC** is the compiled DirectX bytecode container produced by FXC. For this package, the important output is a shader **library** (`lib_4_0*`), not a standalone application pixel shader.

**`.g.h`** is a native C++ header generated from the same DXBC bytes with FXC `/Fh`. It lets a C++ application embed the library directly in its binary.

```text
Effect.hlsl
   |
   | FXC /T lib_4_0*
   v
DXBC library
   |              \
   | loose file    \ /Fh
   v                v
C# content       C++ .g.h byte array
```

A `.cso` or `.dxbc` filename is only a storage convention. The actual format is determined by the bytes and compilation target.

### Native, managed, and MSIX deployment

The native and managed build paths intentionally deploy shader output differently:

| Consumer | Runtime form | Normal package behavior |
| --- | --- | --- |
| C++/WinRT | Generated `.g.h` byte array | DXBC remains an intermediate output and is embedded in the native DLL/application. |
| C# / managed | Loose generated `.dxbc` | DXBC is published as application content under `Hlsl\...`. |

Do not remove all `.dxbc` items globally to fix a native MSIX build. Managed consumers need their generated `Hlsl\...` files. Conversely, do not force the native library's intermediate DXBC into the package unless the application explicitly loads it as a loose file.

When a native application references the HLSL project directly, Visual Studio's C++ `FxCompile` targets can export the intermediate object output as deployment content. If the library output directory and the intermediate shader directory are different, the generated package recipe can contain a path like:

```text
WinUI.Composition.Hlsl\..\..\..\src\WinUI.Composition.Hlsl\obj\x64\Release\LiquidGlassShader.dxbc
```

That path is usable as a source-on-disk relationship, but it is not a valid MSIX package path. The left side of a MakeAppx mapping is allowed to be an absolute source path; the right side must be a clean package-relative path and must not contain a drive letter, `..`, or unresolved MSBuild properties.

For `0x8007007B` during MakeAppx, inspect the generated `.appxrecipe` and `package.map.txt` first. If the malformed entry comes from a referenced native library, fix or filter it at the consuming application's MSIX payload boundary. Do not change the managed `HlslCompositionPublishAsContent` policy as a workaround for a native package recipe problem.

## Shader profile

`HlslShaderProfile` selects the FXC library target:

| API value | FXC target |
| --- | --- |
| `Level91` | `lib_4_0_level_9_1_ps_only` |
| `Level93` | `lib_4_0_level_9_3_ps_only` |
| `Pixel40` | `lib_4_0` |

A profile is **not** an effect kind. It describes the target shader-library capability level.

## Effect kind

`HlslEffectKind` describes the calling contract between Composition and your HLSL.

### Color

A color effect transforms already-sampled input colors.

Single source:

```hlsl
export float4 PSBody(float4 color)
{
    return color;
}
```

Multiple linked sources use `Shade(float4 color0, ...)` and are wrapped into the required exported `PSBody` by the package.

### Sampler

A sampler effect performs explicit texture sampling.

```hlsl
float4 Shade(float2 uv, float4 samplerDataExt)
{
    return texture0.Sample(sampler0, uv);
}
```

The package generates edge-mode `PSBody*` exports required by Composition and supplies `textureN`/`samplerN` declarations.

### MaterializedSampler

A materialized sampler receives a real intermediate surface representing an upstream effect graph. In addition to `uv` and `samplerDataExt`, it receives `samplerData`, which carries the materialized content rectangle contract used by the private backend.

```hlsl
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData)
{
    return texture0.Sample(sampler0, uv);
}
```

The current public contract is one materialized source. This is different from linked `Sampler`, which supports 1-16 linked sources.

### Auto

`Auto` is a compiler/build convenience value. The front end probes the supported public contracts and resolves it to one concrete kind. `Auto` is not stored as the runtime effect kind.

## Linked source versus materialized source

The distinction is important.

A **linked source** participates in the shader-linking contract. Multiple linked inputs can be represented in one generated `Color` or `Sampler` shader library.

A **materialized source** is rendered to a real intermediate surface before the custom pass. This creates a pass/materialization boundary and has different graph, bounds, and lifetime implications.

```text
linked:
source A ---\
             > custom linked pass
source B ---/

materialized:
native graph -> intermediate surface -> custom materialized pass
```

Therefore `SourceCount=2` support for linked `Sampler` does not imply two independently materialized upstream graphs are supported by `MaterializedSampler`.

## Source order and registers

For linked samplers, source order is part of the ABI:

```text
source 0 -> texture0 : t0 -> sampler0 : s0
source 1 -> texture1 : t1 -> sampler1 : s1
...
source N -> textureN : tN -> samplerN : sN
```

The build integration tests reflect generated DXBC and verify this mapping. Application code should bind brushes in the same order as `SourceNames`.

## Properties and Composition animation

`HlslProperty` defines a typed value that is copied into the custom effect's native property/constant-buffer layout. Supported values are scalar, vector, and matrix types.

Properties are separate from source textures:

```text
source brush ---------> texture/sampler input
property value --------> constant-buffer/updater input
Composition animation -> property updater path
```

Changing a property does not recompile the shader.

Matrices use packed contiguous float backing storage because the private updater copies raw float components. Generated HLSL reconstructs the logical matrix, avoiding dependence on default HLSL matrix row/column stride.

## Self-describing generated libraries

Package-generated DXBC contains a reserved metadata export that records the resolved effect kind, profile, and source count. `HlslShaderLibrary.CreateFromGeneratedByteArray` and the `LoadGenerated*` methods use that metadata so callers do not repeat build-time values.

External/legacy DXBC can use the explicit-profile APIs instead.

## Build-time validation versus runtime validation

The package intentionally catches deterministic authoring errors as early as possible:

- HLSL syntax and missing entry points: FXC/MSBuild.
- `Auto` kind resolution: compiler/build front end.
- generated resource/register ABI: build integration tests.
- property schema and precompiled constant-buffer layout: library/effect setup.
- brush/source object validity: runtime API boundary where only runtime objects are known.

There is no reason to repeat these checks on every render or every property update. The rendering hot path consumes the already-created Composition objects.

## Private Composition ABI

The custom backend uses undocumented Composition internals. The public package API is stable WinRT, while the adapter resolves the private implementation needed by the installed runtime.

The current support contract is Windows App SDK 1.6 or later on x86, x64, and ARM64. x86/x64 adapter resolution is version-independent rather than tied to a hard-coded Windows App SDK version table. If a future Windows implementation removes or fundamentally changes the required private ABI, the adapter is designed to fail closed rather than write guessed layouts.

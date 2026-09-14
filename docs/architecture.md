# Architecture overview

`WinUI.Composition.Hlsl` is a WinRT component that adds custom HLSL effects to the normal WinUI 3 Composition pipeline. This page describes the package architecture from application code to the private Composition adapter.

## Layered model

The package is intentionally split into four layers.

```text
Application
  |  HlslEffect / HlslProperty / HlslComposition / LiquidGlassBrush
  v
Public WinRT layer
  |  immutable effect definitions + brush/factory wrappers
  v
Shader/build layer
  |  FXC lib_4_0* + generated wrappers + self-describing DXBC
  v
Effect engine / graph layer
  |  IGraphicsEffect graph + CompositionEffectFactory integration
  v
Private Composition adapter
     architecture-specific resolver + custom compiled pass lowering
```

Application code should normally stay in the first two layers. The private adapter is an implementation detail.

## Public WinRT layer

The main public types are:

- `HlslCompiler` — asynchronous runtime HLSL compilation.
- `HlslShaderLibrary` — immutable compiled DXBC plus reflected metadata.
- `HlslEffect` — shader contract, source names, and typed property schema.
- `HlslEffectFactory` — wrapper around `CompositionEffectFactory`.
- `HlslEffectBrush` — source/property operations on the created brush.
- `HlslComposition` — convenience bridge between effect definitions, Composition, and XAML.
- `LiquidGlassMaterial` / `LiquidGlassBrush` — higher-level packaged material built on the same primitives.

The WinRT layer is shared by C++/WinRT and the CsWinRT C# projection.

## Build-time shader pipeline

`<HlslCompositionShader>` is the production-oriented path.

```text
project item
   |
   +-- Kind (default Auto)
   +-- Profile (default Pixel40)
   +-- SourceCount (default 1)
   +-- Defines/includes
   v
Compile-HlslCompositionShader.ps1
   |
   +-- resolve public contract
   +-- generate private wrapper exports
   +-- generate textureN/samplerN declarations
   +-- generate Kind/Profile/SourceCount metadata
   v
FXC /T lib_4_0*
   |
   +-- intermediate DXBC
   +-- native .g.h (C++)
   +-- loose app content (managed/default)
```

The package uses the same generated ABI for runtime and build-time compilation. This prevents C++ embedded shaders and C# loose DXBC from behaving differently.

## Why linkable shader libraries are used

The application does not own the final Direct3D pixel shader. Composition links effect fragments into its own rendering program. For that reason the package compiles linkable Shader Model 4 libraries (`lib_4_0*`) instead of a normal application `ps_5_0` shader.

`HlslEffectKind` describes the fragment calling contract; `HlslShaderProfile` selects the library target. They are orthogonal.

## Effect definition and factory lifetime

`HlslEffect` stores an immutable definition: kind, source schema, property schema, and either source HLSL or compiled library information.

Creating a factory crosses into Composition setup:

```text
HlslEffect
   -> IGraphicsEffect representation
   -> compositor.CreateEffectFactory(...)
   -> HlslEffectFactory
   -> HlslEffectBrush
```

Factories and brushes are intended to be reused. A property change updates Composition state; it does not rebuild the immutable effect definition or re-run FXC.

## Private adapter

The private adapter exists because public WinUI/Composition APIs do not expose an API for registering arbitrary application HLSL fragments into the internal effect compiler.

The adapter therefore resolves the installed Composition implementation and supplies custom compiled effect metadata at the same internal stage used by native effects.

Key design rules:

1. Do not replace the application's visual tree or create a parallel swap chain.
2. Let the normal effect graph traversal happen first.
3. Replace only the compiled representation for registered custom nodes.
4. Preserve native input bindings, surface metadata, property updaters, and lifetime rules.
5. Fail closed if a required private structure cannot be identified safely.

## Architecture support

The package ships native runtime assets for x86, x64, and ARM64. All three are public supported architectures.

x86/x64 resolution is intentionally not a hard-coded Windows App SDK version switch. The adapter locates the required runtime structures from structural/code relationships, so the compatibility contract is not capped at the versions used during initial reverse engineering. The NuGet package therefore requires Windows App SDK 1.6 or later rather than one specific 2.x release.

Because the backend is private, a future Windows implementation that removes the underlying mechanism entirely can still make a capability unavailable. That case should fail closed; it should not turn into guessed writes to an unknown layout.

## Validation placement and performance

Validation is placed at the cheapest useful boundary.

| Work | When it happens |
| --- | --- |
| HLSL syntax/entry point validation | Build time or explicit `HlslCompiler` call |
| Kind inference | Build time or explicit compile call |
| Generated DXBC metadata/reflection | Library creation/loading |
| Property schema/layout validation | Effect/library setup |
| Source count/object validation | Brush setup |
| Property animation/update | Composition runtime, no shader recompilation |
| Rendering | Composition runtime, no package reflection loop |

The package does not scan private runtime binaries merely to answer `GetRuntimeCapabilities()`, and it does not perform DXBC reflection per frame.

## Current graph lowering boundary

The current public graph contract supports native Composition graph participation and one materialized custom sampler pass. The runtime already contains multi-pass replacement machinery, but public support for multiple custom nodes, native nodes after a custom materialized pass, and multi-source materialization remains disabled until graph topology/bounds behavior is represented and validated explicitly.

The planned internal direction is a graph IR/pass planner:

```text
flattened native graph
        v
Node / Edge / Subgraph IR
        v
pass planner
  +-- linked native/custom fragment
  +-- materialization boundary
  +-- custom pass
        v
compiled Composition subgraphs
```

This planner is an internal implementation detail; it is not intended to expose private Composition structures through the WinRT API.

## Related documentation

- [Get started](get-started.md)
- [Concepts](concepts.md)
- [API reference](api/index.md)
- [Runtime safety](design/runtime-safety.md)
- [Materialized graph runtime](design/materialized-graph-runtime.md)
- [Resource binding contract](design/resource-binding-contract.md)

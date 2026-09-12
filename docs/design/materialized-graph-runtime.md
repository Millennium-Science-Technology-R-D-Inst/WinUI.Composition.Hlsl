# Materialized graph compilation

## Purpose

`MaterializedSampler` solves a Composition-specific problem: a custom sampler needs arbitrary texture/UV access, while an upstream native Composition effect graph normally participates in shader linking as color/dependency fragments rather than as an application-owned texture.

The supported shape is:

```text
one native upstream Windows Graphics Effects graph
    -> native materialization/intermediate texture
    -> one isolated terminal HLSL MaterializedSampler
    -> native output wrapper
    -> CompositionEffectBrush / XAML
```

This stays inside the Windows Graphics Effects/Composition/XAML pipeline. It does not use `SwapChainPanel`, an app-owned swap chain, or an overlay renderer.

## Public contract

A materialized sampler is declared with `HlslEffectKind.MaterializedSampler` or the corresponding `HlslEffect.CreateMaterializedSampler*` API. User code implements:

```hlsl
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData)
{
    return texture0.Sample(sampler0, uv);
}
```

The source/build front end injects `texture0`, `sampler0`, all private edge-mode `PSBody*` wrappers, and an identity `MaterializeColor(float4)` export. The linker argument sequence is the already validated Liquid Glass sequence: UV (`0x0100`), samplerDataExt (`0x0400`), samplerData (`0x0300`), custom result (`0x0200`).

## Native lowering

`CompileMaterializedGraph` inspects the flattened graph produced by WinUI/Composition. It requires exactly one registered custom effect and locates the subgraph containing it. The current backend requires the custom sampler to occupy one isolated terminal subgraph followed by the native composite/output wrapper.

The runtime asks the original native compiler to compile the whole graph while temporarily supplying an identity code-generation body for the custom node. This preserves the native traverser's decisions about upstream materialization, input bindings, edge modes, bounds, and subgraph topology.

The returned synthetic compiled result retains the native compiled result as backing storage. Upstream native subgraph vectors are borrowed rather than reconstructed. Only the custom sampler body and final identity body are replaced with the custom shader library; their input bindings are copied from the native result. Constant-buffer property updaters use the original graph's effect-node index instead of assuming node and subgraph indexes are identical.

## Why the backend is intentionally narrow

The runtime does **not** claim support for:

- multiple custom HLSL nodes in one flattened graph;
- multiple public texture sources to one custom shader;
- arbitrary native effects after the custom sampler;
- guessed private linker argument encodings;
- arbitrary private Composition ABI revisions.

Those cases remain fail-closed. The internal graph/source vectors are structurally capable of representing more than one source, but the private linker argument mapping for multi-texture custom shaders has not been verified. Merely removing a source-count check would create an unsafe ABI rather than real support.

## Architecture capability

The x64 Windows App SDK 2.4 baseline is the currently validated materialized path. x86 and ARM64 have private runtime adapters but `GetRuntimeCapabilities()` does not yet claim `SupportsMaterializedGraphs` for those architectures.

## Regression validation

Final CI builds the native runtime and package consumers. Runtime smoke validation should exercise effect creation, property animation/update, resize, material recreation, and materialized sampling. Passing a smoke test establishes stability of the tested topology; it is not a pixel-perfect rendering conformance test or evidence for unsupported graph shapes.

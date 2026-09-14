# Materialized graph compilation

## Purpose

`MaterializedSampler` is for a custom shader that needs arbitrary texture/UV access to the result of an upstream native Composition effect graph. Ordinary linked `Sampler` inputs do not require this materialization path.

The current public shape is:

```text
one native upstream Windows Graphics Effects graph
    -> native materialization/intermediate surface
    -> one isolated HLSL MaterializedSampler pass
    -> native output wrapper
    -> CompositionEffectBrush / XAML
```

## Shader contract

```hlsl
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData)
{
    return texture0.Sample(sampler0, uv);
}
```

The compiler injects:

- `texture0 : register(t0)`;
- `sampler0 : register(s0)`;
- the clamp/wrap/mirror `PSBody*` exports;
- `MaterializeColor(float4)`;
- Kind/Profile/SourceCount metadata.

The current private linker argument sequence remains the Liquid Glass validated sequence: UV (`0x0100`), samplerDataExt (`0x0400`), samplerData (`0x0300`), custom result (`0x0200`).

## Native lowering

The runtime lets the original Composition compiler flatten/compile the graph and preserves that native compiled result as backing storage. It substitutes the isolated custom subgraph with the compiled HLSL pass, copies the native input binding/surface data, and uses the graph-global effect-node index for property updater mappings.

This design deliberately reuses native decisions about upstream materialization, edge modes, bounds, and subgraph topology rather than reconstructing those undocumented structures from scratch.

## What is not public yet

The runtime contains an exploratory general custom-graph path that can discover multiple custom nodes and materialize custom passes. However, the public contract still does not claim:

- more than one independently materialized input to one `MaterializedSampler`;
- multiple custom HLSL nodes in a general lowered graph;
- arbitrary native nodes after a custom materialized pass;
- pass merging or cross-profile custom-fragment linking.

Those shapes remain capability-gated/fail-closed until graph topology, input mapping, downstream bounds, edge modes, resize/property-update behavior, and real runtime execution are validated. Merely deleting the current single-source/subgraph checks would not constitute support.

## Architecture support

The single-source materialized path is part of the public support contract on x86, x64, and ARM64. The NuGet package requires Windows App SDK 1.6 or later.

x86/x64 private-adapter resolution is version-independent rather than keyed to a Windows App SDK version table. The package therefore does not impose a 2.4 upper bound. A future Windows implementation that removes or fundamentally redesigns the underlying private mechanism can still require adapter work; unrecognized private layouts fail closed.

## Performance notes

Materialization creates a real pass boundary and intermediate surface, so it is more expensive than a linked fragment when linked sampling is sufficient. Use `MaterializedSampler` only when the shader needs arbitrary sampling of a materialized upstream result.

Topology/property validation happens during graph/factory setup. It is not repeated per pixel or per frame by the package.

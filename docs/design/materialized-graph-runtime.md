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

## Architecture validation

Released Windows App SDK 1.6-2.4 have been runtime-tested for the current single-source materialized path on x86 and x64. ARM64 has an adapter/build path but remains Experimental until real-device validation is complete.

Private ABI support must be revalidated for future releases rather than inferred from version numbers.

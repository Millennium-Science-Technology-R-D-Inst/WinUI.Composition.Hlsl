# Sampler resource binding contract

Sampler source order is part of the compiled ABI.

For every linked or materialized sampler source `i`, both the runtime compiler and the MSBuild compiler generate explicit bindings:

```hlsl
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);
Texture2D texture1 : register(t1);
SamplerState sampler1 : register(s1);
```

In general:

```text
source i -> texture{i} at t{i}
         -> sampler{i} at s{i}
```

This removes reliance on FXC assigning resource slots implicitly and keeps build-time and runtime-generated libraries deterministic.

## Validation

The native build-integration fixture uses `D3DReflectLibrary` and `ID3D11FunctionReflection::GetResourceBindingDescByName` to validate generated DXBC. It checks that each expected `textureN` is a `Texture2D` bound at `tN`, each `samplerN` is a sampler bound at `sN`, and each binding has count 1. The same fixture also checks the `PSBody*` exports and Kind/Profile/SourceCount metadata marker.

This validation is intentionally performed on compiled bytecode rather than only on generated HLSL text: it locks the actual FXC output ABI consumed by Composition.

## Scope

The mapping applies to ordinary linked `Sampler` with 1-16 public sources and to the current one-source `MaterializedSampler`. It does not imply that multiple independently materialized textures are supported. Multi-source materialized lowering still needs separate graph/materialization/input-metadata validation and remains outside the public contract.

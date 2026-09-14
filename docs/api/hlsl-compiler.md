# HlslCompiler class

`HlslCompiler` compiles application HLSL into an immutable `HlslShaderLibrary` on a background thread. It emits the same Composition wrappers/resource bindings/self-description metadata as the MSBuild `<HlslCompositionShader>` pipeline.

## Basic compilation

```csharp
var library = await HlslCompiler.CompileAsync(
    shader,
    HlslEffectKind.Auto,
    HlslShaderProfile.Pixel40);
```

`Auto` probes the supported public contracts and requires exactly one match. For one source it can resolve `Color`, `Sampler`, or `MaterializedSampler`; for more than one source it resolves linked `Color` or `Sampler`.

## Multi-source compilation

```csharp
var library = await HlslCompiler.CompileAdvancedAsync(
    shader,
    HlslEffectKind.Auto,
    HlslShaderProfile.Pixel40,
    sourceCount: 2);
```

`sourceCount` must be 1-16. `MaterializedSampler` remains exactly one source.

For a sampler, generated resources are explicitly bound:

```text
source 0 -> texture0 : t0, sampler0 : s0
source 1 -> texture1 : t1, sampler1 : s1
...
```

This mapping is identical to the build-time compiler and is documented in [resource-binding-contract.md](../design/resource-binding-contract.md).

## Typed properties

Use the typed overloads when the shader uses `HlslProperty` descriptors:

```csharp
var library = await HlslCompiler.CompileAdvancedWithTypedPropertiesAsync(
    shader,
    HlslEffectKind.Sampler,
    HlslShaderProfile.Pixel40,
    sourceCount: 2,
    properties);
```

Available property types are Scalar, Vector2/3/4, Matrix3x2, and Matrix4x4. The compiler injects the `UserConstants : register(b0)` representation using the same layout implementation used by native effect construction, then reflects the compiled library to ensure the actual constant-buffer layout matches the schema.

There are matching `...AndDefinesAsync` overloads. Defines use `NAME` or `NAME=VALUE`; the compiler accepts at most 64 definitions.

## Profiles

| API profile | FXC target |
| --- | --- |
| `Level91` | `lib_4_0_level_9_1_ps_only` |
| `Level93` | `lib_4_0_level_9_3_ps_only` |
| `Pixel40` | `lib_4_0` |

These are shader-linking library targets, not standalone `ps_*` stage shaders.

## Production guidance

Prefer `<HlslCompositionShader>` for source known at build time because syntax/contract failures then fail MSBuild and native C++ can embed the resulting `.g.h`. Use `HlslCompiler` for genuinely runtime-generated/development source and cache `HlslShaderLibrary.Bytecode` when appropriate.

The compiler does not create a `Compositor`, brush, or effect factory and does not install the private Composition adapter during background compilation.

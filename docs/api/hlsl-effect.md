# HlslEffect class

`HlslEffect` is an immutable description of a custom Composition effect. It stores the effect kind, ordered source names, property schema, shader source/precompiled library, and stable effect ID used by the private runtime registry.

## Source factories

Convenience single-source factories:

- `CreateColor` / `CreateColorTransform`
- `CreateSampler` / `CreateCustomSampler`
- `CreateMaterializedSampler` / `CreateCustomMaterializedSampler`
- `Create*WithProperties` for the legacy scalar `HlslFloatProperty` surface

Advanced source effects use:

```csharp
HlslEffect.CreateAdvanced(shader, kind, sourceNames, properties)
```

`sourceNames` is ordered. Linked `Color`/`Sampler` support 1-16 sources. `MaterializedSampler` currently requires exactly one source. `properties` is an ordered `HlslProperty` schema supporting Scalar, Vector2/3/4, Matrix3x2, and Matrix4x4.

## Precompiled factories

Generated self-describing libraries can use:

```csharp
var effect = HlslEffect.CreateCompiled(Guid.Empty, library);
```

or, for multiple ordered sources / typed properties:

```csharp
var effect = HlslEffect.CreateCompiledAdvanced(
    Guid.Empty,
    library,
    kind,
    sourceNames,
    properties);
```

`CreateCompiledAdvanced` requires source-name count to match `HlslShaderLibrary.SourceCount` and validates the complete typed `UserConstants` layout against the DXBC before the effect reaches the private Composition backend.

The older `CreateCompiledColor`, `CreateCompiledSampler`, `CreateCompiledMaterializedSampler`, and scalar `*WithProperties` variants remain single-source convenience APIs, especially for external/legacy DXBC with an explicitly supplied profile.

## Graphics-effect projection

- `CreateGraphicsEffect()` creates the node with source parameters.
- `CreateGraphicsEffectWithSource(source)` supplies the single source.
- `CreateGraphicsEffectWithSources(sources)` supplies ordered multi-source inputs.

The returned object is a standard `Windows.Graphics.Effects.IGraphicsEffect`, so native Composition effects can be placed upstream using the normal graph model. Public support for general multiple-custom-node / native-after-custom materialized graphs is still capability-gated even though the runtime contains exploratory multi-pass lowering code.

## Properties

| Property | Meaning |
| --- | --- |
| `Id` | Stable effect GUID. |
| `Kind` | `Color`, `Sampler`, or `MaterializedSampler`. |
| `SourceName` | Single-source convenience name. |
| `SourceNames` | Ordered advanced source names. |
| `IsPrecompiled` | Whether the definition uses DXBC instead of source. |
| `PropertyNames` | Declared property names. |

`GetPropertyPath(name)` and `GetAnimatablePropertyPaths()` expose Composition animation paths for declared properties.

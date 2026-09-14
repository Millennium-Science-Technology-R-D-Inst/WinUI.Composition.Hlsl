# HlslEffect class

Represents an immutable custom HLSL effect definition that can be converted into a Windows Graphics Effects node or a Composition effect factory.

Namespace: `WinUI.Composition.Hlsl`

## Definition

```text
[default_interface] runtimeclass HlslEffect
```

An `HlslEffect` describes the shader contract, ordered sources, typed property schema, and source/compiled shader payload. Property **values** belong to created brushes; the schema belongs to the effect definition.

## Create source effects

### CreateColor

```csharp
public static HlslEffect CreateColor(Guid id, string shader);
```

Creates a single-source `Color` effect from HLSL source.

### CreateSampler

```csharp
public static HlslEffect CreateSampler(Guid id, string shader);
```

Creates a single-source linked `Sampler` effect from HLSL source.

### CreateMaterializedSampler

```csharp
public static HlslEffect CreateMaterializedSampler(Guid id, string shader);
```

Creates a single-source `MaterializedSampler` effect.

### Convenience source methods

`CreateColorTransform`, `CreateCustomSampler`, and `CreateCustomMaterializedSampler` are convenience constructors for the corresponding single-source contracts.

## Create compiled effects

### CreateCompiled

```csharp
public static HlslEffect CreateCompiled(Guid id, HlslShaderLibrary shader);
```

Creates an effect using the concrete kind carried by a compiled library.

For package-generated self-describing bytecode, this is the preferred general compiled path.

### CreateCompiledFromGeneratedByteArray

```csharp
public static HlslEffect CreateCompiledFromGeneratedByteArray(
    Guid id,
    byte[] bytecode);
```

Creates a compiled effect directly from package-generated self-describing bytes. This is especially convenient for native `.g.h` arrays.

```cpp
#include "Glass.g.h"

auto effect = HlslEffect::CreateCompiledFromGeneratedByteArray(
    {}, g_Effects_Glass_Shader);
```

### Kind-specific compiled methods

`CreateCompiledColor`, `CreateCompiledSampler`, and `CreateCompiledMaterializedSampler` require the supplied library to match the requested kind.

## Create effects with properties

The `...WithProperties` overloads use `HlslFloatProperty` for the legacy scalar-only property surface.

For new code requiring vectors or matrices, use `CreateAdvanced` / `CreateCompiledAdvanced` with `HlslProperty`.

## CreateAdvanced

Creates an effect with an ordered source schema and typed properties.

```csharp
public static HlslEffect CreateAdvanced(
    string shader,
    HlslEffectKind kind,
    IReadOnlyList<string> sourceNames,
    IReadOnlyList<HlslProperty> properties);
```

### Parameters

`shader`  
HLSL source text.

`kind`  
Concrete effect contract. `Auto` is a compiler/build convenience and should normally be resolved before a runtime effect definition is needed.

`sourceNames`  
Ordered public source names. For linked multi-source effects, order defines source indices.

`properties`  
Typed property schema.

## CreateCompiledAdvanced

Creates an advanced effect from an already compiled library.

```csharp
public static HlslEffect CreateCompiledAdvanced(
    Guid id,
    HlslShaderLibrary shader,
    HlslEffectKind kind,
    IReadOnlyList<string> sourceNames,
    IReadOnlyList<HlslProperty> properties);
```

The source-name count must match `shader.SourceCount`. The property schema is validated against the compiled `UserConstants` layout before the effect enters the private Composition backend.

## Properties

| Property | Description |
| --- | --- |
| `Id` | Stable effect GUID used by the native effect definition/registry. |
| `Kind` | Concrete `HlslEffectKind`. |
| `SourceName` | Single-source compatibility name. |
| `SourceNames` | Ordered source-name collection for advanced effects. |
| `IsPrecompiled` | `true` when the effect uses a compiled `HlslShaderLibrary`. |
| `PropertyNames` | Names of declared animatable properties. |

## Graphics Effects methods

### CreateGraphicsEffect

Creates the public Windows Graphics Effects representation of the node.

### CreateGraphicsEffectWithSource

Creates a node with one upstream `IGraphicsEffectSource`.

### CreateGraphicsEffectWithSources

Creates a node with an ordered source list.

```csharp
public IGraphicsEffect CreateGraphicsEffectWithSources(
    IReadOnlyList<IGraphicsEffectSource> sources);
```

For advanced effects, `sources[i]` corresponds to `SourceNames[i]`.

## Property paths

`GetPropertyPath(name)` returns the Composition property path for one declared property. `GetAnimatablePropertyPaths()` returns all declared animatable paths.

Use these paths when driving high-frequency values with Composition animations instead of repeatedly marshaling values from the UI thread.

## Remarks

`HlslEffect` is intended to be cheap to reuse after creation. It is a schema/definition object, not a per-frame shader execution object. Creating/validating a compiled effect can perform setup-time bytecode/property checks; rendering occurs later in Composition without repeating those checks.

For packaged static HLSL, prefer build-time compilation plus `CreateCompiled`/`CreateCompiledFromGeneratedByteArray` over source compilation at application startup.

## See also

- [HlslComposition](hlsl-composition.md)
- [HlslEffectBrush](hlsl-effect-brush.md)
- [HlslShaderLibrary](hlsl-shader-library.md)
- [HlslProperty](hlsl-property.md)

# HlslEffect class

Describes an immutable HLSL effect.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v0.1.0-preview.8  
**Assembly:** `WinUI.Composition.Hlsl.dll`

```idl
[default_interface] runtimeclass HlslEffect
```

## Properties

| Property | Type | Description |
| --- | --- | --- |
| `Id` | `Guid` | Gets the deterministic or explicitly supplied effect identifier. |
| `Kind` | `HlslEffectKind` | Gets `Color` or `Sampler`. |

## Methods

### CreateColorTransform

Creates a dynamic color-transform description and derives a stable ID from the shader and schema.

```csharp
public static HlslEffect CreateColorTransform(string shader);
```

The source must export `float4 PSBody(float4 color)`. Input and output use premultiplied color.

### CreateCustomSampler

Creates a dynamic custom-sampler description and derives a stable ID.

```csharp
public static HlslEffect CreateCustomSampler(string shader);
```

The source must define `float4 Shade(float2 uv, float4 samplerDataExt)`. The runtime declares `texture0` and `sampler0`, then generates the `PSBody` clamp/wrap/mirror exports required by the private Composition linker before calling `D3DCompile`.

### CreateColorWithProperties / CreateSamplerWithProperties

Creates a dynamic source effect with one named source and declared scalar properties.

```csharp
public static HlslEffect CreateColorWithProperties(
    string shader,
    string sourceName,
    IReadOnlyList<HlslFloatProperty> properties);
```

Each property is emitted into `cbuffer UserConstants : register(b0)` and becomes animatable through [HlslEffectBrush.SetFloat](hlsl-effect-brush.md#setfloat).

### CreateColor / CreateSampler

Creates a dynamic effect with an explicitly supplied ID. Use these overloads when an external contract already owns the GUID. Reusing a GUID for a different description throws when the effect is registered.

### CreateCompiledColor / CreateCompiledSampler

Creates an effect from an immutable [HlslShaderLibrary](hlsl-shader-library.md). These overloads do not call `D3DCompile` at runtime. The bytecode library must already contain the public compiled-shader ABI expected for its effect kind.

### CreateCompiledColorWithProperties / CreateCompiledSamplerWithProperties

Creates a precompiled effect with one named source and declared scalar properties. This is the production-bytecode counterpart of the source-string property APIs. The runtime reflects `UserConstants` and rejects missing, reordered, or incorrectly sized scalar property layouts before registration.

## Current scope

A public `HlslEffect` has one named source and uses the fixed public entry-point contract above. Multiple custom sources and arbitrary public entry-point names are not supported yet. Those are API/backend capability limits, not general limitations of Windows Composition graphs.

## Exceptions

| Exception | Condition |
| --- | --- |
| `ArgumentException` | Shader text/schema is invalid, compiled DXBC is malformed, an expected export/signature is missing, or the compiled constant-buffer ABI does not match the declared properties. |
| `COMException` | Runtime HLSL compilation, private ABI initialization, or native Composition effect creation fails. Dynamic compilation diagnostics identify `UserShader.hlsl`; incompatible native ABI fingerprints fail closed with a revision-mismatch HRESULT. |

## Applies to

The private custom-effect adapter is currently implemented for x64. Win32 and ARM64 builds expose the WinRT surface but custom effect registration/creation returns `E_NOTIMPL` until an architecture-specific native ABI adapter is implemented.

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

Creates a color-transform description and derives a stable ID from the shader and schema.

```csharp
public static HlslEffect CreateColorTransform(string shader);
```

```cpp
static HlslEffect CreateColorTransform(winrt::hstring const& shader);
```

The shader must export `float4 PSBody(float4 color)`. Input and output use premultiplied color.

### CreateCustomSampler

Creates a custom-sampler description and derives a stable ID.

```csharp
public static HlslEffect CreateCustomSampler(string shader);
```

The shader must define `float4 Shade(float2 uv, float4 samplerDataExt)`. The runtime declares `texture0` and `sampler0` and generates the required linking entry points.

### CreateColorWithProperties / CreateSamplerWithProperties

Creates an effect with one named source and declared scalar properties.

```csharp
public static HlslEffect CreateColorWithProperties(
    string shader,
    string sourceName,
    IReadOnlyList<HlslFloatProperty> properties);
```

Each property is emitted into the shader constant buffer and becomes animatable through [HlslEffectBrush.SetFloat](hlsl-effect-brush.md#setfloat).

### CreateColor / CreateSampler

Creates an effect with an explicitly supplied ID. Use these overloads when an external contract already owns the GUID. Reusing a GUID for a different description throws an exception when the effect is registered.

### CreateCompiledColor / CreateCompiledSampler

Creates an effect from an immutable [HlslShaderLibrary](hlsl-shader-library.md). These overloads do not call `D3DCompile` at runtime.

## Exceptions

| Exception | Condition |
| --- | --- |
| `ArgumentException` | Shader text, source name, or property schema is invalid. |
| `COMException` | Shader compilation or native effect registration fails. Compilation messages identify `UserShader.hlsl`. |

## Applies to

Windows App SDK 2.4; WinUI.Composition.Hlsl 0.1.0-preview.8.

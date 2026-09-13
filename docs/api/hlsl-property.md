# HlslProperty class

Describes a property used by the advanced HLSL effect surface.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v1.0.0

```csharp
public sealed class HlslProperty
```

## Constructor

```csharp
public HlslProperty(
    string name,
    HlslPropertyType type,
    IReadOnlyList<float> defaultValue);
```

The current validated private Composition property-updater ABI supports **scalar properties only**. Therefore `type` must currently be `HlslPropertyType.Scalar`, with exactly one finite default value.

`Vector2`, `Vector3`, `Vector4`, `Matrix3x2`, and `Matrix4x4` remain reserved in the public enum for ABI evolution, but construction now fails closed for those values until their private native metadata, property-value representation, constant-buffer layout, and animation update path are validated end to end. The presence of the enum values and corresponding brush setter projections is not a support claim.

For ordinary scalar properties, prefer [HlslFloatProperty](hlsl-float-property.md) unless the advanced source/multi-source API is required.

## Properties

| Property | Type | Description |
| --- | --- | --- |
| `Name` | `String` | Public Composition property name. |
| `Type` | `HlslPropertyType` | Property type. Currently only `Scalar` can be constructed. |
| `DefaultValue` | `IVectorView<Single>` | Immutable default-value components. |

## Why non-scalar values fail closed

The library maps public WinRT effect properties into private `wuceffectsi`/Composition constant-buffer updater metadata. A correct HLSL cbuffer layout alone is not sufficient: the private expression/property metadata and DWM-side update callable must also agree with the projected value type. Those contracts have been verified for scalar `float` values only.

Until the remaining types are verified against the native runtime, accepting them would create an API that appears functional at compile time but can corrupt or misroute runtime property updates. The implementation therefore rejects them explicitly rather than guessing.

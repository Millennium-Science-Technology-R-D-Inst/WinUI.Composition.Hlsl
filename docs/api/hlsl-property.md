# HlslProperty class

Describes a typed property used by the advanced HLSL effect surface.

```csharp
public HlslProperty(
    string name,
    HlslPropertyType type,
    IReadOnlyList<float> defaultValue);
```

## Supported types

| Type | Required default components | Native expression type |
| --- | ---: | ---: |
| `Scalar` | 1 | 18 |
| `Vector2` | 2 | 35 |
| `Vector3` | 3 | 52 |
| `Vector4` | 4 | 69 |
| `Matrix3x2` | 6 | 104 |
| `Matrix4x4` | 16 | 265 |

Names must be valid unique HLSL identifiers and every default component must be finite. `Scalar` can also be expressed through the older `HlslFloatProperty` API when min/max range metadata is useful.

## Layout contract

The private Composition property updater copies `valueCount * sizeof(float)` bytes from the native property blob to the mapped constant-buffer offset. The library therefore computes the native property layout and shader constant-buffer layout from the same schema.

Vectors use normal aligned float/vector storage. Matrices are intentionally represented in generated HLSL by packed float-vector backing constants and reconstructed into a logical `float3x2`/`float4x4`. This avoids relying on HLSL's implicit 16-byte matrix row/column stride while the private updater performs a contiguous byte copy.

For precompiled advanced effects, `CreateCompiledAdvanced` validates the actual reflected `UserConstants : register(b0)` layout against the complete property schema before the DXBC can enter the private Composition backend. Runtime typed compilation uses the same declaration/layout implementation.

## Updating properties

`HlslEffectBrush` provides matching setters:

```cpp
brush.SetFloat(L"Strength", 0.8f);
brush.SetVector2(L"Offset", { 2.0f, 4.0f });
brush.SetVector3(L"Gain", { 1.0f, 0.9f, 0.8f });
brush.SetVector4(L"Tint", { 1.0f, 0.9f, 0.8f, 1.0f });
brush.SetMatrix3x2(L"Transform", transform);
brush.SetMatrix4x4(L"Projection", projection);
```

The setter must match the declared `HlslPropertyType`. `GetPropertyPath` returns the native animatable Composition property path, so applications can attach the corresponding scalar/vector/matrix Composition animation instead of updating through application code every frame.

## Validation boundary

The type/layout path is implemented for C++/WinRT and C#, and generated/precompiled constant buffers are reflection-checked. Because execution still crosses a private Composition ABI, applications targeting unvalidated architecture/runtime combinations should use `HlslComposition.GetRuntimeCapabilities()` and retain a fallback.

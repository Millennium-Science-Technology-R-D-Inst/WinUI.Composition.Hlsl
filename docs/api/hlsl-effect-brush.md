# HlslEffectBrush class

Wraps the native `CompositionEffectBrush` created for an `HlslEffect` schema.

## Properties

| Property | Description |
| --- | --- |
| `Brush` | General `CompositionBrush` view. |
| `EffectBrush` | Direct `CompositionEffectBrush`. |
| `Properties` | Underlying `CompositionPropertySet`. |

## Sources

```csharp
void SetSource(string name, CompositionBrush source);
```

The source name must be declared by the effect and the brush must belong to the same `Compositor`. For ordered multi-source binding, `HlslComposition.CreateBrushWithSources` binds the provided collection to `SourceNames` in order.

## Property paths and setters

`GetPropertyPath(name)` returns the native animatable path for any declared property. The setter must match the schema type:

- `SetFloat`
- `SetVector2`
- `SetVector3`
- `SetVector4`
- `SetMatrix3x2`
- `SetMatrix4x4`

`SetFloat` additionally enforces the finite/min/max contract of `HlslFloatProperty`. Typed `HlslProperty` vector/matrix values are validated by name and type before insertion into the Composition property set.

For high-frequency updates, prefer Composition animations on `EffectBrush` using `GetPropertyPath` rather than calling a wrapper setter every frame.

```cpp
auto path = brush.GetPropertyPath(L"Offset");
auto animation = compositor.CreateVector2KeyFrameAnimation();
animation.InsertKeyFrame(1.0f, { 8.0f, 4.0f });
brush.EffectBrush().StartAnimation(path, animation);
```

Invalid property/source names, type mismatches, null/cross-compositor sources, and invalid scalar values fail with `ArgumentException`/`E_INVALIDARG`.

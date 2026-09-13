# HlslEffectBrush class

Wraps the native `CompositionEffectBrush` created for an `HlslEffect` schema.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v1.0.0  
**Assembly:** `WinUI.Composition.Hlsl.dll`

## Properties

| Property | Type | Description |
| --- | --- | --- |
| `Brush` | `CompositionBrush` | General brush view for Composition/XAML bridging. |
| `EffectBrush` | `CompositionEffectBrush` | Direct access to the native effect brush. |
| `Properties` | `CompositionPropertySet` | Native property set used by Composition animations. |

## GetPropertyPath

```csharp
public string GetPropertyPath(string name);
```

Returns the native animatable path for a declared scalar property, for example `HlslEffect.RefractionStrength`. It validates the property name once when requested.

## SetSource

```csharp
public void SetSource(string name, CompositionBrush source);
```

Assigns the effect's declared source. The source must belong to the same compositor.

## SetFloat

```csharp
public void SetFloat(string name, float value);
```

Convenience setter for low-frequency application updates. The wrapper checks declaration, finiteness, and the declared range before updating the underlying property set.

For frame-rate animation, do not call `SetFloat` every frame. Start a Composition animation on `EffectBrush` using `GetPropertyPath`; updates then stay on the native Composition animation/constant-buffer path without an application callback per frame.

The declared min/max range is enforced by `SetFloat`, not re-run by this wrapper for every native Composition animation sample. Keep keyframes/expressions inside the shader's valid operating range (or clamp inside the shader) when animating directly.

## Example

```cpp
auto property = brush.GetPropertyPath(L"Strength");
auto animation = compositor.CreateScalarKeyFrameAnimation();
animation.InsertKeyFrame(1.0f, 24.0f);
brush.EffectBrush().StartAnimation(property, animation);
```

## Exceptions

`ArgumentException` is thrown for undeclared source/property names, cross-compositor sources, non-finite values, and values outside the declared scalar range when using `SetFloat`.

# HlslComposition class

Provides static helpers that connect `HlslEffect` definitions to `Microsoft.UI.Composition` factories/brushes and bridge Composition brushes to XAML.

Namespace: `WinUI.Composition.Hlsl`

## Definition

```text
runtimeclass HlslComposition
```

`HlslComposition` has no instances. All members are static.

## Methods

### GetRuntimeCapabilities

Returns the package capability contract for the current native architecture.

```csharp
public static HlslRuntimeCapabilities GetRuntimeCapabilities();
```

```cpp
static HlslRuntimeCapabilities GetRuntimeCapabilities();
```

#### Returns

An `HlslRuntimeCapabilities` object.

#### Remarks

The call is side-effect free. It does not install hooks, scan the installed private runtime, create a `Compositor`, or compile a shader. Use it for feature presentation/diagnostics, not as a per-frame query.

---

### CreateEffectFactory

Creates a Composition effect factory for an HLSL effect.

```csharp
public static HlslEffectFactory CreateEffectFactory(
    Compositor compositor,
    HlslEffect effect);
```

#### Parameters

`compositor`  
The `Microsoft.UI.Composition.Compositor` that owns the factory and brushes created from it.

`effect`  
The immutable HLSL effect definition.

#### Returns

An `HlslEffectFactory` wrapping the native `CompositionEffectFactory`.

#### Exceptions

Throws an invalid-argument error when `compositor` or `effect` is null.

#### Remarks

Factory creation is a setup operation. Reuse the returned factory when creating multiple brushes with the same effect definition.

---

### CreateBackdropBrush

Creates an effect brush and binds a `CompositionBackdropBrush` to every public source of the effect.

```csharp
public static HlslEffectBrush CreateBackdropBrush(
    Compositor compositor,
    HlslEffect effect);
```

#### Parameters

`compositor`  
The owner compositor.

`effect`  
The HLSL effect definition.

#### Returns

A ready-to-use `HlslEffectBrush`.

#### Remarks

For a multi-source effect, the same backdrop brush is bound to each source. Use `CreateBrushWithSources` when each source must be a different Composition brush.

#### Example

```csharp
var effect = HlslEffect.CreateCompiled(Guid.Empty, library);
var brush = HlslComposition.CreateBackdropBrush(compositor, effect);
```

---

### CreateBrushWithSources

Creates an effect brush and binds an ordered collection of Composition brushes to the effect's ordered source schema.

```csharp
public static HlslEffectBrush CreateBrushWithSources(
    Compositor compositor,
    HlslEffect effect,
    IReadOnlyList<CompositionBrush> sources);
```

#### Parameters

`compositor`  
The owner compositor.

`effect`  
The effect whose source schema determines the expected source count/order.

`sources`  
The brushes to bind. Item `i` is bound to source `i`.

#### Returns

A configured `HlslEffectBrush`.

#### Exceptions

Throws an invalid-argument error when a required object is null or when `sources.Count` does not match the effect source count. Invalid/cross-compositor source brushes are rejected by the underlying `SetSource` path.

#### Remarks

For linked samplers, source order also maps to HLSL resources:

```text
sources[0] -> texture0/t0 + sampler0/s0
sources[1] -> texture1/t1 + sampler1/s1
```

The method performs setup-time collection validation only. No per-frame source-count validation is involved.

---

### CreateXamlBrush

Converts the Composition brush owned by an `HlslEffectBrush` into a XAML brush.

```csharp
public static Microsoft.UI.Xaml.Media.Brush CreateXamlBrush(HlslEffectBrush brush);
```

#### Parameters

`brush`  
The HLSL effect brush to bridge into XAML.

#### Returns

A XAML `Brush` backed by the same Composition brush.

#### Exceptions

Throws an invalid-argument error when `brush` is null.

---

### CreateXamlBrushFromCompositionBrush

Bridges an arbitrary `CompositionBrush` to a XAML brush.

```csharp
public static Microsoft.UI.Xaml.Media.Brush CreateXamlBrushFromCompositionBrush(
    CompositionBrush brush);
```

#### Parameters

`brush`  
A valid Composition brush.

#### Returns

A XAML `Brush` backed by the supplied Composition brush.

#### Exceptions

Throws an invalid-argument error when `brush` is null.

## Remarks

`HlslComposition` is a convenience layer. It does not create a custom swap chain or second visual tree. The returned objects remain part of the normal Composition/XAML pipeline.

For maximum reuse, create the immutable `HlslEffect` once, create/reuse a factory at the compositor lifetime you need, then update brush properties/sources rather than recompiling shader code.

## See also

- [HlslEffect](hlsl-effect.md)
- [HlslEffectBrush](hlsl-effect-brush.md)
- [HlslRuntimeCapabilities](hlsl-runtime-capabilities.md)
- [Get started](../get-started.md)

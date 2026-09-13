# HlslComposition class

Provides Composition/XAML bridge helpers and side-effect-free runtime capability reporting.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v1.0.0  
**Assembly:** `WinUI.Composition.Hlsl.dll`

## GetRuntimeCapabilities

```csharp
public static HlslRuntimeCapabilities GetRuntimeCapabilities();
```

Returns the packaged native-adapter capability level without installing private hooks or probing the Composition process image. Use it to gate optional materialized effects or fallback UI. See [HlslRuntimeCapabilities](hlsl-runtime-capabilities.md).

## CreateEffectFactory

```csharp
public static HlslEffectFactory CreateEffectFactory(Compositor compositor, HlslEffect effect);
```

Creates/caches the Composition factory for the effect description.

When a custom shader must consume an already-built **native `IGraphicsEffect` graph**, use `HlslEffectKind.MaterializedSampler`, call `CreateGraphicsEffectWithSource(upstream)`, obtain `GetAnimatablePropertyPaths()`, and then call the standard `Compositor.CreateEffectFactory(graph, paths)`.

## CreateBackdropBrush

```csharp
public static HlslEffectBrush CreateBackdropBrush(Compositor compositor, HlslEffect effect);
```

Convenience path that binds `compositor.CreateBackdropBrush()` to every declared source parameter. For a single-source effect this is equivalent to binding the normal `SourceName`; for a linked multi-source effect all named inputs receive the same backdrop brush.

## CreateBrushWithSources

```csharp
public static HlslEffectBrush CreateBrushWithSources(
    Compositor compositor,
    HlslEffect effect,
    IReadOnlyList<CompositionBrush> sources);
```

Creates the effect brush and binds an ordered list of Composition brushes to the effect's ordered `SourceNames`. The number of brushes must exactly match the effect source count, every source must be non-null, and every source must belong to the same `Compositor` as the destination brush.

This is the direct public helper for linked multi-source effects when each HLSL input should consume a different Composition brush:

```csharp
var effect = HlslEffect.CreateAdvanced(
    shader,
    HlslEffectKind.Sampler,
    new[] { "First", "Second" },
    Array.Empty<HlslProperty>());

var brush = HlslComposition.CreateBrushWithSources(
    compositor,
    effect,
    new CompositionBrush[]
    {
        compositor.CreateBackdropBrush(),
        compositor.CreateColorBrush(),
    });
```

Source ordering is ABI-significant: `sources[0]` is bound to `SourceNames[0]`, `sources[1]` to `SourceNames[1]`, and so on.

## CreateXamlBrush

```csharp
public static Brush CreateXamlBrush(HlslEffectBrush brush);
```

Wraps an HLSL effect brush in `XamlCompositionBrushBase` for XAML brush properties.

## CreateXamlBrushFromCompositionBrush

```csharp
public static Brush CreateXamlBrushFromCompositionBrush(CompositionBrush brush);
```

Bridges any compatible `CompositionBrush` to XAML. This is useful when the graph was assembled through standard Windows Graphics Effects/Composition APIs rather than the HLSL convenience factory.

The bridge stays in the XAML/Composition visual system. It does not create a `SwapChainPanel`, app-owned swap chain, independent HWND overlay, or another rendering tree.

## Validated mixed-graph shape

```text
XAML/Backdrop source
    -> native Windows Graphics Effects nodes
    -> MaterializedSampler HLSL node
    -> CompositionEffectFactory
    -> CompositionEffectBrush
    -> CreateXamlBrushFromCompositionBrush
    -> XAML Brush property
```

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

Creates/caches the Composition factory for the effect description. Advanced linked `Color` and `Sampler` effects may declare multiple named sources; bind them on the returned `HlslEffectBrush` with `SetSource`.

When a custom shader must consume an already-built **native `IGraphicsEffect` graph**, use `HlslEffectKind.MaterializedSampler`, call `CreateGraphicsEffectWithSource(upstream)`, obtain `GetAnimatablePropertyPaths()`, and then call the standard `Compositor.CreateEffectFactory(graph, paths)`. `MaterializedSampler` remains a one-source graph-lowering mode.

## CreateBackdropBrush

```csharp
public static HlslEffectBrush CreateBackdropBrush(Compositor compositor, HlslEffect effect);
```

Convenience path for effects whose inputs should all read the compositor backdrop. For ordinary one-source effects it binds that source to one `compositor.CreateBackdropBrush()`. For advanced linked multi-source `Color`/`Sampler` effects it binds the same backdrop brush to every declared source name, so the helper returns a fully bound brush rather than leaving secondary source parameters unresolved.

Use `CreateEffectFactory(...).CreateBrush()` plus `HlslEffectBrush.SetSource(name, brush)` when each source should come from a different `CompositionBrush`.

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

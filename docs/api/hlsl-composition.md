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

Creates/caches the Composition factory for the effect's normal one-source description.

When a custom shader must consume an already-built **native `IGraphicsEffect` graph**, use `HlslEffectKind.MaterializedSampler`, call `CreateGraphicsEffectWithSource(upstream)`, obtain `GetAnimatablePropertyPaths()`, and then call the standard `Compositor.CreateEffectFactory(graph, paths)`. Ordinary `Color/Sampler` mixed-native upstream graphs are intentionally rejected until their private linked-subgraph ABI is verified.

## CreateBackdropBrush

```csharp
public static HlslEffectBrush CreateBackdropBrush(Compositor compositor, HlslEffect effect);
```

Convenience path for one-source effects whose source should be `compositor.CreateBackdropBrush()`.

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

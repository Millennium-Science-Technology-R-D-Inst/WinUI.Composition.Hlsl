# HlslEffectFactory class

Represents a compiled HLSL effect factory associated with one `Compositor`.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v1.0.0  
**Assembly:** `WinUI.Composition.Hlsl.dll`

## Factory

```csharp
public CompositionEffectFactory Factory { get; }
```

Exposes the underlying standard Composition factory for advanced Composition scenarios.

## CreateBrush

```csharp
public HlslEffectBrush CreateBrush();
```

Creates an independent brush and initializes declared scalar properties to their defaults.

Factories are cached per compositor/effect description when the underlying Composition object supports weak references. The wrapper does not introduce a separate rendering surface or swap chain.

# HlslEffectFactory class

Represents a compiled HLSL effect factory associated with a `Compositor`.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v0.1.0-preview.7  
**Assembly:** `WinUI.Composition.Hlsl.dll`


## Methods

### CreateBrush

```csharp
public HlslEffectBrush CreateBrush();
```

Creates a new brush and initializes every declared scalar property to its default value. Create the factory with [HlslComposition.CreateEffectFactory](hlsl-composition.md#createeffectfactory).

## Remarks

Factories are cached per compositor and effect description when the underlying Composition object supports weak references. Brushes are independent instances.



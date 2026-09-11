# HlslEffectBrush class

Wraps the native `CompositionEffectBrush` created for an `HlslEffect` schema.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v0.1.0-preview.8  
**Assembly:** `WinUI.Composition.Hlsl.dll`


## Properties

| Property | Type | Description |
| --- | --- | --- |
| `Brush` | `CompositionBrush` | Gets the underlying brush for use with Composition visuals or `CreateXamlBrush`. |

## Methods

### SetSource

```csharp
public void SetSource(string name, CompositionBrush source);
```

Assigns the single source declared by the effect. The source must belong to the same compositor.

### SetFloat

```csharp
public void SetFloat(string name, float value);
```

Assigns a declared scalar property. The name must exist in the effect schema and the value must be finite and inside its inclusive range.

## Exceptions

| Exception | Condition |
| --- | --- |
| `ArgumentException` | Source/property name is undeclared, a source belongs to another compositor, or a value is outside its schema. |



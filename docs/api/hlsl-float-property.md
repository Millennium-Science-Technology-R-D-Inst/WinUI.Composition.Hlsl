# HlslFloatProperty class

Declares a scalar shader property.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v0.1.0-preview.8  
**Assembly:** `WinUI.Composition.Hlsl.dll`


```csharp
public HlslFloatProperty(string name, float defaultValue, float minimum, float maximum);
```

## Parameters

| Parameter | Description |
| --- | --- |
| `name` | HLSL identifier used in the generated constant buffer and public property path. |
| `defaultValue` | Initial value assigned to each new brush. |
| `minimum` | Inclusive minimum accepted by `SetFloat`. |
| `maximum` | Inclusive maximum accepted by `SetFloat`. |

## Properties

`Name`, `DefaultValue`, `Minimum`, and `Maximum` return the constructor values.

## Example

```csharp
var gain = new HlslFloatProperty("Gain", 0.5f, 0.0f, 1.0f);
```

## Exceptions

Throws `ArgumentException` for an invalid HLSL identifier, non-finite value, inverted range, or default value outside the declared range.



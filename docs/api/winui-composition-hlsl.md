# WinUI.Composition.Hlsl namespace

Provides HLSL effect descriptions, Composition factories and brushes, and a ready-to-use Liquid Glass material for WinUI 3.

## Classes

| Class | Description |
| --- | --- |
| [HlslComposition](hlsl-composition.md) | Provides static methods that create Composition and XAML objects from an HLSL description. |
| [HlslEffect](hlsl-effect.md) | Describes an immutable color-transform or custom-sampler shader. |
| [HlslEffectBrush](hlsl-effect-brush.md) | Wraps a `CompositionEffectBrush` and applies declared sources and scalar properties. |
| [HlslEffectFactory](hlsl-effect-factory.md) | Creates brush instances from a compiled effect description. |
| [HlslFloatProperty](hlsl-float-property.md) | Declares a named scalar shader property and its accepted range. |
| [HlslShaderLibrary](hlsl-shader-library.md) | Owns validated, precompiled DXBC shader-library bytecode. |
| [LiquidGlassBrush](liquid-glass-brush.md) | XAML brush that renders the built-in Liquid Glass material. |
| [LiquidGlassMaterial](liquid-glass-material.md) | Composition-level Liquid Glass material. |

## Enums

| Enum | Description |
| --- | --- |
| [HlslEffectKind](hlsl-effect-kind.md) | Identifies a `Color` or `Sampler` effect. |
| `HlslShaderProfile` | Identifies the SM4 library profile used by precompiled bytecode. |

## Remarks

The public API is a Windows Runtime API shared by C++/WinRT and C# consumers. Shader compilation and native runtime initialization occur when an effect factory is created. Creating an `HlslEffect` only creates an immutable description.

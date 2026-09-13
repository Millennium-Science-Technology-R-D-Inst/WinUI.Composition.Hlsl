# WinUI.Composition.Hlsl namespace

Provides HLSL-backed Windows Graphics Effects/Composition nodes, async/build-time shader compilation, XAML brush bridging, and the built-in Liquid Glass material for WinUI 3.

## Classes

| Class | Description |
| --- | --- |
| [HlslComposition](hlsl-composition.md) | Composition/XAML bridge and capability entry points. |
| [HlslCompiler](hlsl-compiler.md) | Background FXC compiler for runtime-generated/cached shaders. |
| [HlslEffect](hlsl-effect.md) | Immutable color, sampler, or materialized-sampler description. |
| [HlslEffectBrush](hlsl-effect-brush.md) | Wraps `CompositionEffectBrush` and exposes its native animation property set. |
| [HlslEffectFactory](hlsl-effect-factory.md) | Wraps the standard `CompositionEffectFactory`. |
| [HlslFloatProperty](hlsl-float-property.md) | Declares a named scalar shader property and range. |
| [HlslRuntimeCapabilities](hlsl-runtime-capabilities.md) | Side-effect-free packaged runtime support information. |
| [HlslShaderLibrary](hlsl-shader-library.md) | Owns precompiled/cached FXC SM4 DXBC. |
| [LiquidGlassBrush](liquid-glass-brush.md) | XAML Liquid Glass brush with fallback. |
| [LiquidGlassMaterial](liquid-glass-material.md) | Composition-level Liquid Glass material. |

## Enums

| Enum | Description |
| --- | --- |
| [HlslEffectKind](hlsl-effect-kind.md) | `Color`, `Sampler`, or `MaterializedSampler` shader/graph contract. |
| `HlslShaderProfile` | SM4 shader-linking profile (`Level91`, `Level93`, `Pixel40`). |
| `HlslNativeArchitecture` | Packaged native adapter architecture. |
| `HlslRuntimeSupportLevel` | Validated/experimental/unsupported support claim. |

## Rendering model

The library does not expose an application-owned swap-chain renderer. Public effects become `IGraphicsEffect` nodes, are consumed by normal Composition factories/brushes, and can be bridged back to XAML through `XamlCompositionBrushBase`.

```text
HLSL/DXBC -> IGraphicsEffect -> CompositionEffectFactory
          -> CompositionEffectBrush -> XamlCompositionBrushBase -> XAML
```

Dynamic source compilation and private adapter initialization are separate concerns. Creating an immutable effect description does not itself start rendering.

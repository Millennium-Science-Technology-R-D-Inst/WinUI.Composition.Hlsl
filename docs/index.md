# WinUI.Composition.Hlsl documentation

`WinUI.Composition.Hlsl` is a native Windows Runtime component that turns custom HLSL into Windows Graphics Effects / Microsoft.UI.Composition nodes and bridges the resulting Composition brushes back into WinUI 3 XAML.

The rendering model stays inside Composition/XAML; the library does not require `SwapChainPanel`, an application-owned swap chain, or a separate overlay renderer.

## Install

```xml
<PackageReference Include="WinUI.Composition.Hlsl" Version="1.0.0" />
```

The package contains native x64/x86/ARM64 assets, a .NET projection, and shared C++/C# shader build targets.

## Recommended production path

Declare known shaders at build time:

```xml
<HlslCompositionShader Include="Effects\Glass.hlsl">
  <Kind>MaterializedSampler</Kind>
  <Profile>Pixel40</Profile>
</HlslCompositionShader>
```

FXC then validates source and entry-point contracts during MSBuild and emits Composition-compatible DXBC. Runtime-generated shaders can use [HlslCompiler](api/hlsl-compiler.md) asynchronously and persist [HlslShaderLibrary.Bytecode](api/hlsl-shader-library.md) for later runs.

## Effect contracts

- `Color`: `float4 PSBody(float4 color)`.
- `Sampler`: `float4 Shade(float2 uv, float4 samplerDataExt)`.
- `MaterializedSampler`: `float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData)` for one materialized upstream native graph.

The build/runtime front end generates private sampler edge-mode wrappers. Do not hand-code them in application shaders.

## Standard Composition graph

```text
IGraphicsEffectSource / native Composition effects
    -> HlslEffect.CreateGraphicsEffectWithSource(...)
    -> Compositor.CreateEffectFactory(...)
    -> CompositionEffectBrush
    -> HlslComposition.CreateXamlBrushFromCompositionBrush(...)
    -> XAML Brush property
```

For simple backdrop effects, `HlslComposition.CreateBackdropBrush` remains the convenience API.

## API reference

- [WinUI.Composition.Hlsl namespace](api/winui-composition-hlsl.md)
- [HlslComposition](api/hlsl-composition.md)
- [HlslCompiler](api/hlsl-compiler.md)
- [HlslEffect](api/hlsl-effect.md)
- [HlslEffectKind](api/hlsl-effect-kind.md)
- [HlslFloatProperty](api/hlsl-float-property.md)
- [HlslEffectFactory](api/hlsl-effect-factory.md)
- [HlslEffectBrush](api/hlsl-effect-brush.md)
- [HlslRuntimeCapabilities](api/hlsl-runtime-capabilities.md)
- [HlslShaderLibrary](api/hlsl-shader-library.md)
- [LiquidGlassMaterial](api/liquid-glass-material.md)
- [LiquidGlassBrush](api/liquid-glass-brush.md)

## Design notes

- [Materialized graph compilation](design/materialized-graph-runtime.md)
- [Precompiled and asynchronous shader libraries](design/precompiled-shaders.md)

## Support boundary

Windows App SDK 2.4 x64 is the validated private-ABI baseline. x86 and ARM64 adapters are experimental. Query `HlslComposition.GetRuntimeCapabilities()` before enabling optional materialized effects on architectures/builds where your application requires a fallback.

The current backend intentionally supports one named public source and at most one custom HLSL node in a lowered graph. Multi-texture custom inputs, arbitrary custom-node chains, and unverified private vector/matrix property metadata remain unsupported rather than guessed.

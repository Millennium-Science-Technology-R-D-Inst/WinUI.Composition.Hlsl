# WinUI.Composition.Hlsl documentation

`WinUI.Composition.Hlsl` is a native Windows Runtime component that turns custom HLSL into Windows Graphics Effects / Microsoft.UI.Composition nodes and bridges the resulting Composition brushes back into WinUI 3 XAML.

The rendering model stays inside Composition/XAML; the library does not require `SwapChainPanel`, an application-owned swap chain, or a separate overlay renderer.

## Install

```xml
<PackageReference Include="WinUI.Composition.Hlsl" Version="1.0.0" />
```

The package contains native x64/x86/ARM64 assets, a .NET projection, and shared C++/C# shader build targets.

## Recommended production path

Declare known shaders at build time. `Kind` defaults to `Auto` and `Profile` defaults to `Pixel40`, so the common declaration is simply:

```xml
<HlslCompositionShader Include="Effects\Glass.hlsl" />
```

The build front end probes the supported public contracts (`Color`, `Sampler`, `MaterializedSampler`) and requires exactly one match. Specify `<Kind>` explicitly only when source intentionally matches more than one contract. Per-shader `Defines`, `IncludeDirectories`, `OutputName`, and `HeaderVariableName` remain available when needed.

FXC validates source and entry-point contracts during MSBuild and emits a Composition-compatible SM4 shader-linking library. Shared `.hlsli` files can be listed as `HlslCompositionInclude` so changes invalidate the incremental build.

Native C++ consumers generate a self-contained `.g.h` byte-array header by default and keep the corresponding DXBC only as an intermediate build artifact. The generated HLSL intermediate directory is added to the native compiler include path, so application source can directly `#include` the generated header. Set `HlslCompositionPublishAsContent=true` only when a native application intentionally wants the same compiled shader as a loose `Hlsl\...` asset.

Generated libraries carry the resolved effect kind/profile inside the DXBC as reserved metadata, so normal native code does not repeat the MSBuild configuration:

```cpp
#include "Glass.g.h"
import winrt.WinUI.Composition.Hlsl;

using namespace winrt::WinUI::Composition::Hlsl;

auto effect = HlslEffect::CreateCompiledFromGeneratedByteArray(
    {},
    g_Effects_Glass_Shader);
```

Managed consumers do not generate C++ headers; their compiled shader libraries are published/deployed under the `Hlsl\...` application-content path by default. Generated assets are self-describing there as well:

```csharp
var library = await HlslShaderLibrary.LoadGeneratedFromApplicationUriAsync(
    new Uri("ms-appx:///Hlsl/Effects/Glass.dxbc"));

var effect = HlslEffect.CreateCompiled(Guid.Empty, library);
```

Runtime-generated shaders can use [HlslCompiler](api/hlsl-compiler.md) asynchronously. Passing `HlslEffectKind.Auto` applies the same contract inference model at runtime; a concrete kind avoids the probe cost when the application already knows the contract. Runtime compilation also supports bounded macro variants and [HlslShaderLibrary.Bytecode](api/hlsl-shader-library.md) persistence for later runs.

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
- [HlslProperty](api/hlsl-property.md)
- [HlslEffectFactory](api/hlsl-effect-factory.md)
- [HlslEffectBrush](api/hlsl-effect-brush.md)
- [HlslRuntimeCapabilities](api/hlsl-runtime-capabilities.md)
- [HlslShaderLibrary](api/hlsl-shader-library.md)
- [LiquidGlassMaterial](api/liquid-glass-material.md)
- [LiquidGlassBrush](api/liquid-glass-brush.md)

## Design notes

- [Materialized graph compilation](design/materialized-graph-runtime.md)
- [Precompiled and asynchronous shader libraries](design/precompiled-shaders.md)
- [Runtime safety and lifetime contract](design/runtime-safety.md)

## Support boundary

Windows App SDK 2.4 x64 is the validated private-ABI baseline. x86 and ARM64 adapters are experimental. Query `HlslComposition.GetRuntimeCapabilities()` before enabling optional materialized effects on architectures/builds where your application requires a fallback.

Capability reporting is deliberately side-effect free. Private ABI resolution/patching remains lazy and must fail closed if the loaded native runtime cannot be resolved safely.

The current backend intentionally supports one named public source and at most one custom HLSL node in a lowered graph. Multi-texture custom inputs, arbitrary custom-node chains, and unverified private vector/matrix property metadata remain unsupported rather than guessed. `HlslProperty` therefore currently rejects non-scalar values instead of exposing an unverified private updater path.

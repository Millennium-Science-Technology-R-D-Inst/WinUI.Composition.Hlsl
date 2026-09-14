<p align="center">
  <img src="assets/MainLogo.png" alt="WinUI.Composition.Hlsl logo" width="220" />
</p>

<h1 align="center">WinUI.Composition.Hlsl</h1>

<p align="center">Native HLSL nodes for WinUI 3 Composition, plus XAML material brushes.</p>
<p align="center"><a href="README.md">English</a> · <a href="README_zh_cn.md">简体中文</a></p>

<p align="center">
  <a href="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/ci.yml/badge.svg?branch=master"></a>
  <a href="LICENSE.txt"><img alt="License" src="https://img.shields.io/badge/license-MIT-blue.svg"></a>
  <img alt="C++23" src="https://img.shields.io/badge/C%2B%2B-23-00599C?logo=cplusplus">
  <img alt="WinUI 3" src="https://img.shields.io/badge/WinUI-3-0078D4">
</p>

## What this package does

`WinUI.Composition.Hlsl` lets application HLSL participate in the existing Windows Graphics Effects / `Microsoft.UI.Composition` / WinUI 3 XAML pipeline:

```text
HLSL / FXC linkable library
    -> IGraphicsEffect
    -> CompositionEffectFactory
    -> CompositionEffectBrush
    -> XamlCompositionBrushBase
    -> XAML
```

It is not an app-owned D3D renderer: no `SwapChainPanel`, custom presentation loop, overlay HWND, or second visual tree is required. The public API is WinRT and is consumable from C++/WinRT and C#.

## Requirements and architecture support

The NuGet package requires **Windows App SDK 1.6 or later**. Native runtime assets are shipped for **x86, x64, and ARM64**, and all three are public supported architectures.

The custom backend uses a private Composition implementation ABI. x86/x64 resolution is version-independent rather than selected from a hard-coded Windows App SDK version table, so the package is not capped at Windows App SDK 2.4. If Windows ever removes or fundamentally redesigns the required private mechanism, the adapter is designed to fail closed rather than write guessed layouts.

## Main capabilities

- Linked `Color` and `Sampler` HLSL with **1-16 ordered sources**.
- Deterministic sampler resource ABI: source `i` maps to `texture{i}:t{i}` and `sampler{i}:s{i}`.
- Single-source `MaterializedSampler` for sampling a materialized upstream native Composition graph.
- Typed animatable properties: `Scalar`, `Vector2`, `Vector3`, `Vector4`, `Matrix3x2`, and `Matrix4x4`.
- Build-time FXC compilation with `<HlslCompositionShader>` for C++ and C#.
- Native C++ `.g.h` embedding by default; managed self-describing DXBC content.
- Asynchronous runtime `HlslCompiler` for genuinely dynamic HLSL.
- Immutable/cachable `HlslShaderLibrary` objects.
- Composition property paths/setters and Composition-thread animation.
- `LiquidGlassMaterial` and `LiquidGlassBrush`.

The public contract does not yet claim multi-source `MaterializedSampler`, arbitrary multi-custom-node graph lowering, or arbitrary native nodes after a custom materialized pass. Those graph-planning features are being developed separately rather than being enabled by removing safety checks.

## Quick start

Add the package and a shader item:

```xml
<ItemGroup>
  <PackageReference Include="WinUI.Composition.Hlsl" Version="1.0.0" />
  <HlslCompositionShader Include="Effects\Invert.hlsl" />
</ItemGroup>
```

`Effects/Invert.hlsl`:

```hlsl
export float4 PSBody(float4 color)
{
    return float4(color.a - color.rgb, color.a);
}
```

C++/WinRT:

```cpp
#include "Invert.g.h"
import winrt.WinUI.Composition.Hlsl;

using namespace winrt::WinUI::Composition::Hlsl;

auto effect = HlslEffect::CreateCompiledFromGeneratedByteArray(
    {}, g_Effects_Invert_Shader);
auto brush = HlslComposition::CreateBackdropBrush(compositor, effect);
```

C#:

```csharp
var library = await HlslShaderLibrary.LoadGeneratedFromApplicationUriAsync(
    new Uri("ms-appx:///Hlsl/Effects/Invert.dxbc"));
var effect = HlslEffect.CreateCompiled(Guid.Empty, library);
var brush = HlslComposition.CreateBackdropBrush(compositor, effect);
```

For production shaders, build-time compilation is preferred. HLSL syntax/contract errors fail MSBuild instead of being moved into the render path.

## Shader contracts

`Kind=Auto`, `Profile=Pixel40`, and `SourceCount=1` are build defaults.

```hlsl
// Color, one source
export float4 PSBody(float4 color);

// Color, multiple linked sources
float4 Shade(float4 color0, float4 color1);

// Sampler, one linked source
float4 Shade(float2 uv, float4 samplerDataExt);

// Sampler, two linked sources
float4 Shade(float2 uv0, float4 samplerDataExt0,
             float2 uv1, float4 samplerDataExt1);

// MaterializedSampler, one materialized source
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData);
```

For samplers, the package generates the private `PSBody*` edge-mode wrappers and the `textureN`/`samplerN` declarations. Application shader code uses those resources but does not redeclare them.

## Performance model

Compilation and structural validation are setup operations, not rendering operations:

- static HLSL normally compiles in MSBuild;
- generated DXBC reflection happens when a library is created/loaded;
- factories and brushes should be reused;
- property updates do not recompile HLSL;
- rendering does not repeatedly reflect DXBC;
- `GetRuntimeCapabilities()` is side-effect free and does not probe/patch the private runtime.

The package keeps runtime checks for facts that are only known at runtime (for example source object/count validity), while deterministic shader-authoring errors are handled by the compiler/build pipeline whenever possible.

## Documentation

Start with the documentation set rather than treating the README as the API manual:

1. [Get started](docs/get-started.md)
2. [Concepts](docs/concepts.md)
3. [Architecture](docs/architecture.md)
4. [API reference](docs/api/index.md)
5. [Design notes](docs/index.md#design-reference)

Useful references:

- [HlslComposition](docs/api/hlsl-composition.md)
- [HlslCompiler](docs/api/hlsl-compiler.md)
- [HlslEffect](docs/api/hlsl-effect.md)
- [HlslShaderLibrary](docs/api/hlsl-shader-library.md)
- [HlslProperty](docs/api/hlsl-property.md)
- [HlslRuntimeCapabilities](docs/api/hlsl-runtime-capabilities.md)
- [Sampler resource binding contract](docs/design/resource-binding-contract.md)
- [Materialized graph compilation](docs/design/materialized-graph-runtime.md)

## Build the repository

```powershell
.\build.ps1 -Configuration Release
.\build.ps1 -Configuration Debug -Platform Win32
.\build.ps1 -Configuration Debug -Platform ARM64
.\pack.ps1
.\tests\build.ps1 -Language Cpp
.\tests\build.ps1 -Language CSharp
```

CI builds x64/Win32/ARM64 native assets, the CsWinRT projection, generated shader fixtures, a preview NuGet package, and downstream C++/C# package consumers.

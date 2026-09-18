<p align="center">
  <img src="assets/MainLogo.png" alt="WinUI.Composition.Hlsl logo" width="220" />
</p>

<h1 align="center">WinUI.Composition.Hlsl</h1>

<p align="center">Native HLSL nodes for WinUI 3 Composition, plus XAML material brushes and native liquid-glass controls.</p>
<p align="center"><a href="README.md">English</a> · <a href="README_zh_cn.md">简体中文</a></p>

<p align="center">
  <a href="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/ci.yml/badge.svg?branch=master"></a>
  <a href="https://www.nuget.org/packages/WinUI.Composition.Hlsl"><img alt="WinUI.Composition.Hlsl NuGet" src="https://img.shields.io/nuget/v/WinUI.Composition.Hlsl?logo=nuget"></a>
  <a href="https://www.nuget.org/packages/WinUI.LiquidGlass"><img alt="WinUI.LiquidGlass NuGet" src="https://img.shields.io/nuget/v/WinUI.LiquidGlass?logo=nuget"></a>
  <a href="LICENSE.txt"><img alt="License" src="https://img.shields.io/badge/license-MIT-blue.svg"></a>
  <img alt="C++23" src="https://img.shields.io/badge/C%2B%2B-23-00599C?logo=cplusplus">
  <img alt="WinUI 3" src="https://img.shields.io/badge/WinUI-3-0078D4">
</p>

## What this repository does

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

The repository also contains `WinUI.LiquidGlass`, a native C++/WinRT WinUI 3 control library built on top of the core material. It keeps native WinUI input/layout/UI Automation semantics while adding liquid-glass visuals, optical interaction, spring motion, and reusable control presets.

## Packages

| Package | Purpose |
| --- | --- |
| `WinUI.Composition.Hlsl` | Core HLSL/Composition runtime, effect graph API, `LiquidGlassMaterial`, and `LiquidGlassBrush`. |
| `WinUI.LiquidGlass` | Native WinUI 3 liquid-glass controls and templates; depends on the matching core package. |

For controls, reference both packages using matching versions:

```xml
<ItemGroup>
  <PackageReference Include="WinUI.Composition.Hlsl" Version="1.0.*" />
  <PackageReference Include="WinUI.LiquidGlass" Version="1.0.*" />
</ItemGroup>
```

## Requirements and compatibility

The NuGet packages require **Windows App SDK 1.6 or later** and ship native runtime assets for **x86, x64, and ARM64**.

Each validated `master` push is published automatically to NuGet.org as a stable `1.0.<CI run number>` package.

Custom shader execution relies on a private, undocumented Composition implementation ABI. Unsupported layouts fail closed instead of writing guessed private data. See [Architecture](docs/architecture.md) and [Runtime safety](docs/design/runtime-safety.md) for implementation details and support boundaries.

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
- Native `WinUI.LiquidGlass` controls including card, magnifier, buttons, choice controls, Slider, input controls, ToggleSwitch, and TabBar.
- Separable Gaussian backdrop blur, SDF coverage, refraction/magnification, chromatic dispersion, pointer specular lighting, and control-specific motion/elevation.

The public contract does not yet claim multi-source `MaterializedSampler`, arbitrary multi-custom-node graph lowering, or arbitrary native nodes after a custom materialized pass. Those graph-planning features are being developed separately rather than being enabled by removing safety checks.

## Quick start

Add the core package and a shader item:

```xml
<ItemGroup>
  <PackageReference Include="WinUI.Composition.Hlsl" Version="1.0.*" />
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

Control hot paths follow the same principle. Value/pointer updates should modify existing Composition state, not walk XAML templates or allocate brushes every frame. See the Slider section in [WinUI.LiquidGlass controls](docs/liquid-glass-controls.md).

## Documentation

Start with the documentation set rather than treating the README as the API manual:

1. [Get started](docs/get-started.md)
2. [Concepts](docs/concepts.md)
3. [Architecture](docs/architecture.md)
4. [WinUI.LiquidGlass controls](docs/liquid-glass-controls.md)
5. [API reference](docs/api/index.md)
6. [Design notes](docs/index.md#design-reference)

Useful references:

- [HlslComposition](docs/api/hlsl-composition.md)
- [HlslCompiler](docs/api/hlsl-compiler.md)
- [HlslEffect](docs/api/hlsl-effect.md)
- [HlslShaderLibrary](docs/api/hlsl-shader-library.md)
- [HlslProperty](docs/api/hlsl-property.md)
- [HlslRuntimeCapabilities](docs/api/hlsl-runtime-capabilities.md)
- [LiquidGlassBrush](docs/api/liquid-glass-brush.md)
- [LiquidGlassMaterial](docs/api/liquid-glass-material.md)
- [WinUI.LiquidGlass controls](docs/liquid-glass-controls.md)
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

CI builds x64/Win32/ARM64 native assets, the CsWinRT projection, generated shader fixtures, both NuGet packages, and downstream C++/C# package consumers. Successful `master` push runs publish the validated packages directly from `ci.yml` to NuGet.org through OIDC trusted publishing.

## License

[MIT License](LICENSE.txt).

## Thanks (in no particular order)

Inspired by @apkipa's WUILiquidGlassDemo work. 

https://github.com/luckyelysia/LiquidGlassWinUI

https://github.com/kube/kube.io

More reference: [THIRD_PARTY_NOTICES](/src/WinUI.LiquidGlass/THIRD_PARTY_NOTICES.md)
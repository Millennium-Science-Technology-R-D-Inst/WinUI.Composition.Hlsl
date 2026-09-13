<p align="center">
  <img src="assets/MainLogo.png" alt="WinUI.Composition.Hlsl logo" width="220" />
</p>

<h1 align="center">WinUI.Composition.Hlsl</h1>

<p align="center">Native HLSL nodes for WinUI 3 Composition, plus XAML material brushes.</p>

<p align="center"><a href="README.md">English</a> · <a href="README_zh_cn.md">简体中文</a></p>

<p align="center">
  <a href="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/ci.yml/badge.svg?branch=master"></a>
  <a href="https://www.nuget.org/packages/WinUI.Composition.Hlsl"><img alt="NuGet" src="https://img.shields.io/badge/NuGet-publishing%20soon-004880?logo=nuget&logoColor=white"></a>
  <a href="LICENSE.txt"><img alt="License" src="https://img.shields.io/badge/license-MIT-blue.svg"></a>
  <img alt="C++23" src="https://img.shields.io/badge/C%2B%2B-23-00599C?logo=cplusplus">
  <img alt="WinUI 3" src="https://img.shields.io/badge/WinUI-3-0078D4">
</p>

## What this library is

`WinUI.Composition.Hlsl` makes custom HLSL participate in the **existing Windows Graphics Effects / Microsoft.UI.Composition / XAML pipeline**. It is not an application-owned D3D renderer placed on top of XAML.

```text
HLSL / compiled shader library
    -> IGraphicsEffect
    -> CompositionEffectFactory
    -> CompositionEffectBrush
    -> XamlCompositionBrushBase
    -> XAML
```

There is no `SwapChainPanel`, custom presentation loop, app-owned swap chain, separate HWND overlay, or second visual tree in this path. That distinction is the main goal of the project: custom shader work should keep normal Composition/XAML clipping, transforms, scaling, lifetime, brush composition, and animation behavior instead of bypassing them with an overlay surface.

The WinRT API is consumable from C++/WinRT and C#. The implementation is C++23/C++/WinRT and the package includes a .NET 8 CsWinRT projection.

> [!WARNING]
> Executing private custom shader nodes requires a **private, undocumented Windows Composition / Windows App SDK ABI**. Windows App SDK **2.4 + x64** is the primary validated baseline. x86 and ARM64 adapters are present but experimental. Unknown/incompatible layouts are intended to fail closed rather than emit guessed DWM data.

## Main capabilities

- Standard `IGraphicsEffect` graph nodes through `HlslEffect.CreateGraphicsEffect*`.
- Linked `Color` and `Sampler` contracts with **1-16 ordered named sources**.
- Single-source `MaterializedSampler` for sampling a materialized upstream native Composition graph.
- Native Composition factories/brushes, with direct access to their property sets for Composition-thread animations.
- Composition-to-XAML bridging through `XamlCompositionBrushBase`.
- Build-time FXC compilation for both C++ and C# consumers through `<HlslCompositionShader>`.
- Native C++ `.g.h` embedding by default; no duplicate loose shader asset unless explicitly requested.
- Self-describing package-generated shader libraries: build-time `Kind`, `Profile`, and `SourceCount` travel with the compiled bytecode.
- Background `HlslCompiler.CompileAsync` / `CompileAdvancedAsync` for generated/development shaders.
- Immutable `HlslShaderLibrary` objects with `Bytecode` export for persistent application caches.
- Scalar shader properties mapped to native Composition constant-buffer updaters.
- Built-in `LiquidGlassMaterial` / `LiquidGlassBrush`.
- Side-effect-free runtime capability reporting.
- Native package assets for x64, x86, and ARM64.

## Production shader workflow

For shader source known when the application is built, use the package MSBuild item:

```xml
<ItemGroup>
  <HlslCompositionShader Include="Effects\MyGlass.hlsl">
    <Kind>MaterializedSampler</Kind>
    <Profile>Pixel40</Profile>
  </HlslCompositionShader>
</ItemGroup>
```

For a linked multi-source shader, add `SourceCount` (1-16):

```xml
<ItemGroup>
  <HlslCompositionShader Include="Effects\Blend.hlsl">
    <Kind>Auto</Kind>
    <Profile>Pixel40</Profile>
    <SourceCount>2</SourceCount>
  </HlslCompositionShader>
</ItemGroup>
```

The package invokes Windows SDK FXC with strictness, optimization, and warnings-as-errors. Shader syntax and public entry-point errors fail the **build**, not first render.

Application shader contracts are intentionally small:

```hlsl
// Single-source Color
export float4 PSBody(float4 color);

// Linked two-source Color
float4 Shade(float4 color0, float4 color1);

// Single-source Sampler
float4 Shade(float2 uv, float4 samplerDataExt);

// Linked two-source Sampler
float4 Shade(
    float2 uv0, float4 samplerDataExt0,
    float2 uv1, float4 samplerDataExt1);

// MaterializedSampler (exactly one source)
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData);
```

For samplers, the build front end generates the private clamp/wrap/mirror `PSBody*` exports and one `textureN/samplerN` declaration per source. For `MaterializedSampler` it also generates the identity materialization helper. Applications should not duplicate those private wrapper names themselves.

The compiler also emits a reserved metadata export containing the selected `HlslEffectKind`, `HlslShaderProfile`, and `SourceCount`. `HlslShaderLibrary` validates that metadata against the actual reflected `PSBody` ABI before accepting generated bytecode.

### Native C++

Native projects generate a self-contained `.g.h` byte-array header by default. The backing DXBC stays in the intermediate directory for build tracking and diagnostics, but is not duplicated in the final application output unless `HlslCompositionPublishAsContent=true` is set.

```cpp
#include "MyGlass.g.h"
import winrt.WinUI.Composition.Hlsl;

using namespace winrt::WinUI::Composition::Hlsl;

auto effect = HlslEffect::CreateCompiledFromGeneratedByteArray(
    {},
    g_Effects_MyGlass_Shader);
```

The C++ call does not repeat `MaterializedSampler` or `Pixel40`; those values were already declared in MSBuild and are recovered from the compiled library.

If the application needs the immutable library object separately:

```cpp
auto library = HlslShaderLibrary::CreateFromGeneratedByteArray(
    g_Effects_MyGlass_Shader);

auto effect = HlslEffect::CreateCompiled({}, library);
```

For a multi-source generated library, use `CreateCompiledAdvanced` and supply source names in the same order as the shader inputs. The name count must equal `library.SourceCount()`.

### C# / managed

Managed projects publish generated shader libraries under `Hlsl\...` application content by default. They can load the same self-describing output without repeating the profile:

```csharp
var library = await HlslShaderLibrary.LoadGeneratedFromApplicationUriAsync(
    new Uri("ms-appx:///Hlsl/Effects/MyGlass.dxbc"));

var effect = HlslEffect.CreateCompiled(Guid.Empty, library);
```

Explicit-profile `Create`, `CreateFromByteArray`, `LoadFromFileAsync`, and `LoadFromApplicationUriAsync` remain available for external or legacy DXBC that was not produced by this package.

## Runtime compilation and caching

Runtime-generated shaders can be compiled without blocking the UI thread:

```csharp
var library = await HlslCompiler.CompileAsync(
    shader,
    HlslEffectKind.MaterializedSampler,
    HlslShaderProfile.Pixel40);
```

Multi-source linked shaders use the advanced compiler surface:

```csharp
var library = await HlslCompiler.CompileAdvancedAsync(
    shader,
    HlslEffectKind.Auto,
    HlslShaderProfile.Pixel40,
    sourceCount: 2);
```

The compiler moves FXC work to a background thread and does not create a `Compositor`, XAML object, effect brush/factory, or install the private Composition adapter there. Runtime compilation emits the same self-description metadata as the MSBuild compiler.

`library.Bytecode` lets an application persist the result:

```text
first run: HLSL -> CompileAsync -> self-describing DXBC -> application cache
later:     cache -> LoadGeneratedFromFileAsync/CreateFromGeneratedByteArray
                 -> HlslEffect.CreateCompiled/CreateCompiledAdvanced -> Composition
```

Runtime DXBC reflection is retained as one-time defensive validation. It is not a per-frame check and is not the recommended way to discover ordinary shader authoring errors.

## Standard Composition graph nodes

A custom HLSL node can be materialized as a standard graphics effect:

```cpp
auto effect = WinUI::Composition::Hlsl::HlslEffect::CreateColorTransform(LR"(
export float4 PSBody(float4 color)
{
    return float4(color.a - color.rgb, color.a);
}
)");

auto graphNode = effect.CreateGraphicsEffect();
auto factory = compositor.CreateEffectFactory(graphNode);
auto brush = factory.CreateBrush();
```

For linked multi-source effects, `CreateAdvanced` declares the ordered public inputs and `CreateGraphicsEffectWithSources` supplies them in that order. At the brush level, `HlslEffectBrush.SetSource(name, brush)` binds each named Composition source independently.

For an upstream native effect graph:

```text
CompositionEffectSourceParameter
    -> GaussianBlur / transform / other native nodes
    -> HlslEffect.CreateGraphicsEffectWithSource(upstream)
    -> Compositor.CreateEffectFactory(...)
```

This is the route used to compose HLSL with normal Composition effects instead of creating a parallel rendering surface.

## MaterializedSampler

`Sampler` is the lightweight linked sampler contract and can have 1-16 named linked inputs. `MaterializedSampler` is specifically for a custom shader that needs a **real texture representation of one upstream native effect graph**, so it can perform arbitrary `Texture2D.Sample` operations and use native sampler metadata.

```hlsl
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData)
{
    float2 texel = samplerDataExt.zw;
    return texture0.Sample(sampler0, uv + texel * 2.0f);
}
```

The current validated lowering shape is:

```text
one native upstream graph
    -> materialized intermediate texture
    -> one isolated terminal MaterializedSampler
    -> output wrapper
    -> Composition/XAML
```

This public feature is built from the same private lowering already exercised by Liquid Glass; it is not a new guessed ABI.

`MaterializedSampler` **does not** currently claim support for multiple independently materialized public texture sources, arbitrary native downstream nodes after the custom sampler, or unverified linker argument encodings. Those restrictions do not apply to ordinary linked multi-source `Color`/`Sampler` inputs.

## Composition-native animation

`HlslEffectBrush` exposes its underlying `CompositionEffectBrush` and `CompositionPropertySet`:

```cpp
auto path = brush.GetPropertyPath(L"Strength");
auto animation = compositor.CreateScalarKeyFrameAnimation();
animation.InsertKeyFrame(1.0f, 24.0f);
brush.EffectBrush().StartAnimation(path, animation);
```

Use `SetFloat` for occasional application updates. For frame-rate animation, use Composition animations so application code does not cross the wrapper/API boundary each frame.

## XAML bridge

Simple backdrop usage:

```cpp
auto brush = WinUI::Composition::Hlsl::HlslComposition::CreateBackdropBrush(compositor, effect);
MyBorder().Background(WinUI::Composition::Hlsl::HlslComposition::CreateXamlBrush(brush));
```

For linked multi-source effects, `CreateBackdropBrush` binds the same compositor backdrop brush to **every declared source name**. If inputs should be different brushes, create the factory/brush explicitly and call `SetSource` per name.

Graphs assembled directly through standard Composition can use:

```cpp
MyBorder().Background(
    WinUI::Composition::Hlsl::HlslComposition::CreateXamlBrushFromCompositionBrush(compositionBrush));
```

### Built-in Liquid Glass

```xml
<hlsl:LiquidGlassBrush
    IsEnabled="True"
    BlurRadius="12"
    RefractionStrength="24"
    DispersionStrength="1.2"
    CornerRadius="12"
    BorderThickness="1"
    HighlightStrength="0.8"
    FallbackColor="#CC202020" />
```

`LiquidGlassBrush` derives from `XamlCompositionBrushBase` and can fall back to `FallbackColor` if the advanced material is disabled or unavailable.

## Runtime capability reporting

```csharp
var caps = HlslComposition.GetRuntimeCapabilities();
```

The query is side-effect free; it does not patch or scan the private runtime merely to report the package support claim.

| Architecture | Support | Graph nodes | Materialized graphs |
| --- | --- | --- | --- |
| x64 | Validated baseline | Yes | Yes |
| x86 | Experimental | Yes | Not claimed yet |
| ARM64 | Experimental | Yes | Not claimed yet |

Actual private ABI resolution remains lazy and can still fail closed if the loaded Windows/App SDK build does not match the supported shape.

## NuGet

The package metadata is currently `WinUI.Composition.Hlsl` **1.0.0**; public nuget.org publication is still pending.

```xml
<PackageReference Include="WinUI.Composition.Hlsl" Version="1.0.0" />
```

Local packages:

```powershell
.\pack.ps1
```

are written to `artifacts/packages`.

GitHub Actions uses unique `1.0.0-preview.<run-id>.<attempt>` versions and then builds both C++ and C# consumers against the exact package produced by that run.

## Compatibility

| Area | Current status |
| --- | --- |
| UI framework | WinUI 3 / Windows App SDK |
| Main private-ABI baseline | Windows App SDK 2.4 + x64 |
| x64 adapter | Validated baseline |
| x86 adapter | Experimental |
| ARM64 adapter | Experimental |
| Shader payload | FXC SM4 shader-linking DXBC (`lib_4_0` family) |
| Managed projection | .NET 8 / CsWinRT |
| Native language | C++23 / C++/WinRT |
| Public linked custom sources | 1-16 ordered named sources for `Color` / `Sampler` |
| MaterializedSampler sources | Exactly one |
| Public properties | Scalar float |
| Custom nodes per lowered graph | One |

Preview/experimental Windows App SDK builds may change private resolver fingerprints, object layouts, subgraph rules, or property-updater behavior. Compatibility must be revalidated rather than inferred from version numbers alone.

## Build

Requirements include Visual Studio 2026 with Desktop C++ and WinUI workloads, MSVC v145, a compatible Windows 10/11 SDK, .NET 8 for managed projection/tests, and restored NuGet dependencies.

```powershell
.\build.ps1 -Configuration Debug
.\build.ps1 -Configuration Release
.\build.ps1 -Configuration Debug -Platform Win32
.\build.ps1 -Configuration Debug -Platform ARM64
.\pack.ps1
.\tests\build.ps1 -Language Cpp
.\tests\build.ps1 -Language CSharp
```

The C++ `--smoke` path also exercises linked two-source `Color` and `Sampler` brushes before the existing Liquid Glass/property/resize sequence.

## Documentation

Start at [docs/index.md](docs/index.md).

Key references:

- [HlslEffect](docs/api/hlsl-effect.md)
- [HlslEffectKind](docs/api/hlsl-effect-kind.md)
- [HlslCompiler](docs/api/hlsl-compiler.md)
- [HlslShaderLibrary](docs/api/hlsl-shader-library.md)
- [HlslComposition](docs/api/hlsl-composition.md)
- [HlslEffectBrush](docs/api/hlsl-effect-brush.md)
- [HlslRuntimeCapabilities](docs/api/hlsl-runtime-capabilities.md)
- [Materialized graph compilation](docs/design/materialized-graph-runtime.md)
- [Precompiled and asynchronous shaders](docs/design/precompiled-shaders.md)

## Current limitations

The public model intentionally does not expose capabilities for which the private Composition ABI has not been established. In particular: multi-source `MaterializedSampler`, arbitrary custom-node chains/multiple custom HLSL nodes in one lowered graph, arbitrary native effects after a materialized custom sampler, arbitrary entry-point/linker contracts, and vector/matrix public property metadata are not yet supported.

## License

[MIT License](LICENSE.txt).

## Thanks

Inspired by @apkipa's WUILiquidGlassDemo work.

<p align="center">
  <img src="assets/MainLogo.png" alt="WinUI.Composition.Hlsl logo" width="220" />
</p>

<h1 align="center">WinUI.Composition.Hlsl</h1>

<p align="center">
  Native HLSL composition effects and Fluent-style XAML materials for WinUI 3.
</p>

<p align="center">
  <a href="README.md">English</a> ·
  <a href="README_zh_cn.md">简体中文</a>
</p>

<p align="center">
  <a href="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/validate-cpp-xaml-modules.yml"><img alt="CI" src="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/validate-cpp-xaml-modules.yml/badge.svg"></a>
  <a href="https://www.nuget.org/packages/WinUI.Composition.Hlsl"><img alt="NuGet" src="https://img.shields.io/badge/NuGet-publishing%20soon-004880?logo=nuget&logoColor=white"></a>
  <a href="LICENSE.txt"><img alt="License" src="https://img.shields.io/badge/license-MIT-blue.svg"></a>
  <img alt="C++23" src="https://img.shields.io/badge/C%2B%2B-23-00599C?logo=cplusplus">
  <img alt="WinUI 3" src="https://img.shields.io/badge/WinUI-3-0078D4">
</p>

## Overview

WinUI.Composition.Hlsl is a native Windows Runtime component for using custom HLSL effects in WinUI 3 Composition, together with ready-to-use XAML materials such as `LiquidGlassBrush`.

The public API is shared by C++/WinRT and C# consumers. The native implementation is written in C++23 with C++/WinRT 3.0, while the package also provides a .NET 8 CsWinRT projection. It supports dynamic source shaders, validated precompiled DXBC shader libraries, Composition factories and brushes, animatable scalar parameters, and XAML integration.

> [!WARNING]
> The custom-HLSL backend integrates with a **private, undocumented Windows Composition / Windows App Runtime ABI**. The currently validated runtime baseline is **Windows App SDK 2.4.0 on x64**. Newer, preview, or experimental Windows App SDK versions are not assumed to be binary-compatible. The runtime resolver validates known code/reference patterns and fails closed when an unsupported revision is detected. A project compiling successfully does not by itself prove that the private runtime path is compatible with that Windows App SDK build.

## Features

- Native WinRT API usable from C++/WinRT and C#.
- Dynamic HLSL color transforms and custom samplers.
- Precompiled FXC SM4 DXBC shader-library support through `HlslShaderLibrary`.
- Scalar shader properties exposed as animatable Composition properties.
- `HlslEffectFactory` / `HlslEffectBrush` wrappers for reusable effect instances.
- `HlslComposition.CreateXamlBrush` for assigning custom Composition effects to XAML brush properties.
- Built-in `LiquidGlassMaterial` and `LiquidGlassBrush` with blur, refraction, dispersion, rounded corners, borders, highlights, and fallback rendering.
- Light, Dark, and High Contrast friendly XAML resource usage.
- Materialized graph lowering for the built-in `Backdrop -> GaussianBlur -> custom sampler` pipeline.
- Native NuGet build integration for compiling consumer HLSL files into SM4 shader-linking DXBC libraries.
- Runtime validation for effect schemas, DXBC exports, signatures, scalar constant-buffer layout, and private ABI revisions.

## NuGet

The NuGet package is **not public yet**. The package metadata currently targets `WinUI.Composition.Hlsl` **1.0.0**; the link below is reserved for the future public package.

[![NuGet package placeholder](https://img.shields.io/badge/WinUI.Composition.Hlsl-1.0.0%20%7C%20publishing%20soon-004880?logo=nuget&logoColor=white)](https://www.nuget.org/packages/WinUI.Composition.Hlsl)

Once published, the intended package reference is:

```xml
<PackageReference Include="WinUI.Composition.Hlsl" Version="1.0.0" />
```

For local development, build and pack the repository instead:

```powershell
.\pack.ps1
```

The generated package is written to `artifacts/packages`.

## Quick start

### Liquid Glass in XAML

Add the WinRT namespace to the page or resource dictionary and use the brush directly:

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

`LiquidGlassBrush` derives from `XamlCompositionBrushBase`. Its numeric dependency properties are `Double` for WinUI XAML conversion and are validated and converted to GPU `float` values internally. When advanced effects are unavailable, initialization fails, or `IsEnabled` is false, the brush uses `FallbackColor`.

For theme-aware applications, define the same semantic brush key in Light and Dark theme dictionaries and use a system brush in High Contrast. See [LiquidGlassBrush](docs/api/liquid-glass-brush.md) for a complete example.

### Custom HLSL effect

```cpp
auto compositor = Microsoft::UI::Xaml::Media::CompositionTarget::GetCompositorForCurrentThread();

auto effect = WinUI::Composition::Hlsl::HlslEffect::CreateColorTransform(LR"(
export float4 PSBody(float4 color)
{
    return float4(color.a - color.rgb, color.a);
}
)");

auto brush = WinUI::Composition::Hlsl::HlslComposition::CreateBackdropBrush(compositor, effect);
MyBorder().Background(
    WinUI::Composition::Hlsl::HlslComposition::CreateXamlBrush(brush));
```

Dynamic color shaders export `float4 PSBody(float4 color)`. Dynamic custom samplers define `float4 Shade(float2 uv, float4 samplerDataExt)`; the runtime generates the private linker wrapper exports before compiling the source.

Effects with declared scalar properties automatically expose those properties through the generated constant buffer and effect brush, allowing values to be updated or animated without rebuilding the description.

### Precompiled shaders

Production effects can avoid runtime `D3DCompile` by using an FXC SM4 shader-linking DXBC library and `HlslShaderLibrary`.

A native C++ consumer can also ask the NuGet build target to compile an HLSL file:

```xml
<ItemGroup>
  <HlslCompositionShader Include="Effects\MyGlass.hlsl">
    <Kind>Sampler</Kind>
    <ShaderModel>4.0</ShaderModel>
  </HlslCompositionShader>
</ItemGroup>
```

The target emits the generated header and `.dxbc` payload and copies the DXBC output into the application output directory. SDK-style C# projects can consume precompiled DXBC through the WinRT API, but automatic `<HlslCompositionShader>` compilation is currently a native Visual C++ build integration only.

## Compatibility

| Area | Current status |
| --- | --- |
| UI framework | WinUI 3 / Windows App SDK only |
| UWP / WinUI 2 | Not supported |
| Validated Windows App SDK baseline | 2.4.0 |
| Private custom-HLSL runtime | x64 implemented and validated |
| Win32 / ARM64 | Build/package assets exist; custom shader runtime currently returns `E_NOTIMPL` |
| XAML fallback | `LiquidGlassBrush` can fall back to `FallbackColor` |
| Public API | WinRT, consumable from C++/WinRT and C# |
| Managed projection | .NET 8 / CsWinRT |
| Native language level | C++23, C++/WinRT 3.0 |
| Shader format | FXC SM4 shader-linking DXBC (`lib_4_0` family); DXIL/SM6 is not a drop-in payload |

### Windows App SDK preview / experimental versions

The repository intentionally treats private ABI compatibility as an explicit validation problem rather than assuming that a newer Windows App SDK remains compatible. `RuntimeResolver` resolves the required native entry points from machine-code/reference relationships; historical `Runtime240` audit data is useful evidence but is not a substitute for validating all private layouts and signatures.

When adapting a newer or experimental Windows App SDK release, resolver fingerprints, object layouts, subgraph assumptions, and property-updater behavior must be revalidated. Unsupported layouts should fail rather than emit malformed Composition/DWM packets.

The current materialized-graph implementation supports the topology required by Liquid Glass: a native upstream graph materialized into an intermediate texture and consumed by an isolated terminal custom sampler. Multiple custom shader nodes and arbitrary downstream native/custom graph topologies are not yet general-purpose supported cases.

## Build

### Requirements

- Visual Studio **2026** with the **Desktop development with C++** and **WinUI application development** workloads.
- MSVC `v145` toolset with C++23 support.
- Windows SDK `10.0.26100.0` or a compatible installed SDK for the current projects.
- .NET 8 SDK for the managed projection and C# consumer test.
- NuGet access, or an already populated local package cache when using `-Offline`.

Open `WinUI.Composition.Hlsl.slnx` in Visual Studio, or build from PowerShell:

```powershell
.\build.ps1 -Configuration Debug
.\build.ps1 -Configuration Release
.\pack.ps1
.\tests\build.ps1 -Language Cpp
.\tests\build.ps1 -Language CSharp
```

Use `-Offline` with `build.ps1` / `pack.ps1` when the required packages already exist in the local NuGet cache.

The native project keeps generated C++/WinRT files, IFCs, and intermediate outputs separated by platform/configuration. When creating a package, make sure the runtime outputs for x64, Win32, and ARM64 are current; the package includes all three architecture assets even though the private custom-HLSL adapter is currently implemented only for x64.

## Samples and validation

The repository contains both C++/WinRT and C# consumers under `tests/`.

- `tests/Cpp` exercises WinUI XAML, named C++/WinRT modules, package consumption, Liquid Glass, effect switching, and runtime smoke coverage.
- `tests/CSharp` validates the managed WinRT projection and package consumption path.
- The C++ smoke path covers material creation, scalar updates, resizing, blur extremes, dispersion, effect switching, and material recreation. It is a runtime-stability test rather than a pixel-correctness test.

The package-validation workflow builds the package first, uploads the resulting `.nupkg`, then restores and builds the C++ XAML demo from that exact package artifact. This verifies the actual NuGet consumer path instead of replacing the package with a source `ProjectReference`.

## Documentation

Start with the [documentation index](docs/index.md), or jump directly to a topic.

### API reference

- [WinUI.Composition.Hlsl namespace](docs/api/winui-composition-hlsl.md)
- [HlslComposition](docs/api/hlsl-composition.md)
- [HlslEffect](docs/api/hlsl-effect.md)
- [HlslEffectKind](docs/api/hlsl-effect-kind.md)
- [HlslFloatProperty](docs/api/hlsl-float-property.md)
- [HlslEffectFactory](docs/api/hlsl-effect-factory.md)
- [HlslEffectBrush](docs/api/hlsl-effect-brush.md)
- [HlslShaderLibrary](docs/api/hlsl-shader-library.md)
- [LiquidGlassMaterial](docs/api/liquid-glass-material.md)
- [LiquidGlassBrush](docs/api/liquid-glass-brush.md)

### Design notes

- [Materialized graph compilation](docs/design/materialized-graph-runtime.md)
- [Precompiled shader libraries](docs/design/precompiled-shaders.md)

## Current limitations

The public `HlslEffect` model currently exposes one named source and fixed public shader entry-point contracts. Multiple public custom sources, arbitrary entry-point names, multiple custom shader nodes in one generalized mixed graph, architecture-specific private runtime adapters for Win32/ARM64, and automatic managed-project HLSL compilation are not implemented yet.

These limitations are explicit: unsupported private ABI or graph configurations should return an error rather than silently lowering an invalid effect.

## Contributing

Issues and pull requests are welcome. Changes touching the private Composition runtime should include evidence for the Windows App SDK build being targeted and should preserve the fail-closed behavior for unknown ABI revisions.

For build details, use the repository scripts and the GitHub Actions workflow as the reference build path.

## License

WinUI.Composition.Hlsl is licensed under the [MIT License](LICENSE.txt).
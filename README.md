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
  <a href="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/ci.yml/badge.svg?branch=master"></a>
  <a href="https://www.nuget.org/packages/WinUI.Composition.Hlsl"><img alt="NuGet" src="https://img.shields.io/badge/NuGet-publishing%20soon-004880?logo=nuget&logoColor=white"></a>
  <a href="LICENSE.txt"><img alt="License" src="https://img.shields.io/badge/license-MIT-blue.svg"></a>
  <img alt="C++23" src="https://img.shields.io/badge/C%2B%2B-23-00599C?logo=cplusplus">
  <img alt="WinUI 3" src="https://img.shields.io/badge/WinUI-3-0078D4">
</p>

## Overview

WinUI.Composition.Hlsl is a native Windows Runtime component for using custom HLSL effects in WinUI 3 Composition, together with ready-to-use XAML materials such as `LiquidGlassBrush`.

The public API is consumable from both C++/WinRT and C#. The native implementation is C++23 + C++/WinRT 3.0 and the package also ships a .NET 8 CsWinRT projection. The library supports dynamic HLSL, validated precompiled DXBC shader libraries, reusable Composition factories and brushes, animatable scalar parameters, XAML integration, and the built-in Liquid Glass material pipeline.

> [!WARNING]
> The custom-HLSL backend integrates with a **private, undocumented Windows Composition / Windows App Runtime ABI**. Windows App SDK **2.4.0 on x64** remains the most thoroughly validated baseline. x86 and ARM64 now have **phase-1 native ABI adapters** and are built by CI, but those paths are still experimental and have not yet reached the same validation confidence as x64. Preview or experimental Windows App SDK versions must be revalidated instead of being assumed binary-compatible.

## Features

- Native WinRT API for C++/WinRT and C#.
- Dynamic HLSL color transforms and custom samplers.
- Precompiled FXC SM4 DXBC shader-library support through `HlslShaderLibrary`.
- Scalar shader properties exposed as animatable Composition properties.
- `HlslEffectFactory` / `HlslEffectBrush` wrappers for reusable effect instances.
- `HlslComposition.CreateXamlBrush` for using Composition effects in XAML brush properties.
- Built-in `LiquidGlassMaterial` and `LiquidGlassBrush` with blur, refraction, dispersion, rounded corners, borders, highlights, and fallback rendering.
- Materialized graph lowering for the built-in `Backdrop -> GaussianBlur -> custom sampler` pipeline.
- Native NuGet build integration for compiling consumer HLSL into SM4 shader-linking DXBC libraries.
- Runtime validation for effect schemas, DXBC exports, constant-buffer layout, and private ABI revisions.
- Native package assets for x64, x86, and ARM64.

## NuGet

The package is **not public on nuget.org yet**. The repository's stable package metadata is currently `WinUI.Composition.Hlsl` **1.0.0**.

[![NuGet package placeholder](https://img.shields.io/badge/WinUI.Composition.Hlsl-1.0.0%20%7C%20publishing%20soon-004880?logo=nuget&logoColor=white)](https://www.nuget.org/packages/WinUI.Composition.Hlsl)

The intended stable reference is:

```xml
<PackageReference Include="WinUI.Composition.Hlsl" Version="1.0.0" />
```

For local development, build and pack the repository:

```powershell
.\pack.ps1
```

Packages are written to `artifacts/packages`.

### CI preview packages

Every GitHub Actions run creates a unique prerelease package using this form:

```text
1.0.0-preview.<github-run-id>.<run-attempt>
```

The package is uploaded as an Actions artifact and the C++ / C# consumer jobs download and build against that exact `.nupkg`. This keeps CI preview versions collision-free, including workflow reruns, without changing the repository's stable `1.0.0` metadata.

## Quick start

### Liquid Glass in XAML

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

`LiquidGlassBrush` derives from `XamlCompositionBrushBase`. Numeric dependency properties use `Double` for normal WinUI XAML conversion and are validated before being converted to GPU `float` values. When advanced effects are unavailable, initialization fails, or `IsEnabled` is false, the brush falls back to `FallbackColor`.

See [LiquidGlassBrush](docs/api/liquid-glass-brush.md) for a complete example.

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

Dynamic color shaders export `float4 PSBody(float4 color)`. Dynamic custom samplers define `float4 Shade(float2 uv, float4 samplerDataExt)`.

### Precompiled shaders

Production effects can avoid runtime `D3DCompile` by using an FXC SM4 shader-linking DXBC library and `HlslShaderLibrary`.

A native consumer can also ask the NuGet build target to compile HLSL:

```xml
<ItemGroup>
  <HlslCompositionShader Include="Effects\MyGlass.hlsl">
    <Kind>Sampler</Kind>
    <ShaderModel>4.0</ShaderModel>
  </HlslCompositionShader>
</ItemGroup>
```

The target emits generated headers and `.dxbc` payloads and copies the DXBC output into the application output directory.

## Compatibility

| Area | Current status |
| --- | --- |
| UI framework | WinUI 3 / Windows App SDK only |
| UWP / WinUI 2 | Not supported |
| Main validated Windows App SDK baseline | 2.4.0 |
| x64 private custom-HLSL runtime | Implemented; current primary validation baseline |
| x86 private custom-HLSL runtime | Phase-1 implementation present; experimental / validation in progress |
| ARM64 private custom-HLSL runtime | Phase-1 implementation present; experimental / validation in progress |
| NuGet native assets | x64, x86, ARM64 |
| XAML fallback | `LiquidGlassBrush` can fall back to `FallbackColor` |
| Managed projection | .NET 8 / CsWinRT |
| Native language level | C++23, C++/WinRT 3.0 |
| Shader format | FXC SM4 shader-linking DXBC (`lib_4_0` family) |

### Architecture support

The runtime now contains architecture-specific private-ABI handling for AMD64, x86, and ARM64, including platform machine validation, architecture-specific patch encoding, x86 calling-convention/thunk handling, and ARM64 instruction decoding. This is a significant step beyond merely packaging three DLLs.

However, **implemented** and **equally validated** are not the same claim. x86/ARM64 support is currently treated as experimental phase-1 support until resolver fingerprints, private object layouts, call signatures, graph lowering, and runtime behavior have been exercised to the same degree as x64 on the target Windows App SDK builds.

### Windows App SDK preview / experimental versions

Private ABI compatibility is validated explicitly. Newer Windows App SDK, preview, or experimental builds may change resolver fingerprints, object layouts, subgraph assumptions, or property updater behavior. Unsupported layouts should fail closed rather than emit malformed Composition/DWM data.

The current materialized-graph path is focused on the Liquid Glass topology: a native upstream graph materialized into an intermediate texture and consumed by an isolated terminal custom sampler.

## Build

### Requirements

- Visual Studio **2026** with **Desktop development with C++** and **WinUI application development** workloads.
- MSVC `v145` with C++23 support.
- Windows SDK `10.0.26100.0` or a compatible installed SDK.
- .NET 8 SDK for the managed projection and C# consumer.
- NuGet access, or a populated local package cache when using offline build scripts.

Open `WinUI.Composition.Hlsl.slnx`, or use PowerShell:

```powershell
.\build.ps1 -Configuration Debug
.\build.ps1 -Configuration Release
.\build.ps1 -Configuration Debug -Platform Win32
.\build.ps1 -Configuration Debug -Platform ARM64
.\pack.ps1
.\tests\build.ps1 -Language Cpp
.\tests\build.ps1 -Language CSharp
```

CI builds the native Release package assets for **x64, Win32/x86, and ARM64**, builds the managed projection, creates one unique preview `.nupkg`, then restores the C++ and C# demos from that exact package artifact.

## Samples and validation

- `tests/Cpp` covers WinUI XAML, named C++/WinRT modules, Liquid Glass, effect switching, and native consumer behavior.
- `tests/CSharp` validates the CsWinRT projection and managed package-consumer path.
- `tests/BuildIntegration` validates the native `<HlslCompositionShader>` MSBuild integration and verifies that the emitted payload is a DXBC container.
- `.github/workflows/ci.yml` is the canonical CI build path for `master` and pull requests targeting `master`.

## Documentation

Start with the [documentation index](docs/index.md).

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

The public `HlslEffect` model currently exposes one named source and fixed public shader entry-point contracts. Multiple public custom sources, arbitrary entry-point names, and fully general mixed graphs containing multiple custom shader nodes are not yet supported. x86 and ARM64 private-runtime support is present as phase-1 experimental work and still requires broader runtime validation.

## Contributing

Issues and pull requests are welcome. Changes touching the private Composition runtime should include evidence for the Windows App SDK build and architecture being targeted, and should preserve fail-closed behavior for unknown ABI revisions.

## License

WinUI.Composition.Hlsl is licensed under the [MIT License](LICENSE.txt).

## Thanks

Inspired by @apkipa's WUILiquidGlassDemo work.
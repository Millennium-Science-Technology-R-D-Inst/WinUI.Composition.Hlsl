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

## Overview

`WinUI.Composition.Hlsl` lets application HLSL participate in the existing Windows Graphics Effects / `Microsoft.UI.Composition` / XAML pipeline:

```text
HLSL / FXC shader library
    -> IGraphicsEffect
    -> CompositionEffectFactory
    -> CompositionEffectBrush
    -> XamlCompositionBrushBase
    -> XAML
```

It does not require an app-owned swap chain, `SwapChainPanel`, overlay HWND, or second visual tree. The WinRT surface is consumable from C++/WinRT and C#; the implementation is C++23/C++/WinRT and the package includes a .NET 8 CsWinRT projection.

> [!WARNING]
> Custom shader execution depends on a private, undocumented Windows Composition / Windows App SDK ABI. Released Windows App SDK **1.6 through 2.4** have been runtime-tested on **x86 and x64**. This is an empirical tested range, not a forward-compatibility promise. ARM64 builds are available but remain experimental until real-device runtime validation is complete. Unknown private layouts fail closed.

## Current capability

- Linked `Color` and `Sampler` contracts with **1-16 ordered sources**.
- Deterministic sampler resources: logical source `i` uses `texture{i} : register(t{i})` and `sampler{i} : register(s{i})`.
- Single-source `MaterializedSampler` for sampling a materialized upstream native Composition graph.
- Typed properties: `Scalar`, `Vector2`, `Vector3`, `Vector4`, `Matrix3x2`, and `Matrix4x4`.
- Build-time FXC compilation through `<HlslCompositionShader>` for C++ and C#.
- Native C++ `.g.h` embedding by default; C# consumes loose self-describing DXBC assets.
- Runtime `HlslCompiler` and immutable `HlslShaderLibrary` for generated/cached shaders.
- Native Composition property paths/setters and Composition-thread animation.
- Built-in `LiquidGlassMaterial` / `LiquidGlassBrush`.

The current public contract does **not** yet claim multi-source `MaterializedSampler`, multiple custom HLSL nodes in one lowered graph, or arbitrary native nodes after a custom materialized pass. The runtime contains exploratory multi-pass lowering code, but those capabilities remain fail-closed until their graph topology/bounds/runtime behavior are validated.

## Build-time shaders

The normal production declaration is small because `Kind=Auto`, `Profile=Pixel40`, and `SourceCount=1` are defaults:

```xml
<HlslCompositionShader Include="Effects\Glass.hlsl" />
```

For two linked inputs:

```xml
<HlslCompositionShader Include="Effects\Blend.hlsl">
  <SourceCount>2</SourceCount>
</HlslCompositionShader>
```

Public contracts:

```hlsl
// one-source Color
export float4 PSBody(float4 color);

// multi-source Color
float4 Shade(float4 color0, float4 color1);

// one-source Sampler
float4 Shade(float2 uv, float4 samplerDataExt);

// multi-source Sampler
float4 Shade(float2 uv0, float4 samplerDataExt0,
             float2 uv1, float4 samplerDataExt1);

// MaterializedSampler: exactly one source today
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData);
```

The build front end generates `PSBody*` edge-mode wrappers. Sampler resources have fixed bindings (`texture0/t0`, `sampler0/s0`, `texture1/t1`, `sampler1/s1`, ...). CI reflects generated DXBC and verifies these bindings in addition to Kind/Profile/SourceCount metadata.

### Native C++

```cpp
#include "Glass.g.h"
import winrt.WinUI.Composition.Hlsl;

using namespace winrt::WinUI::Composition::Hlsl;
auto effect = HlslEffect::CreateCompiledFromGeneratedByteArray({}, g_Effects_Glass_Shader);
```

The generated header carries the compiled library; native applications do not need a duplicate loose DXBC unless `HlslCompositionPublishAsContent=true` is explicitly enabled.

### C#

```csharp
var library = await HlslShaderLibrary.LoadGeneratedFromApplicationUriAsync(
    new Uri("ms-appx:///Hlsl/Effects/Glass.dxbc"));
var effect = HlslEffect.CreateCompiled(Guid.Empty, library);
```

## Typed properties

`HlslProperty` supports all public types:

| Type | Components |
| --- | ---: |
| `Scalar` | 1 |
| `Vector2` | 2 |
| `Vector3` | 3 |
| `Vector4` | 4 |
| `Matrix3x2` | 6 |
| `Matrix4x4` | 16 |

The library uses one shared property-layout implementation for source effects, runtime compilation, and precompiled effect validation. Matrix values are stored as contiguous float components in the private property blob/constant buffer and reconstructed in generated HLSL, avoiding dependence on HLSL's implicit matrix row/column stride. Precompiled typed effects are reflected against `UserConstants` before they enter the private runtime.

Brush setters are type checked:

```cpp
brush.SetFloat(L"Strength", 0.8f);
brush.SetVector2(L"Offset", { 2.0f, 4.0f });
brush.SetVector4(L"Tint", { 1.0f, 0.9f, 0.8f, 1.0f });
brush.SetMatrix3x2(L"Transform", matrix);
brush.SetMatrix4x4(L"Projection", projection);
```

## Linked multi-source binding

For an advanced effect, source order is part of the ABI:

```cpp
auto brush = HlslComposition::CreateBrushWithSources(compositor, effect, sources);
```

Source `0` corresponds to `texture0/t0 + sampler0/s0`, source `1` to `texture1/t1 + sampler1/s1`, and so on. `CreateBrushWithSources` rejects count mismatches and cross-compositor brushes still fail through `SetSource` validation.

## MaterializedSampler

`MaterializedSampler` is different from ordinary linked sampling. It requests a real texture representation of one upstream native effect graph:

```text
native upstream graph
    -> materialized intermediate surface
    -> one isolated MaterializedSampler pass
    -> Composition/XAML
```

It currently supports exactly one materialized source. Do not interpret linked 1-16 source support as multi-materialized-texture support.

## Runtime capability reporting

```csharp
var caps = HlslComposition.GetRuntimeCapabilities();
```

The query is side-effect free. It reports the package contract without installing/scanning the private adapter.

| Architecture | Support | Released WASDK runtime validation | Single materialized graph |
| --- | --- | --- | --- |
| x64 | Validated | 1.6-2.4 | Yes |
| x86 | Validated | 1.6-2.4 | Yes |
| ARM64 | Experimental | no real-device validation claim | No public claim |

Granular flags distinguish linked multi-source, materialized multi-source, multiple custom nodes, native-after-custom, vector properties, and matrix properties. See [HlslRuntimeCapabilities](docs/api/hlsl-runtime-capabilities.md).

## Build

```powershell
.\build.ps1 -Configuration Release
.\build.ps1 -Configuration Debug -Platform Win32
.\build.ps1 -Configuration Debug -Platform ARM64
.\pack.ps1
.\tests\build.ps1 -Language Cpp
.\tests\build.ps1 -Language CSharp
```

CI builds x64/Win32/ARM64 native assets, the CsWinRT projection, the build-time shader fixtures, a preview NuGet package, and downstream C++/C# package consumers.

## Documentation

Start at [docs/index.md](docs/index.md). Key design references:

- [HlslCompiler](docs/api/hlsl-compiler.md)
- [HlslProperty](docs/api/hlsl-property.md)
- [HlslEffectBrush](docs/api/hlsl-effect-brush.md)
- [HlslRuntimeCapabilities](docs/api/hlsl-runtime-capabilities.md)
- [Sampler resource binding contract](docs/design/resource-binding-contract.md)
- [Materialized graph compilation](docs/design/materialized-graph-runtime.md)
- [Runtime safety](docs/design/runtime-safety.md)

## Compatibility rule

The project intentionally uses a private ABI. Compatibility is established by testing concrete Windows App SDK releases and architectures, not by assuming version-number continuity. Future Windows/App SDK releases may change resolver fingerprints, object layouts, property-updater behavior, or graph lowering and must be revalidated.

<p align="center">
  <img src="assets/MainLogo.png" alt="WinUI.Composition.Hlsl logo" width="220" />
</p>

<h1 align="center">WinUI.Composition.Hlsl</h1>

<p align="center">让自定义 HLSL 作为 WinUI 3 Composition 原生 effect node 工作，并提供 XAML Brush 与原生液态玻璃控件。</p>
<p align="center"><a href="README.md">English</a> · <a href="README_zh_cn.md">简体中文</a></p>

<p align="center">
  <a href="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/ci.yml/badge.svg?branch=master"></a>
  <a href="https://www.nuget.org/packages/WinUI.Composition.Hlsl"><img alt="WinUI.Composition.Hlsl NuGet" src="https://img.shields.io/nuget/v/WinUI.Composition.Hlsl?logo=nuget"></a>
  <a href="https://www.nuget.org/packages/WinUI.LiquidGlass"><img alt="WinUI.LiquidGlass NuGet" src="https://img.shields.io/nuget/v/WinUI.LiquidGlass?logo=nuget"></a>
  <a href="LICENSE.txt"><img alt="License" src="https://img.shields.io/badge/license-MIT-blue.svg"></a>
  <img alt="C++23" src="https://img.shields.io/badge/C%2B%2B-23-00599C?logo=cplusplus">
  <img alt="WinUI 3" src="https://img.shields.io/badge/WinUI-3-0078D4">
</p>

## 这个仓库解决什么问题

`WinUI.Composition.Hlsl` 把应用自己的 HLSL 接入现有 Windows Graphics Effects / `Microsoft.UI.Composition` / WinUI 3 XAML 渲染链：

```text
HLSL / FXC linkable library
    -> IGraphicsEffect
    -> CompositionEffectFactory
    -> CompositionEffectBrush
    -> XamlCompositionBrushBase
    -> XAML
```

它不是应用自管的 D3D renderer，不需要 `SwapChainPanel`、自己的 present loop、额外 HWND overlay 或第二套 visual tree。公共 API 是 WinRT，可同时供 C++/WinRT 和 C# 使用。

仓库还包含 `WinUI.LiquidGlass`：建立在 core material 之上的原生 C++/WinRT WinUI 3 控件库。它保留 WinUI 原生输入、布局和 UI Automation 语义，同时提供液态玻璃材质、光学交互、spring 动效与控件预设。

## NuGet 包

| 包 | 用途 |
| --- | --- |
| `WinUI.Composition.Hlsl` | HLSL/Composition 核心运行时、effect graph API、`LiquidGlassMaterial`、`LiquidGlassBrush`。 |
| `WinUI.LiquidGlass` | 原生 WinUI 3 液态玻璃控件与模板；依赖相同版本的 core 包。 |

使用控件库时建议两个包保持相同版本：

```xml
<ItemGroup>
  <PackageReference Include="WinUI.Composition.Hlsl" Version="1.0.*" />
  <PackageReference Include="WinUI.LiquidGlass" Version="1.0.*" />
</ItemGroup>
```

## 版本与兼容性

NuGet 包最低依赖 **Windows App SDK 1.6**，并提供 **x86、x64、ARM64** 三种 native runtime asset。

每次通过验证的 `master` push 都会自动发布正式 NuGet 包，版本格式为 `1.0.<CI run number>`。

Custom shader backend 依赖未公开的 Composition 私有实现 ABI。遇到无法安全识别的布局时会 fail closed，而不是继续向未知私有结构写入猜测数据。实现细节、支持边界和运行时安全约束见 [整体架构](docs/architecture.md) 与 [Runtime safety](docs/design/runtime-safety.md)。

## 当前主要能力

- linked `Color` / `Sampler` 支持 **1–16 个有序 source**；
- source `i` 固定映射到 `texture{i}:t{i}` 与 `sampler{i}:s{i}`；
- 单 source `MaterializedSampler`，可采样已经物化的上游 native Composition graph；
- typed property：`Scalar`、`Vector2/3/4`、`Matrix3x2`、`Matrix4x4`；
- C++/C# 共用 `<HlslCompositionShader>` build-time FXC 管线；
- C++ 默认 `.g.h` 内嵌 DXBC；C# 默认部署 self-describing DXBC；
- `HlslCompiler` 提供真正需要动态源码时的异步编译；
- `HlslShaderLibrary` 可缓存、持久化、重新加载；
- Composition property path/setter 与 Composition animation；
- `LiquidGlassMaterial` / `LiquidGlassBrush`；
- `WinUI.LiquidGlass` 原生控件：Card、Magnifier、Button、Choice、Slider、输入控件、ToggleSwitch、TabBar 等；
- separable Gaussian backdrop blur、SDF coverage、折射/放大、色散、pointer specular、按控件区分的 motion/elevation。

当前还没有把 multi-source `MaterializedSampler`、任意多个 custom node 的 graph lowering、custom materialized pass 后任意 native node 宣布为公开稳定能力。这些会通过正式 graph/pass planner 继续推进，而不是简单删掉安全判断。

## 最短上手

项目中加入 core 包和 shader：

```xml
<ItemGroup>
  <PackageReference Include="WinUI.Composition.Hlsl" Version="1.0.*" />
  <HlslCompositionShader Include="Effects\Invert.hlsl" />
</ItemGroup>
```

`Effects/Invert.hlsl`：

```hlsl
export float4 PSBody(float4 color)
{
    return float4(color.a - color.rgb, color.a);
}
```

C++/WinRT：

```cpp
#include "Invert.g.h"
import winrt.WinUI.Composition.Hlsl;

using namespace winrt::WinUI::Composition::Hlsl;

auto effect = HlslEffect::CreateCompiledFromGeneratedByteArray(
    {}, g_Effects_Invert_Shader);
auto brush = HlslComposition::CreateBackdropBrush(compositor, effect);
```

C#：

```csharp
var library = await HlslShaderLibrary.LoadGeneratedFromApplicationUriAsync(
    new Uri("ms-appx:///Hlsl/Effects/Invert.dxbc"));
var effect = HlslEffect.CreateCompiled(Guid.Empty, library);
var brush = HlslComposition.CreateBackdropBrush(compositor, effect);
```

对应用自带的固定 shader，推荐 build-time 编译：语法、entry point、Kind 等错误直接在构建阶段暴露，不要把这些确定性错误搬到渲染阶段再检测。

## Shader contract

默认 `Kind=Auto`、`Profile=Pixel40`、`SourceCount=1`。

```hlsl
// Color，单输入
export float4 PSBody(float4 color);

// Color，多 linked 输入
float4 Shade(float4 color0, float4 color1);

// Sampler，单 linked 输入
float4 Shade(float2 uv, float4 samplerDataExt);

// Sampler，双 linked 输入
float4 Shade(float2 uv0, float4 samplerDataExt0,
             float2 uv1, float4 samplerDataExt1);

// MaterializedSampler，当前一个 materialized 输入
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData);
```

Sampler 所需的 private `PSBody*` edge-mode wrapper 以及 `textureN` / `samplerN` 声明由包自动生成，业务 HLSL 直接使用，不重复声明。

## 性能原则

编译和结构校验属于 setup，不属于 render hot path：

- 静态 HLSL 通常在 MSBuild 里编译；
- generated DXBC reflection 发生在 library 创建/加载时；
- factory/brush 应复用；
- property 更新不会重新编译 HLSL；
- render 时不会反复做 DXBC reflection；
- `GetRuntimeCapabilities()` 无副作用，不会为了查询 capability 去扫描/patch 私有 runtime。

控件 hot path 同样遵守这个原则：高频 Value/pointer 更新应该只改已有 Composition 状态，不能每一帧重新遍历 XAML template、分配 brush 或重写 resource。Slider 的实现约束见 [WinUI.LiquidGlass 控件](docs/liquid-glass-controls.md)。

## 文档

README 只负责项目入口，完整文档从这里开始：

1. [上手指南](docs/get-started.md)
2. [基础概念](docs/concepts.md)
3. [整体架构](docs/architecture.md)
4. [WinUI.LiquidGlass 控件](docs/liquid-glass-controls.md)
5. [API Reference](docs/api/index.md)
6. [设计文档索引](docs/index.md#design-reference)

常用参考：

- [HlslComposition](docs/api/hlsl-composition.md)
- [HlslCompiler](docs/api/hlsl-compiler.md)
- [HlslEffect](docs/api/hlsl-effect.md)
- [HlslShaderLibrary](docs/api/hlsl-shader-library.md)
- [HlslProperty](docs/api/hlsl-property.md)
- [HlslRuntimeCapabilities](docs/api/hlsl-runtime-capabilities.md)
- [LiquidGlassBrush](docs/api/liquid-glass-brush.md)
- [LiquidGlassMaterial](docs/api/liquid-glass-material.md)
- [WinUI.LiquidGlass 控件](docs/liquid-glass-controls.md)
- [Sampler 资源绑定 ABI](docs/design/resource-binding-contract.md)
- [Materialized graph runtime](docs/design/materialized-graph-runtime.md)

## 仓库构建

```powershell
.\build.ps1 -Configuration Release
.\build.ps1 -Configuration Debug -Platform Win32
.\build.ps1 -Configuration Debug -Platform ARM64
.\pack.ps1
.\tests\build.ps1 -Language Cpp
.\tests\build.ps1 -Language CSharp
```

CI 会构建 x64/Win32/ARM64 native asset、CsWinRT projection、generated shader fixture、两个 NuGet 包，并使用生成的 NuGet 再构建下游 C++/C# consumer。`master` push 全部验证通过后，会直接由 `ci.yml` 通过 OIDC trusted publishing 发布经过验证的包。

## License

[MIT License](LICENSE.txt)。

## Thanks

项目受到 @apkipa 的 WUILiquidGlassDemo 工作启发。

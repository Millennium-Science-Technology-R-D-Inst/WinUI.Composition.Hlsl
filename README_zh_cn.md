<p align="center">
  <img src="assets/MainLogo.png" alt="WinUI.Composition.Hlsl logo" width="220" />
</p>

<h1 align="center">WinUI.Composition.Hlsl</h1>

<p align="center">
  为 WinUI 3 提供原生 HLSL Composition 效果与 Fluent 风格 XAML 材质。
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

## 项目简介

WinUI.Composition.Hlsl 是一个原生 Windows Runtime 组件，用于在 WinUI 3 Composition 中使用自定义 HLSL 效果，同时提供可直接用于 XAML 的材质，例如 `LiquidGlassBrush`。

公共 API 同时面向 C++/WinRT 与 C#。原生实现采用 C++23 + C++/WinRT 3.0，并通过 .NET 8 CsWinRT projection 提供托管调用能力。项目支持动态 HLSL、经过验证的预编译 DXBC shader library、Composition factory/brush、可动画标量参数、XAML 集成，以及内置 Liquid Glass 材质管线。

> [!WARNING]
> 自定义 HLSL 后端依赖 **Windows Composition / Windows App Runtime 的私有、未公开 ABI**。目前验证最充分的基线仍是 **Windows App SDK 2.4.0 + x64**。x86 与 ARM64 已加入 **phase-1 原生 ABI 适配实现**，CI 也会实际构建这两个架构，但它们目前仍属于实验性支持，验证成熟度尚未达到 x64。对于 Preview / Experimental Windows App SDK，必须重新验证，不能默认认为二进制兼容。

## 功能

- 原生 WinRT API，可由 C++/WinRT 与 C# 使用。
- 动态 HLSL color transform 与 custom sampler。
- 通过 `HlslShaderLibrary` 使用预编译 FXC SM4 DXBC shader-linking library。
- 标量 shader 属性映射为可动画 Composition 属性。
- `HlslEffectFactory` / `HlslEffectBrush` 封装，可重复创建效果实例。
- `HlslComposition.CreateXamlBrush` 可将 Composition effect 直接用于 XAML Brush 属性。
- 内置 `LiquidGlassMaterial` 与 `LiquidGlassBrush`，支持模糊、折射、色散、圆角、边框、高光及 fallback 渲染。
- 为 `Backdrop -> GaussianBlur -> custom sampler` 内置管线提供 materialized graph lowering。
- Native NuGet build target 可将消费端 HLSL 编译成 SM4 shader-linking DXBC library。
- 对 effect schema、DXBC exports、constant-buffer layout 和私有 ABI revision 进行运行时验证。
- NuGet 内提供 x64、x86、ARM64 三种原生架构资产。

## NuGet

NuGet 包 **目前尚未公开发布到 nuget.org**。仓库中的 stable package metadata 当前为 `WinUI.Composition.Hlsl` **1.0.0**。

[![NuGet package placeholder](https://img.shields.io/badge/WinUI.Composition.Hlsl-1.0.0%20%7C%20publishing%20soon-004880?logo=nuget&logoColor=white)](https://www.nuget.org/packages/WinUI.Composition.Hlsl)

正式 stable 发布后的引用方式为：

```xml
<PackageReference Include="WinUI.Composition.Hlsl" Version="1.0.0" />
```

本地开发可直接：

```powershell
.\pack.ps1
```

生成的包位于 `artifacts/packages`。

### CI Preview 包

每次 GitHub Actions 运行都会生成一个唯一 prerelease 版本：

```text
1.0.0-preview.<github-run-id>.<run-attempt>
```

这样同一个 workflow 的不同运行，以及同一次运行的 rerun，都不会复用同一个 preview 版本。打出的 `.nupkg` 会作为 Actions artifact 上传，随后 C++ / C# consumer job 下载**同一个包**并进行 restore/build。仓库中的 stable `1.0.0` metadata 不需要因此反复修改。

## 快速开始

### 在 XAML 中使用 Liquid Glass

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

`LiquidGlassBrush` 继承自 `XamlCompositionBrushBase`。数值 DependencyProperty 使用 `Double`，以符合 WinUI XAML 正常的文本转换规则；内部验证完成后再转换为 GPU 使用的 `float`。当高级 effect 不可用、初始化失败或 `IsEnabled=false` 时，会回退到 `FallbackColor`。

完整示例见 [LiquidGlassBrush](docs/api/liquid-glass-brush.md)。

### 自定义 HLSL 效果

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

动态 color shader 导出 `float4 PSBody(float4 color)`；动态 custom sampler 定义 `float4 Shade(float2 uv, float4 samplerDataExt)`。

### 预编译 Shader

生产环境可以通过 FXC SM4 shader-linking DXBC library 与 `HlslShaderLibrary` 避免运行时 `D3DCompile`。

Native consumer 还可以通过 NuGet build target 直接声明待编译 HLSL：

```xml
<ItemGroup>
  <HlslCompositionShader Include="Effects\MyGlass.hlsl">
    <Kind>Sampler</Kind>
    <ShaderModel>4.0</ShaderModel>
  </HlslCompositionShader>
</ItemGroup>
```

Target 会输出 generated header 与 `.dxbc`，并将 DXBC 复制到应用输出目录。

## 兼容性

| 项目 | 当前状态 |
| --- | --- |
| UI 框架 | 仅 WinUI 3 / Windows App SDK |
| UWP / WinUI 2 | 不支持 |
| 主要验证 Windows App SDK 基线 | 2.4.0 |
| x64 私有 custom-HLSL runtime | 已实现；当前主要验证基线 |
| x86 私有 custom-HLSL runtime | 已有 phase-1 实现；实验性 / 验证中 |
| ARM64 私有 custom-HLSL runtime | 已有 phase-1 实现；实验性 / 验证中 |
| NuGet 原生资产 | x64、x86、ARM64 |
| XAML fallback | `LiquidGlassBrush` 可回退到 `FallbackColor` |
| 托管 projection | .NET 8 / CsWinRT |
| 原生语言级别 | C++23、C++/WinRT 3.0 |
| Shader 格式 | FXC SM4 shader-linking DXBC（`lib_4_0` 系列） |

### 架构支持状态

当前 runtime 已经加入 AMD64、x86、ARM64 的架构相关私有 ABI 处理，包括平台 machine 校验、架构专用 patch encoding、x86 calling convention/thunk，以及 ARM64 指令解码。因此现在的 x86/ARM64 已经不只是“NuGet 里放了两个占位 DLL”。

但需要区分 **已经实现** 与 **已经达到和 x64 相同验证成熟度**。目前 x86/ARM64 仍按 phase-1 experimental support 处理；resolver fingerprint、私有 object layout、调用签名、graph lowering 以及真实运行时行为，还需要在目标 Windows App SDK build 上继续扩大验证覆盖。

### Windows App SDK Preview / Experimental 版本

私有 ABI 兼容必须显式验证。新的 Windows App SDK、Preview 或 Experimental build 可能改变 resolver fingerprint、object layout、subgraph 假设或 property updater 行为。无法确认兼容时应 fail closed，而不是继续产生可能损坏的 Composition / DWM 数据。

当前 materialized-graph 路径主要服务 Liquid Glass 拓扑：将 native upstream graph 物化为 intermediate texture，再交给 isolated terminal custom sampler。

## 构建

### 环境要求

- Visual Studio **2026**，安装 **Desktop development with C++** 与 **WinUI application development** workload。
- 支持 C++23 的 MSVC `v145` toolset。
- Windows SDK `10.0.26100.0` 或兼容的已安装 SDK。
- Managed projection 与 C# consumer 需要 .NET 8 SDK。
- 需要可访问 NuGet，或者在离线构建时已经准备好本机 package cache。

可以打开 `WinUI.Composition.Hlsl.slnx`，或使用 PowerShell：

```powershell
.\build.ps1 -Configuration Debug
.\build.ps1 -Configuration Release
.\build.ps1 -Configuration Debug -Platform Win32
.\build.ps1 -Configuration Debug -Platform ARM64
.\pack.ps1
.\tests\build.ps1 -Language Cpp
.\tests\build.ps1 -Language CSharp
```

CI 会实际构建 **x64、Win32/x86、ARM64** 三套 Release native package asset，再构建 managed projection，生成唯一 preview `.nupkg`，最后让 C++ 与 C# demo 从这个确切的 package artifact restore/build。

## 示例与验证

- `tests/Cpp`：WinUI XAML、C++/WinRT named modules、Liquid Glass、effect switching 与 native consumer。
- `tests/CSharp`：CsWinRT projection 与 managed package-consumer path。
- `tests/BuildIntegration`：验证 native `<HlslCompositionShader>` MSBuild 集成，并确认输出 payload 是 DXBC container。
- `.github/workflows/ci.yml`：`master` 及目标为 `master` 的 Pull Request 的标准 CI 路径。

## 文档

从 [文档索引](docs/index.md) 开始阅读。

### API Reference

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

### Design Notes

- [Materialized graph compilation](docs/design/materialized-graph-runtime.md)
- [Precompiled shader libraries](docs/design/precompiled-shaders.md)

## 当前限制

公共 `HlslEffect` 模型当前提供一个 named source，并采用固定的公开 shader entry-point contract。多个公开 custom source、任意 entry-point 名称，以及包含多个 custom shader node 的完全通用 mixed graph 尚未支持。x86 与 ARM64 私有 runtime 已进入 phase-1 experimental 阶段，但仍需要更多真实运行时验证。

## 贡献

欢迎提交 Issue 与 Pull Request。涉及私有 Composition runtime 的修改，建议同时给出目标 Windows App SDK build 与目标架构的验证依据，并保持对未知 ABI revision 的 fail-closed 行为。

## License

WinUI.Composition.Hlsl 使用 [MIT License](LICENSE.txt)。

## Thanks

项目受到 @apkipa 的 WUILiquidGlassDemo 工作启发。
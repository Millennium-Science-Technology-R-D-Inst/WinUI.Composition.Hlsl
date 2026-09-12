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
  <a href="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/validate-cpp-xaml-modules.yml"><img alt="CI" src="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/validate-cpp-xaml-modules.yml/badge.svg"></a>
  <a href="https://www.nuget.org/packages/WinUI.Composition.Hlsl"><img alt="NuGet" src="https://img.shields.io/badge/NuGet-publishing%20soon-004880?logo=nuget&logoColor=white"></a>
  <a href="LICENSE.txt"><img alt="License" src="https://img.shields.io/badge/license-MIT-blue.svg"></a>
  <img alt="C++23" src="https://img.shields.io/badge/C%2B%2B-23-00599C?logo=cplusplus">
  <img alt="WinUI 3" src="https://img.shields.io/badge/WinUI-3-0078D4">
</p>

## 项目简介

WinUI.Composition.Hlsl 是一个原生 Windows Runtime 组件，用于在 WinUI 3 Composition 中使用自定义 HLSL 效果，同时提供可直接用于 XAML 的材质，例如 `LiquidGlassBrush`。

公共 API 同时面向 C++/WinRT 与 C#。原生实现采用 C++23 + C++/WinRT 3.0，并通过 .NET 8 CsWinRT projection 提供托管语言调用能力。项目支持动态 HLSL 源码、经过验证的预编译 DXBC shader library、Composition factory/brush、可动画标量参数以及 XAML Brush 集成。

> [!WARNING]
> 自定义 HLSL 后端依赖 **Windows Composition / Windows App Runtime 的私有、未公开 ABI**。当前经过验证的运行时基线是 **Windows App SDK 2.4.0 + x64**。对于更新版本、Preview 或 Experimental Windows App SDK，本项目不会默认假定其二进制兼容。运行时 resolver 会验证已知机器码和引用关系，并在无法确认兼容时直接失败。**项目能够成功编译并不代表对应 Windows App SDK 版本的私有运行时路径已经兼容。**

## 功能

- 原生 WinRT API，可由 C++/WinRT 与 C# 共同使用。
- 动态 HLSL color transform 与 custom sampler。
- 通过 `HlslShaderLibrary` 使用预编译 FXC SM4 DXBC shader-linking library。
- 标量 shader 属性映射为可动画 Composition 属性。
- `HlslEffectFactory` / `HlslEffectBrush` 封装，可重复创建效果实例。
- `HlslComposition.CreateXamlBrush` 可将自定义 Composition 效果直接用于 XAML Brush 属性。
- 内置 `LiquidGlassMaterial` 与 `LiquidGlassBrush`，支持模糊、折射、色散、圆角、边框、高光及 fallback 渲染。
- 支持 Light / Dark / High Contrast 资源设计。
- 为内置 `Backdrop -> GaussianBlur -> custom sampler` 管线实现 materialized graph lowering。
- Native NuGet build target 可将消费端 HLSL 编译为 SM4 shader-linking DXBC library。
- 对 effect schema、DXBC exports、函数签名、标量 constant-buffer layout 以及私有 ABI revision 进行运行时验证。

## NuGet

NuGet 包 **目前尚未公开发布**。当前仓库的 package metadata 已使用 `WinUI.Composition.Hlsl` **1.0.0**；下面的 NuGet 页面链接先作为正式发布后的占位入口。

[![NuGet package placeholder](https://img.shields.io/badge/WinUI.Composition.Hlsl-1.0.0%20%7C%20publishing%20soon-004880?logo=nuget&logoColor=white)](https://www.nuget.org/packages/WinUI.Composition.Hlsl)

公开发布后的预期引用方式：

```xml
<PackageReference Include="WinUI.Composition.Hlsl" Version="1.0.0" />
```

当前进行本地开发时，请直接从仓库构建 NuGet：

```powershell
.\pack.ps1
```

生成的包位于 `artifacts/packages`。

## 快速开始

### 在 XAML 中使用 Liquid Glass

在页面或 ResourceDictionary 中声明 WinRT namespace 后，可以直接使用：

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

`LiquidGlassBrush` 继承自 `XamlCompositionBrushBase`。它的数值 DependencyProperty 使用 `Double`，以符合 WinUI XAML 的文本转换规则；内部完成参数验证后再转换为 GPU 使用的 `float`。当系统不支持高级 Composition effects、初始化失败或 `IsEnabled=false` 时，Brush 会显示 `FallbackColor`。

主题资源建议为 Light 与 Dark 定义相同语义 key，并在 High Contrast 下切换到系统颜色 Brush。完整示例见 [LiquidGlassBrush](docs/api/liquid-glass-brush.md)。

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

动态 color shader 需要导出 `float4 PSBody(float4 color)`。动态 custom sampler 则定义 `float4 Shade(float2 uv, float4 samplerDataExt)`；runtime 会生成私有 Composition linker 所需要的 wrapper exports，再对源码执行编译。

声明了标量属性的 Effect 会自动生成对应 constant buffer 与 effect brush property，因此参数可以在不重新创建 effect description 的情况下更新或执行 Composition 动画。

### 预编译 Shader

生产环境可以通过 FXC SM4 shader-linking DXBC library 和 `HlslShaderLibrary` 避免运行时 `D3DCompile`。

原生 C++ 消费项目还可以通过 NuGet build target 直接声明待编译的 HLSL：

```xml
<ItemGroup>
  <HlslCompositionShader Include="Effects\MyGlass.hlsl">
    <Kind>Sampler</Kind>
    <ShaderModel>4.0</ShaderModel>
  </HlslCompositionShader>
</ItemGroup>
```

该 target 会输出 generated header 与 `.dxbc`，并将 DXBC 复制到应用输出目录。SDK-style C# 工程可以通过 WinRT API 加载已有 DXBC，但 `<HlslCompositionShader>` 的自动编译目前只集成在 Visual C++ 原生构建链中。

## 兼容性

| 项目 | 当前状态 |
| --- | --- |
| UI 框架 | 仅 WinUI 3 / Windows App SDK |
| UWP / WinUI 2 | 不支持 |
| 已验证 Windows App SDK 基线 | 2.4.0 |
| 私有 custom-HLSL runtime | x64 已实现并验证 |
| Win32 / ARM64 | 包与构建产物存在；custom shader runtime 当前返回 `E_NOTIMPL` |
| XAML fallback | `LiquidGlassBrush` 可回退到 `FallbackColor` |
| 公共 API | WinRT，可由 C++/WinRT 与 C# 使用 |
| 托管 projection | .NET 8 / CsWinRT |
| 原生语言级别 | C++23、C++/WinRT 3.0 |
| Shader 格式 | FXC SM4 shader-linking DXBC（`lib_4_0` 系列）；DXIL/SM6 不能直接替代 |

### Windows App SDK Preview / Experimental 版本适配

本项目将私有 ABI 兼容视为必须显式验证的问题，而不是默认认为“更高版本 Windows App SDK 一定兼容”。`RuntimeResolver` 根据机器码和引用关系解析所需 native entry point；`Runtime240` 中保留的历史审计数据可以作为参考，但不能替代对所有私有 object layout 和函数签名的实际验证。

适配新的 Windows App SDK 或 Experimental 版本时，需要重新确认 resolver fingerprint、内部对象布局、subgraph 结构假设以及 property updater 行为。如果无法确认兼容，应直接返回错误，而不是继续生成可能损坏的 Composition / DWM packet。

当前 materialized-graph 路径主要解决 Liquid Glass 所需拓扑：将 native upstream graph 物化为 intermediate texture，再交给 isolated terminal custom sampler。多 custom shader node，以及任意 downstream native/custom 混合图，目前还不是通用支持场景。

## 构建

### 环境要求

- Visual Studio **2026**，安装 **Desktop development with C++** 与 **WinUI application development** workload。
- 支持 C++23 的 MSVC `v145` toolset。
- 当前工程使用 Windows SDK `10.0.26100.0`，或环境中兼容的已安装 SDK。
- 构建 managed projection / C# consumer test 需要 .NET 8 SDK。
- 需要可访问 NuGet，或在使用 `-Offline` 时已经准备好本机 NuGet cache。

可以直接用 Visual Studio 打开 `WinUI.Composition.Hlsl.slnx`，也可以使用 PowerShell：

```powershell
.\build.ps1 -Configuration Debug
.\build.ps1 -Configuration Release
.\pack.ps1
.\tests\build.ps1 -Language Cpp
.\tests\build.ps1 -Language CSharp
```

当依赖已经存在于本地 NuGet cache 时，可以为 `build.ps1` / `pack.ps1` 添加 `-Offline`。

原生工程会按平台和配置分别保存 C++/WinRT generated files、IFC 与中间产物，避免多平台构建互相污染。打 NuGet 包之前应确保 x64、Win32 与 ARM64 的 runtime 输出均为最新版本；包中会包含三种架构资产，但当前私有 custom-HLSL adapter 只有 x64 实现。

## 示例与验证

仓库的 `tests/` 下同时提供 C++/WinRT 与 C# consumer：

- `tests/Cpp` 覆盖 WinUI XAML、C++/WinRT named modules、NuGet 消费、Liquid Glass、effect switching 与 runtime smoke 测试。
- `tests/CSharp` 验证 managed WinRT projection 与 NuGet 消费链。
- C++ smoke path 会覆盖材质创建、标量参数更新、resize、blur 极值、dispersion、effect switching 与 material recreation。该测试主要验证运行时稳定性，而不是像素级画面正确性。

当前 package-validation workflow 会先构建并上传 `.nupkg`，之后 consumer job 下载**同一个包**并从该本地 NuGet 源 restore/build C++ XAML demo。这样测试的是最终 NuGet 消费路径，而不是临时将包替换成源码 `ProjectReference`。

## 文档

可以先阅读 [文档索引](docs/index.md)，也可以直接进入具体条目。

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

公共 `HlslEffect` 模型目前提供一个 named source，并使用固定的公开 shader entry-point contract。多个公开 custom source、任意 entry-point 名称、一个通用 mixed graph 内的多个 custom shader node、Win32/ARM64 私有 runtime adapter，以及 C# 工程自动编译 HLSL 的构建集成，目前均尚未完成。

这些边界是显式的：遇到无法支持的私有 ABI 或 graph configuration 时，应该返回错误，而不是静默生成错误的 effect。

## 贡献

欢迎提交 Issue 与 Pull Request。涉及私有 Composition runtime 的修改，建议同时给出目标 Windows App SDK build 的验证依据，并保持对未知 ABI revision 的 fail-closed 行为。

构建细节请优先参考仓库脚本与 GitHub Actions workflow，它们代表当前实际使用的构建路径。

## License

WinUI.Composition.Hlsl 使用 [MIT License](LICENSE.txt)。
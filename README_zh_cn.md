<p align="center">
  <img src="assets/MainLogo.png" alt="WinUI.Composition.Hlsl logo" width="220" />
</p>

<h1 align="center">WinUI.Composition.Hlsl</h1>

<p align="center">让自定义 HLSL 成为 WinUI 3 Composition 图中的原生节点，并最终回到 XAML Brush。</p>

<p align="center"><a href="README.md">English</a> · <a href="README_zh_cn.md">简体中文</a></p>

<p align="center">
  <a href="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/Millennium-Science-Technology-R-D-Inst/WinUI.Composition.Hlsl/actions/workflows/ci.yml/badge.svg?branch=master"></a>
  <a href="https://www.nuget.org/packages/WinUI.Composition.Hlsl"><img alt="NuGet" src="https://img.shields.io/badge/NuGet-publishing%20soon-004880?logo=nuget&logoColor=white"></a>
  <a href="LICENSE.txt"><img alt="License" src="https://img.shields.io/badge/license-MIT-blue.svg"></a>
  <img alt="C++23" src="https://img.shields.io/badge/C%2B%2B-23-00599C?logo=cplusplus">
  <img alt="WinUI 3" src="https://img.shields.io/badge/WinUI-3-0078D4">
</p>

## 这个库解决什么问题

`WinUI.Composition.Hlsl` 的目标不是在 XAML 上面再盖一块应用自己维护的 D3D 画布，而是让 HLSL 进入 **Windows Graphics Effects / Microsoft.UI.Composition / XAML 原有渲染链**：

```text
HLSL / DXBC
    -> IGraphicsEffect
    -> CompositionEffectFactory
    -> CompositionEffectBrush
    -> XamlCompositionBrushBase
    -> XAML
```

这条路径不需要 `SwapChainPanel`、应用自管 swap chain、额外 Present 循环、独立 HWND overlay 或第二套 visual tree。因此自定义 shader 仍然处于正常的 Composition/XAML 裁剪、变换、缩放、生命周期、Brush 组合和动画体系中，而不是通过覆盖层绕过它们。

公共 API 同时面向 C++/WinRT 与 C#；原生实现使用 C++23/C++/WinRT，NuGet 同时提供 .NET 8 CsWinRT projection。

> [!WARNING]
> 真正执行 private custom shader node 依赖 **Windows Composition / Windows App SDK 未公开私有 ABI**。目前主要验证基线为 **Windows App SDK 2.4 + x64**。x86 与 ARM64 已有适配器，但仍属于实验性支持。未知或不兼容布局应 fail closed，不能靠猜测继续向 DWM 传递数据。

## 主要能力

- `HlslEffect.CreateGraphicsEffect*` 将 HLSL 暴露为标准 `IGraphicsEffect` graph node。
- `Color`、`Sampler`、`MaterializedSampler` 三种公开 shader contract。
- 标准 Composition factory/brush，并直接暴露 property set 供 Composition 动画使用。
- 通过 `XamlCompositionBrushBase` 将 Composition Brush 回接 XAML。
- C++ 与 C# 共用 `<HlslCompositionShader>` build-time FXC 管线。
- `HlslCompiler.CompileAsync` 后台编译运行时生成的 shader。
- `HlslShaderLibrary.Bytecode` 可将 DXBC 持久化到应用自己的 shader cache。
- 标量 shader 属性映射到 native Composition constant-buffer updater。
- 内置 `LiquidGlassMaterial` / `LiquidGlassBrush`。
- 无副作用的 runtime capability 查询。
- NuGet 提供 x64、x86、ARM64 原生资产。

## 生产环境推荐：构建时编译

如果 shader 在应用构建时已经确定，直接声明：

```xml
<ItemGroup>
  <HlslCompositionShader Include="Effects\MyGlass.hlsl">
    <Kind>MaterializedSampler</Kind>
    <Profile>Pixel40</Profile>
  </HlslCompositionShader>
</ItemGroup>
```

NuGet build target 会调用 Windows SDK FXC，并开启 strictness、优化和 warnings-as-errors。shader 语法错误和公开入口签名错误会直接变成 **MSBuild 失败**，而不是等应用第一次渲染才发现。

应用只需要写简化 contract：

```hlsl
// Color
export float4 PSBody(float4 color);

// Sampler
float4 Shade(float2 uv, float4 samplerDataExt);

// MaterializedSampler
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData);
```

Sampler 的 clamp/wrap/mirror `PSBody*` 私有 exports 会自动生成；`MaterializedSampler` 还会自动生成 materialization identity helper。应用不需要手写 Composition 私有 wrapper 名称。

同一 build target 同时支持 native C++ 项目和 SDK-style C# 项目，并产生相同 DXBC contract。

## 异步编译与持久化缓存

运行时动态生成的 shader 可以后台编译：

```csharp
var library = await HlslCompiler.CompileAsync(
    shader,
    HlslEffectKind.MaterializedSampler,
    HlslShaderProfile.Pixel40);
```

编译器会把 FXC 工作切到后台线程；那里不会创建 `Compositor`、XAML 对象、Composition brush/factory，也不会安装 private Composition adapter。

随后可使用 `library.Bytecode` 建立自己的持久缓存：

```text
第一次：HLSL -> CompileAsync -> DXBC -> 应用缓存
以后：  缓存 -> HlslShaderLibrary.Create -> CreateCompiled* -> Composition
```

对任意外部/缓存 DXBC 的 reflection 只作为一次性防御性校验，防止错误 bytecode 进入私有后端；它不属于逐帧渲染路径，也不应该代替 build-time shader 错误检查。

## 标准 Composition graph node

HLSL 可以直接产生标准 graphics effect：

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

如果前面已经有原生 Composition effect graph：

```text
CompositionEffectSourceParameter
    -> GaussianBlur / Transform / 其他 native node
    -> HlslEffect.CreateGraphicsEffectWithSource(upstream)
    -> Compositor.CreateEffectFactory(...)
```

也就是说 HLSL 被插入原本的 effect graph，而不是额外创建一个 swapchain surface 覆盖 XAML。

## MaterializedSampler

普通 `Sampler` 使用较轻的 linked sampler contract。`MaterializedSampler` 专门解决另一类问题：HLSL 需要把**一个上游原生 Composition effect graph 的结果当作真实纹理**来做任意 `Texture2D.Sample`。

```hlsl
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData)
{
    float2 texel = samplerDataExt.zw;
    return texture0.Sample(sampler0, uv + texel * 2.0f);
}
```

当前已经验证的 lowering 拓扑为：

```text
一个 native upstream graph
    -> materialized intermediate texture
    -> 一个 isolated terminal MaterializedSampler
    -> output wrapper
    -> Composition / XAML
```

它复用了 Liquid Glass 已经实际验证过的 `MaterializedTexture + MaterializedInput` 私有后端，并不是重新猜出一套 ABI。

目前**不宣称**支持：多个 custom HLSL node、多个公开纹理 source、custom sampler 后继续任意串 native effect，以及尚未逆向验证的 linker 参数编码。

## Composition 原生动画

`HlslEffectBrush` 直接暴露底层 `CompositionEffectBrush` 和 `CompositionPropertySet`：

```cpp
auto path = brush.GetPropertyPath(L"Strength");
auto animation = compositor.CreateScalarKeyFrameAnimation();
animation.InsertKeyFrame(1.0f, 24.0f);
brush.EffectBrush().StartAnimation(path, animation);
```

`SetFloat` 适合偶尔由应用更新属性；高频动画应交给 Composition animation，这样不需要每帧经过 C++/C# wrapper 做名字/range 校验和跨层调用。

## 回接 XAML

简单 Backdrop：

```cpp
auto brush = WinUI::Composition::Hlsl::HlslComposition::CreateBackdropBrush(compositor, effect);
MyBorder().Background(WinUI::Composition::Hlsl::HlslComposition::CreateXamlBrush(brush));
```

如果 graph 是直接通过标准 Composition API 创建：

```cpp
MyBorder().Background(
    WinUI::Composition::Hlsl::HlslComposition::CreateXamlBrushFromCompositionBrush(compositionBrush));
```

### 内置 Liquid Glass

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

`LiquidGlassBrush` 继承 `XamlCompositionBrushBase`；高级材质不可用或被禁用时可以回退到 `FallbackColor`。

## Runtime capability

```csharp
var caps = HlslComposition.GetRuntimeCapabilities();
```

这个查询没有副作用，不会为了“问一下支不支持”就扫描或 patch private runtime。

| 架构 | 支持级别 | Graph node | Materialized graph |
| --- | --- | --- | --- |
| x64 | Validated baseline | 是 | 是 |
| x86 | Experimental | 是 | 暂不声明 |
| ARM64 | Experimental | 是 | 暂不声明 |

真正 private ABI 初始化仍然是 lazy 的；即使 capability 声明某个架构有 adapter，遇到不兼容的 Windows/App SDK build 仍可能 fail closed。

## NuGet

仓库当前 stable metadata 为 `WinUI.Composition.Hlsl` **1.0.0**，nuget.org 正式发布仍在准备中。

```xml
<PackageReference Include="WinUI.Composition.Hlsl" Version="1.0.0" />
```

本地打包：

```powershell
.\pack.ps1
```

输出到 `artifacts/packages`。CI 使用唯一 `1.0.0-preview.<run-id>.<attempt>` 版本，并让 C++/C# consumer 使用本次 workflow 刚生成的确切 `.nupkg`。

## 兼容性

| 项目 | 当前状态 |
| --- | --- |
| UI 框架 | WinUI 3 / Windows App SDK |
| 私有 ABI 主要基线 | Windows App SDK 2.4 + x64 |
| x64 adapter | 主要验证基线 |
| x86 adapter | Experimental |
| ARM64 adapter | Experimental |
| Shader payload | FXC SM4 shader-linking DXBC（`lib_4_0` 系列） |
| Managed projection | .NET 8 / CsWinRT |
| Native language | C++23 / C++/WinRT |
| 公开 custom source | 一个 named source |
| 公开 shader 属性 | scalar float |
| 每个 lowered graph 的 custom node | 一个 |

Windows App SDK Preview / Experimental 版本可能改变 resolver fingerprint、object layout、subgraph 规则或 property updater ABI，因此必须重新验证，不能只根据版本号推断二进制兼容。

## 构建

需要 Visual Studio 2026、Desktop C++ 与 WinUI workload、MSVC v145、兼容 Windows SDK，以及用于 projection/test 的 .NET 8。

```powershell
.\build.ps1 -Configuration Debug
.\build.ps1 -Configuration Release
.\build.ps1 -Configuration Debug -Platform Win32
.\build.ps1 -Configuration Debug -Platform ARM64
.\pack.ps1
.\tests\build.ps1 -Language Cpp
.\tests\build.ps1 -Language CSharp
```

## 文档

从 [docs/index.md](docs/index.md) 开始。

重点文档：

- [HlslEffect](docs/api/hlsl-effect.md)
- [HlslEffectKind](docs/api/hlsl-effect-kind.md)
- [HlslCompiler](docs/api/hlsl-compiler.md)
- [HlslShaderLibrary](docs/api/hlsl-shader-library.md)
- [HlslComposition](docs/api/hlsl-composition.md)
- [HlslEffectBrush](docs/api/hlsl-effect-brush.md)
- [HlslRuntimeCapabilities](docs/api/hlsl-runtime-capabilities.md)
- [Materialized graph compilation](docs/design/materialized-graph-runtime.md)
- [预编译与异步 shader](docs/design/precompiled-shaders.md)

## 当前明确限制

公共 API 不会提前暴露尚未确认的 private ABI 能力。目前暂不支持：多纹理 custom input、一个 lowered graph 中多个 custom shader node、materialized custom sampler 后继续任意串 native effect、任意 private linker contract，以及尚未验证 native updater metadata 的 Vector/Matrix 公共属性。

## License

[MIT License](LICENSE.txt)。

## Thanks

项目受到 @apkipa 的 WUILiquidGlassDemo 工作启发。

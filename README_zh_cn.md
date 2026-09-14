<p align="center">
  <img src="assets/MainLogo.png" alt="WinUI.Composition.Hlsl logo" width="220" />
</p>

<h1 align="center">WinUI.Composition.Hlsl</h1>

<p align="center">让自定义 HLSL 作为 WinUI 3 Composition 原生 effect node 工作，并最终回到 XAML Brush。</p>
<p align="center"><a href="README.md">English</a> · <a href="README_zh_cn.md">简体中文</a></p>

## 项目定位

`WinUI.Composition.Hlsl` 把应用 HLSL 接入现有 Windows Graphics Effects / `Microsoft.UI.Composition` / XAML 渲染链：

```text
HLSL / FXC shader library
    -> IGraphicsEffect
    -> CompositionEffectFactory
    -> CompositionEffectBrush
    -> XamlCompositionBrushBase
    -> XAML
```

它不是覆盖在 XAML 上的自管 D3D renderer，不需要 `SwapChainPanel`、应用自己的 swap chain、额外 HWND overlay 或第二套 visual tree。公共 WinRT API 可由 C++/WinRT 和 C# 使用，原生实现为 C++23/C++/WinRT，并提供 .NET 8 CsWinRT projection。

> [!WARNING]
> Custom shader 真正执行时依赖 **Windows Composition / Windows App SDK 未公开私有 ABI**。目前已实际验证 Windows App SDK **1.6–2.4 正式版本在 x86、x64 上可运行**。这是测试结果，不是对未来版本的 ABI 兼容承诺。ARM64 可编译并已有适配器，但在真机运行验证完成前仍标记为 Experimental。无法安全识别的私有布局会 fail closed。

## 当前能力

- linked `Color` / `Sampler` 支持 **1–16 个有序 source**。
- sampler 资源绑定固定：逻辑 source `i` 对应 `texture{i}:t{i}` 和 `sampler{i}:s{i}`。
- `MaterializedSampler` 支持一个上游 native Composition graph 物化成纹理后采样。
- typed property 已支持 `Scalar`、`Vector2/3/4`、`Matrix3x2`、`Matrix4x4`。
- C++/C# 共用 `<HlslCompositionShader>` build-time FXC 管线。
- C++ 默认生成 `.g.h` 内嵌字节数组；C# 默认部署 self-describing loose DXBC。
- `HlslCompiler` 支持运行时后台编译，`HlslShaderLibrary` 支持缓存/重新加载。
- Composition property path、setter 与原生 Composition animation。
- 内置 `LiquidGlassMaterial` / `LiquidGlassBrush`。

目前**还没有正式公开承诺**：多 source `MaterializedSampler`、同一 lowered graph 中多个 custom HLSL node、custom materialized pass 后任意继续串 native node。runtime 内已经存在部分 multi-pass 原型，但在 graph topology、bounds、输入映射和实际运行测试完成前 capability 仍保持 false。

## 构建时 shader

默认 `Kind=Auto`、`Profile=Pixel40`、`SourceCount=1`，所以普通声明可以只有：

```xml
<HlslCompositionShader Include="Effects\Glass.hlsl" />
```

两个 linked source：

```xml
<HlslCompositionShader Include="Effects\Blend.hlsl">
  <SourceCount>2</SourceCount>
</HlslCompositionShader>
```

公开 contract：

```hlsl
// 单 source Color
export float4 PSBody(float4 color);

// 多 source Color
float4 Shade(float4 color0, float4 color1);

// 单 source Sampler
float4 Shade(float2 uv, float4 samplerDataExt);

// 多 source Sampler
float4 Shade(float2 uv0, float4 samplerDataExt0,
             float2 uv1, float4 samplerDataExt1);

// MaterializedSampler：当前必须恰好一个 source
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData);
```

build front end 自动生成所有 `PSBody*` edge-mode wrapper。Sampler 的资源寄存器是确定的：`texture0/t0`、`sampler0/s0`、`texture1/t1`、`sampler1/s1`…… CI 会直接反射 generated DXBC，校验 Kind/Profile/SourceCount metadata 和实际资源 register mapping。

### Native C++

```cpp
#include "Glass.g.h"
import winrt.WinUI.Composition.Hlsl;

using namespace winrt::WinUI::Composition::Hlsl;
auto effect = HlslEffect::CreateCompiledFromGeneratedByteArray({}, g_Effects_Glass_Shader);
```

默认最终应用目录不再重复部署同一份 loose DXBC；需要时可显式设置 `HlslCompositionPublishAsContent=true`。

### C#

```csharp
var library = await HlslShaderLibrary.LoadGeneratedFromApplicationUriAsync(
    new Uri("ms-appx:///Hlsl/Effects/Glass.dxbc"));
var effect = HlslEffect.CreateCompiled(Guid.Empty, library);
```

## Typed property

`HlslProperty` 当前支持全部公开类型：

| 类型 | float 分量数 |
| --- | ---: |
| `Scalar` | 1 |
| `Vector2` | 2 |
| `Vector3` | 3 |
| `Vector4` | 4 |
| `Matrix3x2` | 6 |
| `Matrix4x4` | 16 |

source effect、runtime compiler 和 precompiled effect 共用同一套 property layout。矩阵在 native property blob / constant buffer 内使用连续 float backing，再由生成的 HLSL 重建逻辑 matrix，从而不依赖 HLSL 隐式 matrix row/column stride。预编译 typed shader 在进入私有 Composition backend 之前会反射 `UserConstants` 并校验 name/type/offset/size。

Brush setter 会检查声明类型：

```cpp
brush.SetFloat(L"Strength", 0.8f);
brush.SetVector2(L"Offset", { 2.0f, 4.0f });
brush.SetVector4(L"Tint", { 1.0f, 0.9f, 0.8f, 1.0f });
brush.SetMatrix3x2(L"Transform", matrix);
brush.SetMatrix4x4(L"Projection", projection);
```

## linked 多输入资源映射

advanced effect 的 source 顺序属于 ABI：

```cpp
auto brush = HlslComposition::CreateBrushWithSources(compositor, effect, sources);
```

第 0 个 source 对应 `texture0/t0 + sampler0/s0`，第 1 个对应 `texture1/t1 + sampler1/s1`，依次类推。`CreateBrushWithSources` 会检查 source 数量；跨 `Compositor` brush 仍由 `SetSource` fail closed。

## MaterializedSampler

`MaterializedSampler` 与普通 linked sampler 不同。它要求把一个上游 native effect graph 先得到真实 intermediate surface，再交给 custom shader：

```text
native upstream graph
    -> materialized intermediate surface
    -> 一个独立 MaterializedSampler pass
    -> Composition / XAML
```

当前只能有一个 materialized source。linked `Sampler` 的 1–16 source 支持不代表 multi-materialized-texture 已经支持。

## Runtime capability

```csharp
var caps = HlslComposition.GetRuntimeCapabilities();
```

查询本身没有副作用，不会仅为了报告 capability 就扫描或 patch private runtime。

| 架构 | 支持级别 | 已验证 WASDK 正式版本 | 单 source materialized graph |
| --- | --- | --- | --- |
| x64 | Validated | 1.6–2.4 | 是 |
| x86 | Validated | 1.6–2.4 | 是 |
| ARM64 | Experimental | 暂无真机验证声明 | 暂不公开声明 |

此外有细分 capability：linked multi-source、materialized multi-source、multiple custom nodes、native-after-custom、Vector property、Matrix property。详见 [HlslRuntimeCapabilities](docs/api/hlsl-runtime-capabilities.md)。

## 构建

```powershell
.\build.ps1 -Configuration Release
.\build.ps1 -Configuration Debug -Platform Win32
.\build.ps1 -Configuration Debug -Platform ARM64
.\pack.ps1
.\tests\build.ps1 -Language Cpp
.\tests\build.ps1 -Language CSharp
```

CI 会构建 x64/Win32/ARM64 native assets、CsWinRT projection、build-time shader fixtures、preview NuGet，并再用该 NuGet 构建下游 C++/C# consumer。

## 文档

从 [docs/index.md](docs/index.md) 开始。重点：

- [HlslCompiler](docs/api/hlsl-compiler.md)
- [HlslProperty](docs/api/hlsl-property.md)
- [HlslEffectBrush](docs/api/hlsl-effect-brush.md)
- [HlslRuntimeCapabilities](docs/api/hlsl-runtime-capabilities.md)
- [Sampler resource binding contract](docs/design/resource-binding-contract.md)
- [Materialized graph compilation](docs/design/materialized-graph-runtime.md)
- [Runtime safety](docs/design/runtime-safety.md)

## 兼容原则

该项目主动使用私有 ABI，因此兼容性必须通过“具体 Windows App SDK 版本 + 具体架构”的实际验证来建立，不能只凭版本号连续性推断。未来 Windows / Windows App SDK 可能改变 resolver fingerprint、对象布局、property updater 或 graph lowering，届时必须重新验证。

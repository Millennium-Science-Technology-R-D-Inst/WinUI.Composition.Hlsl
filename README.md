# WinUI.Composition.Hlsl

C++/WinRT 3.0、C++23 实现的 WinRT 组件。应用通过一个 NuGet PackageReference 使用；原生项目和 Interop 构建项目的分工沿用 CommunityToolkit.WinUI。

## XAML 材质

~~~xml
<hlsl:LiquidGlassBrush
    IsEnabled="True"
    BlurRadius="10"
    RefractionStrength="16"
    CornerRadius="12"
    FallbackColor="#202020"/>
~~~

LiquidGlassBrush 的 XAML 数值属性是 Double。内部先验证数值，再转换成 GPU 使用的 float。不要把这些 DP 改回 Single：WinUI 的 XAML 文本转换不能按这里原先的 Windows.Foundation.Single 元数据创建值。

界面材质开关与 RequestedTheme 分开。Light、Dark、HighContrast 通过各自的 ThemeDictionary 提供同名 brush；切换材质更新既有 brush 的 IsEnabled，不需要来回切换 Light/Dark。高对比度或关闭高级效果时使用 FallbackColor。

## 自定义 shader 与参数

~~~csharp
var properties = new[]
{
    new HlslFloatProperty("Gain", 0.5f, 0, 1)
};

var effect = HlslEffect.CreateColorWithProperties(
    "export float4 PSBody(float4 color) { return color * Gain; }",
    "Input",
    properties);

var factory = HlslComposition.CreateEffectFactory(compositor, effect);
var brush = factory.CreateBrush();
brush.SetSource("Input", compositor.CreateBackdropBrush());
brush.SetFloat("Gain", 0.75f);

border.Background = HlslComposition.CreateXamlBrush(brush);
~~~

CreateColorTransform(shader) 和 CreateCustomSampler(shader) 自动从 shader/schema 生成稳定 ID。显式 GUID 创建方法保留为高级入口。同 GUID 对应不同定义时，由 runtime registry 拒绝。

HlslEffect 保存不可变描述；构造描述不加载私有 Composition DLL、不安装 hook。CreateEffectFactory 才编译和注册。HLSL 编译失败包含 D3DCompile 文本和 UserShader.hlsl 行号。

颜色变换函数为 export float4 PSBody(float4 color)。采样函数为 float4 Shade(float2 uv, float4 samplerDataExt)，texture0、sampler0 和导出别名由引擎生成。声明的标量属性自动进入常量缓冲、属性路径和 factory 的可动画属性列表。

Generic brush 只读取 schema，不引用 LiquidGlass 类型。LiquidGlass 也提供同一格式的定义。注册中心深拷贝字符串、元数据、常量和 shader 参数，拥有其生命周期；相同定义复用。不同定义的进程内注册数量有明确上限，避免反复生成随机 GUID 导致无界增长。

## 构建

- 用 Visual Studio 打开 WinUI.Composition.Hlsl.slnx。
- 原生工程启用 CppWinRTBuildModule，公共 ABI 仍以 IDL/WinMD 为准。
- 保留 obj/平台/配置 下的 Generated Files、IFC 和 PCH，避免并行平台构建互相覆盖。
- Interop 项目通过 ProjectReference 获得原生 WinMD，不手工指定某次构建产物路径。
- pack.ps1 使用 Visual Studio MSBuild 构建 ProjectReference 并根据 packaging/WinUI.Composition.Hlsl.nuspec 打包。
- 不需要自定义 DllMain；激活导出由生成的 WINRT_GetActivationFactory / WINRT_CanUnloadNow 提供。
- Pack nuget 时，必须保证 Hlsl 项目所有版本构建完成且建议全部为最新构建，在 Visul Studio 中由于配置只能为单独平台只能确保对应平台的版本为最新构建，其他平台的版本可能不是最新构建，所以在打包时需要确保所有平台的版本都是最新构建

~~~powershell
.\build.ps1 -Offline
.\pack.ps1 -Offline
.\tests\build.ps1 -Language CSharp
.\tests\build.ps1 -Language Cpp
~~~

Offline 只使用现有 NuGet 缓存。两个消费测试直接引用本地 NuGet 包，覆盖原始数字 XAML、Light/Dark 资源、材质开关和标量参数。

## 适配实现与当前进度

RuntimeResolver 根据机器码和引用关系解析入口，不把 DLL hash 当作自动兼容性的默认判据。Runtime240 记录历史审计数据，尚不能替代完整 ABI layout 验证。

目前 native lowering 对单 custom-node/source 拓扑有主动检查，复杂图不会被静默替换成其中一个节点。多个效果可以通过独立 CompositionEffectBrush 的 source 串联。完整多节点编译、所有 ABI layout 的运行时推导和 sampler bounds 自验证仍是后续要完成的实现工作，不能仅凭入口 pattern 匹配宣称已经完成。

x64 已有运行时实现。Win32/ARM64 项目配置被保留；当前尚未提供对应指令集的 native resolver，效果创建会明确失败，XAML brush 可回退实色。构建配置存在不等于该架构的 shader 路径已经验证。

消费端无需单独建立 Interop 工程。包里的原生 DLL 包含实际实现；Interop 程序集只是构建资产，没有第二套材质逻辑。

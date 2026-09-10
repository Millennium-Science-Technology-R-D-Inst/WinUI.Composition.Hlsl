# WinUI3 HLSL Composition API

Windows App SDK 2.4.0 x64。保持 lifted Composition backdrop → custom HLSL → CompositionEffectBrush → XAML 管线。

## 使用

~~~cpp
constexpr winrt::guid id{0x339e1f49,0x08f2,0x47c1,{0xb1,0x2e,0xe7,0x27,0x81,0x25,0x29,0xa0}};
auto effect = HlslComposition::CreateColorEffect(id, R"(
export float4 PSBody(float4 color)
{
    return float4(color.a - color.rgb, color.a);
}
)");
auto brush = HlslComposition::CreateBackdropBrush(compositor, effect);
border.Background(HlslComposition::AsXamlBrush(brush));
~~~

自定义采样调用 CreateSamplerEffect(id, source)，源码定义：

~~~hlsl
float4 Shade(float2 uv, float4 samplerDataExt)
{
    float2 delta = samplerDataExt.zw * 4;
    return (texture0.Sample(sampler0, uv - delta) +
            texture0.Sample(sampler0, uv + delta)) * 0.5;
}
~~~

texture0、sampler0、PSBody edge-mode 导出别名与 linking 参数由库生成，不要重复声明。samplerDataExt 保持 Composition 的 sampler 语义，不是 XAML 本地几何尺寸。

简化接口目前支持单输入颜色与采样 shader。动画属性/复杂 subgraph 暂用 CustomEffectRuntime.h 描述符，CustomLiquidGlassEffect.cpp 为完整参考。输入输出是预乘颜色；shader library profile 受 DWM linking 限制，不等同于任意 SM5/SM6。

同一 GUID 不能注册不同代码。源码与底层定义由库保留到进程退出，匹配 native registry 生命周期。Composition/XAML 调用在所属 UI 线程进行。

将 HlslComposition.cpp、CustomEffectRuntime.cpp 及对应头文件编入 C++/WinRT WinUI3 工程。依赖 d3dcompiler.lib、dxguid.lib、bcrypt.lib。本示例使用现有 pch 和生成的 WinUI3 投影。

## 后端

- HlslComposition.h/.cpp：用户 API、源码所有权、shader 包装、XAML brush。
- RuntimeResolver.h：模块特征扫描与引用解析，不包含固定入口 RVA。
- Runtime240.h：验证过的三件套 SHA-256 指纹，限制私有 ABI 支持范围（ABI锁定 2.4.0，如果将来composition/InteractiveExperiences API未进行大规模更新也可能适用）。（未启用）
- CustomEffectRuntime.cpp：effect 注册、compiler hook、compiled graph 和 shader linking。
- CustomInvertEffect.cpp / CustomBlurEffect.cpp：已使用新 API 的示例。
- CustomLiquidGlassEffect.cpp：动画参数与玻璃材质参考。

未来函数仅搬家且特征仍匹配时，不必重新找 RVA；未知二进制仍需 ABI 审核和兼容性验证后添加指纹。特征失配、多重匹配、无效引用均在 hook 安装前拒绝。不能保证任意未来版本自动兼容。

## 构建与检查
VS内部构建或
~~~powershell
.\build.ps1
.\build.ps1 -Offline
.\Output\x64\Debug\WUILiquidGlassDemo_Hlsl.exe
.\Output\x64\Debug\WUILiquidGlassDemo_Hlsl.exe --smoke
~~~

smoke 依次切换 invert、sampler blur、glass，更新参数并 resize，写入工作目录 smoke.log，完成后关闭。它检查创建/更新和持续运行，不替代像素正确性测试。

原 WinUI3 保持旧版参考。本后端只验证 x64，ARM64/Win32 未验证。

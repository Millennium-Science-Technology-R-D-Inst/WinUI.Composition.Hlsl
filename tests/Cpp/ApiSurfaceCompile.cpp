#include "XamlWorkaround.h"

import winrt.WinUI.Composition.Hlsl;

namespace
{
	using namespace winrt;
	using namespace WinUI::Composition::Hlsl;

	wchar_t const kCompileSurfaceShader[] = LR"(
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData)
{
    return texture0.Sample(sampler0, uv + samplerDataExt.zw * 0.0f + samplerData.xy * 0.0f);
}
)";

	// Compile-only coverage for the C++/WinRT projection. This function is never
	// called by the demo; it intentionally avoids activating the private runtime.
	void ValidateHlslApiSurface(
		HlslShaderLibrary const& library,
		Microsoft::UI::Composition::Compositor const& compositor,
		Windows::Storage::StorageFile const& file)
	{
		auto const caps = HlslComposition::GetRuntimeCapabilities();
		(void)caps;

		auto defines = single_threaded_vector<hstring>();
		defines.Append(L"RUNTIME_VARIANT=1");
		auto defineView = defines.GetView();
		(void)HlslCompiler::CompileAsync(
			kCompileSurfaceShader,
			HlslEffectKind::MaterializedSampler,
			HlslShaderProfile::Pixel40);
		(void)HlslCompiler::CompileWithDefinesAsync(
			kCompileSurfaceShader,
			HlslEffectKind::MaterializedSampler,
			HlslShaderProfile::Pixel40,
			defineView);
		(void)HlslCompiler::CompileWithPropertiesAndDefinesAsync(
			kCompileSurfaceShader,
			HlslEffectKind::MaterializedSampler,
			HlslShaderProfile::Pixel40,
			single_threaded_vector<HlslFloatProperty>().GetView(),
			defineView);

		(void)HlslShaderLibrary::LoadFromFileAsync(file, HlslShaderProfile::Pixel40);
		(void)HlslShaderLibrary::LoadFromApplicationUriAsync(
			Windows::Foundation::Uri(L"ms-appx:///Hlsl/ConsumerMaterializedSampler.dxbc"),
			HlslShaderProfile::Pixel40);

		auto effect = HlslEffect::CreateCustomMaterializedSampler(kCompileSurfaceShader);
		(void)HlslEffect::CreateCompiledMaterializedSampler({}, library);
		auto graph = effect.CreateGraphicsEffect();
		auto paths = effect.GetAnimatablePropertyPaths();
		(void)compositor.CreateEffectFactory(graph, paths);

		auto factory = HlslComposition::CreateEffectFactory(compositor, effect);
		auto brush = factory.CreateBrush();
		(void)factory.Factory();
		(void)brush.EffectBrush();
		(void)brush.Properties();
		(void)library.Bytecode();
		(void)HlslComposition::CreateXamlBrushFromCompositionBrush(brush.Brush());
	}
}

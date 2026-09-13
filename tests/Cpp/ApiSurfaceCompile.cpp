#include "XamlWorkaround.h"

import std;
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

	wchar_t const kMultiSourceSamplerShader[] = LR"(
float4 Shade(float2 uv0, float4 samplerDataExt0, float2 uv1, float4 samplerDataExt1)
{
    float4 first = texture0.Sample(sampler0, uv0);
    float4 second = texture1.Sample(sampler1, uv1);
    return lerp(first, second, 0.5f + (samplerDataExt0.x + samplerDataExt1.x) * 0.0f);
}
)";

	wchar_t const kTypedPropertyShader[] = LR"(
float4 Shade(float2 uv, float4 samplerDataExt)
{
    float2 transformed = mul(float3(uv, 1.0f), Transform);
    float4 projected = mul(float4(transformed + Offset, 0.0f, 1.0f), Projection);
    float4 color = texture0.Sample(sampler0, projected.xy + samplerDataExt.zw * 0.0f);
    return color * float4(Gain, 1.0f) * Tint * Strength;
}
)";

	HlslProperty MakeTypedProperty(
		hstring const& name,
		HlslPropertyType type,
		std::initializer_list<float> values)
	{
		auto projected = single_threaded_vector<float>();
		for (auto value : values) projected.Append(value);
		return HlslProperty(name, type, projected.GetView());
	}

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
		auto properties = single_threaded_vector<HlslFloatProperty>();
		auto defineView = defines.GetView();
		(void)HlslCompiler::CompileAsync(
			kCompileSurfaceShader,
			HlslEffectKind::Auto,
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
			properties.GetView(),
			defineView);
		(void)HlslCompiler::CompileAdvancedAsync(
			kMultiSourceSamplerShader,
			HlslEffectKind::Auto,
			HlslShaderProfile::Pixel40,
			2);
		(void)HlslCompiler::CompileAdvancedWithDefinesAsync(
			kMultiSourceSamplerShader,
			HlslEffectKind::Sampler,
			HlslShaderProfile::Pixel40,
			2,
			defineView);
		(void)HlslCompiler::CompileAdvancedWithPropertiesAsync(
			kMultiSourceSamplerShader,
			HlslEffectKind::Sampler,
			HlslShaderProfile::Pixel40,
			2,
			properties.GetView());
		(void)HlslCompiler::CompileAdvancedWithPropertiesAndDefinesAsync(
			kMultiSourceSamplerShader,
			HlslEffectKind::Sampler,
			HlslShaderProfile::Pixel40,
			2,
			properties.GetView(),
			defineView);

		auto typedProperties = single_threaded_vector<HlslProperty>();
		typedProperties.Append(MakeTypedProperty(L"Strength", HlslPropertyType::Scalar, { 1.0f }));
		typedProperties.Append(MakeTypedProperty(L"Offset", HlslPropertyType::Vector2, { 0.0f, 0.0f }));
		typedProperties.Append(MakeTypedProperty(L"Gain", HlslPropertyType::Vector3, { 1.0f, 1.0f, 1.0f }));
		typedProperties.Append(MakeTypedProperty(L"Tint", HlslPropertyType::Vector4, { 1.0f, 1.0f, 1.0f, 1.0f }));
		typedProperties.Append(MakeTypedProperty(L"Transform", HlslPropertyType::Matrix3x2,
			{ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f }));
		typedProperties.Append(MakeTypedProperty(L"Projection", HlslPropertyType::Matrix4x4,
			{ 1.0f,0.0f,0.0f,0.0f, 0.0f,1.0f,0.0f,0.0f,
			  0.0f,0.0f,1.0f,0.0f, 0.0f,0.0f,0.0f,1.0f }));
		(void)HlslCompiler::CompileWithTypedPropertiesAsync(
			kTypedPropertyShader,
			HlslEffectKind::Sampler,
			HlslShaderProfile::Pixel40,
			typedProperties.GetView());
		(void)HlslCompiler::CompileWithTypedPropertiesAndDefinesAsync(
			kTypedPropertyShader,
			HlslEffectKind::Sampler,
			HlslShaderProfile::Pixel40,
			typedProperties.GetView(),
			defineView);
		(void)HlslCompiler::CompileAdvancedWithTypedPropertiesAsync(
			kTypedPropertyShader,
			HlslEffectKind::Sampler,
			HlslShaderProfile::Pixel40,
			1,
			typedProperties.GetView());

		(void)HlslShaderLibrary::LoadFromFileAsync(file, HlslShaderProfile::Pixel40);
		(void)HlslShaderLibrary::LoadFromApplicationUriAsync(
			Windows::Foundation::Uri(L"ms-appx:///Hlsl/ConsumerMaterializedSampler.dxbc"),
			HlslShaderProfile::Pixel40);
		(void)HlslShaderLibrary::LoadGeneratedFromFileAsync(file);
		(void)HlslShaderLibrary::LoadGeneratedFromApplicationUriAsync(
			Windows::Foundation::Uri(L"ms-appx:///Hlsl/ConsumerMaterializedSampler.dxbc"));

		std::uint8_t embeddedShaderBytes[]{ 'D', 'X', 'B', 'C' };
		(void)HlslShaderLibrary::CreateFromByteArray(
			embeddedShaderBytes,
			HlslShaderProfile::Pixel40);
		(void)HlslShaderLibrary::CreateFromGeneratedByteArray(embeddedShaderBytes);
		(void)library.EffectKind();
		(void)library.SourceCount();

		auto effect = HlslEffect::CreateCustomMaterializedSampler(kCompileSurfaceShader);
		(void)HlslEffect::CreateCompiled(winrt::guid{}, library);
		(void)HlslEffect::CreateCompiledFromGeneratedByteArray(winrt::guid{}, embeddedShaderBytes);
		(void)HlslEffect::CreateCompiledMaterializedSampler(winrt::guid{}, library);

		auto sourceNames = single_threaded_vector<hstring>();
		sourceNames.Append(L"First");
		sourceNames.Append(L"Second");
		auto advancedProperties = single_threaded_vector<HlslProperty>();
		auto advancedEffect = HlslEffect::CreateAdvanced(
			kMultiSourceSamplerShader,
			HlslEffectKind::Sampler,
			sourceNames.GetView(),
			advancedProperties.GetView());
		(void)advancedEffect.SourceNames();
		(void)HlslComposition::CreateBackdropBrush(compositor, advancedEffect);

		auto compositionSources = single_threaded_vector<Microsoft::UI::Composition::CompositionBrush>();
		compositionSources.Append(compositor.CreateBackdropBrush());
		compositionSources.Append(compositor.CreateBackdropBrush());
		(void)HlslComposition::CreateBrushWithSources(
			compositor,
			advancedEffect,
			compositionSources.GetView());

		(void)HlslEffect::CreateCompiledAdvanced(
			winrt::guid{},
			library,
			HlslEffectKind::Sampler,
			sourceNames.GetView(),
			advancedProperties.GetView());

		auto typedSourceNames = single_threaded_vector<hstring>();
		typedSourceNames.Append(L"Backdrop");
		auto typedEffect = HlslEffect::CreateAdvanced(
			kTypedPropertyShader,
			HlslEffectKind::Sampler,
			typedSourceNames.GetView(),
			typedProperties.GetView());
		auto typedBrush = HlslComposition::CreateBackdropBrush(compositor, typedEffect);
		typedBrush.SetFloat(L"Strength", 0.75f);
		typedBrush.SetVector2(L"Offset", { 1.0f, 2.0f });
		typedBrush.SetVector3(L"Gain", { 1.0f, 0.9f, 0.8f });
		typedBrush.SetVector4(L"Tint", { 1.0f, 1.0f, 1.0f, 1.0f });
		typedBrush.SetMatrix3x2(L"Transform", { 1.0f,0.0f,0.0f,1.0f,4.0f,8.0f });
		typedBrush.SetMatrix4x4(L"Projection", Windows::Foundation::Numerics::float4x4::identity());

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

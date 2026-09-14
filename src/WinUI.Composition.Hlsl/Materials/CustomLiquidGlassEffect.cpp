#include <Windows.h>
#include <windows.graphics.effects.interop.h>

#include "CustomLiquidGlassEffect.h"

import winrt.Windows.Foundation;
import WinUI.Composition.Hlsl.EffectDef;
import WinUI.Composition.Hlsl.CustomEffectRuntime;
import WinUI.Composition.Hlsl.Shaders.LiquidGlass;

using namespace winrt;

namespace
{
	constexpr GUID kCustomLiquidGlassEffectId{ 0xc690ecdc, 0x9f2d, 0x46a8, { 0xa6, 0x54, 0x48, 0x0b, 0x3a, 0xf1, 0x12, 0x4e } };

	struct LiquidGlassConstants
	{
		float materialParams0[4];
		float materialParams1[4];
	};

	constexpr LiquidGlassConstants kInitialConstants{
		{ 1.5f, 36.0f, 12.0f, 0.0f },
		{ 0.85f, 1.0f, 1.2f, 1.0f },
	};

	static_assert(sizeof(LiquidGlassConstants) == 32);

	enum LiquidGlassPropertyIndex : std::uint32_t
	{
		RefractionStrengthProperty = 0,
		CornerRadiusProperty,
		BorderThicknessProperty,
		HighlightStrengthProperty,
		DispersionStrengthProperty,
	};

	constexpr std::uint32_t kDCompositionExpressionTypeScalar = 18;
	constexpr std::uint32_t kPropertyTypeSingle = 8;
	constexpr std::uint32_t kBorderThicknessOffset = 0;
	constexpr std::uint32_t kCornerRadiusOffset = 4;
	constexpr std::uint32_t kRefractionStrengthOffset = 8;
	constexpr std::uint32_t kHighlightStrengthOffset = 16;
	constexpr std::uint32_t kDispersionStrengthOffset = 24;

	HRESULT CreateScalarProperty(
		float scalar,
		ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		if (!value)
		{
			return E_POINTER;
		}

		*value = nullptr;
		try
		{
			auto propertyValue = Windows::Foundation::PropertyValue::CreateSingle(scalar)
				.as<Windows::Foundation::IPropertyValue>();
			*value = reinterpret_cast<ABI::Windows::Foundation::IPropertyValue*>(
				detach_abi(propertyValue));
			return S_OK;
		}
		catch (...)
		{
			return to_hresult();
		}
	}

	HRESULT GetRefractionStrengthDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams0[2], value);
	}

	HRESULT GetCornerRadiusDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams0[1], value);
	}

	HRESULT GetBorderThicknessDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams0[0], value);
	}

	HRESULT GetHighlightStrengthDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams1[0], value);
	}

	HRESULT GetDispersionStrengthDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams1[2], value);
	}

	CustomEffectRuntime::PropertyDescriptor const kProperties[] = {
		{ L"RefractionStrength", RefractionStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetRefractionStrengthDefault },
		{ L"CornerRadius", CornerRadiusProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetCornerRadiusDefault },
		{ L"BorderThickness", BorderThicknessProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetBorderThicknessDefault },
		{ L"HighlightStrength", HighlightStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetHighlightStrengthDefault },
		{ L"DispersionStrength", DispersionStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDispersionStrengthDefault },
	};

	CustomEffectRuntime::NativePropertyMetadata const kNativePropertyMetadata[] = {
		{ "RefractionStrength", kRefractionStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "CornerRadius", kCornerRadiusOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "BorderThickness", kBorderThicknessOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "HighlightStrength", kHighlightStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "DispersionStrength", kDispersionStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
	};

	CustomEffectRuntime::ConstantBufferPropertyMapping const kConstantBufferProperties[] = {
		{ RefractionStrengthProperty, kRefractionStrengthOffset },
		{ CornerRadiusProperty, kCornerRadiusOffset },
		{ BorderThicknessProperty, kBorderThicknessOffset },
		{ HighlightStrengthProperty, kHighlightStrengthOffset },
		{ DispersionStrengthProperty, kDispersionStrengthOffset },
	};

	constexpr uint16_t kBackdropUvArgument = 0x0100;
	constexpr uint16_t kBackdropSamplerDataExtArgument = 0x0400;
	constexpr uint16_t kBackdropSamplerDataArgument = 0x0300;
	constexpr uint16_t kBackdropCustomSamplerResult = 0x0200;

	CustomEffectRuntime::SourceDescriptor const kSources[] = {
		{ L"Backdrop", CustomEffectRuntime::SourceKind::Backdrop, true, true },
	};

	uint16_t const kShaderArguments[] = {
		kBackdropUvArgument,
		kBackdropSamplerDataExtArgument,
		kBackdropSamplerDataArgument,
	};

	auto const kLiquidGlassShader = WinUI::Composition::Hlsl::Shaders::LiquidGlassShader();

	CustomEffectRuntime::CustomEffectDefinition const kDefinition{
		kCustomLiquidGlassEffectId,
		CustomLiquidGlassEffect::EffectName,
		"CustomLiquidGlassEffect",
		nullptr,
		0,
		kLiquidGlassShader.data,
		kLiquidGlassShader.size,
		"PSBody",
		kSources,
		ARRAYSIZE(kSources),
		kProperties,
		ARRAYSIZE(kProperties),
		kNativePropertyMetadata,
		ARRAYSIZE(kNativePropertyMetadata),
		sizeof(LiquidGlassConstants),
		kConstantBufferProperties,
		ARRAYSIZE(kConstantBufferProperties),
		kShaderArguments,
		ARRAYSIZE(kShaderArguments),
		kBackdropCustomSamplerResult,
		// DWM's private linker uses the SM4 library/profile family. This byte must
		// match the build-time FXC target lib_4_0 generated for LiquidGlass.hlsl.
		CustomEffectRuntime::kShaderProfilePs40,
		sizeof(kInitialConstants),
		&kInitialConstants,
		// A custom sampler needs arbitrary UV access to the blurred input. Ask the
		// runtime to materialize the upstream native graph into a real texture,
		// rather than treating its output as a linked color dependency.
		CustomEffectRuntime::CustomEffectInputMode::MaterializedTexture,
		CustomEffectRuntime::GraphLoweringPolicy::MaterializedInput,
		"MaterializeColor",
	};
}

namespace CustomLiquidGlassEffect
{
	std::shared_ptr<hlsl::engine::EffectDefinition const> Description()
	{
		static auto value=[]
			{
				auto definition=std::make_shared<hlsl::engine::EffectDefinition>();
				definition->id=kCustomLiquidGlassEffectId; definition->sampler=true;
				definition->effectName=EffectName; definition->nativeTemplate=&kDefinition;
				definition->properties={
					{ L"RefractionStrength",24,0,128 },
					{ L"CornerRadius",12,0,512 },{ L"BorderThickness",1,0,32 },
					{ L"HighlightStrength",0.8f,0,4 },{ L"DispersionStrength",1.2f,0,16 } };
				return definition;
			}();
		return value;
	}

	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect()
	{
		return CustomEffectRuntime::CreateEffect(kDefinition);
	}

	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect(
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source)
	{
		if (!source)
		{
			throw hresult_invalid_argument(L"LiquidGlass requires a non-null materialized source graph.");
		}
		return CustomEffectRuntime::CreateEffect(kDefinition, source);
	}
}

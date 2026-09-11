#include <Windows.h>
#include <windows.graphics.effects.interop.h>

#include "CustomLiquidGlassEffect.h"
#include "LiquidGlassShader.g.h"

import winrt.Windows.Foundation;
import WinUI.Composition.Hlsl.EffectDef;
import WinUI.Composition.Hlsl.CustomEffectRuntime;

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
		{ 0.0f, 1.5f, 36.0f, 12.0f },
		{ 0.85f, 1.0f, 1.2f, 1.0f },
	};

	static_assert(sizeof(LiquidGlassConstants) == 32);

	enum LiquidGlassPropertyIndex : std::uint32_t
	{
		BlurRadiusProperty = 0,
		RefractionStrengthProperty,
		CornerRadiusProperty,
		BorderThicknessProperty,
		HighlightStrengthProperty,
		DispersionStrengthProperty,
	};

	constexpr std::uint32_t kDCompositionExpressionTypeScalar = 18;
	constexpr std::uint32_t kPropertyTypeSingle = 8;
	constexpr std::uint32_t kBlurRadiusOffset = 0;
	constexpr std::uint32_t kBorderThicknessOffset = 4;
	constexpr std::uint32_t kCornerRadiusOffset = 8;
	constexpr std::uint32_t kRefractionStrengthOffset = 12;
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

	HRESULT GetBlurRadiusDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams0[0], value);
	}

	HRESULT GetRefractionStrengthDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams0[3], value);
	}

	HRESULT GetCornerRadiusDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams0[2], value);
	}

	HRESULT GetBorderThicknessDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams0[1], value);
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
		{ L"BlurRadius", BlurRadiusProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetBlurRadiusDefault },
		{ L"RefractionStrength", RefractionStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetRefractionStrengthDefault },
		{ L"CornerRadius", CornerRadiusProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetCornerRadiusDefault },
		{ L"BorderThickness", BorderThicknessProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetBorderThicknessDefault },
		{ L"HighlightStrength", HighlightStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetHighlightStrengthDefault },
		{ L"DispersionStrength", DispersionStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDispersionStrengthDefault },
	};

	CustomEffectRuntime::NativePropertyMetadata const kNativePropertyMetadata[] = {
		{ "BlurRadius", kBlurRadiusOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "RefractionStrength", kRefractionStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "CornerRadius", kCornerRadiusOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "BorderThickness", kBorderThicknessOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "HighlightStrength", kHighlightStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "DispersionStrength", kDispersionStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
	};

	CustomEffectRuntime::ConstantBufferPropertyMapping const kConstantBufferProperties[] = {
		{ BlurRadiusProperty, kBlurRadiusOffset },
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

	CustomEffectRuntime::CustomEffectDefinition const kDefinition{
		kCustomLiquidGlassEffectId,
		CustomLiquidGlassEffect::EffectName,
		"CustomLiquidGlassEffect",
		nullptr,
		0,
		g_LiquidGlassShader,
		sizeof(g_LiquidGlassShader),
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
		// LiquidGlassMaterial binds a separate native GaussianBlur brush. That brush
		// boundary already materializes the source texture, so no extra internal
		// FlattenSource subgraph is required for this built-in material.
		false,
		nullptr,
	};
}

namespace CustomLiquidGlassEffect
{
	hlsl::engine::Definition Description()
	{
		static auto value=[]
			{
				auto definition=std::make_shared<hlsl::engine::EffectDefinition>();
				definition->id=kCustomLiquidGlassEffectId; definition->sampler=true;
				definition->effectName=EffectName; definition->nativeTemplate=&kDefinition;
				definition->properties={
					{ L"BlurRadius",12,0,64 },{ L"RefractionStrength",24,0,128 },
					{ L"CornerRadius",12,0,512 },{ L"BorderThickness",1,0,32 },
					{ L"HighlightStrength",0.8f,0,4 },{ L"DispersionStrength",1.2f,0,16 } };
				return definition;
			}();
		return value;
	}

	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect()
	{
		// The native Gaussian pass and private custom sampler are intentionally kept
		// in separate CompositionEffectFactory instances. The custom runtime only has
		// to lower one HLSL node and consumes the blur brush as an external source.
		return CustomEffectRuntime::CreateEffect(kDefinition);
	}
}

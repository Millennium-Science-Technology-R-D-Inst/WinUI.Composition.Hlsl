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
		float materialParams2[4];
		float materialParams3[4];
		float materialParams4[4];
		float materialParams5[4];
	};

	// Keep this layout byte-for-byte synchronized with LiquidGlass.hlsl.
	// P0: border, corner radius, artistic refraction multiplier, optical bezel width.
	// P1: highlight strength, edge softness, dispersion, opacity.
	// P2: physical thickness, IOR, tint opacity, base saturation.
	// P3: light angle, surface profile, magnification, highlight sharpness.
	// P4: tint RGB, inner shadow strength.
	// P5: specular-only saturation, specular width, reserved, reserved.
	constexpr LiquidGlassConstants kInitialConstants{
		{ 1.5f, 36.0f, 24.0f, 32.0f },
		{ 0.85f, 1.0f, 1.2f, 1.0f },
		{ 50.0f, 1.5f, 0.08f, 1.25f },
		{ -0.95f, 0.0f, 0.0f, 1.5f },
		{ 1.0f, 1.0f, 1.0f, 0.09f },
		{ 4.0f, 1.0f, 0.0f, 0.0f },
	};

	static_assert(sizeof(LiquidGlassConstants) == 96);

	enum LiquidGlassPropertyIndex : std::uint32_t
	{
		RefractionStrengthProperty = 0,
		CornerRadiusProperty,
		BorderThicknessProperty,
		HighlightStrengthProperty,
		DispersionStrengthProperty,
		BezelWidthProperty,
		GlassThicknessProperty,
		RefractiveIndexProperty,
		TintOpacityProperty,
		SaturationProperty,
		LightAngleProperty,
		SurfaceProfileProperty,
		MagnificationStrengthProperty,
		HighlightSharpnessProperty,
		TintRedProperty,
		TintGreenProperty,
		TintBlueProperty,
		InnerShadowStrengthProperty,
		SpecularSaturationProperty,
		SpecularWidthProperty,
	};

	constexpr std::uint32_t kDCompositionExpressionTypeScalar = 18;
	constexpr std::uint32_t kPropertyTypeSingle = 8;
	constexpr std::uint32_t kBorderThicknessOffset = 0;
	constexpr std::uint32_t kCornerRadiusOffset = 4;
	constexpr std::uint32_t kRefractionStrengthOffset = 8;
	constexpr std::uint32_t kBezelWidthOffset = 12;
	constexpr std::uint32_t kHighlightStrengthOffset = 16;
	constexpr std::uint32_t kDispersionStrengthOffset = 24;
	constexpr std::uint32_t kGlassThicknessOffset = 32;
	constexpr std::uint32_t kRefractiveIndexOffset = 36;
	constexpr std::uint32_t kTintOpacityOffset = 40;
	constexpr std::uint32_t kSaturationOffset = 44;
	constexpr std::uint32_t kLightAngleOffset = 48;
	constexpr std::uint32_t kSurfaceProfileOffset = 52;
	constexpr std::uint32_t kMagnificationStrengthOffset = 56;
	constexpr std::uint32_t kHighlightSharpnessOffset = 60;
	constexpr std::uint32_t kTintRedOffset = 64;
	constexpr std::uint32_t kTintGreenOffset = 68;
	constexpr std::uint32_t kTintBlueOffset = 72;
	constexpr std::uint32_t kInnerShadowStrengthOffset = 76;
	constexpr std::uint32_t kSpecularSaturationOffset = 80;
	constexpr std::uint32_t kSpecularWidthOffset = 84;

	HRESULT CreateScalarProperty(float scalar, ABI::Windows::Foundation::IPropertyValue** value) noexcept
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
			*value = reinterpret_cast<ABI::Windows::Foundation::IPropertyValue*>(detach_abi(propertyValue));
			return S_OK;
		}
		catch (...)
		{
			return to_hresult();
		}
	}

#define LIQUID_GLASS_DEFAULT_GETTER(Name, Group, Index) \
	HRESULT Get##Name##Default(ABI::Windows::Foundation::IPropertyValue** value) noexcept \
	{ \
		return CreateScalarProperty(kInitialConstants.Group[Index], value); \
	}

	LIQUID_GLASS_DEFAULT_GETTER(RefractionStrength, materialParams0, 2)
	LIQUID_GLASS_DEFAULT_GETTER(CornerRadius, materialParams0, 1)
	LIQUID_GLASS_DEFAULT_GETTER(BorderThickness, materialParams0, 0)
	LIQUID_GLASS_DEFAULT_GETTER(HighlightStrength, materialParams1, 0)
	LIQUID_GLASS_DEFAULT_GETTER(DispersionStrength, materialParams1, 2)
	LIQUID_GLASS_DEFAULT_GETTER(BezelWidth, materialParams0, 3)
	LIQUID_GLASS_DEFAULT_GETTER(GlassThickness, materialParams2, 0)
	LIQUID_GLASS_DEFAULT_GETTER(RefractiveIndex, materialParams2, 1)
	LIQUID_GLASS_DEFAULT_GETTER(TintOpacity, materialParams2, 2)
	LIQUID_GLASS_DEFAULT_GETTER(Saturation, materialParams2, 3)
	LIQUID_GLASS_DEFAULT_GETTER(LightAngle, materialParams3, 0)
	LIQUID_GLASS_DEFAULT_GETTER(SurfaceProfile, materialParams3, 1)
	LIQUID_GLASS_DEFAULT_GETTER(MagnificationStrength, materialParams3, 2)
	LIQUID_GLASS_DEFAULT_GETTER(HighlightSharpness, materialParams3, 3)
	LIQUID_GLASS_DEFAULT_GETTER(TintRed, materialParams4, 0)
	LIQUID_GLASS_DEFAULT_GETTER(TintGreen, materialParams4, 1)
	LIQUID_GLASS_DEFAULT_GETTER(TintBlue, materialParams4, 2)
	LIQUID_GLASS_DEFAULT_GETTER(InnerShadowStrength, materialParams4, 3)
	LIQUID_GLASS_DEFAULT_GETTER(SpecularSaturation, materialParams5, 0)
	LIQUID_GLASS_DEFAULT_GETTER(SpecularWidth, materialParams5, 1)

#undef LIQUID_GLASS_DEFAULT_GETTER

	CustomEffectRuntime::PropertyDescriptor const kProperties[] = {
		{ L"RefractionStrength", RefractionStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetRefractionStrengthDefault },
		{ L"CornerRadius", CornerRadiusProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetCornerRadiusDefault },
		{ L"BorderThickness", BorderThicknessProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetBorderThicknessDefault },
		{ L"HighlightStrength", HighlightStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetHighlightStrengthDefault },
		{ L"DispersionStrength", DispersionStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDispersionStrengthDefault },
		{ L"BezelWidth", BezelWidthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetBezelWidthDefault },
		{ L"GlassThickness", GlassThicknessProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetGlassThicknessDefault },
		{ L"RefractiveIndex", RefractiveIndexProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetRefractiveIndexDefault },
		{ L"TintOpacity", TintOpacityProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetTintOpacityDefault },
		{ L"Saturation", SaturationProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetSaturationDefault },
		{ L"LightAngle", LightAngleProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetLightAngleDefault },
		{ L"SurfaceProfile", SurfaceProfileProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetSurfaceProfileDefault },
		{ L"MagnificationStrength", MagnificationStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetMagnificationStrengthDefault },
		{ L"HighlightSharpness", HighlightSharpnessProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetHighlightSharpnessDefault },
		{ L"TintRed", TintRedProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetTintRedDefault },
		{ L"TintGreen", TintGreenProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetTintGreenDefault },
		{ L"TintBlue", TintBlueProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetTintBlueDefault },
		{ L"InnerShadowStrength", InnerShadowStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetInnerShadowStrengthDefault },
		{ L"SpecularSaturation", SpecularSaturationProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetSpecularSaturationDefault },
		{ L"SpecularWidth", SpecularWidthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetSpecularWidthDefault },
	};

#define LIQUID_GLASS_NATIVE_PROPERTY(Name, Offset) \
	{ #Name, Offset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr }

	CustomEffectRuntime::NativePropertyMetadata const kNativePropertyMetadata[] = {
		LIQUID_GLASS_NATIVE_PROPERTY(RefractionStrength, kRefractionStrengthOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(CornerRadius, kCornerRadiusOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(BorderThickness, kBorderThicknessOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(HighlightStrength, kHighlightStrengthOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(DispersionStrength, kDispersionStrengthOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(BezelWidth, kBezelWidthOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(GlassThickness, kGlassThicknessOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(RefractiveIndex, kRefractiveIndexOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(TintOpacity, kTintOpacityOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(Saturation, kSaturationOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(LightAngle, kLightAngleOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(SurfaceProfile, kSurfaceProfileOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(MagnificationStrength, kMagnificationStrengthOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(HighlightSharpness, kHighlightSharpnessOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(TintRed, kTintRedOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(TintGreen, kTintGreenOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(TintBlue, kTintBlueOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(InnerShadowStrength, kInnerShadowStrengthOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(SpecularSaturation, kSpecularSaturationOffset),
		LIQUID_GLASS_NATIVE_PROPERTY(SpecularWidth, kSpecularWidthOffset),
	};

#undef LIQUID_GLASS_NATIVE_PROPERTY

	CustomEffectRuntime::ConstantBufferPropertyMapping const kConstantBufferProperties[] = {
		{ RefractionStrengthProperty, kRefractionStrengthOffset },
		{ CornerRadiusProperty, kCornerRadiusOffset },
		{ BorderThicknessProperty, kBorderThicknessOffset },
		{ HighlightStrengthProperty, kHighlightStrengthOffset },
		{ DispersionStrengthProperty, kDispersionStrengthOffset },
		{ BezelWidthProperty, kBezelWidthOffset },
		{ GlassThicknessProperty, kGlassThicknessOffset },
		{ RefractiveIndexProperty, kRefractiveIndexOffset },
		{ TintOpacityProperty, kTintOpacityOffset },
		{ SaturationProperty, kSaturationOffset },
		{ LightAngleProperty, kLightAngleOffset },
		{ SurfaceProfileProperty, kSurfaceProfileOffset },
		{ MagnificationStrengthProperty, kMagnificationStrengthOffset },
		{ HighlightSharpnessProperty, kHighlightSharpnessOffset },
		{ TintRedProperty, kTintRedOffset },
		{ TintGreenProperty, kTintGreenOffset },
		{ TintBlueProperty, kTintBlueOffset },
		{ InnerShadowStrengthProperty, kInnerShadowStrengthOffset },
		{ SpecularSaturationProperty, kSpecularSaturationOffset },
		{ SpecularWidthProperty, kSpecularWidthOffset },
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
		CustomEffectRuntime::kShaderProfilePs40,
		sizeof(kInitialConstants),
		&kInitialConstants,
		CustomEffectRuntime::CustomEffectInputMode::MaterializedTexture,
		CustomEffectRuntime::GraphLoweringPolicy::MaterializedInput,
		"MaterializeColor",
	};
}

namespace CustomLiquidGlassEffect
{
	std::shared_ptr<hlsl::engine::EffectDefinition const> Description()
	{
		static auto value = []
			{
				auto definition = std::make_shared<hlsl::engine::EffectDefinition>();
				definition->id = kCustomLiquidGlassEffectId;
				definition->sampler = true;
				definition->effectName = EffectName;
				definition->nativeTemplate = &kDefinition;
				definition->properties = {
					{ L"RefractionStrength", 24.0f, 0.0f, 128.0f },
					{ L"CornerRadius", 36.0f, 0.0f, 512.0f },
					{ L"BorderThickness", 1.5f, 0.0f, 32.0f },
					{ L"HighlightStrength", 0.85f, 0.0f, 4.0f },
					{ L"DispersionStrength", 1.2f, 0.0f, 16.0f },
					{ L"BezelWidth", 32.0f, 1.0f, 256.0f },
					{ L"GlassThickness", 50.0f, 0.0f, 256.0f },
					{ L"RefractiveIndex", 1.5f, 1.0f, 3.5f },
					{ L"TintOpacity", 0.08f, 0.0f, 1.0f },
					{ L"Saturation", 1.25f, 0.0f, 4.0f },
					{ L"LightAngle", -0.95f, -6.2831855f, 6.2831855f },
					{ L"SurfaceProfile", 0.0f, 0.0f, 3.0f },
					{ L"MagnificationStrength", 0.0f, 0.0f, 128.0f },
					{ L"HighlightSharpness", 1.5f, 0.25f, 64.0f },
					{ L"TintRed", 1.0f, 0.0f, 1.0f },
					{ L"TintGreen", 1.0f, 0.0f, 1.0f },
					{ L"TintBlue", 1.0f, 0.0f, 1.0f },
					{ L"InnerShadowStrength", 0.09f, 0.0f, 1.0f },
					{ L"SpecularSaturation", 4.0f, 0.0f, 50.0f },
					{ L"SpecularWidth", 1.0f, 0.25f, 32.0f },
				};
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

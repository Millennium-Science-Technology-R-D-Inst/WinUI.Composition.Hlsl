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
	constexpr GUID kCustomLiquidGlassEffectId{
		0xc690ecdc, 0x9f2d, 0x46a8,
		{ 0xa6, 0x54, 0x48, 0x0b, 0x3a, 0xf1, 0x12, 0x4e }
	};

	struct LiquidGlassConstants
	{
		float materialParams0[4];
		float materialParams1[4];
		float materialParams2[4];
		float materialParams3[4];
		float materialParams4[4];
		float materialParams5[4];
		float materialParams6[4];
		float materialParams7[4];
		float materialParams8[4];
	};

	// Keep this layout byte-for-byte synchronized with LiquidGlass.hlsl.
	// P0: border, corner radius, artistic refraction multiplier, optical bezel width.
	// P1: highlight strength, edge softness, dispersion, material opacity.
	// P2: physical thickness, IOR, tint opacity, base saturation.
	// P3: light angle, surface profile, magnification, highlight sharpness.
	// P4: tint RGB, inner shadow strength.
	// P5: specular-only saturation, specular width, contrast, exposure.
	// P6: normalized pointer X/Y, normalized interaction radius, interaction strength.
	// P7: normalized pointer velocity X/Y per second, normalized outside hover range, active flag.
	// P8: pointer refraction, pointer highlight, motion refraction, reserved.
	constexpr LiquidGlassConstants kInitialConstants{
		{ 1.5f, 36.0f, 24.0f, 32.0f },
		{ 0.85f, 1.0f, 1.2f, 1.0f },
		{ 50.0f, 1.5f, 0.08f, 1.25f },
		{ -0.95f, 0.0f, 0.0f, 1.5f },
		{ 1.0f, 1.0f, 1.0f, 0.09f },
		{ 4.0f, 1.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.65f, 1.0f },
		{ 0.0f, 0.0f, 0.10f, 0.0f },
		{ 5.0f, 0.22f, 5.0f, 0.0f },
	};

	static_assert(sizeof(LiquidGlassConstants) == 144);

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
		EdgeSoftnessProperty,
		MaterialOpacityProperty,
		ContrastProperty,
		ExposureProperty,
		PointerXProperty,
		PointerYProperty,
		PointerInteractionRadiusProperty,
		PointerInteractionStrengthProperty,
		PointerVelocityXProperty,
		PointerVelocityYProperty,
		PointerHoverRangeProperty,
		PointerActiveProperty,
		PointerRefractionStrengthProperty,
		PointerHighlightStrengthProperty,
		PointerMotionRefractionStrengthProperty,
	};

	constexpr std::uint32_t kDCompositionExpressionTypeScalar = 18;
	constexpr std::uint32_t kPropertyTypeSingle = 8;
	constexpr std::uint32_t kBorderThicknessOffset = 0;
	constexpr std::uint32_t kCornerRadiusOffset = 4;
	constexpr std::uint32_t kRefractionStrengthOffset = 8;
	constexpr std::uint32_t kBezelWidthOffset = 12;
	constexpr std::uint32_t kHighlightStrengthOffset = 16;
	constexpr std::uint32_t kEdgeSoftnessOffset = 20;
	constexpr std::uint32_t kDispersionStrengthOffset = 24;
	constexpr std::uint32_t kMaterialOpacityOffset = 28;
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
	constexpr std::uint32_t kContrastOffset = 88;
	constexpr std::uint32_t kExposureOffset = 92;
	constexpr std::uint32_t kPointerXOffset = 96;
	constexpr std::uint32_t kPointerYOffset = 100;
	constexpr std::uint32_t kPointerInteractionRadiusOffset = 104;
	constexpr std::uint32_t kPointerInteractionStrengthOffset = 108;
	constexpr std::uint32_t kPointerVelocityXOffset = 112;
	constexpr std::uint32_t kPointerVelocityYOffset = 116;
	constexpr std::uint32_t kPointerHoverRangeOffset = 120;
	constexpr std::uint32_t kPointerActiveOffset = 124;
	constexpr std::uint32_t kPointerRefractionStrengthOffset = 128;
	constexpr std::uint32_t kPointerHighlightStrengthOffset = 132;
	constexpr std::uint32_t kPointerMotionRefractionStrengthOffset = 136;

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

	template<std::size_t Group, std::size_t Index>
	HRESULT GetDefaultScalar(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		static_assert(Group < 9 && Index < 4);
		if constexpr (Group == 0) return CreateScalarProperty(kInitialConstants.materialParams0[Index], value);
		if constexpr (Group == 1) return CreateScalarProperty(kInitialConstants.materialParams1[Index], value);
		if constexpr (Group == 2) return CreateScalarProperty(kInitialConstants.materialParams2[Index], value);
		if constexpr (Group == 3) return CreateScalarProperty(kInitialConstants.materialParams3[Index], value);
		if constexpr (Group == 4) return CreateScalarProperty(kInitialConstants.materialParams4[Index], value);
		if constexpr (Group == 5) return CreateScalarProperty(kInitialConstants.materialParams5[Index], value);
		if constexpr (Group == 6) return CreateScalarProperty(kInitialConstants.materialParams6[Index], value);
		if constexpr (Group == 7) return CreateScalarProperty(kInitialConstants.materialParams7[Index], value);
		return CreateScalarProperty(kInitialConstants.materialParams8[Index], value);
	}

	CustomEffectRuntime::PropertyDescriptor const kProperties[] = {
		{ L"RefractionStrength", RefractionStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<0, 2> },
		{ L"CornerRadius", CornerRadiusProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<0, 1> },
		{ L"BorderThickness", BorderThicknessProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<0, 0> },
		{ L"HighlightStrength", HighlightStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<1, 0> },
		{ L"DispersionStrength", DispersionStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<1, 2> },
		{ L"BezelWidth", BezelWidthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<0, 3> },
		{ L"GlassThickness", GlassThicknessProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<2, 0> },
		{ L"RefractiveIndex", RefractiveIndexProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<2, 1> },
		{ L"TintOpacity", TintOpacityProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<2, 2> },
		{ L"Saturation", SaturationProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<2, 3> },
		{ L"LightAngle", LightAngleProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<3, 0> },
		{ L"SurfaceProfile", SurfaceProfileProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<3, 1> },
		{ L"MagnificationStrength", MagnificationStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<3, 2> },
		{ L"HighlightSharpness", HighlightSharpnessProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<3, 3> },
		{ L"TintRed", TintRedProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<4, 0> },
		{ L"TintGreen", TintGreenProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<4, 1> },
		{ L"TintBlue", TintBlueProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<4, 2> },
		{ L"InnerShadowStrength", InnerShadowStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<4, 3> },
		{ L"SpecularSaturation", SpecularSaturationProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<5, 0> },
		{ L"SpecularWidth", SpecularWidthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<5, 1> },
		{ L"EdgeSoftness", EdgeSoftnessProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<1, 1> },
		{ L"MaterialOpacity", MaterialOpacityProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<1, 3> },
		{ L"Contrast", ContrastProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<5, 2> },
		{ L"Exposure", ExposureProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<5, 3> },
		{ L"PointerX", PointerXProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<6, 0> },
		{ L"PointerY", PointerYProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<6, 1> },
		{ L"PointerInteractionRadius", PointerInteractionRadiusProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<6, 2> },
		{ L"PointerInteractionStrength", PointerInteractionStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<6, 3> },
		{ L"PointerVelocityX", PointerVelocityXProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<7, 0> },
		{ L"PointerVelocityY", PointerVelocityYProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<7, 1> },
		{ L"PointerHoverRange", PointerHoverRangeProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<7, 2> },
		{ L"PointerActive", PointerActiveProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<7, 3> },
		{ L"PointerRefractionStrength", PointerRefractionStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<8, 0> },
		{ L"PointerHighlightStrength", PointerHighlightStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<8, 1> },
		{ L"PointerMotionRefractionStrength", PointerMotionRefractionStrengthProperty, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, GetDefaultScalar<8, 2> },
	};

	CustomEffectRuntime::NativePropertyMetadata const kNativePropertyMetadata[] = {
		{ "RefractionStrength", kRefractionStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "CornerRadius", kCornerRadiusOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "BorderThickness", kBorderThicknessOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "HighlightStrength", kHighlightStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "DispersionStrength", kDispersionStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "BezelWidth", kBezelWidthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "GlassThickness", kGlassThicknessOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "RefractiveIndex", kRefractiveIndexOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "TintOpacity", kTintOpacityOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "Saturation", kSaturationOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "LightAngle", kLightAngleOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "SurfaceProfile", kSurfaceProfileOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "MagnificationStrength", kMagnificationStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "HighlightSharpness", kHighlightSharpnessOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "TintRed", kTintRedOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "TintGreen", kTintGreenOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "TintBlue", kTintBlueOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "InnerShadowStrength", kInnerShadowStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "SpecularSaturation", kSpecularSaturationOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "SpecularWidth", kSpecularWidthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "EdgeSoftness", kEdgeSoftnessOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "MaterialOpacity", kMaterialOpacityOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "Contrast", kContrastOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "Exposure", kExposureOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "PointerX", kPointerXOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "PointerY", kPointerYOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "PointerInteractionRadius", kPointerInteractionRadiusOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "PointerInteractionStrength", kPointerInteractionStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "PointerVelocityX", kPointerVelocityXOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "PointerVelocityY", kPointerVelocityYOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "PointerHoverRange", kPointerHoverRangeOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "PointerActive", kPointerActiveOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "PointerRefractionStrength", kPointerRefractionStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "PointerHighlightStrength", kPointerHighlightStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "PointerMotionRefractionStrength", kPointerMotionRefractionStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
	};

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
		{ EdgeSoftnessProperty, kEdgeSoftnessOffset },
		{ MaterialOpacityProperty, kMaterialOpacityOffset },
		{ ContrastProperty, kContrastOffset },
		{ ExposureProperty, kExposureOffset },
		{ PointerXProperty, kPointerXOffset },
		{ PointerYProperty, kPointerYOffset },
		{ PointerInteractionRadiusProperty, kPointerInteractionRadiusOffset },
		{ PointerInteractionStrengthProperty, kPointerInteractionStrengthOffset },
		{ PointerVelocityXProperty, kPointerVelocityXOffset },
		{ PointerVelocityYProperty, kPointerVelocityYOffset },
		{ PointerHoverRangeProperty, kPointerHoverRangeOffset },
		{ PointerActiveProperty, kPointerActiveOffset },
		{ PointerRefractionStrengthProperty, kPointerRefractionStrengthOffset },
		{ PointerHighlightStrengthProperty, kPointerHighlightStrengthOffset },
		{ PointerMotionRefractionStrengthProperty, kPointerMotionRefractionStrengthOffset },
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
					{ L"EdgeSoftness", 1.0f, 0.25f, 16.0f },
					{ L"MaterialOpacity", 1.0f, 0.0f, 1.0f },
					{ L"Contrast", 1.0f, 0.0f, 4.0f },
					{ L"Exposure", 0.0f, -4.0f, 4.0f },
					{ L"PointerX", 0.0f, -8.0f, 8.0f },
					{ L"PointerY", 0.0f, -8.0f, 8.0f },
					{ L"PointerInteractionRadius", 0.65f, 0.0f, 8.0f },
					{ L"PointerInteractionStrength", 1.0f, 0.0f, 4.0f },
					{ L"PointerVelocityX", 0.0f, -100.0f, 100.0f },
					{ L"PointerVelocityY", 0.0f, -100.0f, 100.0f },
					{ L"PointerHoverRange", 0.10f, 0.0f, 8.0f },
					{ L"PointerActive", 0.0f, 0.0f, 1.0f },
					{ L"PointerRefractionStrength", 5.0f, 0.0f, 64.0f },
					{ L"PointerHighlightStrength", 0.22f, 0.0f, 4.0f },
					{ L"PointerMotionRefractionStrength", 5.0f, 0.0f, 64.0f },
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

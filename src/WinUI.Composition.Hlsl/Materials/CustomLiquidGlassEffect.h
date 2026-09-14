#pragma once
import WinUI.Composition.Hlsl.EffectDef;
import winrt.Windows.Graphics.Effects;
import std;

namespace CustomLiquidGlassEffect
{
	inline constexpr wchar_t EffectName[] = L"BackdropLiquidGlassEffect";
	inline constexpr wchar_t RefractionStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.RefractionStrength";
	inline constexpr wchar_t CornerRadiusPropertyPath[] = L"BackdropLiquidGlassEffect.CornerRadius";
	inline constexpr wchar_t BorderThicknessPropertyPath[] = L"BackdropLiquidGlassEffect.BorderThickness";
	inline constexpr wchar_t HighlightStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.HighlightStrength";
	inline constexpr wchar_t EdgeSoftnessPropertyPath[] = L"BackdropLiquidGlassEffect.EdgeSoftness";
	inline constexpr wchar_t DispersionStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.DispersionStrength";
	inline constexpr wchar_t MaterialOpacityPropertyPath[] = L"BackdropLiquidGlassEffect.MaterialOpacity";
	inline constexpr wchar_t BezelWidthPropertyPath[] = L"BackdropLiquidGlassEffect.BezelWidth";
	inline constexpr wchar_t GlassThicknessPropertyPath[] = L"BackdropLiquidGlassEffect.GlassThickness";
	inline constexpr wchar_t RefractiveIndexPropertyPath[] = L"BackdropLiquidGlassEffect.RefractiveIndex";
	inline constexpr wchar_t TintOpacityPropertyPath[] = L"BackdropLiquidGlassEffect.TintOpacity";
	inline constexpr wchar_t SaturationPropertyPath[] = L"BackdropLiquidGlassEffect.Saturation";
	inline constexpr wchar_t LightAnglePropertyPath[] = L"BackdropLiquidGlassEffect.LightAngle";
	inline constexpr wchar_t SurfaceProfilePropertyPath[] = L"BackdropLiquidGlassEffect.SurfaceProfile";
	inline constexpr wchar_t MagnificationStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.MagnificationStrength";
	inline constexpr wchar_t HighlightSharpnessPropertyPath[] = L"BackdropLiquidGlassEffect.HighlightSharpness";
	inline constexpr wchar_t TintRedPropertyPath[] = L"BackdropLiquidGlassEffect.TintRed";
	inline constexpr wchar_t TintGreenPropertyPath[] = L"BackdropLiquidGlassEffect.TintGreen";
	inline constexpr wchar_t TintBluePropertyPath[] = L"BackdropLiquidGlassEffect.TintBlue";
	inline constexpr wchar_t InnerShadowStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.InnerShadowStrength";
	inline constexpr wchar_t SpecularSaturationPropertyPath[] = L"BackdropLiquidGlassEffect.SpecularSaturation";
	inline constexpr wchar_t SpecularWidthPropertyPath[] = L"BackdropLiquidGlassEffect.SpecularWidth";
	inline constexpr wchar_t ContrastPropertyPath[] = L"BackdropLiquidGlassEffect.Contrast";
	inline constexpr wchar_t ExposurePropertyPath[] = L"BackdropLiquidGlassEffect.Exposure";

	std::shared_ptr<hlsl::engine::EffectDefinition const> Description();
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect();
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect(
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source);
}

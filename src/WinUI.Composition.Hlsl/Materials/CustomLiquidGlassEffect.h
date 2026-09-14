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
	inline constexpr wchar_t DispersionStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.DispersionStrength";
	inline constexpr wchar_t BezelWidthPropertyPath[] = L"BackdropLiquidGlassEffect.BezelWidth";
	inline constexpr wchar_t GlassThicknessPropertyPath[] = L"BackdropLiquidGlassEffect.GlassThickness";
	inline constexpr wchar_t RefractiveIndexPropertyPath[] = L"BackdropLiquidGlassEffect.RefractiveIndex";
	inline constexpr wchar_t TintOpacityPropertyPath[] = L"BackdropLiquidGlassEffect.TintOpacity";
	inline constexpr wchar_t SaturationPropertyPath[] = L"BackdropLiquidGlassEffect.Saturation";
	inline constexpr wchar_t LightAnglePropertyPath[] = L"BackdropLiquidGlassEffect.LightAngle";

	std::shared_ptr<hlsl::engine::EffectDefinition const> Description();
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect();
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect(
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source);
}

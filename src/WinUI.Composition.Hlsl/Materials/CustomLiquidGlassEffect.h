#pragma once
import WinUI.Composition.Hlsl.EffectDef;
import winrt.Windows.Graphics.Effects;

namespace CustomLiquidGlassEffect
{
	inline constexpr wchar_t EffectName[] = L"BackdropLiquidGlassEffect";
	inline constexpr wchar_t RefractionStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.RefractionStrength";
	inline constexpr wchar_t CornerRadiusPropertyPath[] = L"BackdropLiquidGlassEffect.CornerRadius";
	inline constexpr wchar_t BorderThicknessPropertyPath[] = L"BackdropLiquidGlassEffect.BorderThickness";
	inline constexpr wchar_t HighlightStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.HighlightStrength";
	inline constexpr wchar_t DispersionStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.DispersionStrength";

	hlsl::engine::Definition Description();
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect();
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect(
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source);
}

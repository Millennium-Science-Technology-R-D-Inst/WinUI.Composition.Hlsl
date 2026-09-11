export module WinUI.Composition.Hlsl.GaussianBlurEffect;

import winrt.Windows.Graphics.Effects;

export namespace GaussianBlurEffect
{
	inline constexpr wchar_t BlurAmountPropertyPath[] = L"GaussianBlurEffect.BlurAmount";
	inline constexpr wchar_t LiquidGlassBlurEffectName[] = L"LiquidGlassBlur";
	inline constexpr wchar_t LiquidGlassBlurAmountPropertyPath[] = L"LiquidGlassBlur.BlurAmount";

	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect(
		wchar_t const* sourceName,
		float blurAmount);
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect(
		wchar_t const* effectName,
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source,
		float standardDeviation);
}

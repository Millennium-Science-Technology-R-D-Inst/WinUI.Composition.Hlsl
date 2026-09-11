export module WinUI.Composition.Hlsl.GaussianBlurEffect;

import winrt.Windows.Graphics.Effects;

export namespace GaussianBlurEffect
{
	inline constexpr wchar_t BlurAmountPropertyPath[] = L"GaussianBlurEffect.BlurAmount";

	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect(
		wchar_t const* sourceName,
		float blurAmount);
}

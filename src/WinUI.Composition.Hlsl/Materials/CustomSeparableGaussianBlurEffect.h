#pragma once
import winrt.Windows.Graphics.Effects;

namespace CustomSeparableGaussianBlurEffect
{
	inline constexpr wchar_t HorizontalEffectName[] = L"LiquidGlassBlurHorizontal";
	inline constexpr wchar_t VerticalEffectName[] = L"LiquidGlassBlurVertical";
	inline constexpr wchar_t HorizontalBlurAmountPropertyPath[] = L"LiquidGlassBlurHorizontal.BlurAmount";
	inline constexpr wchar_t VerticalBlurAmountPropertyPath[] = L"LiquidGlassBlurVertical.BlurAmount";

	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateHorizontalEffect(
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source);
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateVerticalEffect(
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source);
}

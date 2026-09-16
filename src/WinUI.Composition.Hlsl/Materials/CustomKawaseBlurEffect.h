#pragma once

import winrt.Windows.Graphics.Effects;

namespace CustomKawaseBlurEffect
{
	inline constexpr wchar_t Down0EffectName[] = L"KawaseDown0";
	inline constexpr wchar_t Down1EffectName[] = L"KawaseDown1";
	inline constexpr wchar_t Down2EffectName[] = L"KawaseDown2";
	inline constexpr wchar_t Down3EffectName[] = L"KawaseDown3";
	inline constexpr wchar_t Up0EffectName[] = L"KawaseUp0";
	inline constexpr wchar_t Up1EffectName[] = L"KawaseUp1";
	inline constexpr wchar_t Up2EffectName[] = L"KawaseUp2";
	inline constexpr wchar_t Up3EffectName[] = L"KawaseUp3";
	inline constexpr wchar_t ResolveEffectName[] = L"KawaseResolve";

	inline constexpr wchar_t Down0SpreadPropertyPath[] = L"KawaseDown0.Spread";
	inline constexpr wchar_t Down1SpreadPropertyPath[] = L"KawaseDown1.Spread";
	inline constexpr wchar_t Down2SpreadPropertyPath[] = L"KawaseDown2.Spread";
	inline constexpr wchar_t Down3SpreadPropertyPath[] = L"KawaseDown3.Spread";
	inline constexpr wchar_t Up0SpreadPropertyPath[] = L"KawaseUp0.Spread";
	inline constexpr wchar_t Up1SpreadPropertyPath[] = L"KawaseUp1.Spread";
	inline constexpr wchar_t Up2SpreadPropertyPath[] = L"KawaseUp2.Spread";
	inline constexpr wchar_t Up3SpreadPropertyPath[] = L"KawaseUp3.Spread";
	inline constexpr wchar_t ResolveMixPropertyPath[] = L"KawaseResolve.Mix";

	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateDownEffect(
		wchar_t const* effectName,
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source);
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateUpEffect(
		wchar_t const* effectName,
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source);
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateResolveEffect(
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& rawSource,
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& blurredSource);
}

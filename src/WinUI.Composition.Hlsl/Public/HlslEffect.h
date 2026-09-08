#pragma once
#include "HlslEffect.g.h"
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct HlslEffect : HlslEffectT<HlslEffect>
	{
		HlslEffect(winrt::guid id, HlslEffectKind kind, hstring const& shader);
		static Hlsl::HlslEffect CreateColor(winrt::guid const& id, hstring const& shader);
		static Hlsl::HlslEffect CreateSampler(winrt::guid const& id, hstring const& shader);
		winrt::guid Id() const
		{
			return m_id;
		}
		HlslEffectKind Kind() const
		{
			return m_kind;
		}
		Windows::Graphics::Effects::IGraphicsEffect NativeEffect() const
		{
			return m_effect;
		}
	private:
		winrt::guid m_id{}; HlslEffectKind m_kind{};
		Windows::Graphics::Effects::IGraphicsEffect m_effect{ nullptr };
	};
}
namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct HlslEffect : HlslEffectT<HlslEffect, implementation::HlslEffect>
	{
	};
}

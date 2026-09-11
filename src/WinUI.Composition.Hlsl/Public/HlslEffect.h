#pragma once
#include "HlslEffect.g.h"

import WinUI.Composition.Hlsl.EffectDef;

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct HlslEffect : HlslEffectT<HlslEffect>
	{
		explicit HlslEffect(std::shared_ptr<EffectDefinition const> definition) :m_definition(std::move(definition))
		{
		}
		static Hlsl::HlslEffect CreateColor(winrt::guid const& id, hstring const& shader);
		static Hlsl::HlslEffect CreateSampler(winrt::guid const& id, hstring const& shader);
		static Hlsl::HlslEffect CreateColorTransform(hstring const& shader);
		static Hlsl::HlslEffect CreateCustomSampler(hstring const& shader);
		static Hlsl::HlslEffect CreateColorWithProperties(hstring const& shader, hstring const& sourceName,
														  Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties);
		static Hlsl::HlslEffect CreateSamplerWithProperties(hstring const& shader, hstring const& sourceName,
															Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties);
		winrt::guid Id() const
		{
			return m_definition->id;
		}
		HlslEffectKind Kind() const
		{
			return m_definition->sampler ? HlslEffectKind::Sampler : HlslEffectKind::Color;
		}
		std::shared_ptr<EffectDefinition const> const& Definition() const
		{
			return m_definition;
		}
	private:
		std::shared_ptr<EffectDefinition const> m_definition;
	};
}
namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct HlslEffect : HlslEffectT<HlslEffect, implementation::HlslEffect>
	{
	};
}

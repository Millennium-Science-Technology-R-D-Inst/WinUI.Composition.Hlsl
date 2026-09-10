#pragma once
#include "HlslEffect.g.h"
#include "EffectDefinition.h"
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct HlslEffect : HlslEffectT<HlslEffect>
	{
		explicit HlslEffect(hlsl::engine::Definition definition) :m_definition(std::move(definition))
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
		hlsl::engine::Definition const& Definition() const
		{
			return m_definition;
		}
	private:
		hlsl::engine::Definition m_definition;
	};
}
namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct HlslEffect : HlslEffectT<HlslEffect, implementation::HlslEffect>
	{
	};
}

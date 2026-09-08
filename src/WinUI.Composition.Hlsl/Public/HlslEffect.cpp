#include "pch.h"
#include "HlslEffect.h"
#include "HlslEffect.g.cpp"
#include "EffectEngine.h"
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	HlslEffect::HlslEffect(winrt::guid id, HlslEffectKind kind, hstring const& shader) :m_id(id), m_kind(kind)
	{
		auto text=to_string(shader);
		m_effect=kind == HlslEffectKind::Color ? hlsl::engine::CreateColorEffect(id, text) : hlsl::engine::CreateSamplerEffect(id, text);
	}
	Hlsl::HlslEffect HlslEffect::CreateColor(winrt::guid const& id, hstring const& shader)
	{
		return make<HlslEffect>(id, HlslEffectKind::Color, shader);
	}
	Hlsl::HlslEffect HlslEffect::CreateSampler(winrt::guid const& id, hstring const& shader)
	{
		return make<HlslEffect>(id, HlslEffectKind::Sampler, shader);
	}
}

#pragma once
#ifndef WINRT_IMPORT_MODULE
#define WINRT_IMPORT_MODULE
#endif
#include "HlslEffect.g.h"

import WinUI.Composition.Hlsl.EffectDef;
import std;
import winrt.Windows.Foundation.Collections;
import winrt.Windows.Graphics.Effects;

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct HlslEffect : HlslEffectT<HlslEffect>
	{
		explicit HlslEffect(hlsl::engine::Definition definition) :m_definition(std::move(definition))
		{
		}
		static Hlsl::HlslEffect CreateColor(winrt::guid const& id, hstring const& shader);
		static Hlsl::HlslEffect CreateSampler(winrt::guid const& id, hstring const& shader);
		static Hlsl::HlslEffect CreateMaterializedSampler(winrt::guid const& id, hstring const& shader);
		static Hlsl::HlslEffect CreateColorTransform(hstring const& shader);
		static Hlsl::HlslEffect CreateCustomSampler(hstring const& shader);
		static Hlsl::HlslEffect CreateCompiledColor(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader);
		static Hlsl::HlslEffect CreateCompiledSampler(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader);
		static Hlsl::HlslEffect CreateCompiledMaterializedSampler(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader);
		static Hlsl::HlslEffect CreateColorWithProperties(hstring const& shader, hstring const& sourceName,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties);
		static Hlsl::HlslEffect CreateSamplerWithProperties(hstring const& shader, hstring const& sourceName,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties);
		static Hlsl::HlslEffect CreateMaterializedSamplerWithProperties(hstring const& shader, hstring const& sourceName,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties);
		static Hlsl::HlslEffect CreateCompiledColorWithProperties(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader, hstring const& sourceName,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties);
		static Hlsl::HlslEffect CreateCompiledSamplerWithProperties(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader, hstring const& sourceName,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties);
		static Hlsl::HlslEffect CreateCompiledMaterializedSamplerWithProperties(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader, hstring const& sourceName,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties);
		winrt::guid Id() const
		{
			return m_definition->id;
		}
		HlslEffectKind Kind() const
		{
			if (m_definition->materializedSampler) return HlslEffectKind::MaterializedSampler;
			return m_definition->sampler ? HlslEffectKind::Sampler : HlslEffectKind::Color;
		}
		hstring SourceName() const
		{
			return m_definition->sourceName;
		}
		bool IsPrecompiled() const
		{
			return !m_definition->shaderBytecode.empty();
		}
		Windows::Foundation::Collections::IVectorView<hstring> PropertyNames() const;
		Windows::Graphics::Effects::IGraphicsEffect CreateGraphicsEffect() const;
		Windows::Graphics::Effects::IGraphicsEffect CreateGraphicsEffectWithSource(Windows::Graphics::Effects::IGraphicsEffectSource const& source) const;
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

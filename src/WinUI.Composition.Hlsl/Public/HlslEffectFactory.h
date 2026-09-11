#pragma once
#include "HlslEffectFactory.g.h"

import WinUI.Composition.Hlsl.EffectDef;

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct HlslEffectFactory : HlslEffectFactoryT<HlslEffectFactory>
	{
		HlslEffectFactory(Microsoft::UI::Composition::CompositionEffectFactory const& factory, hlsl::engine::Definition definition) :m_factory(factory), m_definition(std::move(definition))
		{
		}
		Hlsl::HlslEffectBrush CreateBrush();
	private:
		Microsoft::UI::Composition::CompositionEffectFactory m_factory{ nullptr };
		hlsl::engine::Definition m_definition;
	};
}

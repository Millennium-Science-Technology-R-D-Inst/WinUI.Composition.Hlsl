#pragma once
#ifndef WINRT_IMPORT_MODULE
#define WINRT_IMPORT_MODULE
#endif
#include "HlslEffectFactory.g.h"

import WinUI.Composition.Hlsl.EffectDef;
import winrt.Microsoft.UI.Composition;
import std;

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct HlslEffectFactory : HlslEffectFactoryT<HlslEffectFactory>
	{
		HlslEffectFactory(Microsoft::UI::Composition::CompositionEffectFactory const& factory, std::shared_ptr<hlsl::engine::EffectDefinition const> definition) :m_factory(factory), m_definition(std::move(definition))
		{
		}
		Microsoft::UI::Composition::CompositionEffectFactory Factory() const
		{
			return m_factory;
		}
		Hlsl::HlslEffectBrush CreateBrush();
	private:
		Microsoft::UI::Composition::CompositionEffectFactory m_factory{ nullptr };
		std::shared_ptr<hlsl::engine::EffectDefinition const> m_definition;
	};
}

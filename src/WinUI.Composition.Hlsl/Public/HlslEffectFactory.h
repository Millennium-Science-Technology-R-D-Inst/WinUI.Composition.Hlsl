#pragma once
#include "HlslEffectFactory.g.h"
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct HlslEffectFactory : HlslEffectFactoryT<HlslEffectFactory>
	{
		HlslEffectFactory(Microsoft::UI::Composition::CompositionEffectFactory const& value) :m_factory(value)
		{
		}
		Hlsl::HlslEffectBrush CreateBrush();
	private:
		Microsoft::UI::Composition::CompositionEffectFactory m_factory{ nullptr };
	};
}


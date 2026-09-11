#include "HlslFloatProperty.h"
#include "HlslFloatProperty.g.cpp"

import WinUI.Composition.Hlsl.EffectDef;

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	HlslFloatProperty::HlslFloatProperty(hstring const& name, float initial, float minimum, float maximum) :
		m_name(name), m_initial(initial), m_minimum(minimum), m_maximum(maximum)
	{
		hlsl::engine::EffectDefinition definition; definition.shader="validation";
		definition.properties.push_back({ std::wstring(name),initial,minimum,maximum });
		hlsl::engine::Validate(definition);
	}
}

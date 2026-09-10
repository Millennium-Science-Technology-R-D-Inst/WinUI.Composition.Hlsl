#pragma once
#include "HlslFloatProperty.g.h"
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct HlslFloatProperty : HlslFloatPropertyT<HlslFloatProperty>
	{
		HlslFloatProperty(hstring const& name, float initial, float minimum, float maximum);
		hstring Name() const
		{
			return m_name;
		}
		float DefaultValue() const
		{
			return m_initial;
		}
		float Minimum() const
		{
			return m_minimum;
		}
		float Maximum() const
		{
			return m_maximum;
		}
	private:
		hstring m_name; float m_initial{}, m_minimum{}, m_maximum{};
	};
}
namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct HlslFloatProperty : HlslFloatPropertyT<HlslFloatProperty, implementation::HlslFloatProperty>
	{
	};
}

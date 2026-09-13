#pragma once
#ifndef WINRT_IMPORT_MODULE
#define WINRT_IMPORT_MODULE
#endif
#include "HlslProperty.g.h"

import std;
import winrt.Windows.Foundation.Collections;

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct HlslProperty : HlslPropertyT<HlslProperty>
	{
		HlslProperty(hstring const& name, Hlsl::HlslPropertyType type,
					 Windows::Foundation::Collections::IVectorView<float> const& defaultValue);
		hstring Name() const
		{
			return m_name;
		}
		Hlsl::HlslPropertyType Type() const
		{
			return m_type;
		}
		Windows::Foundation::Collections::IVectorView<float> DefaultValue() const;
		std::vector<float> const& Values() const
		{
			return m_values;
		}
	private:
		hstring m_name;
		Hlsl::HlslPropertyType m_type{};
		std::vector<float> m_values;
	};
}
namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct HlslProperty : HlslPropertyT<HlslProperty, implementation::HlslProperty>
	{
	};
}

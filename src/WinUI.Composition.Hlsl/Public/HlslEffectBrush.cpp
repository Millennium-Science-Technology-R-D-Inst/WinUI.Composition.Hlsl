#include "HlslEffectBrush.h"
#include "HlslEffectBrush.g.cpp"

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	hstring HlslEffectBrush::GetPropertyPath(hstring const& name) const
	{
		for (auto const& property : m_definition->properties)
		{
			if (name == property.name)
			{
				return hstring{ m_definition->effectName + L"." + property.name };
			}
		}
		throw hresult_invalid_argument(L"The float property is not declared by this effect.");
	}

	void HlslEffectBrush::SetSource(hstring const& name, Microsoft::UI::Composition::CompositionBrush const& source)
	{
		if (name != m_definition->sourceName || !source) throw hresult_invalid_argument(L"The source name is not declared by this effect.");
		if (source.Compositor() != m_brush.Compositor()) throw hresult_invalid_argument(L"The source belongs to another compositor.");
		m_brush.SetSourceParameter(name, source);
	}
	void HlslEffectBrush::SetFloat(hstring const& name, float value)
	{
		for (auto const& property : m_definition->properties)
		{
			if (name == property.name)
			{
				if (!std::isfinite(value) || value < property.minimum || value > property.maximum)
					throw hresult_invalid_argument(L"The value is outside the declared property range.");
				m_brush.Properties().InsertScalar(hstring{ m_definition->effectName + L"." + property.name }, value);
				return;
			}
		}
		throw hresult_invalid_argument(L"The float property is not declared by this effect.");
	}
}

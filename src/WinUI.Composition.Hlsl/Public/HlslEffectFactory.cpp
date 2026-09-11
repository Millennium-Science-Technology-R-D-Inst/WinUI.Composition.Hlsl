#include "HlslEffectFactory.h"
#include "HlslEffectFactory.g.cpp"
#include "HlslEffectBrush.h"
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	Hlsl::HlslEffectBrush HlslEffectFactory::CreateBrush()
	{
		auto brush=make<implementation::HlslEffectBrush>(m_factory.CreateBrush(), m_definition);
		for (auto const& property : m_definition->properties) brush.SetFloat(property.name, property.initial);
		return brush;
	}
}

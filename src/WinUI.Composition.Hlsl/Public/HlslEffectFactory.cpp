#include "pch.h"
#include "HlslEffectFactory.h"
#include "HlslEffectFactory.g.cpp"
#include "HlslEffectBrush.h"
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	Hlsl::HlslEffectBrush HlslEffectFactory::CreateBrush()
	{
		return make<implementation::HlslEffectBrush>(m_factory.CreateBrush());
	}
}

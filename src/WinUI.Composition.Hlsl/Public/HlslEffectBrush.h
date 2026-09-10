#pragma once
#include "HlslEffectBrush.g.h"
#include "EffectDefinition.h"
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct HlslEffectBrush : HlslEffectBrushT<HlslEffectBrush>
	{
		HlslEffectBrush(Microsoft::UI::Composition::CompositionEffectBrush const& brush, hlsl::engine::Definition definition) :m_brush(brush), m_definition(std::move(definition))
		{
		}
		Microsoft::UI::Composition::CompositionBrush Brush() const
		{
			return m_brush;
		}
		void SetSource(hstring const& name, Microsoft::UI::Composition::CompositionBrush const& source);
		void SetFloat(hstring const& name, float value);
	private:
		Microsoft::UI::Composition::CompositionEffectBrush m_brush{ nullptr };
		hlsl::engine::Definition m_definition;
	};
}

#pragma once
#ifndef WINRT_IMPORT_MODULE
#define WINRT_IMPORT_MODULE
#endif
#include "HlslEffectBrush.g.h"

import WinUI.Composition.Hlsl.EffectDef;

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
		Microsoft::UI::Composition::CompositionEffectBrush EffectBrush() const
		{
			return m_brush;
		}
		Microsoft::UI::Composition::CompositionPropertySet Properties() const
		{
			return m_brush.Properties();
		}
		hstring GetPropertyPath(hstring const& name) const;
		void SetSource(hstring const& name, Microsoft::UI::Composition::CompositionBrush const& source);
		void SetFloat(hstring const& name, float value);
	private:
		Microsoft::UI::Composition::CompositionEffectBrush m_brush{ nullptr };
		hlsl::engine::Definition m_definition;
	};
}

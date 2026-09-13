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
		void SetVector2(hstring const& name, Windows::Foundation::Numerics::float2 const& value);
		void SetVector3(hstring const& name, Windows::Foundation::Numerics::float3 const& value);
		void SetVector4(hstring const& name, Windows::Foundation::Numerics::float4 const& value);
		void SetMatrix3x2(hstring const& name, Windows::Foundation::Numerics::float3x2 const& value);
		void SetMatrix4x4(hstring const& name, Windows::Foundation::Numerics::float4x4 const& value);
	private:
		Microsoft::UI::Composition::CompositionEffectBrush m_brush{ nullptr };
		hlsl::engine::Definition m_definition;
	};
}

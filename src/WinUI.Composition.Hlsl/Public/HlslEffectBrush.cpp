#include "pch.h"
#include "HlslEffectBrush.h"
#include "HlslEffectBrush.g.cpp"
#include "CustomLiquidGlassEffect.h"
import WinUI.Composition.Hlsl.Validation;
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	void HlslEffectBrush::SetSource(hstring const& name, Microsoft::UI::Composition::CompositionBrush const& source)
	{
		if (name != L"Backdrop" || !source) throw hresult_invalid_argument(L"Unknown source; expected Backdrop.");
		if (source.Compositor() != m_brush.Compositor()) throw hresult_invalid_argument(L"The source belongs to another compositor.");
		m_brush.SetSourceParameter(name, source);
	}
	void HlslEffectBrush::SetFloat(hstring const& name, float value)
	{
		if (!m_glass || !hlsl::validation::IsFiniteNonNegative(value)) throw hresult_invalid_argument(L"Unknown property or invalid value.");
		for (auto property : { L"BlurRadius",L"RefractionStrength",L"DispersionStrength",L"CornerRadius",L"BorderThickness",L"HighlightStrength" })
		{
			if (name == property)
			{
				m_brush.Properties().InsertScalar(hstring(CustomLiquidGlassEffect::EffectName) + L"." + name, value); return;
			}
		}
		throw hresult_invalid_argument(L"Unknown HLSL property.");
	}
}

#include "pch.h"
#include "LiquidGlassMaterial.h"
#include "LiquidGlassMaterial.g.cpp"
#include "HlslEffectBrush.h"
#include "CustomLiquidGlassEffect.h"
import WinUI.Composition.Hlsl.Validation;
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	LiquidGlassMaterial::LiquidGlassMaterial(Microsoft::UI::Composition::Compositor const& compositor)
	{
		if (!compositor) throw hresult_invalid_argument();
		auto names=single_threaded_vector<hstring>();
		for (auto name : { L"BlurRadius",L"RefractionStrength",L"DispersionStrength",L"CornerRadius",L"BorderThickness",L"HighlightStrength" })
			names.Append(hstring(CustomLiquidGlassEffect::EffectName) + L"." + name);
		auto brush=compositor.CreateEffectFactory(CustomLiquidGlassEffect::CreateEffect(), names).CreateBrush();
		m_effect=make<implementation::HlslEffectBrush>(brush, true);
		m_effect.SetSource(L"Backdrop", compositor.CreateBackdropBrush());
		m_effect.SetFloat(L"BlurRadius", m_BlurRadius);
		m_effect.SetFloat(L"RefractionStrength", m_RefractionStrength);
		m_effect.SetFloat(L"DispersionStrength", m_DispersionStrength);
		m_effect.SetFloat(L"CornerRadius", m_CornerRadius);
		m_effect.SetFloat(L"BorderThickness", m_BorderThickness);
		m_effect.SetFloat(L"HighlightStrength", m_HighlightStrength);
	}
	void LiquidGlassMaterial::BlurRadius(float value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be finite and between 0 and 256.");
		m_effect.SetFloat(L"BlurRadius", value); m_BlurRadius=value;
	}
	void LiquidGlassMaterial::RefractionStrength(float value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be finite and between 0 and 256.");
		m_effect.SetFloat(L"RefractionStrength", value); m_RefractionStrength=value;
	}
	void LiquidGlassMaterial::DispersionStrength(float value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be finite and between 0 and 256.");
		m_effect.SetFloat(L"DispersionStrength", value); m_DispersionStrength=value;
	}
	void LiquidGlassMaterial::CornerRadius(float value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be finite and between 0 and 256.");
		m_effect.SetFloat(L"CornerRadius", value); m_CornerRadius=value;
	}
	void LiquidGlassMaterial::BorderThickness(float value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be finite and between 0 and 256.");
		m_effect.SetFloat(L"BorderThickness", value); m_BorderThickness=value;
	}
	void LiquidGlassMaterial::HighlightStrength(float value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be finite and between 0 and 256.");
		m_effect.SetFloat(L"HighlightStrength", value); m_HighlightStrength=value;
	}
}

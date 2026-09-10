#include "pch.h"
#include "LiquidGlassMaterial.h"
#include "LiquidGlassMaterial.g.cpp"
#include "HlslEffectFactory.h"
#include "CustomLiquidGlassEffect.h"
import WinUI.Composition.Hlsl.Validation;
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	LiquidGlassMaterial::LiquidGlassMaterial(Microsoft::UI::Composition::Compositor const& compositor)
	{
		if (!compositor) throw hresult_invalid_argument();
		auto definition = CustomLiquidGlassEffect::Description();
		auto factory = make<implementation::HlslEffectFactory>(hlsl::engine::GetFactory(compositor, definition), definition);
		m_effect=factory.CreateBrush();
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
		m_effect.SetFloat(L"BlurRadius", value); m_BlurRadius=value;
	}
	void LiquidGlassMaterial::RefractionStrength(float value)
	{
		m_effect.SetFloat(L"RefractionStrength", value); m_RefractionStrength=value;
	}
	void LiquidGlassMaterial::DispersionStrength(float value)
	{
		m_effect.SetFloat(L"DispersionStrength", value); m_DispersionStrength=value;
	}
	void LiquidGlassMaterial::CornerRadius(float value)
	{
		m_effect.SetFloat(L"CornerRadius", value); m_CornerRadius=value;
	}
	void LiquidGlassMaterial::BorderThickness(float value)
	{
		m_effect.SetFloat(L"BorderThickness", value); m_BorderThickness=value;
	}
	void LiquidGlassMaterial::HighlightStrength(float value)
	{
		m_effect.SetFloat(L"HighlightStrength", value); m_HighlightStrength=value;
	}
}

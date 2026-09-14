#pragma once
#ifndef WINRT_IMPORT_MODULE
#define WINRT_IMPORT_MODULE
#endif
#include "LiquidGlassMaterial.g.h"
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct LiquidGlassMaterial : LiquidGlassMaterialT<LiquidGlassMaterial>
	{
		LiquidGlassMaterial(Microsoft::UI::Composition::Compositor const& compositor);
		Hlsl::HlslEffectBrush EffectBrush() const
		{
			return m_effect;
		}
		float BlurRadius() const { return m_BlurRadius; }
		void BlurRadius(float value);
		float RefractionStrength() const { return m_RefractionStrength; }
		void RefractionStrength(float value);
		float DispersionStrength() const { return m_DispersionStrength; }
		void DispersionStrength(float value);
		float CornerRadius() const { return m_CornerRadius; }
		void CornerRadius(float value);
		float BorderThickness() const { return m_BorderThickness; }
		void BorderThickness(float value);
		float HighlightStrength() const { return m_HighlightStrength; }
		void HighlightStrength(float value);
		float BezelWidth() const { return m_BezelWidth; }
		void BezelWidth(float value);
		float GlassThickness() const { return m_GlassThickness; }
		void GlassThickness(float value);
		float RefractiveIndex() const { return m_RefractiveIndex; }
		void RefractiveIndex(float value);
		float TintOpacity() const { return m_TintOpacity; }
		void TintOpacity(float value);
		float Saturation() const { return m_Saturation; }
		void Saturation(float value);
		float LightAngle() const { return m_LightAngle; }
		void LightAngle(float value);

	private:
		Hlsl::HlslEffectBrush m_effect{ nullptr };
		Microsoft::UI::Composition::CompositionEffectBrush m_compositionEffect{ nullptr };
		float m_BlurRadius{ 12.0f };
		float m_RefractionStrength{ 24.0f };
		float m_DispersionStrength{ 1.2f };
		float m_CornerRadius{ 36.0f };
		float m_BorderThickness{ 1.5f };
		float m_HighlightStrength{ 0.85f };
		float m_BezelWidth{ 32.0f };
		float m_GlassThickness{ 50.0f };
		float m_RefractiveIndex{ 1.5f };
		float m_TintOpacity{ 0.08f };
		float m_Saturation{ 1.25f };
		float m_LightAngle{ -0.95f };
	};
}
namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct LiquidGlassMaterial : LiquidGlassMaterialT<LiquidGlassMaterial, implementation::LiquidGlassMaterial>
	{
	};
}

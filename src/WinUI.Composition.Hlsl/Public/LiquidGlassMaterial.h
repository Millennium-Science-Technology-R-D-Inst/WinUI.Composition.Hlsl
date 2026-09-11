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
		float BlurRadius() const
		{
			return m_BlurRadius;
		}
		void BlurRadius(float value);
		float RefractionStrength() const
		{
			return m_RefractionStrength;
		}
		void RefractionStrength(float value);
		float DispersionStrength() const
		{
			return m_DispersionStrength;
		}
		void DispersionStrength(float value);
		float CornerRadius() const
		{
			return m_CornerRadius;
		}
		void CornerRadius(float value);
		float BorderThickness() const
		{
			return m_BorderThickness;
		}
		void BorderThickness(float value);
		float HighlightStrength() const
		{
			return m_HighlightStrength;
		}
		void HighlightStrength(float value);
	private:
		Hlsl::HlslEffectBrush m_effect{ nullptr };
		Microsoft::UI::Composition::CompositionEffectBrush m_blurEffect{ nullptr };
		float m_BlurRadius{ 12.0f };
		float m_RefractionStrength{ 24.0f };
		float m_DispersionStrength{ 1.2f };
		float m_CornerRadius{ 12.0f };
		float m_BorderThickness{ 1.0f };
		float m_HighlightStrength{ 0.8f };

	};
}
namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct LiquidGlassMaterial : LiquidGlassMaterialT<LiquidGlassMaterial, implementation::LiquidGlassMaterial>
	{
	};
}

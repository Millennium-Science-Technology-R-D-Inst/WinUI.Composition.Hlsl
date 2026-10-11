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
		Hlsl::HlslEffectBrush EffectBrush() const { return m_effect; }
		Hlsl::HlslEffectBrush PostProcessingEffectBrush() const { return m_postEffect; }
		float BlurRadius() const { return m_BlurRadius; }
		void BlurRadius(float value);
		float RefractionStrength() const { return m_RefractionStrength; }
		void RefractionStrength(float value);
		float DispersionStrength() const { return m_DispersionStrength; }
		void DispersionStrength(float value);
		float CornerRadius() const { return m_CornerRadius; }
		void CornerRadius(float value);
		float ShapeRoundness() const { return m_ShapeRoundness; }
		void ShapeRoundness(float value);
		float RasterizationScale() const { return m_RasterizationScale; }
		void RasterizationScale(float value);
		float BorderThickness() const { return m_BorderThickness; }
		void BorderThickness(float value);
		float HighlightStrength() const { return m_HighlightStrength; }
		void HighlightStrength(float value);
		float EdgeSoftness() const { return m_EdgeSoftness; }
		void EdgeSoftness(float value);
		float MaterialOpacity() const { return m_MaterialOpacity; }
		void MaterialOpacity(float value);
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
		Hlsl::LiquidGlassSurfaceProfile SurfaceProfile() const { return m_SurfaceProfile; }
		void SurfaceProfile(Hlsl::LiquidGlassSurfaceProfile value);
		float MagnificationStrength() const { return m_MagnificationStrength; }
		void MagnificationStrength(float value);
		float HighlightSharpness() const { return m_HighlightSharpness; }
		void HighlightSharpness(float value);
		float TintRed() const { return m_TintRed; }
		void TintRed(float value);
		float TintGreen() const { return m_TintGreen; }
		void TintGreen(float value);
		float TintBlue() const { return m_TintBlue; }
		void TintBlue(float value);
		float InnerShadowStrength() const { return m_InnerShadowStrength; }
		void InnerShadowStrength(float value);
		float SpecularSaturation() const { return m_SpecularSaturation; }
		void SpecularSaturation(float value);
		float SpecularWidth() const { return m_SpecularWidth; }
		void SpecularWidth(float value);
		float Contrast() const { return m_Contrast; }
		void Contrast(float value);
		float Exposure() const { return m_Exposure; }
		void Exposure(float value);

		float DispersionRange() const { return m_DispersionRange; }
		void DispersionRange(float value);
		float RefFresnelRange() const { return m_RefFresnelRange; }
		void RefFresnelRange(float value);
		float RefFresnelHardness() const { return m_RefFresnelHardness; }
		void RefFresnelHardness(float value);
		float RefFresnelFactor() const { return m_RefFresnelFactor; }
		void RefFresnelFactor(float value);
		float GlareRange() const { return m_GlareRange; }
		void GlareRange(float value);
		float GlareHardness() const { return m_GlareHardness; }
		void GlareHardness(float value);
		float GlareFactor() const { return m_GlareFactor; }
		void GlareFactor(float value);
		float GlareConvergence() const { return m_GlareConvergence; }
		void GlareConvergence(float value);
		float GlareOppositeFactor() const { return m_GlareOppositeFactor; }
		void GlareOppositeFactor(float value);
		float GlareAngle() const { return m_GlareAngle; }
		void GlareAngle(float value);
		float ShapeRadius() const { return m_ShapeRadius; }
		void ShapeRadius(float value);
		float Magnification() const { return m_Magnification; }
		void Magnification(float value);
		float BloomAmount() const { return m_BloomAmount; }
		void BloomAmount(float value);
		float Brightness() const { return m_Brightness; }
		void Brightness(float value);
		float Temperature() const { return m_Temperature; }
		void Temperature(float value);
		float Vibrance() const { return m_Vibrance; }
		void Vibrance(float value);

	private:
		void SetFloatProperty(wchar_t const* propertyName, float& storage, float value, float minimum, float maximum, wchar_t const* message);
		void UpdateRefractionNormalization();
		void UpdateRasterizedBlurRadius();
		Hlsl::HlslEffectBrush m_effect{ nullptr };
		Hlsl::HlslEffectBrush m_postEffect{ nullptr };
		Microsoft::UI::Composition::CompositionEffectBrush m_blurHorizontalBrush{ nullptr };
		Microsoft::UI::Composition::CompositionEffectBrush m_blurVerticalBrush{ nullptr };
		float m_BlurRadius{ 12.0f };
		float m_RefractionStrength{ 24.0f };
		float m_DispersionStrength{ 1.2f };
		float m_CornerRadius{ 36.0f };
		float m_ShapeRoundness{ 4.0f };
		float m_RasterizationScale{ 1.0f };
		float m_BorderThickness{ 1.5f };
		float m_HighlightStrength{ 0.85f };
		float m_EdgeSoftness{ 1.0f };
		float m_MaterialOpacity{ 1.0f };
		float m_BezelWidth{ 32.0f };
		float m_GlassThickness{ 50.0f };
		float m_RefractiveIndex{ 1.5f };
		float m_TintOpacity{ 0.08f };
		float m_Saturation{ 1.25f };
		float m_LightAngle{ -0.95f };
		Hlsl::LiquidGlassSurfaceProfile m_SurfaceProfile{ Hlsl::LiquidGlassSurfaceProfile::ConvexSquircle };
		float m_MagnificationStrength{ 0.0f };
		float m_HighlightSharpness{ 1.5f };
		float m_TintRed{ 1.0f };
		float m_TintGreen{ 1.0f };
		float m_TintBlue{ 1.0f };
		float m_InnerShadowStrength{ 0.09f };
		float m_SpecularSaturation{ 4.0f };
		float m_SpecularWidth{ 1.0f };
		float m_Contrast{ 1.0f };
		float m_Exposure{ 0.0f };
		float m_DispersionRange{ 1.0f };
		float m_RefFresnelRange{ 30.0f };
		float m_RefFresnelHardness{ 20.0f };
		float m_RefFresnelFactor{ 0.0f };
		float m_GlareRange{ 30.0f };
		float m_GlareHardness{ 20.0f };
		float m_GlareFactor{ 0.0f };
		float m_GlareConvergence{ 50.0f };
		float m_GlareOppositeFactor{ 80.0f };
		float m_GlareAngle{ -45.0f };
		float m_ShapeRadius{ -1.0f };
		float m_Magnification{ 1.0f };
		float m_BloomAmount{ 1.0f };
		float m_Brightness{ 0.0f };
		float m_Temperature{ 0.0f };
		float m_Vibrance{ 0.0f };

	};
}
namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct LiquidGlassMaterial : LiquidGlassMaterialT<LiquidGlassMaterial, implementation::LiquidGlassMaterial>
	{
	};
}

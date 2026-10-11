#pragma once
import WinUI.Composition.Hlsl.EffectDef;
import winrt.Windows.Graphics.Effects;
import std;

namespace CustomLiquidGlassEffect
{
	inline constexpr wchar_t EffectName[] = L"BackdropLiquidGlassEffect";
	inline constexpr wchar_t RefractionStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.RefractionStrength";
	inline constexpr wchar_t CornerRadiusPropertyPath[] = L"BackdropLiquidGlassEffect.CornerRadius";
	inline constexpr wchar_t ShapeRoundnessPropertyPath[] = L"BackdropLiquidGlassEffect.ShapeRoundness";
	inline constexpr wchar_t RasterizationScalePropertyPath[] = L"BackdropLiquidGlassEffect.RasterizationScale";
	inline constexpr wchar_t BorderThicknessPropertyPath[] = L"BackdropLiquidGlassEffect.BorderThickness";
	inline constexpr wchar_t HighlightStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.HighlightStrength";
	inline constexpr wchar_t EdgeSoftnessPropertyPath[] = L"BackdropLiquidGlassEffect.EdgeSoftness";
	inline constexpr wchar_t DispersionStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.DispersionStrength";
	inline constexpr wchar_t MaterialOpacityPropertyPath[] = L"BackdropLiquidGlassEffect.MaterialOpacity";
	inline constexpr wchar_t BezelWidthPropertyPath[] = L"BackdropLiquidGlassEffect.BezelWidth";
	inline constexpr wchar_t GlassThicknessPropertyPath[] = L"BackdropLiquidGlassEffect.GlassThickness";
	inline constexpr wchar_t RefractiveIndexPropertyPath[] = L"BackdropLiquidGlassEffect.RefractiveIndex";
	inline constexpr wchar_t TintOpacityPropertyPath[] = L"BackdropLiquidGlassEffect.TintOpacity";
	inline constexpr wchar_t SaturationPropertyPath[] = L"BackdropLiquidGlassEffect.Saturation";
	inline constexpr wchar_t LightAnglePropertyPath[] = L"BackdropLiquidGlassEffect.LightAngle";
	inline constexpr wchar_t SurfaceProfilePropertyPath[] = L"BackdropLiquidGlassEffect.SurfaceProfile";
	inline constexpr wchar_t MagnificationStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.MagnificationStrength";
	inline constexpr wchar_t HighlightSharpnessPropertyPath[] = L"BackdropLiquidGlassEffect.HighlightSharpness";
	inline constexpr wchar_t TintRedPropertyPath[] = L"BackdropLiquidGlassEffect.TintRed";
	inline constexpr wchar_t TintGreenPropertyPath[] = L"BackdropLiquidGlassEffect.TintGreen";
	inline constexpr wchar_t TintBluePropertyPath[] = L"BackdropLiquidGlassEffect.TintBlue";
	inline constexpr wchar_t InnerShadowStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.InnerShadowStrength";
	inline constexpr wchar_t SpecularSaturationPropertyPath[] = L"BackdropLiquidGlassEffect.SpecularSaturation";
	inline constexpr wchar_t SpecularWidthPropertyPath[] = L"BackdropLiquidGlassEffect.SpecularWidth";
	inline constexpr wchar_t ContrastPropertyPath[] = L"BackdropLiquidGlassEffect.Contrast";
	inline constexpr wchar_t ExposurePropertyPath[] = L"BackdropLiquidGlassEffect.Exposure";
	inline constexpr wchar_t PointerXPropertyPath[] = L"BackdropLiquidGlassEffect.PointerX";
	inline constexpr wchar_t PointerYPropertyPath[] = L"BackdropLiquidGlassEffect.PointerY";
	inline constexpr wchar_t PointerInteractionRadiusPropertyPath[] = L"BackdropLiquidGlassEffect.PointerInteractionRadius";
	inline constexpr wchar_t PointerInteractionStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.PointerInteractionStrength";
	inline constexpr wchar_t PointerVelocityXPropertyPath[] = L"BackdropLiquidGlassEffect.PointerVelocityX";
	inline constexpr wchar_t PointerVelocityYPropertyPath[] = L"BackdropLiquidGlassEffect.PointerVelocityY";
	inline constexpr wchar_t PointerHoverRangePropertyPath[] = L"BackdropLiquidGlassEffect.PointerHoverRange";
	inline constexpr wchar_t PointerActivePropertyPath[] = L"BackdropLiquidGlassEffect.PointerActive";
	inline constexpr wchar_t PointerRefractionStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.PointerRefractionStrength";
	inline constexpr wchar_t PointerHighlightStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.PointerHighlightStrength";
	inline constexpr wchar_t PointerMotionRefractionStrengthPropertyPath[] = L"BackdropLiquidGlassEffect.PointerMotionRefractionStrength";
	inline constexpr wchar_t RefractionNormalizationPropertyPath[] = L"BackdropLiquidGlassEffect.RefractionNormalization";

	inline constexpr wchar_t DispersionRangePropertyPath[] = L"BackdropLiquidGlassEffect.DispersionRange";
	inline constexpr wchar_t RefFresnelRangePropertyPath[] = L"BackdropLiquidGlassEffect.RefFresnelRange";
	inline constexpr wchar_t RefFresnelHardnessPropertyPath[] = L"BackdropLiquidGlassEffect.RefFresnelHardness";
	inline constexpr wchar_t RefFresnelFactorPropertyPath[] = L"BackdropLiquidGlassEffect.RefFresnelFactor";
	inline constexpr wchar_t GlareRangePropertyPath[] = L"BackdropLiquidGlassEffect.GlareRange";
	inline constexpr wchar_t GlareHardnessPropertyPath[] = L"BackdropLiquidGlassEffect.GlareHardness";
	inline constexpr wchar_t GlareFactorPropertyPath[] = L"BackdropLiquidGlassEffect.GlareFactor";
	inline constexpr wchar_t GlareConvergencePropertyPath[] = L"BackdropLiquidGlassEffect.GlareConvergence";
	inline constexpr wchar_t GlareOppositeFactorPropertyPath[] = L"BackdropLiquidGlassEffect.GlareOppositeFactor";
	inline constexpr wchar_t GlareAnglePropertyPath[] = L"BackdropLiquidGlassEffect.GlareAngle";
	inline constexpr wchar_t ShapeRadiusPropertyPath[] = L"BackdropLiquidGlassEffect.ShapeRadius";
	inline constexpr wchar_t MagnificationPropertyPath[] = L"BackdropLiquidGlassEffect.Magnification";

	std::shared_ptr<hlsl::engine::EffectDefinition const> Description();
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect();
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect(
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source);
}

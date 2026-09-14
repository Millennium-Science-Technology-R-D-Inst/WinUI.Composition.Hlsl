#include <cmath>

#include "LiquidGlassMaterial.h"
#include "LiquidGlassMaterial.g.cpp"
#include "HlslEffectFactory.h"
#include "CustomLiquidGlassEffect.h"

import winrt.Microsoft.UI.Composition;
import winrt.Windows.Graphics.Effects;
import WinUI.Composition.Hlsl.GaussianBlurEffect;
import WinUI.Composition.Hlsl.Validation;

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	namespace
	{
		constexpr float RadiusToStandardDeviation(float radius) noexcept
		{
			return radius / 3.0f;
		}

		void ValidateRange(float value, float minimum, float maximum, wchar_t const* message)
		{
			if (!std::isfinite(value) || value < minimum || value > maximum)
			{
				throw hresult_invalid_argument(message);
			}
		}

		void ValidateBlurRadius(float value)
		{
			ValidateRange(value, 0.0f, 64.0f, L"BlurRadius must be finite and between 0 and 64 DIPs.");
		}
	}

	LiquidGlassMaterial::LiquidGlassMaterial(Microsoft::UI::Composition::Compositor const& compositor)
	{
		if (!compositor)
		{
			throw hresult_invalid_argument();
		}

		auto blurEffect = GaussianBlurEffect::CreateEffect(
			GaussianBlurEffect::LiquidGlassBlurEffectName,
			Microsoft::UI::Composition::CompositionEffectSourceParameter(L"Backdrop"),
			RadiusToStandardDeviation(m_BlurRadius));
		auto graph = CustomLiquidGlassEffect::CreateEffect(blurEffect);

		auto animatableProperties = single_threaded_vector<hstring>();
		animatableProperties.Append(GaussianBlurEffect::LiquidGlassBlurAmountPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::RefractionStrengthPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::CornerRadiusPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::BorderThicknessPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::HighlightStrengthPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::DispersionStrengthPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::BezelWidthPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::GlassThicknessPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::RefractiveIndexPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::TintOpacityPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::SaturationPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::LightAnglePropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::SurfaceProfilePropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::MagnificationStrengthPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::HighlightSharpnessPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::TintRedPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::TintGreenPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::TintBluePropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::InnerShadowStrengthPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::SpecularSaturationPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::SpecularWidthPropertyPath);

		auto definition = CustomLiquidGlassEffect::Description();
		auto compositionFactory = compositor.CreateEffectFactory(graph, animatableProperties);
		auto factory = make<implementation::HlslEffectFactory>(compositionFactory, definition);
		m_effect = factory.CreateBrush();
		m_compositionEffect = m_effect.Brush().as<Microsoft::UI::Composition::CompositionEffectBrush>();
		m_effect.SetSource(L"Backdrop", compositor.CreateBackdropBrush());
		m_effect.SetFloat(L"RefractionStrength", m_RefractionStrength);
		m_effect.SetFloat(L"DispersionStrength", m_DispersionStrength);
		m_effect.SetFloat(L"CornerRadius", m_CornerRadius);
		m_effect.SetFloat(L"BorderThickness", m_BorderThickness);
		m_effect.SetFloat(L"HighlightStrength", m_HighlightStrength);
		m_effect.SetFloat(L"BezelWidth", m_BezelWidth);
		m_effect.SetFloat(L"GlassThickness", m_GlassThickness);
		m_effect.SetFloat(L"RefractiveIndex", m_RefractiveIndex);
		m_effect.SetFloat(L"TintOpacity", m_TintOpacity);
		m_effect.SetFloat(L"Saturation", m_Saturation);
		m_effect.SetFloat(L"LightAngle", m_LightAngle);
		m_effect.SetFloat(L"SurfaceProfile", static_cast<float>(m_SurfaceProfile));
		m_effect.SetFloat(L"MagnificationStrength", m_MagnificationStrength);
		m_effect.SetFloat(L"HighlightSharpness", m_HighlightSharpness);
		m_effect.SetFloat(L"TintRed", m_TintRed);
		m_effect.SetFloat(L"TintGreen", m_TintGreen);
		m_effect.SetFloat(L"TintBlue", m_TintBlue);
		m_effect.SetFloat(L"InnerShadowStrength", m_InnerShadowStrength);
		m_effect.SetFloat(L"SpecularSaturation", m_SpecularSaturation);
		m_effect.SetFloat(L"SpecularWidth", m_SpecularWidth);
	}

	void LiquidGlassMaterial::BlurRadius(float value)
	{
		ValidateBlurRadius(value);
		m_compositionEffect.Properties().InsertScalar(GaussianBlurEffect::LiquidGlassBlurAmountPropertyPath, RadiusToStandardDeviation(value));
		m_BlurRadius = value;
	}

#define LIQUID_GLASS_FLOAT_PROPERTY(Name, PropertyName, Minimum, Maximum, Message) \
	void LiquidGlassMaterial::Name(float value) \
	{ \
		ValidateRange(value, Minimum, Maximum, Message); \
		m_effect.SetFloat(PropertyName, value); \
		m_##Name = value; \
	}

	LIQUID_GLASS_FLOAT_PROPERTY(RefractionStrength, L"RefractionStrength", 0.0f, 128.0f, L"RefractionStrength must be between 0 and 128.")
	LIQUID_GLASS_FLOAT_PROPERTY(DispersionStrength, L"DispersionStrength", 0.0f, 16.0f, L"DispersionStrength must be between 0 and 16.")
	LIQUID_GLASS_FLOAT_PROPERTY(CornerRadius, L"CornerRadius", 0.0f, 512.0f, L"CornerRadius must be between 0 and 512 DIPs.")
	LIQUID_GLASS_FLOAT_PROPERTY(BorderThickness, L"BorderThickness", 0.0f, 32.0f, L"BorderThickness must be between 0 and 32 DIPs.")
	LIQUID_GLASS_FLOAT_PROPERTY(HighlightStrength, L"HighlightStrength", 0.0f, 4.0f, L"HighlightStrength must be between 0 and 4.")
	LIQUID_GLASS_FLOAT_PROPERTY(BezelWidth, L"BezelWidth", 1.0f, 256.0f, L"BezelWidth must be between 1 and 256 DIPs.")
	LIQUID_GLASS_FLOAT_PROPERTY(GlassThickness, L"GlassThickness", 0.0f, 256.0f, L"GlassThickness must be between 0 and 256 DIPs.")
	LIQUID_GLASS_FLOAT_PROPERTY(RefractiveIndex, L"RefractiveIndex", 1.0f, 3.5f, L"RefractiveIndex must be between 1 and 3.5.")
	LIQUID_GLASS_FLOAT_PROPERTY(TintOpacity, L"TintOpacity", 0.0f, 1.0f, L"TintOpacity must be between 0 and 1.")
	LIQUID_GLASS_FLOAT_PROPERTY(Saturation, L"Saturation", 0.0f, 4.0f, L"Saturation must be between 0 and 4.")
	LIQUID_GLASS_FLOAT_PROPERTY(LightAngle, L"LightAngle", -6.2831855f, 6.2831855f, L"LightAngle must be between -2pi and 2pi radians.")
	LIQUID_GLASS_FLOAT_PROPERTY(MagnificationStrength, L"MagnificationStrength", 0.0f, 128.0f, L"MagnificationStrength must be between 0 and 128 pixels.")
	LIQUID_GLASS_FLOAT_PROPERTY(HighlightSharpness, L"HighlightSharpness", 0.25f, 64.0f, L"HighlightSharpness must be between 0.25 and 64.")
	LIQUID_GLASS_FLOAT_PROPERTY(TintRed, L"TintRed", 0.0f, 1.0f, L"TintRed must be between 0 and 1.")
	LIQUID_GLASS_FLOAT_PROPERTY(TintGreen, L"TintGreen", 0.0f, 1.0f, L"TintGreen must be between 0 and 1.")
	LIQUID_GLASS_FLOAT_PROPERTY(TintBlue, L"TintBlue", 0.0f, 1.0f, L"TintBlue must be between 0 and 1.")
	LIQUID_GLASS_FLOAT_PROPERTY(InnerShadowStrength, L"InnerShadowStrength", 0.0f, 1.0f, L"InnerShadowStrength must be between 0 and 1.")
	LIQUID_GLASS_FLOAT_PROPERTY(SpecularSaturation, L"SpecularSaturation", 0.0f, 50.0f, L"SpecularSaturation must be between 0 and 50.")
	LIQUID_GLASS_FLOAT_PROPERTY(SpecularWidth, L"SpecularWidth", 0.25f, 32.0f, L"SpecularWidth must be between 0.25 and 32 DIPs.")

#undef LIQUID_GLASS_FLOAT_PROPERTY

	void LiquidGlassMaterial::SurfaceProfile(Hlsl::LiquidGlassSurfaceProfile value)
	{
		auto const raw = static_cast<int32_t>(value);
		if (raw < static_cast<int32_t>(Hlsl::LiquidGlassSurfaceProfile::ConvexSquircle) ||
			raw > static_cast<int32_t>(Hlsl::LiquidGlassSurfaceProfile::Lip))
		{
			throw hresult_invalid_argument(L"SurfaceProfile is not a supported LiquidGlassSurfaceProfile value.");
		}
		m_effect.SetFloat(L"SurfaceProfile", static_cast<float>(raw));
		m_SurfaceProfile = value;
	}
}

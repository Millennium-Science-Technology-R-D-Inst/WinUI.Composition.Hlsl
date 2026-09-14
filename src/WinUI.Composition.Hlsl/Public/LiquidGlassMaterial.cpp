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
			throw hresult_invalid_argument();

		// Keep GaussianBlur and the custom sampler in one effect description. A
		// CompositionEffectBrush is not a supported SetSourceParameter input for
		// another CompositionEffectBrush; the MaterializedTexture lowering path is
		// responsible for turning this native upstream graph into the Texture2D that
		// the custom sampler needs.
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
	}

	void LiquidGlassMaterial::BlurRadius(float value)
	{
		ValidateBlurRadius(value);
		m_compositionEffect.Properties().InsertScalar(
			GaussianBlurEffect::LiquidGlassBlurAmountPropertyPath,
			RadiusToStandardDeviation(value));
		m_BlurRadius = value;
	}

	void LiquidGlassMaterial::RefractionStrength(float value)
	{
		ValidateRange(value, 0.0f, 128.0f, L"RefractionStrength must be between 0 and 128.");
		m_effect.SetFloat(L"RefractionStrength", value);
		m_RefractionStrength = value;
	}

	void LiquidGlassMaterial::DispersionStrength(float value)
	{
		ValidateRange(value, 0.0f, 16.0f, L"DispersionStrength must be between 0 and 16.");
		m_effect.SetFloat(L"DispersionStrength", value);
		m_DispersionStrength = value;
	}

	void LiquidGlassMaterial::CornerRadius(float value)
	{
		ValidateRange(value, 0.0f, 512.0f, L"CornerRadius must be between 0 and 512 DIPs.");
		m_effect.SetFloat(L"CornerRadius", value);
		m_CornerRadius = value;
	}

	void LiquidGlassMaterial::BorderThickness(float value)
	{
		ValidateRange(value, 0.0f, 32.0f, L"BorderThickness must be between 0 and 32 DIPs.");
		m_effect.SetFloat(L"BorderThickness", value);
		m_BorderThickness = value;
	}

	void LiquidGlassMaterial::HighlightStrength(float value)
	{
		ValidateRange(value, 0.0f, 4.0f, L"HighlightStrength must be between 0 and 4.");
		m_effect.SetFloat(L"HighlightStrength", value);
		m_HighlightStrength = value;
	}

	void LiquidGlassMaterial::BezelWidth(float value)
	{
		ValidateRange(value, 1.0f, 256.0f, L"BezelWidth must be between 1 and 256 DIPs.");
		m_effect.SetFloat(L"BezelWidth", value);
		m_BezelWidth = value;
	}

	void LiquidGlassMaterial::GlassThickness(float value)
	{
		ValidateRange(value, 0.0f, 256.0f, L"GlassThickness must be between 0 and 256 DIPs.");
		m_effect.SetFloat(L"GlassThickness", value);
		m_GlassThickness = value;
	}

	void LiquidGlassMaterial::RefractiveIndex(float value)
	{
		ValidateRange(value, 1.0f, 3.5f, L"RefractiveIndex must be between 1 and 3.5.");
		m_effect.SetFloat(L"RefractiveIndex", value);
		m_RefractiveIndex = value;
	}

	void LiquidGlassMaterial::TintOpacity(float value)
	{
		ValidateRange(value, 0.0f, 1.0f, L"TintOpacity must be between 0 and 1.");
		m_effect.SetFloat(L"TintOpacity", value);
		m_TintOpacity = value;
	}

	void LiquidGlassMaterial::Saturation(float value)
	{
		ValidateRange(value, 0.0f, 4.0f, L"Saturation must be between 0 and 4.");
		m_effect.SetFloat(L"Saturation", value);
		m_Saturation = value;
	}

	void LiquidGlassMaterial::LightAngle(float value)
	{
		ValidateRange(value, -6.2831855f, 6.2831855f, L"LightAngle must be between -2pi and 2pi radians.");
		m_effect.SetFloat(L"LightAngle", value);
		m_LightAngle = value;
	}
}

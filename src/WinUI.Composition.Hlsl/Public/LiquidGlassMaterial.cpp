#include "LiquidGlassMaterial.h"
#include "LiquidGlassMaterial.g.cpp"
#include "HlslEffectFactory.h"
#include "CustomLiquidGlassEffect.h"

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

		void ValidateBlurRadius(float value)
		{
			if (!hlsl::validation::IsFiniteNonNegative(value) || value > 64.0f)
			{
				throw hresult_invalid_argument(L"BlurRadius must be finite and between 0 and 64 DIPs.");
			}
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
			Windows::Graphics::Effects::CompositionEffectSourceParameter(L"Backdrop"),
			RadiusToStandardDeviation(m_BlurRadius));
		auto graph = CustomLiquidGlassEffect::CreateEffect(blurEffect);

		auto animatableProperties = single_threaded_vector<hstring>();
		animatableProperties.Append(GaussianBlurEffect::LiquidGlassBlurAmountPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::RefractionStrengthPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::CornerRadiusPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::BorderThicknessPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::HighlightStrengthPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::DispersionStrengthPropertyPath);

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
		m_effect.SetFloat(L"RefractionStrength", value); m_RefractionStrength = value;
	}
	void LiquidGlassMaterial::DispersionStrength(float value)
	{
		m_effect.SetFloat(L"DispersionStrength", value); m_DispersionStrength = value;
	}
	void LiquidGlassMaterial::CornerRadius(float value)
	{
		m_effect.SetFloat(L"CornerRadius", value); m_CornerRadius = value;
	}
	void LiquidGlassMaterial::BorderThickness(float value)
	{
		m_effect.SetFloat(L"BorderThickness", value); m_BorderThickness = value;
	}
	void LiquidGlassMaterial::HighlightStrength(float value)
	{
		m_effect.SetFloat(L"HighlightStrength", value); m_HighlightStrength = value;
	}
}

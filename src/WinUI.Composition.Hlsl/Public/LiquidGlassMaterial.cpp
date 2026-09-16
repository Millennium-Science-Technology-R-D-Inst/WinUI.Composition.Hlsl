#include <algorithm>
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

		float ConvexCircle(float x) noexcept
		{
			return std::sqrt(std::max(1.0f - (1.0f - x) * (1.0f - x), 0.0f));
		}

		float ConvexSquircleRaw(float x) noexcept
		{
			auto const s = 1.0f - x;
			return std::pow(std::max(1.0f - s * s * s * s, 0.0f), 0.25f);
		}

		float ConcaveCircle(float x) noexcept
		{
			return 1.0f - ConvexCircle(x);
		}

		float SmootherStep(float x) noexcept
		{
			return 6.0f * x * x * x * x * x -
				15.0f * x * x * x * x +
				10.0f * x * x * x;
		}

		float KubeSurfaceHeight(float x, Hlsl::LiquidGlassSurfaceProfile profile) noexcept
		{
			switch (profile)
			{
			case Hlsl::LiquidGlassSurfaceProfile::ConvexCircle:
				return ConvexCircle(x);
			case Hlsl::LiquidGlassSurfaceProfile::Concave:
				return ConcaveCircle(x);
			case Hlsl::LiquidGlassSurfaceProfile::Lip:
			{
				auto const blend = SmootherStep(x);
				auto const convex = ConvexSquircleRaw(x * 2.0f);
				auto const concave = ConcaveCircle(x) + 0.1f;
				return convex * (1.0f - blend) + concave * blend;
			}
			case Hlsl::LiquidGlassSurfaceProfile::ConvexSquircle:
			default:
				return ConvexSquircleRaw(x);
			}
		}

		float KubeReferenceDistance(
			float x,
			float bezelWidth,
			float glassThickness,
			float refractiveIndex,
			Hlsl::LiquidGlassSurfaceProfile profile) noexcept
		{
			auto const y = KubeSurfaceHeight(x, profile);
			// Kube samples i/128 for i=0..127, so x never reaches one and its derivative
			// always uses the positive 0.0001 probe.
			constexpr float dx = 0.0001f;
			auto const y2 = KubeSurfaceHeight(x + dx, profile);
			auto const derivative = (y2 - y) / dx;
			auto const magnitude = std::sqrt(derivative * derivative + 1.0f);
			auto const normalX = -derivative / magnitude;
			auto const normalY = -1.0f / magnitude;
			auto const eta = 1.0f / refractiveIndex;
			auto const dot = normalY;
			auto const k = 1.0f - eta * eta * (1.0f - dot * dot);
			if (k < 0.0f)
			{
				return 0.0f;
			}

			auto const q = eta * dot + std::sqrt(k);
			auto const refractedX = -q * normalX;
			auto const refractedY = eta - q * normalY;
			if (std::abs(refractedY) <= 1e-6f)
			{
				return 0.0f;
			}

			auto const remainingHeight = y * bezelWidth + glassThickness;
			return refractedX * (remainingHeight / refractedY);
		}

		float KubeRefractionNormalization(
			float bezelWidth,
			float glassThickness,
			float refractiveIndex,
			Hlsl::LiquidGlassSurfaceProfile profile) noexcept
		{
			float maxDisplacement = 0.0f;
			for (int32_t i = 0; i < 128; ++i)
			{
				auto const x = static_cast<float>(i) / 128.0f;
				maxDisplacement = std::max(
					maxDisplacement,
					std::abs(KubeReferenceDistance(
						x,
						bezelWidth,
						glassThickness,
						refractiveIndex,
						profile)));
			}

			// Kube first divides the displacement-map vector by maximumDisplacement=100,
			// encodes it as 128 + d*127, then feDisplacementMap decodes channel/255-.5
			// and multiplies by maxDisplacement*scaleRatio. This is the resulting factor
			// that must multiply our analytical ray distance for scaleRatio=1.
			return maxDisplacement * (127.0f / 255.0f) / 100.0f;
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
		animatableProperties.Append(CustomLiquidGlassEffect::EdgeSoftnessPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::DispersionStrengthPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::MaterialOpacityPropertyPath);
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
		animatableProperties.Append(CustomLiquidGlassEffect::ContrastPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::ExposurePropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::PointerXPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::PointerYPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::PointerInteractionRadiusPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::PointerInteractionStrengthPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::PointerVelocityXPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::PointerVelocityYPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::PointerHoverRangePropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::PointerActivePropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::PointerRefractionStrengthPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::PointerHighlightStrengthPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::PointerMotionRefractionStrengthPropertyPath);
		animatableProperties.Append(CustomLiquidGlassEffect::RefractionNormalizationPropertyPath);

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
		m_effect.SetFloat(L"EdgeSoftness", m_EdgeSoftness);
		m_effect.SetFloat(L"MaterialOpacity", m_MaterialOpacity);
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
		m_effect.SetFloat(L"Contrast", m_Contrast);
		m_effect.SetFloat(L"Exposure", m_Exposure);
		UpdateRefractionNormalization();

		// Pointer-field state is transient Composition state rather than XAML dependency
		// properties. Spatial values are normalized so XAML layout DIPs and shader raster
		// coordinates remain aligned under DPI/compositor scaling.
		m_effect.SetFloat(L"PointerX", 0.0f);
		m_effect.SetFloat(L"PointerY", 0.0f);
		m_effect.SetFloat(L"PointerInteractionRadius", 0.65f);
		m_effect.SetFloat(L"PointerInteractionStrength", 1.0f);
		m_effect.SetFloat(L"PointerVelocityX", 0.0f);
		m_effect.SetFloat(L"PointerVelocityY", 0.0f);
		m_effect.SetFloat(L"PointerHoverRange", 0.10f);
		m_effect.SetFloat(L"PointerActive", 0.0f);
		m_effect.SetFloat(L"PointerRefractionStrength", 5.0f);
		m_effect.SetFloat(L"PointerHighlightStrength", 0.22f);
		m_effect.SetFloat(L"PointerMotionRefractionStrength", 5.0f);
	}

	void LiquidGlassMaterial::UpdateRefractionNormalization()
	{
		m_effect.SetFloat(
			L"RefractionNormalization",
			KubeRefractionNormalization(
				m_BezelWidth,
				m_GlassThickness,
				m_RefractiveIndex,
				m_SurfaceProfile));
	}

	void LiquidGlassMaterial::BlurRadius(float value)
	{
		ValidateBlurRadius(value);
		m_compositionEffect.Properties().InsertScalar(GaussianBlurEffect::LiquidGlassBlurAmountPropertyPath, RadiusToStandardDeviation(value));
		m_BlurRadius = value;
	}

	void LiquidGlassMaterial::SetFloatProperty(wchar_t const* propertyName, float& storage, float value, float minimum, float maximum, wchar_t const* message)
	{
		ValidateRange(value, minimum, maximum, message);
		m_effect.SetFloat(propertyName, value);
		storage = value;
	}

	void LiquidGlassMaterial::RefractionStrength(float value)
	{
		SetFloatProperty(L"RefractionStrength", m_RefractionStrength, value, 0.0f, 128.0f, L"RefractionStrength must be between 0 and 128.");
	}

	void LiquidGlassMaterial::DispersionStrength(float value)
	{
		SetFloatProperty(L"DispersionStrength", m_DispersionStrength, value, 0.0f, 16.0f, L"DispersionStrength must be between 0 and 16.");
	}

	void LiquidGlassMaterial::CornerRadius(float value)
	{
		SetFloatProperty(L"CornerRadius", m_CornerRadius, value, 0.0f, 512.0f, L"CornerRadius must be between 0 and 512 DIPs.");
	}

	void LiquidGlassMaterial::BorderThickness(float value)
	{
		SetFloatProperty(L"BorderThickness", m_BorderThickness, value, 0.0f, 32.0f, L"BorderThickness must be between 0 and 32 DIPs.");
	}

	void LiquidGlassMaterial::HighlightStrength(float value)
	{
		SetFloatProperty(L"HighlightStrength", m_HighlightStrength, value, 0.0f, 4.0f, L"HighlightStrength must be between 0 and 4.");
	}

	void LiquidGlassMaterial::EdgeSoftness(float value)
	{
		SetFloatProperty(L"EdgeSoftness", m_EdgeSoftness, value, 0.25f, 16.0f, L"EdgeSoftness must be between 0.25 and 16 DIPs.");
	}

	void LiquidGlassMaterial::MaterialOpacity(float value)
	{
		SetFloatProperty(L"MaterialOpacity", m_MaterialOpacity, value, 0.0f, 1.0f, L"MaterialOpacity must be between 0 and 1.");
	}

	void LiquidGlassMaterial::BezelWidth(float value)
	{
		SetFloatProperty(L"BezelWidth", m_BezelWidth, value, 1.0f, 256.0f, L"BezelWidth must be between 1 and 256 DIPs.");
		UpdateRefractionNormalization();
	}

	void LiquidGlassMaterial::GlassThickness(float value)
	{
		SetFloatProperty(L"GlassThickness", m_GlassThickness, value, 0.0f, 256.0f, L"GlassThickness must be between 0 and 256 DIPs.");
		UpdateRefractionNormalization();
	}

	void LiquidGlassMaterial::RefractiveIndex(float value)
	{
		SetFloatProperty(L"RefractiveIndex", m_RefractiveIndex, value, 1.0f, 3.5f, L"RefractiveIndex must be between 1 and 3.5.");
		UpdateRefractionNormalization();
	}

	void LiquidGlassMaterial::TintOpacity(float value)
	{
		SetFloatProperty(L"TintOpacity", m_TintOpacity, value, 0.0f, 1.0f, L"TintOpacity must be between 0 and 1.");
	}

	void LiquidGlassMaterial::Saturation(float value)
	{
		SetFloatProperty(L"Saturation", m_Saturation, value, 0.0f, 4.0f, L"Saturation must be between 0 and 4.");
	}

	void LiquidGlassMaterial::LightAngle(float value)
	{
		SetFloatProperty(L"LightAngle", m_LightAngle, value, -6.2831855f, 6.2831855f, L"LightAngle must be between -2pi and 2pi radians.");
	}

	void LiquidGlassMaterial::MagnificationStrength(float value)
	{
		SetFloatProperty(L"MagnificationStrength", m_MagnificationStrength, value, 0.0f, 128.0f, L"MagnificationStrength must be between 0 and 128 pixels.");
	}

	void LiquidGlassMaterial::HighlightSharpness(float value)
	{
		SetFloatProperty(L"HighlightSharpness", m_HighlightSharpness, value, 0.25f, 64.0f, L"HighlightSharpness must be between 0.25 and 64.");
	}

	void LiquidGlassMaterial::TintRed(float value)
	{
		SetFloatProperty(L"TintRed", m_TintRed, value, 0.0f, 1.0f, L"TintRed must be between 0 and 1.");
	}

	void LiquidGlassMaterial::TintGreen(float value)
	{
		SetFloatProperty(L"TintGreen", m_TintGreen, value, 0.0f, 1.0f, L"TintGreen must be between 0 and 1.");
	}

	void LiquidGlassMaterial::TintBlue(float value)
	{
		SetFloatProperty(L"TintBlue", m_TintBlue, value, 0.0f, 1.0f, L"TintBlue must be between 0 and 1.");
	}

	void LiquidGlassMaterial::InnerShadowStrength(float value)
	{
		SetFloatProperty(L"InnerShadowStrength", m_InnerShadowStrength, value, 0.0f, 1.0f, L"InnerShadowStrength must be between 0 and 1.");
	}

	void LiquidGlassMaterial::SpecularSaturation(float value)
	{
		SetFloatProperty(L"SpecularSaturation", m_SpecularSaturation, value, 0.0f, 50.0f, L"SpecularSaturation must be between 0 and 50.");
	}

	void LiquidGlassMaterial::SpecularWidth(float value)
	{
		SetFloatProperty(L"SpecularWidth", m_SpecularWidth, value, 0.25f, 32.0f, L"SpecularWidth must be between 0.25 and 32 DIPs.");
	}

	void LiquidGlassMaterial::Contrast(float value)
	{
		SetFloatProperty(L"Contrast", m_Contrast, value, 0.0f, 4.0f, L"Contrast must be between 0 and 4.");
	}

	void LiquidGlassMaterial::Exposure(float value)
	{
		SetFloatProperty(L"Exposure", m_Exposure, value, -4.0f, 4.0f, L"Exposure must be between -4 and 4 stops.");
	}

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
		UpdateRefractionNormalization();
	}
}

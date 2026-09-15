#include <cmath>

#include "LiquidGlassBrush.h"
#include "LiquidGlassBrush.g.cpp"

import winrt.Windows.UI.Xaml.Interop;

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	using namespace Microsoft::UI::Xaml;

	namespace
	{
		void ValidateRange(double value, double minimum, double maximum, wchar_t const* message)
		{
			if (!std::isfinite(value) || value < minimum || value > maximum)
			{
				throw hresult_invalid_argument(message);
			}
		}
	}

	LiquidGlassBrush::LiquidGlassBrush()
	{
		RegisterPropertyChangedCallback(Media::XamlCompositionBrushBase::FallbackColorProperty(), [this](auto const&, auto const&)
			{
				Update();
			});
	}

	void LiquidGlassBrush::Changed(DependencyObject const& object, DependencyPropertyChangedEventArgs const&)
	{
		get_self<LiquidGlassBrush>(object.as<Hlsl::LiquidGlassBrush>())->Update();
	}

	DependencyProperty LiquidGlassBrush::RegisterDoubleProperty(wchar_t const* name, double defaultValue)
	{
		return DependencyProperty::Register(
			name,
			xaml_typename<double>(),
			xaml_typename<class_type>(),
			PropertyMetadata(box_value(defaultValue), PropertyChangedCallback{ Changed }));
	}

	double LiquidGlassBrush::GetDoubleProperty(DependencyProperty const& property) const
	{
		return unbox_value<double>(GetValue(property));
	}

	void LiquidGlassBrush::SetDoubleProperty(DependencyProperty const& property, double value, double minimum, double maximum, wchar_t const* message)
	{
		ValidateRange(value, minimum, maximum, message);
		SetValue(property, box_value(value));
	}

	DependencyProperty LiquidGlassBrush::IsEnabledProperty()
	{
		static auto const property = DependencyProperty::Register(L"IsEnabled", xaml_typename<bool>(), xaml_typename<class_type>(), PropertyMetadata(box_value(true), PropertyChangedCallback{ Changed }));
		return property;
	}

	bool LiquidGlassBrush::IsEnabled() const
	{
		return unbox_value<bool>(GetValue(IsEnabledProperty()));
	}

	void LiquidGlassBrush::IsEnabled(bool value)
	{
		SetValue(IsEnabledProperty(), box_value(value));
	}

	DependencyProperty LiquidGlassBrush::BlurRadiusProperty()
	{
		static auto const property = RegisterDoubleProperty(L"BlurRadius", 12.0);
		return property;
	}

	double LiquidGlassBrush::BlurRadius() const { return GetDoubleProperty(BlurRadiusProperty()); }
	void LiquidGlassBrush::BlurRadius(double value) { SetDoubleProperty(BlurRadiusProperty(), value, 0.0, 64.0, L"BlurRadius must be between 0 and 64 DIPs."); }

	DependencyProperty LiquidGlassBrush::RefractionStrengthProperty()
	{
		static auto const property = RegisterDoubleProperty(L"RefractionStrength", 24.0);
		return property;
	}

	double LiquidGlassBrush::RefractionStrength() const { return GetDoubleProperty(RefractionStrengthProperty()); }
	void LiquidGlassBrush::RefractionStrength(double value) { SetDoubleProperty(RefractionStrengthProperty(), value, 0.0, 128.0, L"RefractionStrength must be between 0 and 128."); }

	DependencyProperty LiquidGlassBrush::DispersionStrengthProperty()
	{
		static auto const property = RegisterDoubleProperty(L"DispersionStrength", 1.2);
		return property;
	}

	double LiquidGlassBrush::DispersionStrength() const { return GetDoubleProperty(DispersionStrengthProperty()); }
	void LiquidGlassBrush::DispersionStrength(double value) { SetDoubleProperty(DispersionStrengthProperty(), value, 0.0, 16.0, L"DispersionStrength must be between 0 and 16."); }

	DependencyProperty LiquidGlassBrush::CornerRadiusProperty()
	{
		static auto const property = RegisterDoubleProperty(L"CornerRadius", 36.0);
		return property;
	}

	double LiquidGlassBrush::CornerRadius() const { return GetDoubleProperty(CornerRadiusProperty()); }
	void LiquidGlassBrush::CornerRadius(double value) { SetDoubleProperty(CornerRadiusProperty(), value, 0.0, 512.0, L"CornerRadius must be between 0 and 512 DIPs."); }

	DependencyProperty LiquidGlassBrush::BorderThicknessProperty()
	{
		static auto const property = RegisterDoubleProperty(L"BorderThickness", 1.5);
		return property;
	}

	double LiquidGlassBrush::BorderThickness() const { return GetDoubleProperty(BorderThicknessProperty()); }
	void LiquidGlassBrush::BorderThickness(double value) { SetDoubleProperty(BorderThicknessProperty(), value, 0.0, 32.0, L"BorderThickness must be between 0 and 32 DIPs."); }

	DependencyProperty LiquidGlassBrush::HighlightStrengthProperty()
	{
		static auto const property = RegisterDoubleProperty(L"HighlightStrength", 0.85);
		return property;
	}

	double LiquidGlassBrush::HighlightStrength() const { return GetDoubleProperty(HighlightStrengthProperty()); }
	void LiquidGlassBrush::HighlightStrength(double value) { SetDoubleProperty(HighlightStrengthProperty(), value, 0.0, 4.0, L"HighlightStrength must be between 0 and 4."); }

	DependencyProperty LiquidGlassBrush::EdgeSoftnessProperty()
	{
		static auto const property = RegisterDoubleProperty(L"EdgeSoftness", 1.0);
		return property;
	}

	double LiquidGlassBrush::EdgeSoftness() const { return GetDoubleProperty(EdgeSoftnessProperty()); }
	void LiquidGlassBrush::EdgeSoftness(double value) { SetDoubleProperty(EdgeSoftnessProperty(), value, 0.25, 16.0, L"EdgeSoftness must be between 0.25 and 16 DIPs."); }

	DependencyProperty LiquidGlassBrush::MaterialOpacityProperty()
	{
		static auto const property = RegisterDoubleProperty(L"MaterialOpacity", 1.0);
		return property;
	}

	double LiquidGlassBrush::MaterialOpacity() const { return GetDoubleProperty(MaterialOpacityProperty()); }
	void LiquidGlassBrush::MaterialOpacity(double value) { SetDoubleProperty(MaterialOpacityProperty(), value, 0.0, 1.0, L"MaterialOpacity must be between 0 and 1."); }

	DependencyProperty LiquidGlassBrush::BezelWidthProperty()
	{
		static auto const property = RegisterDoubleProperty(L"BezelWidth", 32.0);
		return property;
	}

	double LiquidGlassBrush::BezelWidth() const { return GetDoubleProperty(BezelWidthProperty()); }
	void LiquidGlassBrush::BezelWidth(double value) { SetDoubleProperty(BezelWidthProperty(), value, 1.0, 256.0, L"BezelWidth must be between 1 and 256 DIPs."); }

	DependencyProperty LiquidGlassBrush::GlassThicknessProperty()
	{
		static auto const property = RegisterDoubleProperty(L"GlassThickness", 50.0);
		return property;
	}

	double LiquidGlassBrush::GlassThickness() const { return GetDoubleProperty(GlassThicknessProperty()); }
	void LiquidGlassBrush::GlassThickness(double value) { SetDoubleProperty(GlassThicknessProperty(), value, 0.0, 256.0, L"GlassThickness must be between 0 and 256 DIPs."); }

	DependencyProperty LiquidGlassBrush::RefractiveIndexProperty()
	{
		static auto const property = RegisterDoubleProperty(L"RefractiveIndex", 1.5);
		return property;
	}

	double LiquidGlassBrush::RefractiveIndex() const { return GetDoubleProperty(RefractiveIndexProperty()); }
	void LiquidGlassBrush::RefractiveIndex(double value) { SetDoubleProperty(RefractiveIndexProperty(), value, 1.0, 3.5, L"RefractiveIndex must be between 1 and 3.5."); }

	DependencyProperty LiquidGlassBrush::TintOpacityProperty()
	{
		static auto const property = RegisterDoubleProperty(L"TintOpacity", 0.08);
		return property;
	}

	double LiquidGlassBrush::TintOpacity() const { return GetDoubleProperty(TintOpacityProperty()); }
	void LiquidGlassBrush::TintOpacity(double value) { SetDoubleProperty(TintOpacityProperty(), value, 0.0, 1.0, L"TintOpacity must be between 0 and 1."); }

	DependencyProperty LiquidGlassBrush::SaturationProperty()
	{
		static auto const property = RegisterDoubleProperty(L"Saturation", 1.25);
		return property;
	}

	double LiquidGlassBrush::Saturation() const { return GetDoubleProperty(SaturationProperty()); }
	void LiquidGlassBrush::Saturation(double value) { SetDoubleProperty(SaturationProperty(), value, 0.0, 4.0, L"Saturation must be between 0 and 4."); }

	DependencyProperty LiquidGlassBrush::LightAngleProperty()
	{
		static auto const property = RegisterDoubleProperty(L"LightAngle", -0.95);
		return property;
	}

	double LiquidGlassBrush::LightAngle() const { return GetDoubleProperty(LightAngleProperty()); }
	void LiquidGlassBrush::LightAngle(double value) { SetDoubleProperty(LightAngleProperty(), value, -6.2831855, 6.2831855, L"LightAngle must be between -2pi and 2pi radians."); }

	DependencyProperty LiquidGlassBrush::MagnificationStrengthProperty()
	{
		static auto const property = RegisterDoubleProperty(L"MagnificationStrength", 0.0);
		return property;
	}

	double LiquidGlassBrush::MagnificationStrength() const { return GetDoubleProperty(MagnificationStrengthProperty()); }
	void LiquidGlassBrush::MagnificationStrength(double value) { SetDoubleProperty(MagnificationStrengthProperty(), value, 0.0, 128.0, L"MagnificationStrength must be between 0 and 128 pixels."); }

	DependencyProperty LiquidGlassBrush::HighlightSharpnessProperty()
	{
		static auto const property = RegisterDoubleProperty(L"HighlightSharpness", 1.5);
		return property;
	}

	double LiquidGlassBrush::HighlightSharpness() const { return GetDoubleProperty(HighlightSharpnessProperty()); }
	void LiquidGlassBrush::HighlightSharpness(double value) { SetDoubleProperty(HighlightSharpnessProperty(), value, 0.25, 64.0, L"HighlightSharpness must be between 0.25 and 64."); }

	DependencyProperty LiquidGlassBrush::TintRedProperty()
	{
		static auto const property = RegisterDoubleProperty(L"TintRed", 1.0);
		return property;
	}

	double LiquidGlassBrush::TintRed() const { return GetDoubleProperty(TintRedProperty()); }
	void LiquidGlassBrush::TintRed(double value) { SetDoubleProperty(TintRedProperty(), value, 0.0, 1.0, L"TintRed must be between 0 and 1."); }

	DependencyProperty LiquidGlassBrush::TintGreenProperty()
	{
		static auto const property = RegisterDoubleProperty(L"TintGreen", 1.0);
		return property;
	}

	double LiquidGlassBrush::TintGreen() const { return GetDoubleProperty(TintGreenProperty()); }
	void LiquidGlassBrush::TintGreen(double value) { SetDoubleProperty(TintGreenProperty(), value, 0.0, 1.0, L"TintGreen must be between 0 and 1."); }

	DependencyProperty LiquidGlassBrush::TintBlueProperty()
	{
		static auto const property = RegisterDoubleProperty(L"TintBlue", 1.0);
		return property;
	}

	double LiquidGlassBrush::TintBlue() const { return GetDoubleProperty(TintBlueProperty()); }
	void LiquidGlassBrush::TintBlue(double value) { SetDoubleProperty(TintBlueProperty(), value, 0.0, 1.0, L"TintBlue must be between 0 and 1."); }

	DependencyProperty LiquidGlassBrush::InnerShadowStrengthProperty()
	{
		static auto const property = RegisterDoubleProperty(L"InnerShadowStrength", 0.09);
		return property;
	}

	double LiquidGlassBrush::InnerShadowStrength() const { return GetDoubleProperty(InnerShadowStrengthProperty()); }
	void LiquidGlassBrush::InnerShadowStrength(double value) { SetDoubleProperty(InnerShadowStrengthProperty(), value, 0.0, 1.0, L"InnerShadowStrength must be between 0 and 1."); }

	DependencyProperty LiquidGlassBrush::SpecularSaturationProperty()
	{
		static auto const property = RegisterDoubleProperty(L"SpecularSaturation", 4.0);
		return property;
	}

	double LiquidGlassBrush::SpecularSaturation() const { return GetDoubleProperty(SpecularSaturationProperty()); }
	void LiquidGlassBrush::SpecularSaturation(double value) { SetDoubleProperty(SpecularSaturationProperty(), value, 0.0, 50.0, L"SpecularSaturation must be between 0 and 50."); }

	DependencyProperty LiquidGlassBrush::SpecularWidthProperty()
	{
		static auto const property = RegisterDoubleProperty(L"SpecularWidth", 1.0);
		return property;
	}

	double LiquidGlassBrush::SpecularWidth() const { return GetDoubleProperty(SpecularWidthProperty()); }
	void LiquidGlassBrush::SpecularWidth(double value) { SetDoubleProperty(SpecularWidthProperty(), value, 0.25, 32.0, L"SpecularWidth must be between 0.25 and 32 DIPs."); }

	DependencyProperty LiquidGlassBrush::ContrastProperty()
	{
		static auto const property = RegisterDoubleProperty(L"Contrast", 1.0);
		return property;
	}

	double LiquidGlassBrush::Contrast() const { return GetDoubleProperty(ContrastProperty()); }
	void LiquidGlassBrush::Contrast(double value) { SetDoubleProperty(ContrastProperty(), value, 0.0, 4.0, L"Contrast must be between 0 and 4."); }

	DependencyProperty LiquidGlassBrush::ExposureProperty()
	{
		static auto const property = RegisterDoubleProperty(L"Exposure", 0.0);
		return property;
	}

	double LiquidGlassBrush::Exposure() const { return GetDoubleProperty(ExposureProperty()); }
	void LiquidGlassBrush::Exposure(double value) { SetDoubleProperty(ExposureProperty(), value, -4.0, 4.0, L"Exposure must be between -4 and 4 stops."); }

	DependencyProperty LiquidGlassBrush::SurfaceProfileProperty()
	{
		static auto const property = DependencyProperty::Register(L"SurfaceProfile", xaml_typename<Hlsl::LiquidGlassSurfaceProfile>(), xaml_typename<class_type>(), PropertyMetadata(box_value(Hlsl::LiquidGlassSurfaceProfile::ConvexSquircle), PropertyChangedCallback{ Changed }));
		return property;
	}

	Hlsl::LiquidGlassSurfaceProfile LiquidGlassBrush::SurfaceProfile() const
	{
		return unbox_value<Hlsl::LiquidGlassSurfaceProfile>(GetValue(SurfaceProfileProperty()));
	}

	void LiquidGlassBrush::SurfaceProfile(Hlsl::LiquidGlassSurfaceProfile value)
	{
		auto const raw = static_cast<int32_t>(value);
		if (raw < 0 || raw > 3)
		{
			throw hresult_invalid_argument(L"SurfaceProfile is not a supported LiquidGlassSurfaceProfile value.");
		}
		SetValue(SurfaceProfileProperty(), box_value(value));
	}

	void LiquidGlassBrush::OnConnected() { m_lifecycle.Connect(*this); }
	void LiquidGlassBrush::OnDisconnected() { m_lifecycle.Disconnect(*this); }
	void LiquidGlassBrush::Update() { m_lifecycle.Update(*this); }

	Microsoft::UI::Composition::CompositionBrush LiquidGlassBrush::BuildPipeline(Microsoft::UI::Composition::Compositor const& compositor)
	{
		if (!m_material)
		{
			m_material = Hlsl::LiquidGlassMaterial(compositor);
		}

		m_material.BlurRadius(static_cast<float>(BlurRadius()));
		m_material.RefractionStrength(static_cast<float>(RefractionStrength()));
		m_material.DispersionStrength(static_cast<float>(DispersionStrength()));
		m_material.CornerRadius(static_cast<float>(CornerRadius()));
		m_material.BorderThickness(static_cast<float>(BorderThickness()));
		m_material.HighlightStrength(static_cast<float>(HighlightStrength()));
		m_material.EdgeSoftness(static_cast<float>(EdgeSoftness()));
		m_material.MaterialOpacity(static_cast<float>(MaterialOpacity()));
		m_material.BezelWidth(static_cast<float>(BezelWidth()));
		m_material.GlassThickness(static_cast<float>(GlassThickness()));
		m_material.RefractiveIndex(static_cast<float>(RefractiveIndex()));
		m_material.TintOpacity(static_cast<float>(TintOpacity()));
		m_material.Saturation(static_cast<float>(Saturation()));
		m_material.LightAngle(static_cast<float>(LightAngle()));
		m_material.SurfaceProfile(SurfaceProfile());
		m_material.MagnificationStrength(static_cast<float>(MagnificationStrength()));
		m_material.HighlightSharpness(static_cast<float>(HighlightSharpness()));
		m_material.TintRed(static_cast<float>(TintRed()));
		m_material.TintGreen(static_cast<float>(TintGreen()));
		m_material.TintBlue(static_cast<float>(TintBlue()));
		m_material.InnerShadowStrength(static_cast<float>(InnerShadowStrength()));
		m_material.SpecularSaturation(static_cast<float>(SpecularSaturation()));
		m_material.SpecularWidth(static_cast<float>(SpecularWidth()));
		m_material.Contrast(static_cast<float>(Contrast()));
		m_material.Exposure(static_cast<float>(Exposure()));
		return m_material.EffectBrush().Brush();
	}
}

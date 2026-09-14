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

	DependencyProperty LiquidGlassBrush::IsEnabledProperty()
	{
		static auto property = DependencyProperty::Register(L"IsEnabled", winrt::xaml_typename<bool>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(true), PropertyChangedCallback{ Changed }));
		return property;
	}
	bool LiquidGlassBrush::IsEnabled() const { return unbox_value<bool>(GetValue(IsEnabledProperty())); }
	void LiquidGlassBrush::IsEnabled(bool value) { SetValue(IsEnabledProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::BlurRadiusProperty()
	{
		static auto property = DependencyProperty::Register(L"BlurRadius", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(12.0), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::BlurRadius() const { return unbox_value<double>(GetValue(BlurRadiusProperty())); }
	void LiquidGlassBrush::BlurRadius(double value) { ValidateRange(value, 0.0, 64.0, L"BlurRadius must be between 0 and 64 DIPs."); SetValue(BlurRadiusProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::RefractionStrengthProperty()
	{
		static auto property = DependencyProperty::Register(L"RefractionStrength", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(24.0), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::RefractionStrength() const { return unbox_value<double>(GetValue(RefractionStrengthProperty())); }
	void LiquidGlassBrush::RefractionStrength(double value) { ValidateRange(value, 0.0, 128.0, L"RefractionStrength must be between 0 and 128."); SetValue(RefractionStrengthProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::DispersionStrengthProperty()
	{
		static auto property = DependencyProperty::Register(L"DispersionStrength", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(1.2), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::DispersionStrength() const { return unbox_value<double>(GetValue(DispersionStrengthProperty())); }
	void LiquidGlassBrush::DispersionStrength(double value) { ValidateRange(value, 0.0, 16.0, L"DispersionStrength must be between 0 and 16."); SetValue(DispersionStrengthProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::CornerRadiusProperty()
	{
		static auto property = DependencyProperty::Register(L"CornerRadius", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(36.0), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::CornerRadius() const { return unbox_value<double>(GetValue(CornerRadiusProperty())); }
	void LiquidGlassBrush::CornerRadius(double value) { ValidateRange(value, 0.0, 512.0, L"CornerRadius must be between 0 and 512 DIPs."); SetValue(CornerRadiusProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::BorderThicknessProperty()
	{
		static auto property = DependencyProperty::Register(L"BorderThickness", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(1.5), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::BorderThickness() const { return unbox_value<double>(GetValue(BorderThicknessProperty())); }
	void LiquidGlassBrush::BorderThickness(double value) { ValidateRange(value, 0.0, 32.0, L"BorderThickness must be between 0 and 32 DIPs."); SetValue(BorderThicknessProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::HighlightStrengthProperty()
	{
		static auto property = DependencyProperty::Register(L"HighlightStrength", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(0.85), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::HighlightStrength() const { return unbox_value<double>(GetValue(HighlightStrengthProperty())); }
	void LiquidGlassBrush::HighlightStrength(double value) { ValidateRange(value, 0.0, 4.0, L"HighlightStrength must be between 0 and 4."); SetValue(HighlightStrengthProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::BezelWidthProperty()
	{
		static auto property = DependencyProperty::Register(L"BezelWidth", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(32.0), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::BezelWidth() const { return unbox_value<double>(GetValue(BezelWidthProperty())); }
	void LiquidGlassBrush::BezelWidth(double value) { ValidateRange(value, 1.0, 256.0, L"BezelWidth must be between 1 and 256 DIPs."); SetValue(BezelWidthProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::GlassThicknessProperty()
	{
		static auto property = DependencyProperty::Register(L"GlassThickness", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(50.0), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::GlassThickness() const { return unbox_value<double>(GetValue(GlassThicknessProperty())); }
	void LiquidGlassBrush::GlassThickness(double value) { ValidateRange(value, 0.0, 256.0, L"GlassThickness must be between 0 and 256 DIPs."); SetValue(GlassThicknessProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::RefractiveIndexProperty()
	{
		static auto property = DependencyProperty::Register(L"RefractiveIndex", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(1.5), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::RefractiveIndex() const { return unbox_value<double>(GetValue(RefractiveIndexProperty())); }
	void LiquidGlassBrush::RefractiveIndex(double value) { ValidateRange(value, 1.0, 3.5, L"RefractiveIndex must be between 1 and 3.5."); SetValue(RefractiveIndexProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::TintOpacityProperty()
	{
		static auto property = DependencyProperty::Register(L"TintOpacity", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(0.08), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::TintOpacity() const { return unbox_value<double>(GetValue(TintOpacityProperty())); }
	void LiquidGlassBrush::TintOpacity(double value) { ValidateRange(value, 0.0, 1.0, L"TintOpacity must be between 0 and 1."); SetValue(TintOpacityProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::SaturationProperty()
	{
		static auto property = DependencyProperty::Register(L"Saturation", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(1.25), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::Saturation() const { return unbox_value<double>(GetValue(SaturationProperty())); }
	void LiquidGlassBrush::Saturation(double value) { ValidateRange(value, 0.0, 4.0, L"Saturation must be between 0 and 4."); SetValue(SaturationProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::LightAngleProperty()
	{
		static auto property = DependencyProperty::Register(L"LightAngle", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(-0.95), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::LightAngle() const { return unbox_value<double>(GetValue(LightAngleProperty())); }
	void LiquidGlassBrush::LightAngle(double value) { ValidateRange(value, -6.2831855, 6.2831855, L"LightAngle must be between -2pi and 2pi radians."); SetValue(LightAngleProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::SurfaceProfileProperty()
	{
		static auto property = DependencyProperty::Register(L"SurfaceProfile", winrt::xaml_typename<Hlsl::LiquidGlassSurfaceProfile>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(Hlsl::LiquidGlassSurfaceProfile::ConvexSquircle), PropertyChangedCallback{ Changed }));
		return property;
	}
	Hlsl::LiquidGlassSurfaceProfile LiquidGlassBrush::SurfaceProfile() const { return unbox_value<Hlsl::LiquidGlassSurfaceProfile>(GetValue(SurfaceProfileProperty())); }
	void LiquidGlassBrush::SurfaceProfile(Hlsl::LiquidGlassSurfaceProfile value)
	{
		auto const raw = static_cast<int32_t>(value);
		if (raw < 0 || raw > 3) { throw hresult_invalid_argument(L"SurfaceProfile is not a supported LiquidGlassSurfaceProfile value."); }
		SetValue(SurfaceProfileProperty(), box_value(value));
	}

	DependencyProperty LiquidGlassBrush::MagnificationStrengthProperty()
	{
		static auto property = DependencyProperty::Register(L"MagnificationStrength", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(0.0), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::MagnificationStrength() const { return unbox_value<double>(GetValue(MagnificationStrengthProperty())); }
	void LiquidGlassBrush::MagnificationStrength(double value) { ValidateRange(value, 0.0, 128.0, L"MagnificationStrength must be between 0 and 128 pixels."); SetValue(MagnificationStrengthProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::HighlightSharpnessProperty()
	{
		static auto property = DependencyProperty::Register(L"HighlightSharpness", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(1.5), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::HighlightSharpness() const { return unbox_value<double>(GetValue(HighlightSharpnessProperty())); }
	void LiquidGlassBrush::HighlightSharpness(double value) { ValidateRange(value, 0.25, 64.0, L"HighlightSharpness must be between 0.25 and 64."); SetValue(HighlightSharpnessProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::TintRedProperty()
	{
		static auto property = DependencyProperty::Register(L"TintRed", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(1.0), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::TintRed() const { return unbox_value<double>(GetValue(TintRedProperty())); }
	void LiquidGlassBrush::TintRed(double value) { ValidateRange(value, 0.0, 1.0, L"TintRed must be between 0 and 1."); SetValue(TintRedProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::TintGreenProperty()
	{
		static auto property = DependencyProperty::Register(L"TintGreen", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(1.0), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::TintGreen() const { return unbox_value<double>(GetValue(TintGreenProperty())); }
	void LiquidGlassBrush::TintGreen(double value) { ValidateRange(value, 0.0, 1.0, L"TintGreen must be between 0 and 1."); SetValue(TintGreenProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::TintBlueProperty()
	{
		static auto property = DependencyProperty::Register(L"TintBlue", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(1.0), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::TintBlue() const { return unbox_value<double>(GetValue(TintBlueProperty())); }
	void LiquidGlassBrush::TintBlue(double value) { ValidateRange(value, 0.0, 1.0, L"TintBlue must be between 0 and 1."); SetValue(TintBlueProperty(), box_value(value)); }

	DependencyProperty LiquidGlassBrush::InnerShadowStrengthProperty()
	{
		static auto property = DependencyProperty::Register(L"InnerShadowStrength", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(0.09), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::InnerShadowStrength() const { return unbox_value<double>(GetValue(InnerShadowStrengthProperty())); }
	void LiquidGlassBrush::InnerShadowStrength(double value) { ValidateRange(value, 0.0, 1.0, L"InnerShadowStrength must be between 0 and 1."); SetValue(InnerShadowStrengthProperty(), box_value(value)); }

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
		return m_material.EffectBrush().Brush();
	}
}

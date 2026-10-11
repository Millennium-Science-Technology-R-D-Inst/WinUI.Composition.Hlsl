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
		// XAML must find every DP before it applies a Binding to this instance.
		// Lazy registration in a getter/setter is too late: an unregistered DP
		// falls through to the generated value setter, which tries to unbox the
		// Binding as a Double and throws E_NOINTERFACE during LoadComponent.
		(void)IsEnabledProperty();
		(void)BlurRadiusProperty();
		(void)RefractionStrengthProperty();
		(void)DispersionStrengthProperty();
		(void)CornerRadiusProperty();
		(void)ShapeRoundnessProperty();
		(void)RasterizationScaleProperty();
		(void)BorderThicknessProperty();
		(void)HighlightStrengthProperty();
		(void)EdgeSoftnessProperty();
		(void)MaterialOpacityProperty();
		(void)BezelWidthProperty();
		(void)GlassThicknessProperty();
		(void)RefractiveIndexProperty();
		(void)TintOpacityProperty();
		(void)SaturationProperty();
		(void)LightAngleProperty();
		(void)SurfaceProfileProperty();
		(void)MagnificationStrengthProperty();
		(void)HighlightSharpnessProperty();
		(void)TintRedProperty();
		(void)TintGreenProperty();
		(void)TintBlueProperty();
		(void)InnerShadowStrengthProperty();
		(void)SpecularSaturationProperty();
		(void)SpecularWidthProperty();
		(void)ContrastProperty();
		(void)ExposureProperty();
		(void)DispersionRangeProperty();
		(void)RefFresnelRangeProperty();
		(void)RefFresnelHardnessProperty();
		(void)RefFresnelFactorProperty();
		(void)GlareRangeProperty();
		(void)GlareHardnessProperty();
		(void)GlareFactorProperty();
		(void)GlareConvergenceProperty();
		(void)GlareOppositeFactorProperty();
		(void)GlareAngleProperty();
		(void)ShapeRadiusProperty();
		(void)MagnificationProperty();
		(void)BloomAmountProperty();
		(void)BrightnessProperty();
		(void)TemperatureProperty();
		(void)VibranceProperty();


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

	DependencyProperty LiquidGlassBrush::ShapeRoundnessProperty()
	{
		static auto const property = RegisterDoubleProperty(L"ShapeRoundness", 4.0);
		return property;
	}

	double LiquidGlassBrush::ShapeRoundness() const { return GetDoubleProperty(ShapeRoundnessProperty()); }
	void LiquidGlassBrush::ShapeRoundness(double value) { SetDoubleProperty(ShapeRoundnessProperty(), value, 2.0, 8.0, L"ShapeRoundness must be between 2 and 8."); }

	DependencyProperty LiquidGlassBrush::RasterizationScaleProperty()
	{
		static auto const property = RegisterDoubleProperty(L"RasterizationScale", 1.0);
		return property;
	}

	double LiquidGlassBrush::RasterizationScale() const { return GetDoubleProperty(RasterizationScaleProperty()); }
	void LiquidGlassBrush::RasterizationScale(double value) { SetDoubleProperty(RasterizationScaleProperty(), value, 0.01, 16.0, L"RasterizationScale must be between 0.01 and 16."); }

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

	DependencyProperty LiquidGlassBrush::DispersionRangeProperty()
	{
		static auto const property = RegisterDoubleProperty(L"DispersionRange", 1.0);
		return property;
	}
	double LiquidGlassBrush::DispersionRange() const { return GetDoubleProperty(DispersionRangeProperty()); }
	void LiquidGlassBrush::DispersionRange(double value) { SetDoubleProperty(DispersionRangeProperty(), value, 0.0, 1.0, L"DispersionRange is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::RefFresnelRangeProperty()
	{
		static auto const property = RegisterDoubleProperty(L"RefFresnelRange", 30.0);
		return property;
	}
	double LiquidGlassBrush::RefFresnelRange() const { return GetDoubleProperty(RefFresnelRangeProperty()); }
	void LiquidGlassBrush::RefFresnelRange(double value) { SetDoubleProperty(RefFresnelRangeProperty(), value, 0.0, 100.0, L"RefFresnelRange is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::RefFresnelHardnessProperty()
	{
		static auto const property = RegisterDoubleProperty(L"RefFresnelHardness", 20.0);
		return property;
	}
	double LiquidGlassBrush::RefFresnelHardness() const { return GetDoubleProperty(RefFresnelHardnessProperty()); }
	void LiquidGlassBrush::RefFresnelHardness(double value) { SetDoubleProperty(RefFresnelHardnessProperty(), value, 0.0, 100.0, L"RefFresnelHardness is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::RefFresnelFactorProperty()
	{
		static auto const property = RegisterDoubleProperty(L"RefFresnelFactor", 0.0);
		return property;
	}
	double LiquidGlassBrush::RefFresnelFactor() const { return GetDoubleProperty(RefFresnelFactorProperty()); }
	void LiquidGlassBrush::RefFresnelFactor(double value) { SetDoubleProperty(RefFresnelFactorProperty(), value, 0.0, 100.0, L"RefFresnelFactor is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::GlareRangeProperty()
	{
		static auto const property = RegisterDoubleProperty(L"GlareRange", 30.0);
		return property;
	}
	double LiquidGlassBrush::GlareRange() const { return GetDoubleProperty(GlareRangeProperty()); }
	void LiquidGlassBrush::GlareRange(double value) { SetDoubleProperty(GlareRangeProperty(), value, 0.0, 100.0, L"GlareRange is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::GlareHardnessProperty()
	{
		static auto const property = RegisterDoubleProperty(L"GlareHardness", 20.0);
		return property;
	}
	double LiquidGlassBrush::GlareHardness() const { return GetDoubleProperty(GlareHardnessProperty()); }
	void LiquidGlassBrush::GlareHardness(double value) { SetDoubleProperty(GlareHardnessProperty(), value, 0.0, 100.0, L"GlareHardness is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::GlareFactorProperty()
	{
		static auto const property = RegisterDoubleProperty(L"GlareFactor", 0.0);
		return property;
	}
	double LiquidGlassBrush::GlareFactor() const { return GetDoubleProperty(GlareFactorProperty()); }
	void LiquidGlassBrush::GlareFactor(double value) { SetDoubleProperty(GlareFactorProperty(), value, 0.0, 120.0, L"GlareFactor is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::GlareConvergenceProperty()
	{
		static auto const property = RegisterDoubleProperty(L"GlareConvergence", 50.0);
		return property;
	}
	double LiquidGlassBrush::GlareConvergence() const { return GetDoubleProperty(GlareConvergenceProperty()); }
	void LiquidGlassBrush::GlareConvergence(double value) { SetDoubleProperty(GlareConvergenceProperty(), value, 0.0, 100.0, L"GlareConvergence is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::GlareOppositeFactorProperty()
	{
		static auto const property = RegisterDoubleProperty(L"GlareOppositeFactor", 80.0);
		return property;
	}
	double LiquidGlassBrush::GlareOppositeFactor() const { return GetDoubleProperty(GlareOppositeFactorProperty()); }
	void LiquidGlassBrush::GlareOppositeFactor(double value) { SetDoubleProperty(GlareOppositeFactorProperty(), value, 0.0, 100.0, L"GlareOppositeFactor is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::GlareAngleProperty()
	{
		static auto const property = RegisterDoubleProperty(L"GlareAngle", -45.0);
		return property;
	}
	double LiquidGlassBrush::GlareAngle() const { return GetDoubleProperty(GlareAngleProperty()); }
	void LiquidGlassBrush::GlareAngle(double value) { SetDoubleProperty(GlareAngleProperty(), value, -180.0, 180.0, L"GlareAngle is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::ShapeRadiusProperty()
	{
		static auto const property = RegisterDoubleProperty(L"ShapeRadius", -1.0);
		return property;
	}
	double LiquidGlassBrush::ShapeRadius() const { return GetDoubleProperty(ShapeRadiusProperty()); }
	void LiquidGlassBrush::ShapeRadius(double value) { SetDoubleProperty(ShapeRadiusProperty(), value, -1.0, 1.0, L"ShapeRadius is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::MagnificationProperty()
	{
		static auto const property = RegisterDoubleProperty(L"Magnification", 1.0);
		return property;
	}
	double LiquidGlassBrush::Magnification() const { return GetDoubleProperty(MagnificationProperty()); }
	void LiquidGlassBrush::Magnification(double value) { SetDoubleProperty(MagnificationProperty(), value, 1.0, 3.0, L"Magnification is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::BloomAmountProperty()
	{
		static auto const property = RegisterDoubleProperty(L"BloomAmount", 1.0);
		return property;
	}
	double LiquidGlassBrush::BloomAmount() const { return GetDoubleProperty(BloomAmountProperty()); }
	void LiquidGlassBrush::BloomAmount(double value) { SetDoubleProperty(BloomAmountProperty(), value, 0.0, 1.0, L"BloomAmount is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::BrightnessProperty()
	{
		static auto const property = RegisterDoubleProperty(L"Brightness", 0.0);
		return property;
	}
	double LiquidGlassBrush::Brightness() const { return GetDoubleProperty(BrightnessProperty()); }
	void LiquidGlassBrush::Brightness(double value) { SetDoubleProperty(BrightnessProperty(), value, -1.0, 1.0, L"Brightness is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::TemperatureProperty()
	{
		static auto const property = RegisterDoubleProperty(L"Temperature", 0.0);
		return property;
	}
	double LiquidGlassBrush::Temperature() const { return GetDoubleProperty(TemperatureProperty()); }
	void LiquidGlassBrush::Temperature(double value) { SetDoubleProperty(TemperatureProperty(), value, -1.0, 1.0, L"Temperature is outside its supported range."); }

	DependencyProperty LiquidGlassBrush::VibranceProperty()
	{
		static auto const property = RegisterDoubleProperty(L"Vibrance", 0.0);
		return property;
	}
	double LiquidGlassBrush::Vibrance() const { return GetDoubleProperty(VibranceProperty()); }
	void LiquidGlassBrush::Vibrance(double value) { SetDoubleProperty(VibranceProperty(), value, 0.0, 1.0, L"Vibrance is outside its supported range."); }

	void LiquidGlassBrush::OnConnected() { m_lifecycle.Connect(*this); }
	void LiquidGlassBrush::OnDisconnected() { m_lifecycle.Disconnect(*this); }
	void LiquidGlassBrush::Update() { m_lifecycle.Update(*this); }

	Microsoft::UI::Composition::CompositionBrush LiquidGlassBrush::BuildPipeline(Microsoft::UI::Composition::Compositor const& compositor)
	{
		if (!m_material)
		{
			m_material = Hlsl::LiquidGlassMaterial(compositor);
		}

		m_material.RasterizationScale(static_cast<float>(RasterizationScale()));
		m_material.BlurRadius(static_cast<float>(BlurRadius()));
		m_material.RefractionStrength(static_cast<float>(RefractionStrength()));
		m_material.DispersionStrength(static_cast<float>(DispersionStrength()));
		m_material.CornerRadius(static_cast<float>(CornerRadius()));
		m_material.ShapeRoundness(static_cast<float>(ShapeRoundness()));
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
		m_material.DispersionRange(static_cast<float>(DispersionRange()));
		m_material.RefFresnelRange(static_cast<float>(RefFresnelRange()));
		m_material.RefFresnelHardness(static_cast<float>(RefFresnelHardness()));
		m_material.RefFresnelFactor(static_cast<float>(RefFresnelFactor()));
		m_material.GlareRange(static_cast<float>(GlareRange()));
		m_material.GlareHardness(static_cast<float>(GlareHardness()));
		m_material.GlareFactor(static_cast<float>(GlareFactor()));
		m_material.GlareConvergence(static_cast<float>(GlareConvergence()));
		m_material.GlareOppositeFactor(static_cast<float>(GlareOppositeFactor()));
		m_material.GlareAngle(static_cast<float>(GlareAngle()));
		m_material.ShapeRadius(static_cast<float>(ShapeRadius()));
		m_material.Magnification(static_cast<float>(Magnification()));
		m_material.BloomAmount(static_cast<float>(BloomAmount()));
		m_material.Brightness(static_cast<float>(Brightness()));
		m_material.Temperature(static_cast<float>(Temperature()));
		m_material.Vibrance(static_cast<float>(Vibrance()));
		return m_material.EffectBrush().Brush();
	}
}

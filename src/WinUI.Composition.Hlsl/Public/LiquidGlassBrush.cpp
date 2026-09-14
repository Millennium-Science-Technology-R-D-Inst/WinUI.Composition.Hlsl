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

#define LIQUID_GLASS_WIDEN_IMPL(Value) L##Value
#define LIQUID_GLASS_WIDEN(Value) LIQUID_GLASS_WIDEN_IMPL(Value)
#define LIQUID_GLASS_DOUBLE_DP(Name, DefaultValue, Minimum, Maximum, Message) \
	DependencyProperty LiquidGlassBrush::Name##Property() \
	{ \
		static auto property = DependencyProperty::Register(LIQUID_GLASS_WIDEN(#Name), winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(DefaultValue), PropertyChangedCallback{ Changed })); \
		return property; \
	} \
	double LiquidGlassBrush::Name() const { return unbox_value<double>(GetValue(Name##Property())); } \
	void LiquidGlassBrush::Name(double value) { ValidateRange(value, Minimum, Maximum, Message); SetValue(Name##Property(), box_value(value)); }

	LIQUID_GLASS_DOUBLE_DP(BlurRadius, 12.0, 0.0, 64.0, L"BlurRadius must be between 0 and 64 DIPs.")
	LIQUID_GLASS_DOUBLE_DP(RefractionStrength, 24.0, 0.0, 128.0, L"RefractionStrength must be between 0 and 128.")
	LIQUID_GLASS_DOUBLE_DP(DispersionStrength, 1.2, 0.0, 16.0, L"DispersionStrength must be between 0 and 16.")
	LIQUID_GLASS_DOUBLE_DP(CornerRadius, 36.0, 0.0, 512.0, L"CornerRadius must be between 0 and 512 DIPs.")
	LIQUID_GLASS_DOUBLE_DP(BorderThickness, 1.5, 0.0, 32.0, L"BorderThickness must be between 0 and 32 DIPs.")
	LIQUID_GLASS_DOUBLE_DP(HighlightStrength, 0.85, 0.0, 4.0, L"HighlightStrength must be between 0 and 4.")
	LIQUID_GLASS_DOUBLE_DP(BezelWidth, 32.0, 1.0, 256.0, L"BezelWidth must be between 1 and 256 DIPs.")
	LIQUID_GLASS_DOUBLE_DP(GlassThickness, 50.0, 0.0, 256.0, L"GlassThickness must be between 0 and 256 DIPs.")
	LIQUID_GLASS_DOUBLE_DP(RefractiveIndex, 1.5, 1.0, 3.5, L"RefractiveIndex must be between 1 and 3.5.")
	LIQUID_GLASS_DOUBLE_DP(TintOpacity, 0.08, 0.0, 1.0, L"TintOpacity must be between 0 and 1.")
	LIQUID_GLASS_DOUBLE_DP(Saturation, 1.25, 0.0, 4.0, L"Saturation must be between 0 and 4.")
	LIQUID_GLASS_DOUBLE_DP(LightAngle, -0.95, -6.2831855, 6.2831855, L"LightAngle must be between -2pi and 2pi radians.")
	LIQUID_GLASS_DOUBLE_DP(MagnificationStrength, 0.0, 0.0, 128.0, L"MagnificationStrength must be between 0 and 128 pixels.")
	LIQUID_GLASS_DOUBLE_DP(HighlightSharpness, 1.5, 0.25, 64.0, L"HighlightSharpness must be between 0.25 and 64.")
	LIQUID_GLASS_DOUBLE_DP(TintRed, 1.0, 0.0, 1.0, L"TintRed must be between 0 and 1.")
	LIQUID_GLASS_DOUBLE_DP(TintGreen, 1.0, 0.0, 1.0, L"TintGreen must be between 0 and 1.")
	LIQUID_GLASS_DOUBLE_DP(TintBlue, 1.0, 0.0, 1.0, L"TintBlue must be between 0 and 1.")
	LIQUID_GLASS_DOUBLE_DP(InnerShadowStrength, 0.09, 0.0, 1.0, L"InnerShadowStrength must be between 0 and 1.")
	LIQUID_GLASS_DOUBLE_DP(SpecularSaturation, 4.0, 0.0, 50.0, L"SpecularSaturation must be between 0 and 50.")
	LIQUID_GLASS_DOUBLE_DP(SpecularWidth, 1.0, 0.25, 32.0, L"SpecularWidth must be between 0.25 and 32 DIPs.")

#undef LIQUID_GLASS_DOUBLE_DP
#undef LIQUID_GLASS_WIDEN
#undef LIQUID_GLASS_WIDEN_IMPL

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
		m_material.SpecularSaturation(static_cast<float>(SpecularSaturation()));
		m_material.SpecularWidth(static_cast<float>(SpecularWidth()));
		return m_material.EffectBrush().Brush();
	}
}

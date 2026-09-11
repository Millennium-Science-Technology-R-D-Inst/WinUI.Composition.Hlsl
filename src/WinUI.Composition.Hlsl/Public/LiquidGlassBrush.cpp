#include "LiquidGlassBrush.h"
#include "LiquidGlassBrush.g.cpp"

import winrt.Windows.UI.Xaml.Interop;

import WinUI.Composition.Hlsl.Validation;

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	using namespace Microsoft::UI::Xaml;
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
		static auto property = DependencyProperty::Register(L"BlurRadius", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(12.0), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::BlurRadius() const
	{
		return unbox_value<double>(GetValue(BlurRadiusProperty()));
	}
	void LiquidGlassBrush::BlurRadius(double value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be between 0 and 256.");
		SetValue(BlurRadiusProperty(), box_value(value));
	}
	DependencyProperty LiquidGlassBrush::RefractionStrengthProperty()
	{
		static auto property = DependencyProperty::Register(L"RefractionStrength", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(24.0), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::RefractionStrength() const
	{
		return unbox_value<double>(GetValue(RefractionStrengthProperty()));
	}
	void LiquidGlassBrush::RefractionStrength(double value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be between 0 and 256.");
		SetValue(RefractionStrengthProperty(), box_value(value));
	}
	DependencyProperty LiquidGlassBrush::DispersionStrengthProperty()
	{
		static auto property = DependencyProperty::Register(L"DispersionStrength", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(1.2), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::DispersionStrength() const
	{
		return unbox_value<double>(GetValue(DispersionStrengthProperty()));
	}
	void LiquidGlassBrush::DispersionStrength(double value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be between 0 and 256.");
		SetValue(DispersionStrengthProperty(), box_value(value));
	}
	DependencyProperty LiquidGlassBrush::CornerRadiusProperty()
	{
		static auto property = DependencyProperty::Register(L"CornerRadius", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(12.0), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::CornerRadius() const
	{
		return unbox_value<double>(GetValue(CornerRadiusProperty()));
	}
	void LiquidGlassBrush::CornerRadius(double value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be between 0 and 256.");
		SetValue(CornerRadiusProperty(), box_value(value));
	}
	DependencyProperty LiquidGlassBrush::BorderThicknessProperty()
	{
		static auto property = DependencyProperty::Register(L"BorderThickness", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(1.0), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::BorderThickness() const
	{
		return unbox_value<double>(GetValue(BorderThicknessProperty()));
	}
	void LiquidGlassBrush::BorderThickness(double value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be between 0 and 256.");
		SetValue(BorderThicknessProperty(), box_value(value));
	}
	DependencyProperty LiquidGlassBrush::HighlightStrengthProperty()
	{
		static auto property = DependencyProperty::Register(L"HighlightStrength", winrt::xaml_typename<double>(), winrt::xaml_typename<class_type>(), PropertyMetadata(box_value(0.8), PropertyChangedCallback{ Changed }));
		return property;
	}
	double LiquidGlassBrush::HighlightStrength() const
	{
		return unbox_value<double>(GetValue(HighlightStrengthProperty()));
	}
	void LiquidGlassBrush::HighlightStrength(double value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be between 0 and 256.");
		SetValue(HighlightStrengthProperty(), box_value(value));
	}
	void LiquidGlassBrush::OnConnected()
	{
		m_lifecycle.Connect(*this);
	}
	void LiquidGlassBrush::OnDisconnected()
	{
		m_lifecycle.Disconnect(*this);
	}
	void LiquidGlassBrush::Update()
	{
		m_lifecycle.Update(*this);
	}
	Microsoft::UI::Composition::CompositionBrush LiquidGlassBrush::BuildPipeline(Microsoft::UI::Composition::Compositor const& compositor)
	{
		if (!m_material)m_material = Hlsl::LiquidGlassMaterial(compositor);
		m_material.BlurRadius(static_cast<float>(BlurRadius()));
		m_material.RefractionStrength(static_cast<float>(RefractionStrength()));
		m_material.DispersionStrength(static_cast<float>(DispersionStrength()));
		m_material.CornerRadius(static_cast<float>(CornerRadius()));
		m_material.BorderThickness(static_cast<float>(BorderThickness()));
		m_material.HighlightStrength(static_cast<float>(HighlightStrength()));
		return m_material.EffectBrush().Brush();
	}
}

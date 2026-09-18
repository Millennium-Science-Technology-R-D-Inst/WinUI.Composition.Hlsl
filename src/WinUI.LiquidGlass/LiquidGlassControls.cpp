#include "pch.h"
#include "winrt_module_imports.h"
#include "LiquidGlassControls.h"

#if __has_include("LiquidGlassCard.g.cpp")
#include "LiquidGlassCard.g.cpp"
#include "LiquidGlassMagnifier.g.cpp"
#include "LiquidGlassButton.g.cpp"
#include "LiquidGlassToggleButton.g.cpp"
#include "LiquidGlassHyperlinkButton.g.cpp"
#include "LiquidGlassCheckBox.g.cpp"
#include "LiquidGlassRadioButton.g.cpp"
#include "LiquidGlassSlider.g.cpp"
#include "LiquidGlassTextBox.g.cpp"
#include "LiquidGlassPasswordBox.g.cpp"
#include "LiquidGlassComboBox.g.cpp"
#include "LiquidGlassToggleSwitch.g.cpp"
#endif

namespace winrt::WinUI::LiquidGlass::implementation
{
	namespace
	{
		namespace Xaml = Microsoft::UI::Xaml;
		namespace Controls = Xaml::Controls;
		namespace Media = Xaml::Media;

		using Brush = WinUI::Composition::Hlsl::LiquidGlassBrush;
		using Preset = WinUI::LiquidGlass::LiquidGlassPreset;

		Media::Brush AsBrush(Brush const& value)
		{
			return value ? value.as<Media::Brush>() : Media::Brush{ nullptr };
		}

		Brush CreateBrush(Preset preset)
		{
			return WinUI::LiquidGlass::LiquidGlassPresets::CreateBrush(preset);
		}
	}

	LiquidGlassCard::LiquidGlassCard()
	{
		DefaultStyleKey(box_value(xaml_typename<class_type>()));
		GlassBrush(CreateBrush(Preset::Panel));
	}

	LiquidGlassButton::LiquidGlassButton()
	{
		DefaultStyleKey(box_value(xaml_typename<class_type>()));
		GlassBrush(CreateBrush(Preset::Button));
	}

	LiquidGlassToggleButton::LiquidGlassToggleButton()
	{
		DefaultStyleKey(box_value(xaml_typename<class_type>()));
		GlassBrush(CreateBrush(Preset::Button));
	}

	LiquidGlassHyperlinkButton::LiquidGlassHyperlinkButton()
	{
		DefaultStyleKey(box_value(xaml_typename<class_type>()));
		GlassBrush(CreateBrush(Preset::Button));
	}

	LiquidGlassCheckBox::LiquidGlassCheckBox()
	{
		DefaultStyleKey(box_value(xaml_typename<class_type>()));
		auto brush = CreateBrush(Preset::Choice);
		brush.CornerRadius(5.0);
		GlassBrush(brush);
	}

	LiquidGlassRadioButton::LiquidGlassRadioButton()
	{
		DefaultStyleKey(box_value(xaml_typename<class_type>()));
		GlassBrush(CreateBrush(Preset::Choice));
	}

	LiquidGlassComboBox::LiquidGlassComboBox()
	{
		DefaultStyleKey(box_value(xaml_typename<class_type>()));
		GlassBrush(CreateBrush(Preset::Choice));
		SetValue(LiquidGlassInteraction::FocusedScaleProperty(), box_value(1.01));
		SetValue(LiquidGlassInteraction::FocusedTintBoostProperty(), box_value(.04));
	}

	LiquidGlassTextBox::LiquidGlassTextBox()
	{
		DefaultStyleKey(box_value(xaml_typename<class_type>()));
		GlassBrush(CreateBrush(Preset::Input));
		SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.98));
		SetValue(LiquidGlassInteraction::FocusedScaleProperty(), box_value(1.0));
		SetValue(LiquidGlassInteraction::FocusedTintBoostProperty(), box_value(.15));
		SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(140.0));
	}

	void LiquidGlassPasswordBox::ApplyGlassBrush(Brush const& value)
	{
		m_glassBrush = value;
		if (m_passwordBox) m_passwordBox.Background(AsBrush(value));
	}

	LiquidGlassPasswordBox::LiquidGlassPasswordBox()
	{
		m_passwordBox = Controls::PasswordBox{};
		m_passwordBox.HorizontalAlignment(Xaml::HorizontalAlignment::Stretch);
		HorizontalContentAlignment(Xaml::HorizontalAlignment::Stretch);
		Content(m_passwordBox);
		GlassBrush(CreateBrush(Preset::Input));
		SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.99));
		SetValue(LiquidGlassInteraction::FocusedScaleProperty(), box_value(1.0));
		SetValue(LiquidGlassInteraction::FocusedTintBoostProperty(), box_value(.04));
		SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(140.0));
	}

	hstring LiquidGlassPasswordBox::PlaceholderText() const
	{
		return m_passwordBox ? m_passwordBox.PlaceholderText() : hstring{};
	}
	void LiquidGlassPasswordBox::PlaceholderText(hstring const& value)
	{
		if (m_passwordBox) m_passwordBox.PlaceholderText(value);
	}
	hstring LiquidGlassPasswordBox::Password() const
	{
		return m_passwordBox ? m_passwordBox.Password() : hstring{};
	}
	void LiquidGlassPasswordBox::Password(hstring const& value)
	{
		if (m_passwordBox) m_passwordBox.Password(value);
	}

	LiquidGlassMagnifier::LiquidGlassMagnifier()
	{
		DefaultStyleKey(box_value(xaml_typename<class_type>()));
		GlassBrush(CreateBrush(Preset::Magnifier));
		SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.8));
		SetValue(LiquidGlassInteraction::PressedScaleProperty(), box_value(1.0));
		SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(105.0));
		SetValue(LiquidGlassInteraction::OpticsTransitionDurationProperty(), box_value(180.0));
		SetValue(LiquidGlassInteraction::ElasticityProperty(), box_value(.13));
		SetValue(LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(1.25));
		SetValue(LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(0.0));
		SetValue(LiquidGlassInteraction::PressedTintBoostProperty(), box_value(0.0));
		SetValue(LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.08));
		SetValue(LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(.02));
		SetValue(LiquidGlassInteraction::PressedInnerShadowBoostProperty(), box_value(.07));
		SetValue(LiquidGlassInteraction::ActiveMagnificationMultiplierProperty(), box_value(2.0));
	}

	void LiquidGlassSlider::ApplyGlassBrush(Brush const& value)
	{
		m_glassBrush = value;
		detail::KubeSliderVisualModel<LiquidGlassSlider>::RefreshVisual();
	}

	LiquidGlassSlider::LiquidGlassSlider()
	{
		DefaultStyleKey(box_value(xaml_typename<class_type>()));
		GlassBrush(CreateBrush(Preset::SliderThumb));
		SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.6));
		SetValue(LiquidGlassInteraction::PressedScaleProperty(), box_value(1.0));
		SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(60.0));
		SetValue(LiquidGlassInteraction::OpticsTransitionDurationProperty(), box_value(55.0));
		SetValue(LiquidGlassInteraction::PointerOverRefractionMultiplierProperty(), box_value(1.12));
		SetValue(LiquidGlassInteraction::PointerOverHighlightMultiplierProperty(), box_value(1.15));
		SetValue(LiquidGlassInteraction::PointerOverTintBoostProperty(), box_value(0.0));
		// Kube's slider drives the refraction scale from 0.4 at rest to 0.9 while active.
		// Preserve that 2.25x relation instead of the older, much weaker 1.45x response.
		SetValue(LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(2.25));
		SetValue(LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(0.0));
		// Kube's displacement map keeps strong colored/specular edges while the white
		// body fades away. Our HLSL has an explicit RGB dispersion control, so raise it
		// only for the active lens rather than baking excessive chroma into the rest state.
		SetValue(LiquidGlassInteraction::PressedDispersionMultiplierProperty(), box_value(2.6));
		// Kube fades the white body from 1.0 to 0.1 while active.
		SetValue(LiquidGlassInteraction::PressedTintBoostProperty(), box_value(-.90));
		SetValue(LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.25));
		SetValue(LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(.03));
		SetValue(LiquidGlassInteraction::PressedInnerShadowBoostProperty(), box_value(.03));
	}

	LiquidGlassToggleSwitch::LiquidGlassToggleSwitch()
	{
		DefaultStyleKey(box_value(xaml_typename<class_type>()));
		GlassBrush(CreateBrush(Preset::ToggleSwitchKnob));
		SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.65));
		SetValue(LiquidGlassInteraction::PressedScaleProperty(), box_value(.9));
		SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(75.0));
		SetValue(LiquidGlassInteraction::OpticsTransitionDurationProperty(), box_value(60.0));
		SetValue(LiquidGlassInteraction::PointerOverRefractionMultiplierProperty(), box_value(1.10));
		SetValue(LiquidGlassInteraction::PointerOverHighlightMultiplierProperty(), box_value(1.14));
		SetValue(LiquidGlassInteraction::PointerOverTintBoostProperty(), box_value(0.0));
		// Kube drives the optical scale from 0.4 at rest to 0.9 while active.
		SetValue(LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(2.25));
		SetValue(LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(0.0));
		SetValue(LiquidGlassInteraction::PressedDispersionMultiplierProperty(), box_value(1.45));
		// The authored knob body is .78 tint; pressed should reveal the backdrop at ~.10.
		SetValue(LiquidGlassInteraction::PressedTintBoostProperty(), box_value(-.68));
		SetValue(LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.25));
		SetValue(LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(.03));
		SetValue(LiquidGlassInteraction::PressedInnerShadowBoostProperty(), box_value(.05));
	}

	Windows::Foundation::IInspectable LiquidGlassToggleSwitch::Header() const
	{
		return m_header;
	}
	void LiquidGlassToggleSwitch::Header(Windows::Foundation::IInspectable const& value)
	{
		m_header = value; Content(value);
	}
	bool LiquidGlassToggleSwitch::IsOn() const
	{
		auto v = IsChecked(); return v && v.Value();
	}
	void LiquidGlassToggleSwitch::IsOn(bool value)
	{
		IsChecked(box_value(value).as<Windows::Foundation::IReference<bool>>());
	}
}

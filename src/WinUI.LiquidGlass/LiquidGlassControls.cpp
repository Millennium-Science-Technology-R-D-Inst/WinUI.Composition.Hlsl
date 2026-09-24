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

		struct SyncFlagGuard
		{
			explicit SyncFlagGuard(bool& value) : m_value(value) { m_value = true; }
			~SyncFlagGuard() { m_value = false; }

			SyncFlagGuard(SyncFlagGuard const&) = delete;
			SyncFlagGuard& operator=(SyncFlagGuard const&) = delete;

		private:
			bool& m_value;
		};

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

	Xaml::PropertyChangedCallback LiquidGlassPasswordBox::ForwardedPropertyChangedCallback()
	{
		return Xaml::PropertyChangedCallback{
			[](Xaml::DependencyObject const& object, Xaml::DependencyPropertyChangedEventArgs const& args)
			{
				auto self = detail::EnsureDependencyProperty<LiquidGlassPasswordBox>::GetSelf(object);
				self->OnForwardedPropertyChanged(args);
			}
		};
	}

	template<typename T>
	Xaml::DependencyProperty LiquidGlassPasswordBox::RegisterForwardedProperty(
		wchar_t const* name,
		Windows::Foundation::IInspectable const& defaultValue)
	{
		return Xaml::DependencyProperty::Register(
			name,
			xaml_typename<T>(),
			xaml_typename<class_type>(),
			Xaml::PropertyMetadata{ defaultValue, ForwardedPropertyChangedCallback() });
	}

	void LiquidGlassPasswordBox::EnsureDependencyProperties()
	{
		(void)GlassBrushProperty();
		(void)PlaceholderTextProperty();
		(void)PasswordProperty();
	}

	Xaml::DependencyProperty LiquidGlassPasswordBox::PlaceholderTextProperty()
	{
		static auto const property = RegisterForwardedProperty<hstring>(L"PlaceholderText", box_value(hstring{}));
		return property;
	}

	Xaml::DependencyProperty LiquidGlassPasswordBox::PasswordProperty()
	{
		static auto const property = RegisterForwardedProperty<hstring>(L"Password", box_value(hstring{}));
		return property;
	}

	Xaml::DependencyProperty LiquidGlassPasswordBox::InnerPropertyFor(Xaml::DependencyProperty const& outerProperty)
	{
		if (outerProperty == PlaceholderTextProperty()) return Controls::PasswordBox::PlaceholderTextProperty();
		if (outerProperty == PasswordProperty()) return Controls::PasswordBox::PasswordProperty();
		return nullptr;
	}

	void LiquidGlassPasswordBox::OnForwardedPropertyChanged(Xaml::DependencyPropertyChangedEventArgs const& args)
	{
		ForwardPropertyToInner(args.Property(), args.NewValue());
	}

	void LiquidGlassPasswordBox::ForwardPropertyToInner(
		Xaml::DependencyProperty const& outerProperty,
		Windows::Foundation::IInspectable const& value)
	{
		if (!m_passwordBox || m_syncingFromInner) return;

		auto const innerProperty = InnerPropertyFor(outerProperty);
		if (!innerProperty) return;

		SyncFlagGuard guard{ m_syncingToInner };
		m_passwordBox.SetValue(innerProperty, value);
	}

	void LiquidGlassPasswordBox::MirrorPropertyFromInner(
		Xaml::DependencyProperty const& outerProperty,
		Windows::Foundation::IInspectable const& value)
	{
		if (m_syncingToInner) return;

		SyncFlagGuard guard{ m_syncingFromInner };
		SetValue(outerProperty, value);
	}

	void LiquidGlassPasswordBox::AttachInnerPropertyMirrors()
	{
		auto weak = get_weak();
		auto mirror = [this, weak](Xaml::DependencyProperty const& outerProperty)
		{
			auto const innerProperty = InnerPropertyFor(outerProperty);
			if (!innerProperty) return;

			(void)m_passwordBox.RegisterPropertyChangedCallback(
				innerProperty,
				[weak, outerProperty](
					Xaml::DependencyObject const& sender,
					Xaml::DependencyProperty const& changedProperty)
				{
					if (auto self = weak.get())
					{
						self->MirrorPropertyFromInner(outerProperty, sender.GetValue(changedProperty));
					}
				});
		};

		mirror(PlaceholderTextProperty());
		mirror(PasswordProperty());
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
		m_passwordBox.VerticalAlignment(Xaml::VerticalAlignment::Stretch);
		AttachInnerPropertyMirrors();

		HorizontalContentAlignment(Xaml::HorizontalAlignment::Stretch);
		VerticalContentAlignment(Xaml::VerticalAlignment::Stretch);
		IsTabStop(false);
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
		SetValue(PlaceholderTextProperty(), box_value(value));
	}

	hstring LiquidGlassPasswordBox::Password() const
	{
		return m_passwordBox ? m_passwordBox.Password() : hstring{};
	}

	void LiquidGlassPasswordBox::Password(hstring const& value)
	{
		SetValue(PasswordProperty(), box_value(value));
	}

	Controls::PasswordBox LiquidGlassPasswordBox::InnerPasswordBox() const
	{
		return m_passwordBox;
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
		// Kube Slider.tsx drives scaleRatio from .4 to .9. SliderThumb's resting
		// RefractionStrength=9.6 encodes .4, so the active multiplier is exactly .9/.4.
		// PointerField refraction is disabled for this control; this is the sole bend.
		SetValue(LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(2.25));
		SetValue(LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(0.0));
		// Kube changes displacement strength, not a separate chromatic-spread control.
		// Keep our RGB extension stable while the authored displacement grows.
		SetValue(LiquidGlassInteraction::PressedDispersionMultiplierProperty(), box_value(1.0));
		// Kube fades the white body from 1.0 to 0.1 while active.
		SetValue(LiquidGlassInteraction::PressedTintBoostProperty(), box_value(-.90));
		// Kube keeps specularOpacity at .4 in both rest and active states. Keep the
		// fixed-angle rim stable; the spatial PointerField owns the local pointer boost.
		SetValue(LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.0));
		SetValue(LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(0.0));
		// Kube Slider has no pressed inset shadow; the shader specular and ThemeShadow
		// already provide the edge/outer depth without a second dark capsule.
		SetValue(LiquidGlassInteraction::PressedInnerShadowBoostProperty(), box_value(0.0));
	}

	void LiquidGlassToggleSwitch::EnsureDependencyProperties()
	{
		(void)GlassBrushProperty();
		(void)HeaderProperty();
		(void)IsOnProperty();
	}

	Xaml::DependencyProperty LiquidGlassToggleSwitch::HeaderProperty()
	{
		static auto const property = Xaml::DependencyProperty::Register(
			L"Header",
			xaml_typename<Windows::Foundation::IInspectable>(),
			xaml_typename<class_type>(),
			Xaml::PropertyMetadata{
				Windows::Foundation::IInspectable{ nullptr },
				Xaml::PropertyChangedCallback{
					[](Xaml::DependencyObject const& object, Xaml::DependencyPropertyChangedEventArgs const& args)
					{
						auto self = detail::EnsureDependencyProperty<LiquidGlassToggleSwitch>::GetSelf(object);
						self->Content(args.NewValue());
					}
				}
			});
		return property;
	}

	Xaml::DependencyProperty LiquidGlassToggleSwitch::IsOnProperty()
	{
		static auto const property = Xaml::DependencyProperty::Register(
			L"IsOn",
			xaml_typename<bool>(),
			xaml_typename<class_type>(),
			Xaml::PropertyMetadata{
				box_value(false),
				Xaml::PropertyChangedCallback{
					[](Xaml::DependencyObject const& object, Xaml::DependencyPropertyChangedEventArgs const& args)
					{
						auto self = detail::EnsureDependencyProperty<LiquidGlassToggleSwitch>::GetSelf(object);
						self->OnIsOnPropertyChanged(unbox_value<bool>(args.NewValue()));
					}
				}
			});
		return property;
	}

	LiquidGlassToggleSwitch::LiquidGlassToggleSwitch()
	{
		DefaultStyleKey(box_value(xaml_typename<class_type>()));

		auto weak = get_weak();
		(void)RegisterPropertyChangedCallback(
			Controls::Primitives::ToggleButton::IsCheckedProperty(),
			[weak](Xaml::DependencyObject const&, Xaml::DependencyProperty const&)
			{
				if (auto self = weak.get())
				{
					self->MirrorIsCheckedToIsOn();
				}
			});

		GlassBrush(CreateBrush(Preset::ToggleSwitchKnob));
		SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.65));
		SetValue(LiquidGlassInteraction::PressedScaleProperty(), box_value(.9));
		SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(75.0));
		SetValue(LiquidGlassInteraction::OpticsTransitionDurationProperty(), box_value(60.0));
		// Kube does not change the material merely because the pointer is hovering.
		SetValue(LiquidGlassInteraction::PointerOverRefractionMultiplierProperty(), box_value(1.0));
		SetValue(LiquidGlassInteraction::PointerOverDispersionMultiplierProperty(), box_value(1.0));
		SetValue(LiquidGlassInteraction::PointerOverSaturationMultiplierProperty(), box_value(1.0));
		SetValue(LiquidGlassInteraction::PointerOverContrastMultiplierProperty(), box_value(1.0));
		SetValue(LiquidGlassInteraction::PointerOverTintBoostProperty(), box_value(0.0));
		SetValue(LiquidGlassInteraction::PointerOverHighlightMultiplierProperty(), box_value(1.0));
		SetValue(LiquidGlassInteraction::PointerOverHighlightBoostProperty(), box_value(0.0));
		SetValue(LiquidGlassInteraction::PointerOverInnerShadowBoostProperty(), box_value(0.0));
		// Kube drives the optical scale from 0.4 at rest to 0.9 while active.
		SetValue(LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(2.25));
		SetValue(LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(0.0));
		SetValue(LiquidGlassInteraction::PressedDispersionMultiplierProperty(), box_value(1.45));
		// Kube fades the white body from 1.0 to 0.1 while active.
		SetValue(LiquidGlassInteraction::PressedTintBoostProperty(), box_value(-.90));
		SetValue(LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.0));
		SetValue(LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(0.0));
		SetValue(LiquidGlassInteraction::PressedInnerShadowBoostProperty(), box_value(.09));
	}

	Windows::Foundation::IInspectable LiquidGlassToggleSwitch::Header() const
	{
		return GetValue(HeaderProperty());
	}

	void LiquidGlassToggleSwitch::Header(Windows::Foundation::IInspectable const& value)
	{
		SetValue(HeaderProperty(), value);
	}

	bool LiquidGlassToggleSwitch::IsOn() const
	{
		return unbox_value_or<bool>(GetValue(IsOnProperty()), false);
	}

	void LiquidGlassToggleSwitch::IsOn(bool value)
	{
		SetValue(IsOnProperty(), box_value(value));
	}

	void LiquidGlassToggleSwitch::OnIsOnPropertyChanged(bool value)
	{
		if (m_syncingIsOn)
		{
			return;
		}

		SyncFlagGuard guard{ m_syncingIsOn };
		IsChecked(box_value(value).as<Windows::Foundation::IReference<bool>>());
	}

	void LiquidGlassToggleSwitch::MirrorIsCheckedToIsOn()
	{
		if (m_syncingIsOn)
		{
			return;
		}

		auto const checked = IsChecked();
		auto const value = checked && checked.Value();
		if (IsOn() == value)
		{
			return;
		}

		SyncFlagGuard guard{ m_syncingIsOn };
		SetValue(IsOnProperty(), box_value(value));
	}

}

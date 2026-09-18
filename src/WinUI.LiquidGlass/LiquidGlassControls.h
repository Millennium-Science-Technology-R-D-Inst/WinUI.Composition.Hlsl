#pragma once

#include <chrono>

#include "winrt_module_imports.h"
#include "LiquidGlassInteraction.h"
#include "include/GlassBrushHelper.hpp"
#include "include/TemplateControlHelper.hpp"
#include "include/PointerLightHelper.hpp"
#include "include/PointerFieldHelper.hpp"
#include "include/PointerMotionHelper.hpp"
#include "include/ChildSurfaceInteraction.hpp"
#include "include/PressOpticsHelper.hpp"
#include "include/FocusOpticsHelper.hpp"
#include "include/ControlInteractionProfiles.hpp"
#include "include/ControlVisualMotion.hpp"
#include "include/ChoiceGlyphMotionHelper.hpp"
#include "include/CompactControlMotionDefaults.hpp"
#include "include/KubeSliderVisualModel.hpp"
#include "include/KubeMagnifierMotionHelper.hpp"
#include "include/KubeToggleSwitchVisualModel.hpp"

#include "LiquidGlassCard.g.h"
#include "LiquidGlassMagnifier.g.h"
#include "LiquidGlassButton.g.h"
#include "LiquidGlassToggleButton.g.h"
#include "LiquidGlassHyperlinkButton.g.h"
#include "LiquidGlassCheckBox.g.h"
#include "LiquidGlassRadioButton.g.h"
#include "LiquidGlassSlider.g.h"
#include "LiquidGlassTextBox.g.h"
#include "LiquidGlassPasswordBox.g.h"
#include "LiquidGlassComboBox.g.h"
#include "LiquidGlassToggleSwitch.g.h"
#include "LiquidGlassTabBarItem.g.h"
#include "LiquidGlassTabBar.g.h"

namespace winrt::WinUI::LiquidGlass::detail
{
	inline constexpr wchar_t ThemeResourceUri[] = L"ms-appx:///WinUI.LiquidGlass/Themes/Generic.xaml";
}

namespace winrt::WinUI::LiquidGlass::implementation
{
	struct LiquidGlassCard :
		LiquidGlassCardT<LiquidGlassCard>,
		detail::GlassBrushHelper<LiquidGlassCard>,
		detail::TemplateControlHelper<LiquidGlassCard>,
		detail::PointerLightHelper<LiquidGlassCard>,
		detail::PointerFieldHelper<LiquidGlassCard>
	{
		constexpr static auto ResourceUri = detail::ThemeResourceUri;
		LiquidGlassCard();
	};

	struct LiquidGlassButton :
		LiquidGlassButtonT<LiquidGlassButton>,
		detail::GlassBrushHelper<LiquidGlassButton>,
		detail::TemplateControlHelper<LiquidGlassButton>,
		detail::PointerLightHelper<LiquidGlassButton>,
		detail::PointerFieldHelper<LiquidGlassButton>,
		detail::PointerMotionHelper<LiquidGlassButton>,
		detail::PressOpticsHelper<LiquidGlassButton, detail::PersistentOpticsKind::None>,
		detail::CompactControlMotionDefaults<LiquidGlassButton, detail::CompactControlMotionProfile::Button>
	{
		constexpr static auto ResourceUri = detail::ThemeResourceUri;
		LiquidGlassButton();
	};

	struct LiquidGlassToggleButton :
		LiquidGlassToggleButtonT<LiquidGlassToggleButton>,
		detail::GlassBrushHelper<LiquidGlassToggleButton>,
		detail::TemplateControlHelper<LiquidGlassToggleButton>,
		detail::PointerLightHelper<LiquidGlassToggleButton>,
		detail::PointerFieldHelper<LiquidGlassToggleButton>,
		detail::PointerMotionHelper<LiquidGlassToggleButton>,
		detail::PressOpticsHelper<LiquidGlassToggleButton, detail::PersistentOpticsKind::Toggle>,
		detail::CompactControlMotionDefaults<LiquidGlassToggleButton, detail::CompactControlMotionProfile::Button>
	{
		constexpr static auto ResourceUri = detail::ThemeResourceUri;
		LiquidGlassToggleButton();
	};

	struct LiquidGlassHyperlinkButton :
		LiquidGlassHyperlinkButtonT<LiquidGlassHyperlinkButton>,
		detail::GlassBrushHelper<LiquidGlassHyperlinkButton>,
		detail::TemplateControlHelper<LiquidGlassHyperlinkButton>,
		detail::PointerLightHelper<LiquidGlassHyperlinkButton>,
		detail::PointerFieldHelper<LiquidGlassHyperlinkButton>,
		detail::PointerMotionHelper<LiquidGlassHyperlinkButton>,
		detail::PressOpticsHelper<LiquidGlassHyperlinkButton, detail::PersistentOpticsKind::None>,
		detail::CompactControlMotionDefaults<LiquidGlassHyperlinkButton, detail::CompactControlMotionProfile::Button>
	{
		constexpr static auto ResourceUri = detail::ThemeResourceUri;
		LiquidGlassHyperlinkButton();
	};

	struct LiquidGlassMagnifier :
		LiquidGlassMagnifierT<LiquidGlassMagnifier>,
		detail::GlassBrushHelper<LiquidGlassMagnifier>,
		detail::TemplateControlHelper<LiquidGlassMagnifier>,
		detail::PointerLightHelper<LiquidGlassMagnifier>,
		detail::PointerFieldHelper<LiquidGlassMagnifier>,
		detail::KubeMagnifierMotionHelper<LiquidGlassMagnifier>,
		detail::MotionDefaults<LiquidGlassMagnifier, detail::MotionProfile::Magnifier>
	{
		constexpr static auto ResourceUri = detail::ThemeResourceUri;
		LiquidGlassMagnifier();
	};

	struct LiquidGlassCheckBox :
		LiquidGlassCheckBoxT<LiquidGlassCheckBox>,
		detail::GlassBrushHelper<LiquidGlassCheckBox>,
		detail::TemplateControlHelper<LiquidGlassCheckBox>,
		detail::PointerLightHelper<LiquidGlassCheckBox>,
		detail::ChoiceGlyphMotionHelper<LiquidGlassCheckBox>,
		detail::PressOpticsHelper<LiquidGlassCheckBox, detail::PersistentOpticsKind::Toggle>,
		detail::CompactControlMotionDefaults<LiquidGlassCheckBox, detail::CompactControlMotionProfile::Choice>
	{
		constexpr static auto ResourceUri = detail::ThemeResourceUri;
		LiquidGlassCheckBox();
	};

	struct LiquidGlassRadioButton :
		LiquidGlassRadioButtonT<LiquidGlassRadioButton>,
		detail::GlassBrushHelper<LiquidGlassRadioButton>,
		detail::TemplateControlHelper<LiquidGlassRadioButton>,
		detail::PointerLightHelper<LiquidGlassRadioButton>,
		detail::ChoiceGlyphMotionHelper<LiquidGlassRadioButton>,
		detail::PressOpticsHelper<LiquidGlassRadioButton, detail::PersistentOpticsKind::Toggle>,
		detail::CompactControlMotionDefaults<LiquidGlassRadioButton, detail::CompactControlMotionProfile::Choice>
	{
		constexpr static auto ResourceUri = detail::ThemeResourceUri;
		LiquidGlassRadioButton();
	};

	struct LiquidGlassSlider :
		LiquidGlassSliderT<LiquidGlassSlider>,
		detail::GlassBrushHelper<LiquidGlassSlider>,
		detail::TemplateControlHelper<LiquidGlassSlider>,
		// Kube's specular map has a fixed light angle. The spatial PointerField already
		// supplies the local pointer reveal; rotating LightAngle at the same time creates
		// a second independent rim that reads as layered glass.
		detail::KubeSliderVisualModel<LiquidGlassSlider>,
		detail::MotionDefaults<LiquidGlassSlider, detail::MotionProfile::Slider>
	{
		constexpr static auto ResourceUri = detail::ThemeResourceUri;
		LiquidGlassSlider();
		void ApplyGlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value);

		void OnApplyTemplate()
		{
			base_type::OnApplyTemplate();
			detail::KubeSliderVisualModel<LiquidGlassSlider>::RefreshVisual();
		}
	};

	struct LiquidGlassTextBox :
		LiquidGlassTextBoxT<LiquidGlassTextBox>,
		detail::GlassBrushHelper<LiquidGlassTextBox>,
		detail::PointerLightHelper<LiquidGlassTextBox>,
		detail::PointerFieldHelper<LiquidGlassTextBox>,
		detail::FocusOpticsHelper<LiquidGlassTextBox>
	{
		LiquidGlassTextBox();
	};

	struct LiquidGlassPasswordBox :
		LiquidGlassPasswordBoxT<LiquidGlassPasswordBox>,
		detail::GlassBrushHelper<LiquidGlassPasswordBox>,
		detail::PointerLightHelper<LiquidGlassPasswordBox>,
		detail::PointerFieldHelper<LiquidGlassPasswordBox>,
		detail::FocusOpticsHelper<LiquidGlassPasswordBox>
	{
		LiquidGlassPasswordBox();
		hstring PlaceholderText() const;
		void PlaceholderText(hstring const& value);
		hstring Password() const;
		void Password(hstring const& value);
		void ApplyGlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value);

	private:
		Microsoft::UI::Xaml::Controls::PasswordBox m_passwordBox{ nullptr };
	};

	struct LiquidGlassComboBox :
		LiquidGlassComboBoxT<LiquidGlassComboBox>,
		detail::GlassBrushHelper<LiquidGlassComboBox>,
		detail::PointerLightHelper<LiquidGlassComboBox>,
		detail::PointerFieldHelper<LiquidGlassComboBox>,
		detail::FocusOpticsHelper<LiquidGlassComboBox>
	{
		LiquidGlassComboBox();
	};

	struct LiquidGlassToggleSwitch :
		LiquidGlassToggleSwitchT<LiquidGlassToggleSwitch>,
		detail::GlassBrushHelper<LiquidGlassToggleSwitch>,
		detail::TemplateControlHelper<LiquidGlassToggleSwitch>,
		// Keep one Kube-style fixed-angle specular layer plus the local PointerField.
		// PointerLightHelper would rotate the global rim underneath that field.
		detail::KubeToggleSwitchVisualModel<LiquidGlassToggleSwitch>,
		// Kube's knob optics are independent of checked state; only pointer-down activates
		// the 0.4 -> 0.9 refraction/body transition. Track color/position represent IsChecked.
		detail::PressOpticsHelper<LiquidGlassToggleSwitch, detail::PersistentOpticsKind::None>,
		detail::MotionDefaults<LiquidGlassToggleSwitch, detail::MotionProfile::Switch>
	{
		constexpr static auto ResourceUri = detail::ThemeResourceUri;
		LiquidGlassToggleSwitch();
		Windows::Foundation::IInspectable Header() const;
		void Header(Windows::Foundation::IInspectable const& value);
		bool IsOn() const;
		void IsOn(bool value);

		void OnApplyTemplate()
		{
			base_type::OnApplyTemplate();
			detail::KubeToggleSwitchVisualModel<LiquidGlassToggleSwitch>::RefreshVisualModel();
		}

		void OnToggle()
		{
			if (!detail::KubeToggleSwitchVisualModel<LiquidGlassToggleSwitch>::TryHandleToggle())
				base_type::OnToggle();
		}

	private:
		Windows::Foundation::IInspectable m_header{ nullptr };
	};

	struct LiquidGlassTabBarItem :
		LiquidGlassTabBarItemT<LiquidGlassTabBarItem>,
		detail::GlassBrushHelper<LiquidGlassTabBarItem>,
		detail::TemplateControlHelper<LiquidGlassTabBarItem, false>,
		detail::PointerLightHelper<LiquidGlassTabBarItem>,
		detail::PointerFieldHelper<LiquidGlassTabBarItem>,
		detail::PointerMotionHelper<LiquidGlassTabBarItem>,
		detail::PressOpticsHelper<LiquidGlassTabBarItem, detail::PersistentOpticsKind::Selector>
	{
		LiquidGlassTabBarItem();
	};

	struct LiquidGlassTabBar :
		LiquidGlassTabBarT<LiquidGlassTabBar>,
		detail::GlassBrushHelper<LiquidGlassTabBar>,
		detail::TemplateControlHelper<LiquidGlassTabBar, false>,
		detail::PointerLightHelper<LiquidGlassTabBar>,
		detail::PointerFieldHelper<LiquidGlassTabBar>,
		detail::CompactControlMotionDefaults<LiquidGlassTabBar, detail::CompactControlMotionProfile::Selector>
	{
		LiquidGlassTabBar();
		Microsoft::UI::Xaml::DependencyObject GetContainerForItemOverride();
		bool IsItemItsOwnContainerOverride(Windows::Foundation::IInspectable const& item);
	};
}

namespace winrt::WinUI::LiquidGlass::factory_implementation
{
	struct LiquidGlassCard : LiquidGlassCardT<LiquidGlassCard, implementation::LiquidGlassCard>
	{
	};
	struct LiquidGlassMagnifier : LiquidGlassMagnifierT<LiquidGlassMagnifier, implementation::LiquidGlassMagnifier>
	{
	};
	struct LiquidGlassButton : LiquidGlassButtonT<LiquidGlassButton, implementation::LiquidGlassButton>
	{
	};
	struct LiquidGlassToggleButton : LiquidGlassToggleButtonT<LiquidGlassToggleButton, implementation::LiquidGlassToggleButton>
	{
	};
	struct LiquidGlassHyperlinkButton : LiquidGlassHyperlinkButtonT<LiquidGlassHyperlinkButton, implementation::LiquidGlassHyperlinkButton>
	{
	};
	struct LiquidGlassCheckBox : LiquidGlassCheckBoxT<LiquidGlassCheckBox, implementation::LiquidGlassCheckBox>
	{
	};
	struct LiquidGlassRadioButton : LiquidGlassRadioButtonT<LiquidGlassRadioButton, implementation::LiquidGlassRadioButton>
	{
	};
	struct LiquidGlassSlider : LiquidGlassSliderT<LiquidGlassSlider, implementation::LiquidGlassSlider>
	{
	};
	struct LiquidGlassTextBox : LiquidGlassTextBoxT<LiquidGlassTextBox, implementation::LiquidGlassTextBox>
	{
	};
	struct LiquidGlassPasswordBox : LiquidGlassPasswordBoxT<LiquidGlassPasswordBox, implementation::LiquidGlassPasswordBox>
	{
	};
	struct LiquidGlassComboBox : LiquidGlassComboBoxT<LiquidGlassComboBox, implementation::LiquidGlassComboBox>
	{
	};
	struct LiquidGlassToggleSwitch : LiquidGlassToggleSwitchT<LiquidGlassToggleSwitch, implementation::LiquidGlassToggleSwitch>
	{
	};
	struct LiquidGlassTabBarItem : LiquidGlassTabBarItemT<LiquidGlassTabBarItem, implementation::LiquidGlassTabBarItem>
	{
	};
	struct LiquidGlassTabBar : LiquidGlassTabBarT<LiquidGlassTabBar, implementation::LiquidGlassTabBar>
	{
	};
}

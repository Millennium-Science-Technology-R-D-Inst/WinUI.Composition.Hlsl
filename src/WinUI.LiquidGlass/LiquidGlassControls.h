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
    inline constexpr wchar_t ThemeResourceUri[] =
        L"ms-appx:///WinUI.LiquidGlass/Themes/Generic.xaml";
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
        detail::PressOpticsHelper<LiquidGlassButton, detail::PersistentOpticsKind::None>
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
        detail::PressOpticsHelper<LiquidGlassToggleButton, detail::PersistentOpticsKind::Toggle>
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
        detail::PressOpticsHelper<LiquidGlassHyperlinkButton, detail::PersistentOpticsKind::None>
    {
        constexpr static auto ResourceUri = detail::ThemeResourceUri;
        LiquidGlassHyperlinkButton();
    };

    struct LiquidGlassMagnifier :
        LiquidGlassMagnifierT<LiquidGlassMagnifier>,
        detail::GlassBrushHelper<LiquidGlassMagnifier>,
        detail::TemplateControlHelper<LiquidGlassMagnifier>,
        detail::PointerLightHelper<LiquidGlassMagnifier>,
        detail::PointerFieldHelper<LiquidGlassMagnifier>
    {
        constexpr static auto ResourceUri = detail::ThemeResourceUri;
        LiquidGlassMagnifier();

    private:
        Microsoft::UI::Xaml::Media::CompositeTransform m_dragTransform{ nullptr };
        Microsoft::UI::Xaml::UIElement m_dragCoordinateRoot{ nullptr };
        Windows::Foundation::Point m_dragStartPointer{};
        Windows::Foundation::Point m_lastPointer{};
        std::chrono::steady_clock::time_point m_lastPointerTime{};
        detail::OpticsSnapshot m_dragOptics;
        double m_dragStartTranslateX{};
        double m_dragStartTranslateY{};
        double m_dragMagnification{};
        double m_smoothedVelocityX{};
        uint32_t m_activePointerId{};
        bool m_dragging{};
    };

    struct LiquidGlassCheckBox :
        LiquidGlassCheckBoxT<LiquidGlassCheckBox>,
        detail::GlassBrushHelper<LiquidGlassCheckBox>,
        detail::TemplateControlHelper<LiquidGlassCheckBox>,
        detail::PointerLightHelper<LiquidGlassCheckBox>,
        detail::PointerMotionHelper<LiquidGlassCheckBox>,
        detail::PressOpticsHelper<LiquidGlassCheckBox, detail::PersistentOpticsKind::Toggle>
    {
        constexpr static auto ResourceUri = detail::ThemeResourceUri;
        LiquidGlassCheckBox();
    };

    struct LiquidGlassRadioButton :
        LiquidGlassRadioButtonT<LiquidGlassRadioButton>,
        detail::GlassBrushHelper<LiquidGlassRadioButton>,
        detail::TemplateControlHelper<LiquidGlassRadioButton>,
        detail::PointerLightHelper<LiquidGlassRadioButton>,
        detail::PointerMotionHelper<LiquidGlassRadioButton>,
        detail::PressOpticsHelper<LiquidGlassRadioButton, detail::PersistentOpticsKind::Toggle>
    {
        constexpr static auto ResourceUri = detail::ThemeResourceUri;
        LiquidGlassRadioButton();
    };

    struct LiquidGlassSlider :
        LiquidGlassSliderT<LiquidGlassSlider>,
        detail::GlassBrushHelper<LiquidGlassSlider>,
        detail::TemplateControlHelper<LiquidGlassSlider>,
        detail::PointerLightHelper<LiquidGlassSlider>,
        detail::SliderPointerFieldHelper<LiquidGlassSlider>
    {
        constexpr static auto ResourceUri = detail::ThemeResourceUri;
        LiquidGlassSlider();

        // C++ implementation hook used by GlassBrushHelper; it is not projected by the IDL.
        void ApplyGlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value);

        void OnApplyTemplate()
        {
            base_type::OnApplyTemplate();
            detail::SliderPointerFieldHelper<LiquidGlassSlider>::RefreshPointerFieldTarget();
        }

    private:
        Microsoft::UI::Xaml::Controls::Primitives::Thumb m_thumb{ nullptr };
        detail::OpticsSnapshot m_dragOptics;
        bool m_interactionsWired{};
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

        // C++ implementation hook used by GlassBrushHelper; it is not projected by the IDL.
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
        detail::PointerLightHelper<LiquidGlassToggleSwitch>,
        detail::ToggleSwitchInteractionHelper<LiquidGlassToggleSwitch>,
        detail::PressOpticsHelper<LiquidGlassToggleSwitch, detail::PersistentOpticsKind::Toggle>
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
            detail::ToggleSwitchInteractionHelper<LiquidGlassToggleSwitch>::RefreshInteractionTarget();
        }

        void OnToggle()
        {
            if (!detail::ToggleSwitchInteractionHelper<LiquidGlassToggleSwitch>::TryHandleToggle())
            {
                base_type::OnToggle();
            }
        }

    private:
        Windows::Foundation::IInspectable m_header{ nullptr };
        bool m_interactionsWired{};
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
        detail::PointerFieldHelper<LiquidGlassTabBar>
    {
        LiquidGlassTabBar();

        Microsoft::UI::Xaml::DependencyObject GetContainerForItemOverride();
        bool IsItemItsOwnContainerOverride(Windows::Foundation::IInspectable const& item);
    };
}

namespace winrt::WinUI::LiquidGlass::factory_implementation
{
    struct LiquidGlassCard : LiquidGlassCardT<LiquidGlassCard, implementation::LiquidGlassCard> {};
    struct LiquidGlassMagnifier : LiquidGlassMagnifierT<LiquidGlassMagnifier, implementation::LiquidGlassMagnifier> {};
    struct LiquidGlassButton : LiquidGlassButtonT<LiquidGlassButton, implementation::LiquidGlassButton> {};
    struct LiquidGlassToggleButton : LiquidGlassToggleButtonT<LiquidGlassToggleButton, implementation::LiquidGlassToggleButton> {};
    struct LiquidGlassHyperlinkButton : LiquidGlassHyperlinkButtonT<LiquidGlassHyperlinkButton, implementation::LiquidGlassHyperlinkButton> {};
    struct LiquidGlassCheckBox : LiquidGlassCheckBoxT<LiquidGlassCheckBox, implementation::LiquidGlassCheckBox> {};
    struct LiquidGlassRadioButton : LiquidGlassRadioButtonT<LiquidGlassRadioButton, implementation::LiquidGlassRadioButton> {};
    struct LiquidGlassSlider : LiquidGlassSliderT<LiquidGlassSlider, implementation::LiquidGlassSlider> {};
    struct LiquidGlassTextBox : LiquidGlassTextBoxT<LiquidGlassTextBox, implementation::LiquidGlassTextBox> {};
    struct LiquidGlassPasswordBox : LiquidGlassPasswordBoxT<LiquidGlassPasswordBox, implementation::LiquidGlassPasswordBox> {};
    struct LiquidGlassComboBox : LiquidGlassComboBoxT<LiquidGlassComboBox, implementation::LiquidGlassComboBox> {};
    struct LiquidGlassToggleSwitch : LiquidGlassToggleSwitchT<LiquidGlassToggleSwitch, implementation::LiquidGlassToggleSwitch> {};
    struct LiquidGlassTabBarItem : LiquidGlassTabBarItemT<LiquidGlassTabBarItem, implementation::LiquidGlassTabBarItem> {};
    struct LiquidGlassTabBar : LiquidGlassTabBarT<LiquidGlassTabBar, implementation::LiquidGlassTabBar> {};
}

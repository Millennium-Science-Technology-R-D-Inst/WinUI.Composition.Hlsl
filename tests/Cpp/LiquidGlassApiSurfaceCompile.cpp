#include "XamlWorkaround.h"

import std;
import winrt.WinUI.Composition.Hlsl;
import winrt.WinUI.LiquidGlass;

namespace
{
    using namespace winrt;
    using namespace Microsoft::UI::Xaml;
    using namespace WinUI::LiquidGlass;

    // Compile-only coverage for the WinUI.LiquidGlass projection. This function is
    // intentionally never called; package-consumer CI still compiles every projected
    // constructor, dependency property and wrapper API below from the packed NuGet.
    void ValidateLiquidGlassApiSurface()
    {
        auto card = LiquidGlassCard{};
        auto magnifier = LiquidGlassMagnifier{};
        auto button = LiquidGlassButton{};
        auto toggleButton = LiquidGlassToggleButton{};
        auto hyperlinkButton = LiquidGlassHyperlinkButton{};
        auto checkBox = LiquidGlassCheckBox{};
        auto radioButton = LiquidGlassRadioButton{};
        auto slider = LiquidGlassSlider{};
        auto textBox = LiquidGlassTextBox{};
        auto passwordBox = LiquidGlassPasswordBox{};
        auto comboBox = LiquidGlassComboBox{};
        auto toggleSwitch = LiquidGlassToggleSwitch{};
        auto tabBarItem = LiquidGlassTabBarItem{};
        auto tabBar = LiquidGlassTabBar{};
        auto floatingPanel = LiquidGlassFloatingPanel{};
        auto searchBox = LiquidGlassSearchBox{};

        auto presetBrush = LiquidGlassPresets::CreateBrush(LiquidGlassPreset::Button);
        button.GlassBrush(presetBrush);
        (void)button.GlassBrush();

        (void)LiquidGlassCard::GlassBrushProperty();
        (void)LiquidGlassMagnifier::GlassBrushProperty();
        (void)LiquidGlassButton::GlassBrushProperty();
        (void)LiquidGlassToggleButton::GlassBrushProperty();
        (void)LiquidGlassHyperlinkButton::GlassBrushProperty();
        (void)LiquidGlassCheckBox::GlassBrushProperty();
        (void)LiquidGlassRadioButton::GlassBrushProperty();
        (void)LiquidGlassSlider::GlassBrushProperty();
        (void)LiquidGlassTextBox::GlassBrushProperty();
        (void)LiquidGlassPasswordBox::GlassBrushProperty();
        (void)LiquidGlassComboBox::GlassBrushProperty();
        (void)LiquidGlassToggleSwitch::GlassBrushProperty();
        (void)LiquidGlassTabBarItem::GlassBrushProperty();
        (void)LiquidGlassTabBar::GlassBrushProperty();
        (void)LiquidGlassFloatingPanel::GlassBrushProperty();
        (void)LiquidGlassSearchBox::GlassBrushProperty();
        (void)LiquidGlassSearchBox::MaxSuggestionListHeightProperty();
        (void)LiquidGlassSearchBox::IsSuggestionListOpenProperty();
        (void)LiquidGlassSearchBox::TextMemberPathProperty();
        (void)LiquidGlassSearchBox::TextProperty();
        (void)LiquidGlassSearchBox::UpdateTextOnSelectProperty();
        (void)LiquidGlassSearchBox::PlaceholderTextProperty();
        (void)LiquidGlassSearchBox::HeaderProperty();
        (void)LiquidGlassSearchBox::AutoMaximizeSuggestionAreaProperty();
        (void)LiquidGlassSearchBox::TextBoxStyleProperty();
        (void)LiquidGlassSearchBox::QueryIconProperty();
        (void)LiquidGlassSearchBox::LightDismissOverlayModeProperty();
        (void)LiquidGlassSearchBox::DescriptionProperty();
        (void)LiquidGlassSearchBox::ItemsSourceProperty();
        (void)LiquidGlassSearchBox::ItemTemplateProperty();

        passwordBox.PlaceholderText(L"Password");
        passwordBox.Password(L"Glass");
        (void)passwordBox.PlaceholderText();
        (void)passwordBox.Password();

        toggleSwitch.Header(box_value(hstring{ L"Switch" }));
        toggleSwitch.IsOn(true);
        (void)toggleSwitch.Header();
        (void)toggleSwitch.IsOn();

        searchBox.Text(L"Refraction");
        searchBox.PlaceholderText(L"Search liquid glass controls");
        searchBox.IsSuggestionListOpen(false);
        searchBox.AutoMaximizeSuggestionArea(true);
        searchBox.MaxSuggestionListHeight(320.0);
        searchBox.UpdateTextOnSelect(true);
        searchBox.TextMemberPath(L"Name");
        searchBox.Header(box_value(hstring{ L"Search" }));
        searchBox.LightDismissOverlayMode(Microsoft::UI::Xaml::Controls::LightDismissOverlayMode::Auto);
        searchBox.Description(box_value(hstring{ L"Search suggestions" }));
        (void)searchBox.Text();
        (void)searchBox.PlaceholderText();
        (void)searchBox.ItemsSource();
        (void)searchBox.ItemTemplate();
        (void)searchBox.IsSuggestionListOpen();
        (void)searchBox.AutoMaximizeSuggestionArea();
        (void)searchBox.MaxSuggestionListHeight();
        (void)searchBox.UpdateTextOnSelect();
        (void)searchBox.TextMemberPath();
        (void)searchBox.Header();
        (void)searchBox.TextBoxStyle();
        (void)searchBox.QueryIcon();
        (void)searchBox.LightDismissOverlayMode();
        (void)searchBox.Description();
        (void)searchBox.InnerAutoSuggestBox();

        auto owner = button.as<DependencyObject>();
        (void)LiquidGlassInteraction::PointerLightingEnabledProperty();
        LiquidGlassInteraction::SetPointerLightingEnabled(owner, true);
        (void)LiquidGlassInteraction::GetPointerLightingEnabled(owner);

        (void)LiquidGlassInteraction::RestScaleProperty();
        LiquidGlassInteraction::SetRestScale(owner, 1.0);
        (void)LiquidGlassInteraction::GetRestScale(owner);

        (void)LiquidGlassInteraction::PointerOverRefractionMultiplierProperty();
        LiquidGlassInteraction::SetPointerOverRefractionMultiplier(owner, 1.08);
        (void)LiquidGlassInteraction::GetPointerOverRefractionMultiplier(owner);

        (void)LiquidGlassInteraction::PressedRefractionMultiplierProperty();
        LiquidGlassInteraction::SetPressedRefractionMultiplier(owner, 1.2);
        (void)LiquidGlassInteraction::GetPressedRefractionMultiplier(owner);

        (void)LiquidGlassInteraction::FocusedRefractionMultiplierProperty();
        LiquidGlassInteraction::SetFocusedRefractionMultiplier(owner, 1.05);
        (void)LiquidGlassInteraction::GetFocusedRefractionMultiplier(owner);

        (void)LiquidGlassInteraction::ActivatedRefractionMultiplierProperty();
        LiquidGlassInteraction::SetActivatedRefractionMultiplier(owner, 1.04);
        (void)LiquidGlassInteraction::GetActivatedRefractionMultiplier(owner);

        (void)card;
        (void)magnifier;
        (void)toggleButton;
        (void)hyperlinkButton;
        (void)checkBox;
        (void)radioButton;
        (void)slider;
        (void)textBox;
        (void)comboBox;
        (void)tabBarItem;
        (void)tabBar;
        (void)floatingPanel;
    }
}

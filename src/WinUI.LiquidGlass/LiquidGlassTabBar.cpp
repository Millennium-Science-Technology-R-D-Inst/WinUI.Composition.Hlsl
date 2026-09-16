#include "pch.h"
#include "winrt_module_imports.h"
#include "LiquidGlassControls.h"
#include "include/ResourceDictionaryLoader.hpp"

#if __has_include("LiquidGlassTabBarItem.g.cpp")
#include "LiquidGlassTabBarItem.g.cpp"
#include "LiquidGlassTabBar.g.cpp"
#endif

namespace winrt::WinUI::LiquidGlass::implementation
{
    namespace Xaml = Microsoft::UI::Xaml;
    namespace Controls = Xaml::Controls;

    namespace
    {
        void EnsureTabBarResources()
        {
            // A first construction attempt can happen before Application::Current() is
            // available (for example during metadata/type activation). Cache only success;
            // the shared loader also detects an already merged dictionary before appending.
            static bool loaded{};
            if (!loaded)
            {
                loaded = detail::EnsureMergedResourceDictionary(
                    L"ms-appx:///WinUI.LiquidGlass/Themes/TabBar.xaml");
            }
        }
    }

    LiquidGlassTabBarItem::LiquidGlassTabBarItem()
    {
        EnsureTabBarResources();
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(WinUI::LiquidGlass::LiquidGlassPresets::CreateBrush(
            WinUI::LiquidGlass::LiquidGlassPreset::TabBarItem));
        UseSystemFocusVisuals(true);
    }

    LiquidGlassTabBar::LiquidGlassTabBar()
    {
        EnsureTabBarResources();
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(WinUI::LiquidGlass::LiquidGlassPresets::CreateBrush(
            WinUI::LiquidGlass::LiquidGlassPreset::TabBar));

        SelectionMode(Controls::ListViewSelectionMode::Single);
        IsMultiSelectCheckBoxEnabled(false);
        SingleSelectionFollowsFocus(false);

        // Generated item containers inherit these attached values through the visual-tree
        // fallback. Keep selector motion compact: selection is the primary state change;
        // pointer feedback should acknowledge input without making the whole tab drift.
        SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::PointerOverScaleProperty(), box_value(1.018));
        SetValue(LiquidGlassInteraction::PressedScaleProperty(), box_value(.965));
        SetValue(LiquidGlassInteraction::UseSpringMotionProperty(), box_value(false));
        SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(100.0));
        SetValue(LiquidGlassInteraction::OpticsTransitionDurationProperty(), box_value(95.0));
        SetValue(LiquidGlassInteraction::ElasticityProperty(), box_value(.05));
        SetValue(LiquidGlassInteraction::PointerDisplacementProperty(), box_value(.8));
        SetValue(LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(1.10));
        SetValue(LiquidGlassInteraction::PressedDispersionMultiplierProperty(), box_value(1.06));
        SetValue(LiquidGlassInteraction::PressedSaturationMultiplierProperty(), box_value(1.02));
        SetValue(LiquidGlassInteraction::PressedContrastMultiplierProperty(), box_value(1.02));
        SetValue(LiquidGlassInteraction::PressedTintBoostProperty(), box_value(.045));
        SetValue(LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.10));
    }

    Xaml::DependencyObject LiquidGlassTabBar::GetContainerForItemOverride()
    {
        return winrt::make<LiquidGlassTabBarItem>();
    }

    bool LiquidGlassTabBar::IsItemItsOwnContainerOverride(Windows::Foundation::IInspectable const& item)
    {
        return static_cast<bool>(item.try_as<WinUI::LiquidGlass::LiquidGlassTabBarItem>());
    }
}

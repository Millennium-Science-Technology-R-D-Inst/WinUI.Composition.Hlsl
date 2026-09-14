#include "pch.h"
#include "winrt_module_imports.h"
#include "LiquidGlassInteraction.h"

#if __has_include("LiquidGlassInteraction.g.cpp")
#include "LiquidGlassInteraction.g.cpp"
#endif

namespace winrt::WinUI::LiquidGlass::implementation
{
    namespace Xaml = Microsoft::UI::Xaml;

#define LIQUID_GLASS_WIDEN2(Value) L##Value
#define LIQUID_GLASS_WIDEN(Value) LIQUID_GLASS_WIDEN2(Value)

#define LIQUID_GLASS_BOOL_DP(Name, DefaultValue) \
    Xaml::DependencyProperty LiquidGlassInteraction::s_##Name##Property = \
        Xaml::DependencyProperty::RegisterAttached( \
            LIQUID_GLASS_WIDEN(#Name), xaml_typename<bool>(), xaml_typename<winrt::WinUI::LiquidGlass::LiquidGlassInteraction>(), \
            Xaml::PropertyMetadata{ box_value(DefaultValue) }); \
    Xaml::DependencyProperty LiquidGlassInteraction::Name##Property() { return s_##Name##Property; } \
    bool LiquidGlassInteraction::Get##Name(Xaml::DependencyObject const& element) \
    { \
        return unbox_value<bool>(element.GetValue(s_##Name##Property)); \
    } \
    void LiquidGlassInteraction::Set##Name(Xaml::DependencyObject const& element, bool value) \
    { \
        element.SetValue(s_##Name##Property, box_value(value)); \
    }

#define LIQUID_GLASS_DOUBLE_DP(Name, DefaultValue) \
    Xaml::DependencyProperty LiquidGlassInteraction::s_##Name##Property = \
        Xaml::DependencyProperty::RegisterAttached( \
            LIQUID_GLASS_WIDEN(#Name), xaml_typename<double>(), xaml_typename<winrt::WinUI::LiquidGlass::LiquidGlassInteraction>(), \
            Xaml::PropertyMetadata{ box_value(static_cast<double>(DefaultValue)) }); \
    Xaml::DependencyProperty LiquidGlassInteraction::Name##Property() { return s_##Name##Property; } \
    double LiquidGlassInteraction::Get##Name(Xaml::DependencyObject const& element) \
    { \
        return unbox_value<double>(element.GetValue(s_##Name##Property)); \
    } \
    void LiquidGlassInteraction::Set##Name(Xaml::DependencyObject const& element, double value) \
    { \
        element.SetValue(s_##Name##Property, box_value(value)); \
    }

    LIQUID_GLASS_BOOL_DP(PointerLightingEnabled, true)
    LIQUID_GLASS_DOUBLE_DP(PointerLightInfluence, 1.0)
    LIQUID_GLASS_DOUBLE_DP(RestScale, 1.0)
    LIQUID_GLASS_DOUBLE_DP(PointerOverScale, 1.025)
    LIQUID_GLASS_DOUBLE_DP(PressedScale, 0.965)
    LIQUID_GLASS_DOUBLE_DP(FocusedScale, 1.0)
    LIQUID_GLASS_DOUBLE_DP(MotionDuration, 120.0)
    LIQUID_GLASS_DOUBLE_DP(Elasticity, 0.18)
    LIQUID_GLASS_DOUBLE_DP(PressedRefractionMultiplier, 1.18)
    LIQUID_GLASS_DOUBLE_DP(PressedRefractionBoost, 1.5)
    LIQUID_GLASS_DOUBLE_DP(PressedTintBoost, 0.08)
    LIQUID_GLASS_DOUBLE_DP(PressedHighlightMultiplier, 1.12)
    LIQUID_GLASS_DOUBLE_DP(PressedHighlightBoost, 0.03)
    LIQUID_GLASS_DOUBLE_DP(PressedInnerShadowBoost, 0.04)
    LIQUID_GLASS_DOUBLE_DP(FocusedTintBoost, 0.08)
    LIQUID_GLASS_DOUBLE_DP(ActiveMagnificationMultiplier, 2.0)

#undef LIQUID_GLASS_DOUBLE_DP
#undef LIQUID_GLASS_BOOL_DP
#undef LIQUID_GLASS_WIDEN
#undef LIQUID_GLASS_WIDEN2
}

#include "pch.h"
#include "winrt_module_imports.h"
#include "LiquidGlassInteraction.h"

#if __has_include("LiquidGlassInteraction.g.cpp")
#include "LiquidGlassInteraction.g.cpp"
#endif

namespace winrt::WinUI::LiquidGlass::implementation
{
    namespace Xaml = Microsoft::UI::Xaml;

    namespace
    {
        Windows::Foundation::IInspectable ResolveAttachedValue(
            Xaml::DependencyObject const& element,
            Xaml::DependencyProperty const& property)
        {
            if (!element) return nullptr;

            auto current = element;
            while (current)
            {
                auto local = current.ReadLocalValue(property);
                if (local != Xaml::DependencyProperty::UnsetValue()) return local;
                current = Xaml::Media::VisualTreeHelper::GetParent(current);
            }
            return element.GetValue(property);
        }
    }

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
        return unbox_value<bool>(ResolveAttachedValue(element, s_##Name##Property)); \
    } \
    void LiquidGlassInteraction::Set##Name(Xaml::DependencyObject const& element, bool value) \
    { \
        if (!element) throw hresult_invalid_argument(L"element must not be null."); \
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
        return unbox_value<double>(ResolveAttachedValue(element, s_##Name##Property)); \
    } \
    void LiquidGlassInteraction::Set##Name(Xaml::DependencyObject const& element, double value) \
    { \
        if (!element) throw hresult_invalid_argument(L"element must not be null."); \
        if (!std::isfinite(value)) throw hresult_invalid_argument(L"LiquidGlassInteraction values must be finite."); \
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
    LIQUID_GLASS_DOUBLE_DP(PointerDisplacement, 2.5)
    LIQUID_GLASS_DOUBLE_DP(PressedDisplacementMultiplier, 1.45)

    LIQUID_GLASS_DOUBLE_DP(PointerOverBlurBoost, 0.0)
    LIQUID_GLASS_DOUBLE_DP(PointerOverRefractionMultiplier, 1.05)
    LIQUID_GLASS_DOUBLE_DP(PointerOverRefractionBoost, 0.0)
    LIQUID_GLASS_DOUBLE_DP(PointerOverDispersionMultiplier, 1.03)
    LIQUID_GLASS_DOUBLE_DP(PointerOverSaturationMultiplier, 1.01)
    LIQUID_GLASS_DOUBLE_DP(PointerOverContrastMultiplier, 1.02)
    LIQUID_GLASS_DOUBLE_DP(PointerOverExposureBoost, 0.0)
    LIQUID_GLASS_DOUBLE_DP(PointerOverTintBoost, 0.025)
    LIQUID_GLASS_DOUBLE_DP(PointerOverHighlightMultiplier, 1.08)
    LIQUID_GLASS_DOUBLE_DP(PointerOverHighlightBoost, 0.01)
    LIQUID_GLASS_DOUBLE_DP(PointerOverInnerShadowBoost, 0.01)

    LIQUID_GLASS_DOUBLE_DP(PressedBlurBoost, 0.0)
    LIQUID_GLASS_DOUBLE_DP(PressedRefractionMultiplier, 1.18)
    LIQUID_GLASS_DOUBLE_DP(PressedRefractionBoost, 1.5)
    LIQUID_GLASS_DOUBLE_DP(PressedDispersionMultiplier, 1.08)
    LIQUID_GLASS_DOUBLE_DP(PressedSaturationMultiplier, 1.02)
    LIQUID_GLASS_DOUBLE_DP(PressedContrastMultiplier, 1.02)
    LIQUID_GLASS_DOUBLE_DP(PressedExposureBoost, 0.0)
    LIQUID_GLASS_DOUBLE_DP(PressedTintBoost, 0.08)
    LIQUID_GLASS_DOUBLE_DP(PressedHighlightMultiplier, 1.12)
    LIQUID_GLASS_DOUBLE_DP(PressedHighlightBoost, 0.03)
    LIQUID_GLASS_DOUBLE_DP(PressedInnerShadowBoost, 0.04)

    LIQUID_GLASS_DOUBLE_DP(FocusedBlurBoost, 0.0)
    LIQUID_GLASS_DOUBLE_DP(FocusedRefractionMultiplier, 1.04)
    LIQUID_GLASS_DOUBLE_DP(FocusedDispersionMultiplier, 1.0)
    LIQUID_GLASS_DOUBLE_DP(FocusedSaturationMultiplier, 1.02)
    LIQUID_GLASS_DOUBLE_DP(FocusedContrastMultiplier, 1.03)
    LIQUID_GLASS_DOUBLE_DP(FocusedExposureBoost, 0.0)
    LIQUID_GLASS_DOUBLE_DP(FocusedTintBoost, 0.08)
    LIQUID_GLASS_DOUBLE_DP(FocusedHighlightBoost, 0.03)
    LIQUID_GLASS_DOUBLE_DP(FocusedInnerShadowBoost, 0.0)

    LIQUID_GLASS_DOUBLE_DP(ActiveMagnificationMultiplier, 2.0)

#undef LIQUID_GLASS_DOUBLE_DP
#undef LIQUID_GLASS_BOOL_DP
#undef LIQUID_GLASS_WIDEN
#undef LIQUID_GLASS_WIDEN2
}

#pragma once

#include "winrt_module_imports.h"
#include "LiquidGlassInteraction.g.h"

namespace winrt::WinUI::LiquidGlass::implementation
{
    struct LiquidGlassInteraction
    {
#define LIQUID_GLASS_INTERACTION_BOOL(Name) \
        static Microsoft::UI::Xaml::DependencyProperty Name##Property(); \
        static bool Get##Name(Microsoft::UI::Xaml::DependencyObject const& element); \
        static void Set##Name(Microsoft::UI::Xaml::DependencyObject const& element, bool value); \
    private: \
        static Microsoft::UI::Xaml::DependencyProperty s_##Name##Property; \
    public:

#define LIQUID_GLASS_INTERACTION_DOUBLE(Name) \
        static Microsoft::UI::Xaml::DependencyProperty Name##Property(); \
        static double Get##Name(Microsoft::UI::Xaml::DependencyObject const& element); \
        static void Set##Name(Microsoft::UI::Xaml::DependencyObject const& element, double value); \
    private: \
        static Microsoft::UI::Xaml::DependencyProperty s_##Name##Property; \
    public:

        LIQUID_GLASS_INTERACTION_BOOL(PointerLightingEnabled)
        LIQUID_GLASS_INTERACTION_DOUBLE(PointerLightInfluence)
        LIQUID_GLASS_INTERACTION_DOUBLE(RestScale)
        LIQUID_GLASS_INTERACTION_DOUBLE(PointerOverScale)
        LIQUID_GLASS_INTERACTION_DOUBLE(PressedScale)
        LIQUID_GLASS_INTERACTION_DOUBLE(FocusedScale)
        LIQUID_GLASS_INTERACTION_DOUBLE(MotionDuration)
        LIQUID_GLASS_INTERACTION_DOUBLE(Elasticity)

        LIQUID_GLASS_INTERACTION_DOUBLE(PressedBlurBoost)
        LIQUID_GLASS_INTERACTION_DOUBLE(PressedRefractionMultiplier)
        LIQUID_GLASS_INTERACTION_DOUBLE(PressedRefractionBoost)
        LIQUID_GLASS_INTERACTION_DOUBLE(PressedDispersionMultiplier)
        LIQUID_GLASS_INTERACTION_DOUBLE(PressedSaturationMultiplier)
        LIQUID_GLASS_INTERACTION_DOUBLE(PressedContrastMultiplier)
        LIQUID_GLASS_INTERACTION_DOUBLE(PressedExposureBoost)
        LIQUID_GLASS_INTERACTION_DOUBLE(PressedTintBoost)
        LIQUID_GLASS_INTERACTION_DOUBLE(PressedHighlightMultiplier)
        LIQUID_GLASS_INTERACTION_DOUBLE(PressedHighlightBoost)
        LIQUID_GLASS_INTERACTION_DOUBLE(PressedInnerShadowBoost)

        LIQUID_GLASS_INTERACTION_DOUBLE(FocusedBlurBoost)
        LIQUID_GLASS_INTERACTION_DOUBLE(FocusedRefractionMultiplier)
        LIQUID_GLASS_INTERACTION_DOUBLE(FocusedDispersionMultiplier)
        LIQUID_GLASS_INTERACTION_DOUBLE(FocusedSaturationMultiplier)
        LIQUID_GLASS_INTERACTION_DOUBLE(FocusedContrastMultiplier)
        LIQUID_GLASS_INTERACTION_DOUBLE(FocusedExposureBoost)
        LIQUID_GLASS_INTERACTION_DOUBLE(FocusedTintBoost)
        LIQUID_GLASS_INTERACTION_DOUBLE(FocusedHighlightBoost)
        LIQUID_GLASS_INTERACTION_DOUBLE(FocusedInnerShadowBoost)

        LIQUID_GLASS_INTERACTION_DOUBLE(ActiveMagnificationMultiplier)

#undef LIQUID_GLASS_INTERACTION_DOUBLE
#undef LIQUID_GLASS_INTERACTION_BOOL
    };
}

namespace winrt::WinUI::LiquidGlass::factory_implementation
{
    struct LiquidGlassInteraction : LiquidGlassInteractionT<LiquidGlassInteraction, implementation::LiquidGlassInteraction>
    {
    };
}

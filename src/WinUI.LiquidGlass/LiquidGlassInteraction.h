#pragma once

#include "winrt_module_imports.h"
#include "LiquidGlassInteraction.g.h"

namespace winrt::WinUI::LiquidGlass::implementation
{
    struct LiquidGlassInteraction
    {
        static Microsoft::UI::Xaml::DependencyProperty PointerLightingEnabledProperty();
        static bool GetPointerLightingEnabled(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerLightingEnabled(Microsoft::UI::Xaml::DependencyObject const& element, bool value);

        static Microsoft::UI::Xaml::DependencyProperty PointerLightInfluenceProperty();
        static double GetPointerLightInfluence(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerLightInfluence(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty RestScaleProperty();
        static double GetRestScale(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetRestScale(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PointerOverScaleProperty();
        static double GetPointerOverScale(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerOverScale(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PressedScaleProperty();
        static double GetPressedScale(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPressedScale(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty FocusedScaleProperty();
        static double GetFocusedScale(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetFocusedScale(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty MotionDurationProperty();
        static double GetMotionDuration(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetMotionDuration(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty OpticsTransitionDurationProperty();
        static double GetOpticsTransitionDuration(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetOpticsTransitionDuration(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty UseSpringMotionProperty();
        static bool GetUseSpringMotion(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetUseSpringMotion(Microsoft::UI::Xaml::DependencyObject const& element, bool value);

        static Microsoft::UI::Xaml::DependencyProperty SpringDampingRatioProperty();
        static double GetSpringDampingRatio(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetSpringDampingRatio(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty SpringPeriodProperty();
        static double GetSpringPeriod(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetSpringPeriod(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty RespectSystemAnimationsProperty();
        static bool GetRespectSystemAnimations(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetRespectSystemAnimations(Microsoft::UI::Xaml::DependencyObject const& element, bool value);

        static Microsoft::UI::Xaml::DependencyProperty ElasticityProperty();
        static double GetElasticity(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetElasticity(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PointerDisplacementProperty();
        static double GetPointerDisplacement(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerDisplacement(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PressedDisplacementMultiplierProperty();
        static double GetPressedDisplacementMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPressedDisplacementMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty ActivatedRefractionMultiplierProperty();
        static double GetActivatedRefractionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetActivatedRefractionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty ActivatedDispersionMultiplierProperty();
        static double GetActivatedDispersionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetActivatedDispersionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty ActivatedSaturationMultiplierProperty();
        static double GetActivatedSaturationMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetActivatedSaturationMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty ActivatedContrastMultiplierProperty();
        static double GetActivatedContrastMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetActivatedContrastMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty ActivatedTintBoostProperty();
        static double GetActivatedTintBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetActivatedTintBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty ActivatedHighlightMultiplierProperty();
        static double GetActivatedHighlightMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetActivatedHighlightMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty ActivatedInnerShadowBoostProperty();
        static double GetActivatedInnerShadowBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetActivatedInnerShadowBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PointerOverBlurBoostProperty();
        static double GetPointerOverBlurBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerOverBlurBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PointerOverRefractionMultiplierProperty();
        static double GetPointerOverRefractionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerOverRefractionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PointerOverRefractionBoostProperty();
        static double GetPointerOverRefractionBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerOverRefractionBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PointerOverDispersionMultiplierProperty();
        static double GetPointerOverDispersionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerOverDispersionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PointerOverSaturationMultiplierProperty();
        static double GetPointerOverSaturationMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerOverSaturationMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PointerOverContrastMultiplierProperty();
        static double GetPointerOverContrastMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerOverContrastMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PointerOverExposureBoostProperty();
        static double GetPointerOverExposureBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerOverExposureBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PointerOverTintBoostProperty();
        static double GetPointerOverTintBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerOverTintBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PointerOverHighlightMultiplierProperty();
        static double GetPointerOverHighlightMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerOverHighlightMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PointerOverHighlightBoostProperty();
        static double GetPointerOverHighlightBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerOverHighlightBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PointerOverInnerShadowBoostProperty();
        static double GetPointerOverInnerShadowBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPointerOverInnerShadowBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PressedBlurBoostProperty();
        static double GetPressedBlurBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPressedBlurBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PressedRefractionMultiplierProperty();
        static double GetPressedRefractionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPressedRefractionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PressedRefractionBoostProperty();
        static double GetPressedRefractionBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPressedRefractionBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PressedDispersionMultiplierProperty();
        static double GetPressedDispersionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPressedDispersionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PressedSaturationMultiplierProperty();
        static double GetPressedSaturationMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPressedSaturationMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PressedContrastMultiplierProperty();
        static double GetPressedContrastMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPressedContrastMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PressedExposureBoostProperty();
        static double GetPressedExposureBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPressedExposureBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PressedTintBoostProperty();
        static double GetPressedTintBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPressedTintBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PressedHighlightMultiplierProperty();
        static double GetPressedHighlightMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPressedHighlightMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PressedHighlightBoostProperty();
        static double GetPressedHighlightBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPressedHighlightBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty PressedInnerShadowBoostProperty();
        static double GetPressedInnerShadowBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetPressedInnerShadowBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty FocusedBlurBoostProperty();
        static double GetFocusedBlurBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetFocusedBlurBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty FocusedRefractionMultiplierProperty();
        static double GetFocusedRefractionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetFocusedRefractionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty FocusedDispersionMultiplierProperty();
        static double GetFocusedDispersionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetFocusedDispersionMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty FocusedSaturationMultiplierProperty();
        static double GetFocusedSaturationMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetFocusedSaturationMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty FocusedContrastMultiplierProperty();
        static double GetFocusedContrastMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetFocusedContrastMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty FocusedExposureBoostProperty();
        static double GetFocusedExposureBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetFocusedExposureBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty FocusedTintBoostProperty();
        static double GetFocusedTintBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetFocusedTintBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty FocusedHighlightBoostProperty();
        static double GetFocusedHighlightBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetFocusedHighlightBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty FocusedInnerShadowBoostProperty();
        static double GetFocusedInnerShadowBoost(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetFocusedInnerShadowBoost(Microsoft::UI::Xaml::DependencyObject const& element, double value);

        static Microsoft::UI::Xaml::DependencyProperty ActiveMagnificationMultiplierProperty();
        static double GetActiveMagnificationMultiplier(Microsoft::UI::Xaml::DependencyObject const& element);
        static void SetActiveMagnificationMultiplier(Microsoft::UI::Xaml::DependencyObject const& element, double value);
    };
}

namespace winrt::WinUI::LiquidGlass::factory_implementation
{
    struct LiquidGlassInteraction : LiquidGlassInteractionT<LiquidGlassInteraction, implementation::LiquidGlassInteraction>
    {
    };
}

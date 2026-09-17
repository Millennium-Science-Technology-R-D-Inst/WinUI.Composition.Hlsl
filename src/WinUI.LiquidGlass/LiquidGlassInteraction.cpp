#include "pch.h"
#include "winrt_module_imports.h"
#include "LiquidGlassInteraction.h"

#include <cmath>
#include <type_traits>

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

        template<typename T>
        Xaml::DependencyProperty RegisterAttachedProperty(wchar_t const* name, T defaultValue)
        {
            return Xaml::DependencyProperty::RegisterAttached(
                name,
                xaml_typename<T>(),
                xaml_typename<winrt::WinUI::LiquidGlass::LiquidGlassInteraction>(),
                Xaml::PropertyMetadata{ box_value(defaultValue) });
        }

        template<typename T>
        T GetAttachedValue(Xaml::DependencyObject const& element, Xaml::DependencyProperty const& property)
        {
            return unbox_value<T>(ResolveAttachedValue(element, property));
        }

        template<typename T>
        void SetAttachedValue(Xaml::DependencyObject const& element, Xaml::DependencyProperty const& property, T value)
        {
            if (!element) throw hresult_invalid_argument(L"element must not be null.");
            if constexpr (std::is_floating_point_v<T>)
            {
                if (!std::isfinite(value))
                    throw hresult_invalid_argument(L"LiquidGlassInteraction values must be finite.");
            }
            element.SetValue(property, box_value(value));
        }
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerLightingEnabledProperty()
    {
        static auto const property = RegisterAttachedProperty<bool>(L"PointerLightingEnabled", true);
        return property;
    }

    bool LiquidGlassInteraction::GetPointerLightingEnabled(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<bool>(element, PointerLightingEnabledProperty());
    }

    void LiquidGlassInteraction::SetPointerLightingEnabled(Xaml::DependencyObject const& element, bool value)
    {
        SetAttachedValue(element, PointerLightingEnabledProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerLightInfluenceProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PointerLightInfluence", 1.0);
        return property;
    }

    double LiquidGlassInteraction::GetPointerLightInfluence(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PointerLightInfluenceProperty());
    }

    void LiquidGlassInteraction::SetPointerLightInfluence(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PointerLightInfluenceProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::RestScaleProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"RestScale", 1.0);
        return property;
    }

    double LiquidGlassInteraction::GetRestScale(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, RestScaleProperty());
    }

    void LiquidGlassInteraction::SetRestScale(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, RestScaleProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerOverScaleProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PointerOverScale", 1.025);
        return property;
    }

    double LiquidGlassInteraction::GetPointerOverScale(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PointerOverScaleProperty());
    }

    void LiquidGlassInteraction::SetPointerOverScale(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PointerOverScaleProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PressedScaleProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PressedScale", 0.965);
        return property;
    }

    double LiquidGlassInteraction::GetPressedScale(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PressedScaleProperty());
    }

    void LiquidGlassInteraction::SetPressedScale(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PressedScaleProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::FocusedScaleProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"FocusedScale", 1.0);
        return property;
    }

    double LiquidGlassInteraction::GetFocusedScale(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, FocusedScaleProperty());
    }

    void LiquidGlassInteraction::SetFocusedScale(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, FocusedScaleProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::MotionDurationProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"MotionDuration", 120.0);
        return property;
    }

    double LiquidGlassInteraction::GetMotionDuration(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, MotionDurationProperty());
    }

    void LiquidGlassInteraction::SetMotionDuration(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, MotionDurationProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::OpticsTransitionDurationProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"OpticsTransitionDuration", 110.0);
        return property;
    }

    double LiquidGlassInteraction::GetOpticsTransitionDuration(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, OpticsTransitionDurationProperty());
    }

    void LiquidGlassInteraction::SetOpticsTransitionDuration(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, OpticsTransitionDurationProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::UseSpringMotionProperty()
    {
        static auto const property = RegisterAttachedProperty<bool>(L"UseSpringMotion", true);
        return property;
    }

    bool LiquidGlassInteraction::GetUseSpringMotion(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<bool>(element, UseSpringMotionProperty());
    }

    void LiquidGlassInteraction::SetUseSpringMotion(Xaml::DependencyObject const& element, bool value)
    {
        SetAttachedValue(element, UseSpringMotionProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::SpringDampingRatioProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"SpringDampingRatio", 0.82);
        return property;
    }

    double LiquidGlassInteraction::GetSpringDampingRatio(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, SpringDampingRatioProperty());
    }

    void LiquidGlassInteraction::SetSpringDampingRatio(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, SpringDampingRatioProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::SpringPeriodProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"SpringPeriod", 180.0);
        return property;
    }

    double LiquidGlassInteraction::GetSpringPeriod(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, SpringPeriodProperty());
    }

    void LiquidGlassInteraction::SetSpringPeriod(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, SpringPeriodProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::RespectSystemAnimationsProperty()
    {
        static auto const property = RegisterAttachedProperty<bool>(L"RespectSystemAnimations", true);
        return property;
    }

    bool LiquidGlassInteraction::GetRespectSystemAnimations(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<bool>(element, RespectSystemAnimationsProperty());
    }

    void LiquidGlassInteraction::SetRespectSystemAnimations(Xaml::DependencyObject const& element, bool value)
    {
        SetAttachedValue(element, RespectSystemAnimationsProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::ElasticityProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"Elasticity", 0.18);
        return property;
    }

    double LiquidGlassInteraction::GetElasticity(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, ElasticityProperty());
    }

    void LiquidGlassInteraction::SetElasticity(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, ElasticityProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerDisplacementProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PointerDisplacement", 2.5);
        return property;
    }

    double LiquidGlassInteraction::GetPointerDisplacement(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PointerDisplacementProperty());
    }

    void LiquidGlassInteraction::SetPointerDisplacement(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PointerDisplacementProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PressedDisplacementMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PressedDisplacementMultiplier", 1.45);
        return property;
    }

    double LiquidGlassInteraction::GetPressedDisplacementMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PressedDisplacementMultiplierProperty());
    }

    void LiquidGlassInteraction::SetPressedDisplacementMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PressedDisplacementMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::ActivatedRefractionMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"ActivatedRefractionMultiplier", 1.08);
        return property;
    }

    double LiquidGlassInteraction::GetActivatedRefractionMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, ActivatedRefractionMultiplierProperty());
    }

    void LiquidGlassInteraction::SetActivatedRefractionMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, ActivatedRefractionMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::ActivatedDispersionMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"ActivatedDispersionMultiplier", 1.06);
        return property;
    }

    double LiquidGlassInteraction::GetActivatedDispersionMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, ActivatedDispersionMultiplierProperty());
    }

    void LiquidGlassInteraction::SetActivatedDispersionMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, ActivatedDispersionMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::ActivatedSaturationMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"ActivatedSaturationMultiplier", 1.03);
        return property;
    }

    double LiquidGlassInteraction::GetActivatedSaturationMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, ActivatedSaturationMultiplierProperty());
    }

    void LiquidGlassInteraction::SetActivatedSaturationMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, ActivatedSaturationMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::ActivatedContrastMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"ActivatedContrastMultiplier", 1.04);
        return property;
    }

    double LiquidGlassInteraction::GetActivatedContrastMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, ActivatedContrastMultiplierProperty());
    }

    void LiquidGlassInteraction::SetActivatedContrastMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, ActivatedContrastMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::ActivatedTintBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"ActivatedTintBoost", 0.10);
        return property;
    }

    double LiquidGlassInteraction::GetActivatedTintBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, ActivatedTintBoostProperty());
    }

    void LiquidGlassInteraction::SetActivatedTintBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, ActivatedTintBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::ActivatedHighlightMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"ActivatedHighlightMultiplier", 1.12);
        return property;
    }

    double LiquidGlassInteraction::GetActivatedHighlightMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, ActivatedHighlightMultiplierProperty());
    }

    void LiquidGlassInteraction::SetActivatedHighlightMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, ActivatedHighlightMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::ActivatedInnerShadowBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"ActivatedInnerShadowBoost", 0.03);
        return property;
    }

    double LiquidGlassInteraction::GetActivatedInnerShadowBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, ActivatedInnerShadowBoostProperty());
    }

    void LiquidGlassInteraction::SetActivatedInnerShadowBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, ActivatedInnerShadowBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerOverBlurBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PointerOverBlurBoost", 0.0);
        return property;
    }

    double LiquidGlassInteraction::GetPointerOverBlurBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PointerOverBlurBoostProperty());
    }

    void LiquidGlassInteraction::SetPointerOverBlurBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PointerOverBlurBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerOverRefractionMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PointerOverRefractionMultiplier", 1.05);
        return property;
    }

    double LiquidGlassInteraction::GetPointerOverRefractionMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PointerOverRefractionMultiplierProperty());
    }

    void LiquidGlassInteraction::SetPointerOverRefractionMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PointerOverRefractionMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerOverRefractionBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PointerOverRefractionBoost", 0.0);
        return property;
    }

    double LiquidGlassInteraction::GetPointerOverRefractionBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PointerOverRefractionBoostProperty());
    }

    void LiquidGlassInteraction::SetPointerOverRefractionBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PointerOverRefractionBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerOverDispersionMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PointerOverDispersionMultiplier", 1.03);
        return property;
    }

    double LiquidGlassInteraction::GetPointerOverDispersionMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PointerOverDispersionMultiplierProperty());
    }

    void LiquidGlassInteraction::SetPointerOverDispersionMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PointerOverDispersionMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerOverSaturationMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PointerOverSaturationMultiplier", 1.01);
        return property;
    }

    double LiquidGlassInteraction::GetPointerOverSaturationMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PointerOverSaturationMultiplierProperty());
    }

    void LiquidGlassInteraction::SetPointerOverSaturationMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PointerOverSaturationMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerOverContrastMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PointerOverContrastMultiplier", 1.02);
        return property;
    }

    double LiquidGlassInteraction::GetPointerOverContrastMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PointerOverContrastMultiplierProperty());
    }

    void LiquidGlassInteraction::SetPointerOverContrastMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PointerOverContrastMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerOverExposureBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PointerOverExposureBoost", 0.0);
        return property;
    }

    double LiquidGlassInteraction::GetPointerOverExposureBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PointerOverExposureBoostProperty());
    }

    void LiquidGlassInteraction::SetPointerOverExposureBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PointerOverExposureBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerOverTintBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PointerOverTintBoost", 0.025);
        return property;
    }

    double LiquidGlassInteraction::GetPointerOverTintBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PointerOverTintBoostProperty());
    }

    void LiquidGlassInteraction::SetPointerOverTintBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PointerOverTintBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerOverHighlightMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PointerOverHighlightMultiplier", 1.08);
        return property;
    }

    double LiquidGlassInteraction::GetPointerOverHighlightMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PointerOverHighlightMultiplierProperty());
    }

    void LiquidGlassInteraction::SetPointerOverHighlightMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PointerOverHighlightMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerOverHighlightBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PointerOverHighlightBoost", 0.01);
        return property;
    }

    double LiquidGlassInteraction::GetPointerOverHighlightBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PointerOverHighlightBoostProperty());
    }

    void LiquidGlassInteraction::SetPointerOverHighlightBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PointerOverHighlightBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PointerOverInnerShadowBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PointerOverInnerShadowBoost", 0.01);
        return property;
    }

    double LiquidGlassInteraction::GetPointerOverInnerShadowBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PointerOverInnerShadowBoostProperty());
    }

    void LiquidGlassInteraction::SetPointerOverInnerShadowBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PointerOverInnerShadowBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PressedBlurBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PressedBlurBoost", 0.0);
        return property;
    }

    double LiquidGlassInteraction::GetPressedBlurBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PressedBlurBoostProperty());
    }

    void LiquidGlassInteraction::SetPressedBlurBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PressedBlurBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PressedRefractionMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PressedRefractionMultiplier", 1.18);
        return property;
    }

    double LiquidGlassInteraction::GetPressedRefractionMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PressedRefractionMultiplierProperty());
    }

    void LiquidGlassInteraction::SetPressedRefractionMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PressedRefractionMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PressedRefractionBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PressedRefractionBoost", 1.5);
        return property;
    }

    double LiquidGlassInteraction::GetPressedRefractionBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PressedRefractionBoostProperty());
    }

    void LiquidGlassInteraction::SetPressedRefractionBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PressedRefractionBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PressedDispersionMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PressedDispersionMultiplier", 1.08);
        return property;
    }

    double LiquidGlassInteraction::GetPressedDispersionMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PressedDispersionMultiplierProperty());
    }

    void LiquidGlassInteraction::SetPressedDispersionMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PressedDispersionMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PressedSaturationMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PressedSaturationMultiplier", 1.02);
        return property;
    }

    double LiquidGlassInteraction::GetPressedSaturationMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PressedSaturationMultiplierProperty());
    }

    void LiquidGlassInteraction::SetPressedSaturationMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PressedSaturationMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PressedContrastMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PressedContrastMultiplier", 1.02);
        return property;
    }

    double LiquidGlassInteraction::GetPressedContrastMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PressedContrastMultiplierProperty());
    }

    void LiquidGlassInteraction::SetPressedContrastMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PressedContrastMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PressedExposureBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PressedExposureBoost", 0.0);
        return property;
    }

    double LiquidGlassInteraction::GetPressedExposureBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PressedExposureBoostProperty());
    }

    void LiquidGlassInteraction::SetPressedExposureBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PressedExposureBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PressedTintBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PressedTintBoost", 0.08);
        return property;
    }

    double LiquidGlassInteraction::GetPressedTintBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PressedTintBoostProperty());
    }

    void LiquidGlassInteraction::SetPressedTintBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PressedTintBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PressedHighlightMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PressedHighlightMultiplier", 1.12);
        return property;
    }

    double LiquidGlassInteraction::GetPressedHighlightMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PressedHighlightMultiplierProperty());
    }

    void LiquidGlassInteraction::SetPressedHighlightMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PressedHighlightMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PressedHighlightBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PressedHighlightBoost", 0.03);
        return property;
    }

    double LiquidGlassInteraction::GetPressedHighlightBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PressedHighlightBoostProperty());
    }

    void LiquidGlassInteraction::SetPressedHighlightBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PressedHighlightBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::PressedInnerShadowBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"PressedInnerShadowBoost", 0.04);
        return property;
    }

    double LiquidGlassInteraction::GetPressedInnerShadowBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, PressedInnerShadowBoostProperty());
    }

    void LiquidGlassInteraction::SetPressedInnerShadowBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, PressedInnerShadowBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::FocusedBlurBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"FocusedBlurBoost", 0.0);
        return property;
    }

    double LiquidGlassInteraction::GetFocusedBlurBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, FocusedBlurBoostProperty());
    }

    void LiquidGlassInteraction::SetFocusedBlurBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, FocusedBlurBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::FocusedRefractionMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"FocusedRefractionMultiplier", 1.04);
        return property;
    }

    double LiquidGlassInteraction::GetFocusedRefractionMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, FocusedRefractionMultiplierProperty());
    }

    void LiquidGlassInteraction::SetFocusedRefractionMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, FocusedRefractionMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::FocusedDispersionMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"FocusedDispersionMultiplier", 1.0);
        return property;
    }

    double LiquidGlassInteraction::GetFocusedDispersionMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, FocusedDispersionMultiplierProperty());
    }

    void LiquidGlassInteraction::SetFocusedDispersionMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, FocusedDispersionMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::FocusedSaturationMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"FocusedSaturationMultiplier", 1.02);
        return property;
    }

    double LiquidGlassInteraction::GetFocusedSaturationMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, FocusedSaturationMultiplierProperty());
    }

    void LiquidGlassInteraction::SetFocusedSaturationMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, FocusedSaturationMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::FocusedContrastMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"FocusedContrastMultiplier", 1.03);
        return property;
    }

    double LiquidGlassInteraction::GetFocusedContrastMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, FocusedContrastMultiplierProperty());
    }

    void LiquidGlassInteraction::SetFocusedContrastMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, FocusedContrastMultiplierProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::FocusedExposureBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"FocusedExposureBoost", 0.0);
        return property;
    }

    double LiquidGlassInteraction::GetFocusedExposureBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, FocusedExposureBoostProperty());
    }

    void LiquidGlassInteraction::SetFocusedExposureBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, FocusedExposureBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::FocusedTintBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"FocusedTintBoost", 0.08);
        return property;
    }

    double LiquidGlassInteraction::GetFocusedTintBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, FocusedTintBoostProperty());
    }

    void LiquidGlassInteraction::SetFocusedTintBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, FocusedTintBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::FocusedHighlightBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"FocusedHighlightBoost", 0.03);
        return property;
    }

    double LiquidGlassInteraction::GetFocusedHighlightBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, FocusedHighlightBoostProperty());
    }

    void LiquidGlassInteraction::SetFocusedHighlightBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, FocusedHighlightBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::FocusedInnerShadowBoostProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"FocusedInnerShadowBoost", 0.0);
        return property;
    }

    double LiquidGlassInteraction::GetFocusedInnerShadowBoost(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, FocusedInnerShadowBoostProperty());
    }

    void LiquidGlassInteraction::SetFocusedInnerShadowBoost(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, FocusedInnerShadowBoostProperty(), value);
    }

    Xaml::DependencyProperty LiquidGlassInteraction::ActiveMagnificationMultiplierProperty()
    {
        static auto const property = RegisterAttachedProperty<double>(L"ActiveMagnificationMultiplier", 2.0);
        return property;
    }

    double LiquidGlassInteraction::GetActiveMagnificationMultiplier(Xaml::DependencyObject const& element)
    {
        return GetAttachedValue<double>(element, ActiveMagnificationMultiplierProperty());
    }

    void LiquidGlassInteraction::SetActiveMagnificationMultiplier(Xaml::DependencyObject const& element, double value)
    {
        SetAttachedValue(element, ActiveMagnificationMultiplierProperty(), value);
    }
}

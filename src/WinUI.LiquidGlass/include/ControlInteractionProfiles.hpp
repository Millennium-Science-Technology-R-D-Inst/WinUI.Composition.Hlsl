#pragma once

namespace winrt::WinUI::LiquidGlass::detail
{
    enum class MotionProfile
    {
        Slider,
        Switch,
        Search,
        Magnifier
    };

    template<typename Self>
    void SetNeutralTransientOptics(Self* self)
    {
        self->SetValue(implementation::LiquidGlassInteraction::ActivatedRefractionMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::ActivatedDispersionMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::ActivatedSaturationMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::ActivatedContrastMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::ActivatedTintBoostProperty(), box_value(0.0));
        self->SetValue(implementation::LiquidGlassInteraction::ActivatedHighlightMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::ActivatedInnerShadowBoostProperty(), box_value(0.0));

        self->SetValue(implementation::LiquidGlassInteraction::PointerOverBlurBoostProperty(), box_value(0.0));
        self->SetValue(implementation::LiquidGlassInteraction::PointerOverRefractionMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::PointerOverRefractionBoostProperty(), box_value(0.0));
        self->SetValue(implementation::LiquidGlassInteraction::PointerOverDispersionMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::PointerOverSaturationMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::PointerOverContrastMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::PointerOverExposureBoostProperty(), box_value(0.0));
        self->SetValue(implementation::LiquidGlassInteraction::PointerOverTintBoostProperty(), box_value(0.0));
        self->SetValue(implementation::LiquidGlassInteraction::PointerOverHighlightMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::PointerOverHighlightBoostProperty(), box_value(0.0));
        self->SetValue(implementation::LiquidGlassInteraction::PointerOverInnerShadowBoostProperty(), box_value(0.0));

        self->SetValue(implementation::LiquidGlassInteraction::PressedBlurBoostProperty(), box_value(0.0));
        self->SetValue(implementation::LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(0.0));
        self->SetValue(implementation::LiquidGlassInteraction::PressedDispersionMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::PressedSaturationMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::PressedContrastMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::PressedExposureBoostProperty(), box_value(0.0));
        self->SetValue(implementation::LiquidGlassInteraction::PressedTintBoostProperty(), box_value(0.0));
        self->SetValue(implementation::LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.0));
        self->SetValue(implementation::LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(0.0));
        self->SetValue(implementation::LiquidGlassInteraction::PressedInnerShadowBoostProperty(), box_value(0.0));
    }

    template<typename Self, MotionProfile Profile>
    class MotionDefaults
    {
    public:
        MotionDefaults()
        {
            auto self = static_cast<Self*>(this);
            if constexpr (Profile != MotionProfile::Search)
            {
                SetNeutralTransientOptics(self);
            }

            if constexpr (Profile == MotionProfile::Slider)
            {
                // stiffness=2000, damping=80, mass=1
                self->SetValue(implementation::LiquidGlassInteraction::SpringDampingRatioProperty(), box_value(.8944271909999159));
                self->SetValue(implementation::LiquidGlassInteraction::SpringPeriodProperty(), box_value(140.49629462081452));
            }
            else if constexpr (Profile == MotionProfile::Switch)
            {
                // stiffness=1000, damping=80, mass=1
                self->SetValue(implementation::LiquidGlassInteraction::SpringDampingRatioProperty(), box_value(1.2649110640673518));
                self->SetValue(implementation::LiquidGlassInteraction::SpringPeriodProperty(), box_value(198.69176531592203));
            }
            else if constexpr (Profile == MotionProfile::Search)
            {
                // Native input focus needs less latency than the browser demo while keeping
                // the same .8 -> 1.0 -> .99 target model.
                self->SetValue(implementation::LiquidGlassInteraction::SpringDampingRatioProperty(), box_value(.82));
                self->SetValue(implementation::LiquidGlassInteraction::SpringPeriodProperty(), box_value(150.0));
            }
            else
            {
                // Magnifier body spring: stiffness=340, damping=20, mass=1.
                self->SetValue(implementation::LiquidGlassInteraction::SpringDampingRatioProperty(), box_value(.5423261445466404));
                self->SetValue(implementation::LiquidGlassInteraction::SpringPeriodProperty(), box_value(340.7636269136104));
            }
        }
    };
}

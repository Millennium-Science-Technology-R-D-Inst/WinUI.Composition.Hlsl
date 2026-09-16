#pragma once

namespace winrt::WinUI::LiquidGlass::detail
{
    enum class CompactControlMotionProfile
    {
        Button,
        Choice,
        Selector
    };

    template<typename Self, CompactControlMotionProfile Profile>
    class CompactControlMotionDefaults
    {
    public:
        CompactControlMotionDefaults()
        {
            auto self = static_cast<Self*>(this);

            // Kube's interactive examples are deliberately stiff and highly damped: input
            // is acknowledged immediately and settles without a long elastic tail. Standard
            // WinUI controls keep that timing character while restraining whole-control drift.
            self->SetValue(implementation::LiquidGlassInteraction::UseSpringMotionProperty(), box_value(false));

            if constexpr (Profile == CompactControlMotionProfile::Choice)
            {
                // Only the 20x20 glass glyph scales, so it can carry a stronger tactile pulse
                // than a full text/content control without making the label jump.
                self->SetValue(implementation::LiquidGlassInteraction::PointerOverScaleProperty(), box_value(1.04));
                self->SetValue(implementation::LiquidGlassInteraction::PressedScaleProperty(), box_value(.94));
                self->SetValue(implementation::LiquidGlassInteraction::MotionDurationProperty(), box_value(85.0));
                self->SetValue(implementation::LiquidGlassInteraction::OpticsTransitionDurationProperty(), box_value(90.0));
                self->SetValue(implementation::LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(1.12));
                self->SetValue(implementation::LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(.4));
                self->SetValue(implementation::LiquidGlassInteraction::PressedDispersionMultiplierProperty(), box_value(1.05));
                self->SetValue(implementation::LiquidGlassInteraction::PressedTintBoostProperty(), box_value(.06));
                self->SetValue(implementation::LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.10));
                self->SetValue(implementation::LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(.02));
                self->SetValue(implementation::LiquidGlassInteraction::PressedInnerShadowBoostProperty(), box_value(.03));
                return;
            }

            if constexpr (Profile == CompactControlMotionProfile::Selector)
            {
                self->SetValue(implementation::LiquidGlassInteraction::PointerOverScaleProperty(), box_value(1.018));
                self->SetValue(implementation::LiquidGlassInteraction::PressedScaleProperty(), box_value(.965));
                self->SetValue(implementation::LiquidGlassInteraction::MotionDurationProperty(), box_value(100.0));
                self->SetValue(implementation::LiquidGlassInteraction::OpticsTransitionDurationProperty(), box_value(95.0));
                self->SetValue(implementation::LiquidGlassInteraction::ElasticityProperty(), box_value(.05));
                self->SetValue(implementation::LiquidGlassInteraction::PointerDisplacementProperty(), box_value(.8));
                self->SetValue(implementation::LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(1.10));
                self->SetValue(implementation::LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(0.0));
                self->SetValue(implementation::LiquidGlassInteraction::PressedDispersionMultiplierProperty(), box_value(1.06));
                self->SetValue(implementation::LiquidGlassInteraction::PressedSaturationMultiplierProperty(), box_value(1.02));
                self->SetValue(implementation::LiquidGlassInteraction::PressedContrastMultiplierProperty(), box_value(1.02));
                self->SetValue(implementation::LiquidGlassInteraction::PressedTintBoostProperty(), box_value(.045));
                self->SetValue(implementation::LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.10));
                self->SetValue(implementation::LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(0.0));
                return;
            }

            self->SetValue(implementation::LiquidGlassInteraction::PointerOverScaleProperty(), box_value(1.018));
            self->SetValue(implementation::LiquidGlassInteraction::PressedScaleProperty(), box_value(.965));
            self->SetValue(implementation::LiquidGlassInteraction::MotionDurationProperty(), box_value(90.0));
            self->SetValue(implementation::LiquidGlassInteraction::OpticsTransitionDurationProperty(), box_value(90.0));
            self->SetValue(implementation::LiquidGlassInteraction::ElasticityProperty(), box_value(.06));
            self->SetValue(implementation::LiquidGlassInteraction::PointerDisplacementProperty(), box_value(1.0));
            self->SetValue(implementation::LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(1.12));
            self->SetValue(implementation::LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(.75));
            self->SetValue(implementation::LiquidGlassInteraction::PressedDispersionMultiplierProperty(), box_value(1.05));
            self->SetValue(implementation::LiquidGlassInteraction::PressedTintBoostProperty(), box_value(.05));
            self->SetValue(implementation::LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.08));
            self->SetValue(implementation::LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(.015));
            self->SetValue(implementation::LiquidGlassInteraction::PressedInnerShadowBoostProperty(), box_value(.025));
        }
    };
}

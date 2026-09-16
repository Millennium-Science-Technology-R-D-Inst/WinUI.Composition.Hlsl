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

            // Kube's interactive examples are deliberately stiff and highly damped: the
            // pointer is acknowledged immediately and the surface settles without a long
            // elastic tail. Standard WinUI controls use the same timing character here,
            // but with restrained whole-control displacement so text/content stays stable.
            self->SetValue(implementation::LiquidGlassInteraction::UseSpringMotionProperty(), box_value(false));

            if constexpr (Profile == CompactControlMotionProfile::Choice)
            {
                // The 20x20 glass glyph can carry more scale than a full content control.
                self->SetValue(implementation::LiquidGlassInteraction::PointerOverScaleProperty(), box_value(1.04));
                self->SetValue(implementation::LiquidGlassInteraction::PressedScaleProperty(), box_value(.94));
                self->SetValue(implementation::LiquidGlassInteraction::MotionDurationProperty(), box_value(85.0));
                self->SetValue(implementation::LiquidGlassInteraction::OpticsTransitionDurationProperty(), box_value(90.0));
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
                return;
            }

            self->SetValue(implementation::LiquidGlassInteraction::PointerOverScaleProperty(), box_value(1.018));
            self->SetValue(implementation::LiquidGlassInteraction::PressedScaleProperty(), box_value(.965));
            self->SetValue(implementation::LiquidGlassInteraction::MotionDurationProperty(), box_value(90.0));
            self->SetValue(implementation::LiquidGlassInteraction::OpticsTransitionDurationProperty(), box_value(90.0));
            self->SetValue(implementation::LiquidGlassInteraction::ElasticityProperty(), box_value(.06));
            self->SetValue(implementation::LiquidGlassInteraction::PointerDisplacementProperty(), box_value(1.0));
        }
    };
}

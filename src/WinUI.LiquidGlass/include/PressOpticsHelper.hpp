#pragma once

namespace winrt::WinUI::LiquidGlass::detail
{
    enum class PersistentOpticsKind
    {
        None,
        Toggle,
        Selector
    };

    struct OpticsSnapshot
    {
        WinUI::Composition::Hlsl::LiquidGlassBrush brush{ nullptr };
        double blur{};
        double refraction{};
        double dispersion{};
        double saturation{};
        double contrast{};
        double exposure{};
        double tintOpacity{};
        double highlight{};
        double innerShadow{};
        bool active{};
    };

    inline void CaptureOptics(
        WinUI::Composition::Hlsl::LiquidGlassBrush const& brush,
        OpticsSnapshot& state)
    {
        state.brush = brush;
        state.blur = brush.BlurRadius();
        state.refraction = brush.RefractionStrength();
        state.dispersion = brush.DispersionStrength();
        state.saturation = brush.Saturation();
        state.contrast = brush.Contrast();
        state.exposure = brush.Exposure();
        state.tintOpacity = brush.TintOpacity();
        state.highlight = brush.HighlightStrength();
        state.innerShadow = brush.InnerShadowStrength();
        state.active = true;
    }

    inline void ApplySnapshot(OpticsSnapshot const& state)
    {
        if (!state.active || !state.brush) return;
        state.brush.BlurRadius(state.blur);
        state.brush.RefractionStrength(state.refraction);
        state.brush.DispersionStrength(state.dispersion);
        state.brush.Saturation(state.saturation);
        state.brush.Contrast(state.contrast);
        state.brush.Exposure(state.exposure);
        state.brush.TintOpacity(state.tintOpacity);
        state.brush.HighlightStrength(state.highlight);
        state.brush.InnerShadowStrength(state.innerShadow);
    }

    inline void RestoreOptics(OpticsSnapshot& state)
    {
        if (!state.active) return;
        ApplySnapshot(state);
        state.brush = nullptr;
        state.active = false;
    }

    inline void ApplyOpticsDelta(
        WinUI::Composition::Hlsl::LiquidGlassBrush const& brush,
        double blurBoost,
        double refractionMultiplier,
        double refractionBoost,
        double dispersionMultiplier,
        double saturationMultiplier,
        double contrastMultiplier,
        double exposureBoost,
        double tintBoost,
        double highlightMultiplier,
        double highlightBoost,
        double shadowBoost)
    {
        if (!brush) return;
        brush.BlurRadius(std::clamp(brush.BlurRadius() + blurBoost, 0.0, 64.0));
        brush.RefractionStrength(std::clamp(
            brush.RefractionStrength() * refractionMultiplier + refractionBoost, 0.0, 128.0));
        brush.DispersionStrength(std::clamp(
            brush.DispersionStrength() * dispersionMultiplier, 0.0, 16.0));
        brush.Saturation(std::clamp(
            brush.Saturation() * saturationMultiplier, 0.0, 4.0));
        brush.Contrast(std::clamp(
            brush.Contrast() * contrastMultiplier, 0.0, 4.0));
        brush.Exposure(std::clamp(brush.Exposure() + exposureBoost, -4.0, 4.0));
        brush.TintOpacity(std::clamp(brush.TintOpacity() + tintBoost, 0.0, 1.0));
        brush.HighlightStrength(std::clamp(
            brush.HighlightStrength() * highlightMultiplier + highlightBoost, 0.0, 4.0));
        brush.InnerShadowStrength(std::clamp(
            brush.InnerShadowStrength() + shadowBoost, 0.0, 1.0));
    }

    inline void ApplyActivatedOptics(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
    {
        ApplyOpticsDelta(
            brush,
            0.0,
            std::clamp(implementation::LiquidGlassInteraction::GetActivatedRefractionMultiplier(owner), 0.0, 4.0),
            0.0,
            std::clamp(implementation::LiquidGlassInteraction::GetActivatedDispersionMultiplier(owner), 0.0, 8.0),
            std::clamp(implementation::LiquidGlassInteraction::GetActivatedSaturationMultiplier(owner), 0.0, 8.0),
            std::clamp(implementation::LiquidGlassInteraction::GetActivatedContrastMultiplier(owner), 0.0, 8.0),
            0.0,
            std::clamp(implementation::LiquidGlassInteraction::GetActivatedTintBoost(owner), -1.0, 1.0),
            std::clamp(implementation::LiquidGlassInteraction::GetActivatedHighlightMultiplier(owner), 0.0, 4.0),
            0.0,
            std::clamp(implementation::LiquidGlassInteraction::GetActivatedInnerShadowBoost(owner), -1.0, 1.0));
    }

    inline void ApplyPointerOverOptics(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
    {
        ApplyOpticsDelta(
            brush,
            std::clamp(implementation::LiquidGlassInteraction::GetPointerOverBlurBoost(owner), -64.0, 64.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPointerOverRefractionMultiplier(owner), 0.0, 4.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPointerOverRefractionBoost(owner), -128.0, 128.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPointerOverDispersionMultiplier(owner), 0.0, 8.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPointerOverSaturationMultiplier(owner), 0.0, 8.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPointerOverContrastMultiplier(owner), 0.0, 8.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPointerOverExposureBoost(owner), -8.0, 8.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPointerOverTintBoost(owner), -1.0, 1.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPointerOverHighlightMultiplier(owner), 0.0, 4.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPointerOverHighlightBoost(owner), -4.0, 4.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPointerOverInnerShadowBoost(owner), -1.0, 1.0));
    }

    inline void ApplyPressedOptics(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
    {
        ApplyOpticsDelta(
            brush,
            std::clamp(implementation::LiquidGlassInteraction::GetPressedBlurBoost(owner), -64.0, 64.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPressedRefractionMultiplier(owner), 0.0, 4.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPressedRefractionBoost(owner), -128.0, 128.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPressedDispersionMultiplier(owner), 0.0, 8.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPressedSaturationMultiplier(owner), 0.0, 8.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPressedContrastMultiplier(owner), 0.0, 8.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPressedExposureBoost(owner), -8.0, 8.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPressedTintBoost(owner), -1.0, 1.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPressedHighlightMultiplier(owner), 0.0, 4.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPressedHighlightBoost(owner), -4.0, 4.0),
            std::clamp(implementation::LiquidGlassInteraction::GetPressedInnerShadowBoost(owner), -1.0, 1.0));
    }

    inline void EnterPointerOverOptics(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        WinUI::Composition::Hlsl::LiquidGlassBrush const& brush,
        OpticsSnapshot& state)
    {
        if (state.active || !owner || !brush) return;
        CaptureOptics(brush, state);
        ApplyPointerOverOptics(owner, brush);
    }

    inline void LeavePointerOverOptics(OpticsSnapshot& state)
    {
        RestoreOptics(state);
    }

    inline void EnterPressedOptics(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        WinUI::Composition::Hlsl::LiquidGlassBrush const& brush,
        OpticsSnapshot& state)
    {
        if (state.active || !owner || !brush) return;
        CaptureOptics(brush, state);
        ApplyPressedOptics(owner, brush);
    }

    inline void LeavePressedOptics(OpticsSnapshot& state)
    {
        RestoreOptics(state);
    }

    template<typename Self, PersistentOpticsKind PersistentKind = PersistentOpticsKind::None>
    class PressOpticsHelper
    {
    public:
        PressOpticsHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const& sender, auto const&)
            {
                InitializePersistentState(sender);
            });
            self->PointerEntered([this](auto const& sender, auto const&)
            {
                m_pointerOver = true;
                Recompute(sender);
            });
            self->PointerExited([this](auto const& sender, auto const&)
            {
                m_pointerOver = false;
                Recompute(sender);
            });
            self->PointerPressed([this](auto const& sender, auto const&)
            {
                m_pressed = true;
                Recompute(sender);
            });
            self->PointerReleased([this](auto const& sender, auto const&)
            {
                m_pressed = false;
                Recompute(sender);
            });
            self->PointerCaptureLost([this](auto const& sender, auto const&)
            {
                m_pressed = false;
                Recompute(sender);
            });
            self->PointerCanceled([this](auto const& sender, auto const&)
            {
                m_pressed = false;
                Recompute(sender);
            });

            if constexpr (PersistentKind == PersistentOpticsKind::Toggle)
            {
                self->Checked([this](auto const& sender, auto const&)
                {
                    SetActivated(sender, true);
                });
                self->Unchecked([this](auto const& sender, auto const&)
                {
                    SetActivated(sender, false);
                });
                self->Indeterminate([this](auto const& sender, auto const&)
                {
                    SetActivated(sender, true);
                });
            }
            else if constexpr (PersistentKind == PersistentOpticsKind::Selector)
            {
                self->RegisterPropertyChangedCallback(
                    Microsoft::UI::Xaml::Controls::Primitives::SelectorItem::IsSelectedProperty(),
                    [this](Microsoft::UI::Xaml::DependencyObject const& sender,
                           Microsoft::UI::Xaml::DependencyProperty const&)
                    {
                        auto item = sender.try_as<Microsoft::UI::Xaml::Controls::Primitives::SelectorItem>();
                        SetActivated(sender, item && item.IsSelected());
                    });
            }
        }

    private:
        template<typename Sender>
        void InitializePersistentState(Sender const& sender)
        {
            if constexpr (PersistentKind == PersistentOpticsKind::Toggle)
            {
                auto self = static_cast<Self*>(this);
                auto value = self->IsChecked();
                m_activated = !value || value.Value();
            }
            else if constexpr (PersistentKind == PersistentOpticsKind::Selector)
            {
                m_activated = static_cast<Self*>(this)->IsSelected();
            }
            Recompute(sender);
        }

        template<typename Sender>
        void SetActivated(Sender const& sender, bool value)
        {
            m_activated = value;
            Recompute(sender);
        }

        template<typename Sender>
        void Recompute(Sender const& sender)
        {
            auto object = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto owner = static_cast<Self*>(this);
            auto brush = owner->GlassBrush();
            if (!object || !brush) return;

            auto const anyState = m_activated || m_pointerOver || m_pressed;
            if (!anyState)
            {
                RestoreOptics(m_baseline);
                return;
            }

            if (!m_baseline.active || get_abi(m_baseline.brush) != get_abi(brush))
            {
                if (m_baseline.active) RestoreOptics(m_baseline);
                CaptureOptics(brush, m_baseline);
            }
            else
            {
                ApplySnapshot(m_baseline);
            }

            if (m_activated) ApplyActivatedOptics(object, brush);
            if (m_pointerOver) ApplyPointerOverOptics(object, brush);
            if (m_pressed) ApplyPressedOptics(object, brush);
        }

        OpticsSnapshot m_baseline;
        bool m_activated{};
        bool m_pointerOver{};
        bool m_pressed{};
    };
}

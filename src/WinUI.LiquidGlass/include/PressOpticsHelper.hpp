#pragma once

#include "MotionAnimation.hpp"

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
        Microsoft::UI::Xaml::DependencyObject owner{ nullptr };
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
        state.owner = nullptr;
        state.active = false;
    }

    inline void AnimateOpticsScalar(
        WinUI::Composition::Hlsl::HlslEffectBrush const& effect,
        Microsoft::UI::Composition::CompositionEffectBrush const& compositionBrush,
        Microsoft::UI::Composition::CompositionEasingFunction const& easing,
        std::chrono::milliseconds duration,
        wchar_t const* propertyName,
        double from,
        double to)
    {
        if (!effect || !compositionBrush || std::abs(from - to) <= 1e-5) return;

        auto const path = effect.GetPropertyPath(hstring{ propertyName });
        compositionBrush.StopAnimation(path);

        auto animation = compositionBrush.Compositor().CreateScalarKeyFrameAnimation();
        animation.InsertKeyFrame(0.0f, static_cast<float>(from));
        animation.InsertKeyFrame(1.0f, static_cast<float>(to), easing);
        animation.Duration(duration);
        compositionBrush.StartAnimation(path, animation);
    }

    inline void AnimateOpticsTransition(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        WinUI::Composition::Hlsl::LiquidGlassBrush const& brush,
        OpticsSnapshot const& from)
    {
        if (!owner || !brush || !from.active || !MotionAnimationsEnabled(owner)) return;

        auto const durationMs = std::clamp(
            implementation::LiquidGlassInteraction::GetOpticsTransitionDuration(owner), 0.0, 2000.0);
        if (durationMs <= 0.0) return;

        auto material = brush.Material();
        if (!material) return;
        auto effect = material.EffectBrush();
        if (!effect) return;
        auto compositionBrush = effect.EffectBrush();
        if (!compositionBrush) return;

        auto compositor = compositionBrush.Compositor();
        auto easing = compositor.CreateCubicBezierEasingFunction(
            Windows::Foundation::Numerics::float2{ 0.20f, 0.0f },
            Windows::Foundation::Numerics::float2{ 0.0f, 1.0f });
        auto const duration = std::chrono::milliseconds{
            static_cast<int64_t>(std::lround(durationMs)) };

        AnimateOpticsScalar(effect, compositionBrush, easing, duration,
            L"RefractionStrength", from.refraction, brush.RefractionStrength());
        AnimateOpticsScalar(effect, compositionBrush, easing, duration,
            L"DispersionStrength", from.dispersion, brush.DispersionStrength());
        AnimateOpticsScalar(effect, compositionBrush, easing, duration,
            L"Saturation", from.saturation, brush.Saturation());
        AnimateOpticsScalar(effect, compositionBrush, easing, duration,
            L"Contrast", from.contrast, brush.Contrast());
        AnimateOpticsScalar(effect, compositionBrush, easing, duration,
            L"Exposure", from.exposure, brush.Exposure());
        AnimateOpticsScalar(effect, compositionBrush, easing, duration,
            L"TintOpacity", from.tintOpacity, brush.TintOpacity());
        AnimateOpticsScalar(effect, compositionBrush, easing, duration,
            L"HighlightStrength", from.highlight, brush.HighlightStrength());
        AnimateOpticsScalar(effect, compositionBrush, easing, duration,
            L"InnerShadowStrength", from.innerShadow, brush.InnerShadowStrength());
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
        state.owner = owner;
        ApplyPointerOverOptics(owner, brush);
        AnimateOpticsTransition(owner, brush, state);
    }

    inline void LeavePointerOverOptics(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        OpticsSnapshot& state)
    {
        if (!state.active || !state.brush) return;
        auto brush = state.brush;
        OpticsSnapshot from;
        CaptureOptics(brush, from);
        RestoreOptics(state);
        AnimateOpticsTransition(owner, brush, from);
    }

    inline void LeavePointerOverOptics(OpticsSnapshot& state)
    {
        if (!state.active || !state.brush) return;
        auto owner = state.owner;
        auto brush = state.brush;
        OpticsSnapshot from;
        CaptureOptics(brush, from);
        RestoreOptics(state);
        if (owner) AnimateOpticsTransition(owner, brush, from);
    }

    inline void EnterPressedOptics(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        WinUI::Composition::Hlsl::LiquidGlassBrush const& brush,
        OpticsSnapshot& state)
    {
        if (state.active || !owner || !brush) return;
        CaptureOptics(brush, state);
        state.owner = owner;
        ApplyPressedOptics(owner, brush);
        AnimateOpticsTransition(owner, brush, state);
    }

    inline void LeavePressedOptics(
        Microsoft::UI::Xaml::DependencyObject const& owner,
        OpticsSnapshot& state)
    {
        if (!state.active || !state.brush) return;
        auto brush = state.brush;
        OpticsSnapshot from;
        CaptureOptics(brush, from);
        RestoreOptics(state);
        AnimateOpticsTransition(owner, brush, from);
    }

    inline void LeavePressedOptics(OpticsSnapshot& state)
    {
        if (!state.active || !state.brush) return;
        auto owner = state.owner;
        auto brush = state.brush;
        OpticsSnapshot from;
        CaptureOptics(brush, from);
        RestoreOptics(state);
        if (owner) AnimateOpticsTransition(owner, brush, from);
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
                m_loaded = true;
                InitializePersistentState(sender);
            });
            self->Unloaded([this](auto const&, auto const&)
            {
                // Teardown is deliberately no-write: retain an active numeric baseline so a
                // later Loaded can restore/reapply it, but never touch a possibly closed
                // CompositionEffectBrush while the XAML tree is being dismantled.
                m_loaded = false;
                m_pointerOver = false;
                m_pressed = false;
            });

            auto bindPointerHandler = [self](
                auto routedEvent,
                Windows::Foundation::IInspectable& storage,
                auto&& callback)
            {
                using Handler = Microsoft::UI::Xaml::Input::PointerEventHandler;
                storage = winrt::box_value<Handler>({ std::forward<decltype(callback)>(callback) });
                self->AddHandler(routedEvent, storage, true);
            };

            // ButtonBase consumes press/release pointer events in its class handler. Optical
            // state must observe those handled events or derived native controls otherwise
            // keep the resting material while their visual state says Pressed.
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerEnteredEvent(),
                m_pointerEnteredHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    m_pointerOver = true;
                    Recompute(sender);
                });
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerExitedEvent(),
                m_pointerExitedHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    m_pointerOver = false;
                    Recompute(sender);
                });
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerPressedEvent(),
                m_pointerPressedHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    m_pressed = true;
                    Recompute(sender);
                });
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerReleasedEvent(),
                m_pointerReleasedHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    m_pressed = false;
                    Recompute(sender);
                });
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerCaptureLostEvent(),
                m_pointerCaptureLostHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    m_pressed = false;
                    Recompute(sender);
                });
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(),
                m_pointerCanceledHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    m_pressed = false;
                    Recompute(sender);
                });

            self->RegisterPropertyChangedCallback(
                Self::GlassBrushProperty(),
                [this](Microsoft::UI::Xaml::DependencyObject const& sender,
                       Microsoft::UI::Xaml::DependencyProperty const&)
                {
                    // A brush replacement is an ownership boundary. The previous brush
                    // may already have disconnected/closed its CompositionEffectBrush, so
                    // never restore the old baseline as part of this callback.
                    m_baseline = {};
                    if (m_loaded) Recompute(sender);
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
            if (m_loaded) Recompute(sender);
        }

        void RefreshPointerFieldConfiguration()
        {
            // Do not probe for this helper's own member name through Self. Because Self
            // inherits PressOpticsHelper, such a requires-expression succeeds even when
            // the derived control has no override, and the call resolves straight back
            // to this function (infinite recursion / stack overflow).
            //
            // Use a deliberately distinct opt-in hook name that only controls with an
            // additional pointer-field owner implement.
            if constexpr (requires(Self* value) { value->RefreshPressOpticsPointerFieldConfiguration(); })
            {
                static_cast<Self*>(this)->RefreshPressOpticsPointerFieldConfiguration();
            }
        }

        template<typename Sender>
        void Recompute(Sender const& sender)
        {
            if (!m_loaded) return;
            auto object = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!object) return;

            auto owner = static_cast<Self*>(this);
            auto brush = owner->GlassBrush();
            if (m_baseline.active && (!brush || get_abi(m_baseline.brush) != get_abi(brush)))
            {
                // The old brush is no longer owned by this control. Dropping the
                // snapshot is both sufficient and teardown-safe; restoring it would
                // write through a possibly closed CompositionEffectBrush.
                m_baseline = {};
            }
            if (!brush) return;

            auto const anyState = m_activated || m_pointerOver || m_pressed;
            if (!anyState)
            {
                if (!m_baseline.active) return;
                OpticsSnapshot from;
                CaptureOptics(brush, from);
                RestoreOptics(m_baseline);
                AnimateOpticsTransition(object, brush, from);
                RefreshPointerFieldConfiguration();
                return;
            }

            OpticsSnapshot from;
            CaptureOptics(brush, from);

            if (!m_baseline.active)
            {
                CaptureOptics(brush, m_baseline);
            }
            else
            {
                ApplySnapshot(m_baseline);
            }

            if (m_activated) ApplyActivatedOptics(object, brush);
            if (m_pointerOver) ApplyPointerOverOptics(object, brush);
            if (m_pressed) ApplyPressedOptics(object, brush);
            AnimateOpticsTransition(object, brush, from);
            RefreshPointerFieldConfiguration();
        }

        OpticsSnapshot m_baseline;
        Windows::Foundation::IInspectable m_pointerEnteredHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerExitedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerPressedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerReleasedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCaptureLostHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCanceledHandler{ nullptr };
        bool m_loaded{};
        bool m_activated{};
        bool m_pointerOver{};
        bool m_pressed{};
    };
}

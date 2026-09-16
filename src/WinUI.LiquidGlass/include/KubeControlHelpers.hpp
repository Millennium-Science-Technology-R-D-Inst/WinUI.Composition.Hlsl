#pragma once

#include "ChildSurfaceInteraction.hpp"
#include "PressOpticsHelper.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    enum class KubeMotionProfile
    {
        Slider,
        Switch,
        Search,
        Magnifier
    };

    template<typename Self>
    void SetKubeNeutralTransientOptics(Self* self)
    {
        // Kube's Slider/Switch/Magnifier filters change only the explicitly animated
        // quantities (not generic hover saturation/contrast/dispersion). Start these
        // profiles from neutral deltas and let each concrete constructor opt into the
        // refraction/tint/shadow values that exist in the reference component.
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

    template<typename Self, KubeMotionProfile Profile>
    class KubeMotionDefaults
    {
    public:
        KubeMotionDefaults()
        {
            auto self = static_cast<Self*>(this);
            if constexpr (Profile != KubeMotionProfile::Search)
            {
                SetKubeNeutralTransientOptics(self);
            }

            if constexpr (Profile == KubeMotionProfile::Slider)
            {
                // motion/react: stiffness=2000, damping=80, mass=1.
                self->SetValue(implementation::LiquidGlassInteraction::SpringDampingRatioProperty(), box_value(.8944271909999159));
                self->SetValue(implementation::LiquidGlassInteraction::SpringPeriodProperty(), box_value(140.49629462081452));
            }
            else if constexpr (Profile == KubeMotionProfile::Switch)
            {
                // motion/react: stiffness=1000, damping=80, mass=1.
                self->SetValue(implementation::LiquidGlassInteraction::SpringDampingRatioProperty(), box_value(1.2649110640673518));
                self->SetValue(implementation::LiquidGlassInteraction::SpringPeriodProperty(), box_value(198.69176531592203));
            }
            else if constexpr (Profile == KubeMotionProfile::Search)
            {
                // motion/react: stiffness=800, damping=40, mass=1.
                self->SetValue(implementation::LiquidGlassInteraction::SpringDampingRatioProperty(), box_value(.7071067811865475));
                self->SetValue(implementation::LiquidGlassInteraction::SpringPeriodProperty(), box_value(222.1441469079183));
            }
            else
            {
                // MagnifyingGlass scale spring: stiffness=340, damping=20, mass=1.
                self->SetValue(implementation::LiquidGlassInteraction::SpringDampingRatioProperty(), box_value(.5423261445466404));
                self->SetValue(implementation::LiquidGlassInteraction::SpringPeriodProperty(), box_value(340.7636269136104));
            }
        }
    };

    template<typename Self>
    class KubeSliderVisualHelper
    {
    public:
        KubeSliderVisualHelper()
        {
            auto self = static_cast<Self*>(this);
            ConfigureResources();
            self->Loaded([this](auto const&, auto const&) { RefreshVisual(); });
            self->Unloaded([this](auto const&, auto const&) { m_thumb = nullptr; });
            self->RegisterPropertyChangedCallback(
                Self::GlassBrushProperty(),
                [this](auto const&, auto const&)
                {
                    ConfigureResources();
                    RefreshVisual();
                });
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Slider::OrientationProperty(),
                [this](auto const&, auto const&) { RefreshVisual(); });
        }

        void RefreshVisual()
        {
            auto self = static_cast<Self*>(this);
            auto root = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!root) return;

            auto const horizontal =
                self->Orientation() == Microsoft::UI::Xaml::Controls::Orientation::Horizontal;
            auto const name = horizontal ? L"HorizontalThumb" : L"VerticalThumb";
            auto thumb = FindNamedDescendant(root, name)
                .try_as<Microsoft::UI::Xaml::Controls::Primitives::Thumb>();
            if (!thumb)
            {
                thumb = FindFirstDescendant<Microsoft::UI::Xaml::Controls::Primitives::Thumb>(root);
            }
            if (!thumb) return;

            m_thumb = thumb;
            thumb.Width(horizontal ? 90.0 : 60.0);
            thumb.Height(horizontal ? 60.0 : 90.0);
            thumb.CornerRadius({ 30.0, 30.0, 30.0, 30.0 });

            auto glass = self->GlassBrush();
            if (glass)
            {
                glass.CornerRadius(30.0);
                glass.BezelWidth(16.0);
            }

            // The WinUI SliderThumbStyle is a 12x12 ellipse nested in an outer Border.
            // Kube's lens is the full 90x60 rounded capsule, so render the liquid brush on
            // that outer surface and suppress only the decorative inner ellipse. Native
            // Slider drag, keyboard, UIA, focus engagement and value layout stay untouched.
            if (auto outer = FindFirstDescendant<Microsoft::UI::Xaml::Controls::Border>(thumb))
            {
                outer.Margin({ 0.0, 0.0, 0.0, 0.0 });
                outer.CornerRadius({ 30.0, 30.0, 30.0, 30.0 });
                outer.Background(glass
                    ? glass.as<Microsoft::UI::Xaml::Media::Brush>()
                    : Microsoft::UI::Xaml::Media::Brush{ nullptr });
                outer.BorderBrush(SolidBrush(0x33, 0xff, 0xff, 0xff));
                outer.BorderThickness({ 1.0, 1.0, 1.0, 1.0 });
            }

            if (auto inner = FindNamedDescendant(thumb, L"SliderInnerThumb")
                .try_as<Microsoft::UI::Xaml::Shapes::Ellipse>())
            {
                inner.Visibility(Microsoft::UI::Xaml::Visibility::Collapsed);
            }
        }

    private:
        static Microsoft::UI::Xaml::Media::SolidColorBrush SolidBrush(
            uint8_t alpha,
            uint8_t red,
            uint8_t green,
            uint8_t blue)
        {
            Microsoft::UI::Xaml::Media::SolidColorBrush brush;
            brush.Color({ alpha, red, green, blue });
            return brush;
        }

        void ConfigureResources()
        {
            auto self = static_cast<Self*>(this);
            auto const track = SolidBrush(0x66, 0x89, 0x89, 0x8f); // #89898F66
            auto const value = SolidBrush(0xff, 0x03, 0x77, 0xf7); // #0377F7
            auto const transparent = SolidBrush(0x00, 0x00, 0x00, 0x00);
            auto const stroke = SolidBrush(0x33, 0xff, 0xff, 0xff);

            self->Background(track);
            self->Foreground(value);
            self->CornerRadius({ 7.0, 7.0, 7.0, 7.0 });

            auto resources = self->Resources();
            auto insert = [&resources](wchar_t const* key, Windows::Foundation::IInspectable const& resourceValue)
            {
                if (resourceValue) resources.Insert(box_value(hstring{ key }), resourceValue);
            };

            auto glass = self->GlassBrush();
            if (glass)
            {
                auto mediaGlass = glass.as<Microsoft::UI::Xaml::Media::Brush>();
                insert(L"SliderThumbBackground", mediaGlass);
                insert(L"SliderThumbBackgroundPointerOver", mediaGlass);
                insert(L"SliderThumbBackgroundPressed", mediaGlass);
                insert(L"SliderThumbBackgroundDisabled", mediaGlass);
            }
            insert(L"SliderOuterThumbBackground", transparent);
            insert(L"SliderThumbBorderBrush", stroke);

            insert(L"SliderTrackFill", track);
            insert(L"SliderTrackFillPointerOver", track);
            insert(L"SliderTrackFillPressed", track);
            insert(L"SliderTrackFillDisabled", track);
            insert(L"SliderTrackValueFill", value);
            insert(L"SliderTrackValueFillPointerOver", value);
            insert(L"SliderTrackValueFillPressed", value);
            insert(L"SliderTrackValueFillDisabled", value);

            resources.Insert(box_value(L"SliderTrackThemeHeight"), box_value(14.0));
            resources.Insert(box_value(L"SliderHorizontalHeight"), box_value(60.0));
            resources.Insert(box_value(L"SliderVerticalWidth"), box_value(60.0));
            resources.Insert(
                box_value(L"SliderThumbCornerRadius"),
                box_value(Microsoft::UI::Xaml::CornerRadius{ 30.0, 30.0, 30.0, 30.0 }));
        }

        Microsoft::UI::Xaml::Controls::Primitives::Thumb m_thumb{ nullptr };
    };

    // Kube Searchbox combines focus and pointer-down from a single authored baseline:
    // background alpha is max(5% rest, 20% focused, 30% pressed), while scale is
    // (.8 or 1.0 when focused) * .99 during pointer-down. Keeping those states in one
    // helper avoids snapshot ordering bugs between independent focus and press helpers.
    template<typename Self>
    class KubeSearchInteractionHelper
    {
    public:
        KubeSearchInteractionHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const& sender, auto const&) { Recompute(sender, false); });
            self->Unloaded([this](auto const&, auto const&)
            {
                RestoreOptics(m_baseline);
                m_focused = false;
                m_pressed = false;
            });
            self->GotFocus([this](auto const& sender, auto const&)
            {
                m_focused = true;
                Recompute(sender, true);
            });
            self->LostFocus([this](auto const& sender, auto const&)
            {
                m_focused = false;
                Recompute(sender, true);
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
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerPressedEvent(),
                m_pointerPressedHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    m_pressed = true;
                    Recompute(sender, true);
                });
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerReleasedEvent(),
                m_pointerReleasedHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    m_pressed = false;
                    Recompute(sender, true);
                });
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerCaptureLostEvent(),
                m_pointerCaptureLostHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    m_pressed = false;
                    Recompute(sender, true);
                });
            bindPointerHandler(
                Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(),
                m_pointerCanceledHandler,
                [this](auto const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    m_pressed = false;
                    Recompute(sender, true);
                });

            self->RegisterPropertyChangedCallback(
                Self::GlassBrushProperty(),
                [this](Microsoft::UI::Xaml::DependencyObject const& sender, auto const&)
                {
                    if (m_baseline.active) RestoreOptics(m_baseline);
                    Recompute(sender, false);
                });
        }

    private:
        template<typename Sender>
        void Recompute(Sender const& sender, bool animate)
        {
            auto owner = sender.template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = sender.template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element) return;

            auto self = static_cast<Self*>(this);
            auto brush = self->GlassBrush();
            if (!brush) return;

            if (m_baseline.active && get_abi(m_baseline.brush) != get_abi(brush))
            {
                RestoreOptics(m_baseline);
            }

            auto const active = m_focused || m_pressed;
            OpticsSnapshot from;
            CaptureOptics(brush, from);

            if (!active)
            {
                if (m_baseline.active)
                {
                    RestoreOptics(m_baseline);
                    if (animate) AnimateOpticsTransition(owner, brush, from);
                }
            }
            else
            {
                if (!m_baseline.active)
                {
                    CaptureOptics(brush, m_baseline);
                }
                else
                {
                    ApplySnapshot(m_baseline);
                }

                auto const focusedBoost = m_focused
                    ? implementation::LiquidGlassInteraction::GetFocusedTintBoost(owner)
                    : 0.0;
                auto const pressedBoost = m_pressed
                    ? implementation::LiquidGlassInteraction::GetPressedTintBoost(owner)
                    : 0.0;
                brush.TintOpacity(std::clamp(
                    m_baseline.tintOpacity + std::max(focusedBoost, pressedBoost),
                    0.0,
                    1.0));
                if (animate) AnimateOpticsTransition(owner, brush, from);
            }

            auto scale = m_focused
                ? implementation::LiquidGlassInteraction::GetFocusedScale(owner)
                : implementation::LiquidGlassInteraction::GetRestScale(owner);
            if (m_pressed) scale *= .99;
            scale = std::clamp(scale, .25, 4.0);

            if (animate)
            {
                AnimateElementScale(
                    owner,
                    element,
                    scale,
                    scale,
                    implementation::LiquidGlassInteraction::GetMotionDuration(owner));
            }
            else
            {
                SetElementScale(element, scale, scale);
            }
        }

        OpticsSnapshot m_baseline;
        Windows::Foundation::IInspectable m_pointerPressedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerReleasedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCaptureLostHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCanceledHandler{ nullptr };
        bool m_focused{};
        bool m_pressed{};
    };
}

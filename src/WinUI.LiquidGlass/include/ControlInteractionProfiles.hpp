#pragma once

#include "ChildSurfaceInteraction.hpp"
#include "PressOpticsHelper.hpp"

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

    template<typename Self>
    class SliderVisualHelper
    {
    public:
        SliderVisualHelper()
        {
            auto self = static_cast<Self*>(this);
            ConfigureResources();
            self->Loaded([this](auto const&, auto const&) { RefreshVisual(); });
            self->Unloaded([this](auto const&, auto const&) { DetachProgressVisual(); m_thumb = nullptr; });
            self->RegisterPropertyChangedCallback(Self::GlassBrushProperty(), [this](auto const&, auto const&)
            {
                ConfigureResources();
                RefreshVisual();
            });
            self->RegisterPropertyChangedCallback(Microsoft::UI::Xaml::Controls::Slider::OrientationProperty(), [this](auto const&, auto const&)
            {
                RefreshVisual();
            });
            self->RegisterPropertyChangedCallback(Microsoft::UI::Xaml::Controls::Primitives::RangeBase::ValueProperty(), [this](auto const&, auto const&)
            {
                UpdateProgressVisual();
            });
            self->RegisterPropertyChangedCallback(Microsoft::UI::Xaml::Controls::Primitives::RangeBase::MinimumProperty(), [this](auto const&, auto const&)
            {
                UpdateProgressVisual();
            });
            self->RegisterPropertyChangedCallback(Microsoft::UI::Xaml::Controls::Primitives::RangeBase::MaximumProperty(), [this](auto const&, auto const&)
            {
                UpdateProgressVisual();
            });
        }

        void RefreshVisual()
        {
            auto self = static_cast<Self*>(this);
            auto root = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!root) return;

            auto const horizontal = self->Orientation() == Microsoft::UI::Xaml::Controls::Orientation::Horizontal;
            auto const thumbName = horizontal ? L"HorizontalThumb" : L"VerticalThumb";
            auto thumb = FindNamedDescendant(root, thumbName).try_as<Microsoft::UI::Xaml::Controls::Primitives::Thumb>();
            if (!thumb) thumb = FindFirstDescendant<Microsoft::UI::Xaml::Controls::Primitives::Thumb>(root);
            if (!thumb) return;

            m_thumb = thumb;
            thumb.Width(horizontal ? 90.0 : 60.0);
            thumb.Height(horizontal ? 60.0 : 90.0);
            thumb.Margin(horizontal
                ? Microsoft::UI::Xaml::Thickness{ -18.0, 0.0, -18.0, 0.0 }
                : Microsoft::UI::Xaml::Thickness{ 0.0, -18.0, 0.0, -18.0 });
            thumb.CornerRadius({ 30.0, 30.0, 30.0, 30.0 });

            auto glass = self->GlassBrush();
            if (glass)
            {
                glass.CornerRadius(30.0);
                glass.BezelWidth(16.0);
            }

            if (auto outer = FindFirstDescendant<Microsoft::UI::Xaml::Controls::Border>(thumb))
            {
                outer.Margin({ 0.0, 0.0, 0.0, 0.0 });
                outer.CornerRadius({ 30.0, 30.0, 30.0, 30.0 });
                outer.Background(glass ? glass.as<Microsoft::UI::Xaml::Media::Brush>() : Microsoft::UI::Xaml::Media::Brush{ nullptr });
                outer.BorderBrush(SolidBrush(0x33, 0xff, 0xff, 0xff));
                outer.BorderThickness({ 1.0, 1.0, 1.0, 1.0 });
            }

            if (auto inner = FindNamedDescendant(thumb, L"SliderInnerThumb").try_as<Microsoft::UI::Xaml::Shapes::Ellipse>())
            {
                inner.Visibility(Microsoft::UI::Xaml::Visibility::Collapsed);
            }

            auto const trackName = horizontal ? L"HorizontalTrackRect" : L"VerticalTrackRect";
            auto const decreaseName = horizontal ? L"HorizontalDecreaseRect" : L"VerticalDecreaseRect";
            auto track = FindNamedDescendant(root, trackName).try_as<Microsoft::UI::Xaml::Shapes::Rectangle>();
            auto decrease = FindNamedDescendant(root, decreaseName).try_as<Microsoft::UI::Xaml::Shapes::Rectangle>();
            if (decrease) decrease.Opacity(0.0);
            AttachProgressVisual(track, horizontal);
        }

    private:
        static Microsoft::UI::Xaml::Media::SolidColorBrush SolidBrush(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue)
        {
            Microsoft::UI::Xaml::Media::SolidColorBrush brush;
            brush.Color({ alpha, red, green, blue });
            return brush;
        }

        void ConfigureResources()
        {
            auto self = static_cast<Self*>(this);
            auto const track = SolidBrush(0x66, 0x89, 0x89, 0x8f);
            auto const value = SolidBrush(0xff, 0x03, 0x77, 0xf7);
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
            insert(L"SliderTrackValueFill", transparent);
            insert(L"SliderTrackValueFillPointerOver", transparent);
            insert(L"SliderTrackValueFillPressed", transparent);
            insert(L"SliderTrackValueFillDisabled", transparent);
            resources.Insert(box_value(L"SliderTrackThemeHeight"), box_value(14.0));
            resources.Insert(box_value(L"SliderHorizontalHeight"), box_value(60.0));
            resources.Insert(box_value(L"SliderVerticalWidth"), box_value(60.0));
            resources.Insert(box_value(L"SliderThumbCornerRadius"), box_value(Microsoft::UI::Xaml::CornerRadius{ 30.0, 30.0, 30.0, 30.0 }));
        }

        void AttachProgressVisual(Microsoft::UI::Xaml::Shapes::Rectangle const& track, bool horizontal)
        {
            if (!track)
            {
                DetachProgressVisual();
                return;
            }
            if (m_track && get_abi(m_track) == get_abi(track) && m_horizontal == horizontal)
            {
                UpdateProgressVisual();
                return;
            }

            DetachProgressVisual();
            m_track = track;
            m_horizontal = horizontal;
            auto host = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(track);
            auto compositor = host.Compositor();
            m_progressGeometry = compositor.CreateRoundedRectangleGeometry();
            m_progressGeometry.CornerRadius({ 7.0f, 7.0f });
            auto shape = compositor.CreateSpriteShape(m_progressGeometry);
            shape.FillBrush(compositor.CreateColorBrush({ 0xff, 0x03, 0x77, 0xf7 }));
            m_progressVisual = compositor.CreateShapeVisual();
            m_progressVisual.Shapes().Append(shape);
            Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetElementChildVisual(track, m_progressVisual);
            m_trackSizeToken = track.SizeChanged([this](auto const&, auto const&) { UpdateProgressVisual(); });
            UpdateProgressVisual();
        }

        void DetachProgressVisual()
        {
            if (m_track)
            {
                if (m_trackSizeToken.value) m_track.SizeChanged(m_trackSizeToken);
                Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetElementChildVisual(m_track, nullptr);
            }
            m_trackSizeToken = {};
            m_track = nullptr;
            m_progressVisual = nullptr;
            m_progressGeometry = nullptr;
        }

        void UpdateProgressVisual()
        {
            if (!m_track || !m_progressVisual || !m_progressGeometry) return;
            auto self = static_cast<Self*>(this);
            auto const minimum = self->Minimum();
            auto const maximum = self->Maximum();
            auto const range = maximum - minimum;
            auto const ratio = range > 1e-9 ? std::clamp((self->Value() - minimum) / range, 0.0, 1.0) : 0.0;
            auto const width = static_cast<float>(std::max(m_track.ActualWidth(), 0.0));
            auto const height = static_cast<float>(std::max(m_track.ActualHeight(), 0.0));
            m_progressVisual.Size({ width, height });
            if (m_horizontal)
            {
                m_progressGeometry.Size({ static_cast<float>(width * ratio), height });
                m_progressVisual.Offset({ 0.0f, 0.0f, 0.0f });
            }
            else
            {
                auto const progressHeight = static_cast<float>(height * ratio);
                m_progressGeometry.Size({ width, progressHeight });
                m_progressVisual.Offset({ 0.0f, height - progressHeight, 0.0f });
            }
        }

        Microsoft::UI::Xaml::Controls::Primitives::Thumb m_thumb{ nullptr };
        Microsoft::UI::Xaml::Shapes::Rectangle m_track{ nullptr };
        Microsoft::UI::Composition::ShapeVisual m_progressVisual{ nullptr };
        Microsoft::UI::Composition::CompositionRoundedRectangleGeometry m_progressGeometry{ nullptr };
        event_token m_trackSizeToken{};
        bool m_horizontal{ true };
    };

    template<typename Self>
    class SearchInteractionHelper
    {
    public:
        SearchInteractionHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const& sender, auto const&) { Recompute(sender, false); });
            self->Unloaded([this](auto const&, auto const&)
            {
                RestoreOptics(m_baseline);
                m_focused = false;
                m_pressed = false;
            });
            self->GotFocus([this](auto const& sender, auto const&) { m_focused = true; Recompute(sender, true); });
            self->LostFocus([this](auto const& sender, auto const&) { m_focused = false; Recompute(sender, true); });

            auto bind = [self](auto routedEvent, Windows::Foundation::IInspectable& storage, auto&& callback)
            {
                using Handler = Microsoft::UI::Xaml::Input::PointerEventHandler;
                storage = winrt::box_value<Handler>({ std::forward<decltype(callback)>(callback) });
                self->AddHandler(routedEvent, storage, true);
            };
            bind(Microsoft::UI::Xaml::UIElement::PointerPressedEvent(), m_pointerPressedHandler,
                [this](auto const& sender, auto const&) { m_pressed = true; Recompute(sender, true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerReleasedEvent(), m_pointerReleasedHandler,
                [this](auto const& sender, auto const&) { m_pressed = false; Recompute(sender, true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCaptureLostEvent(), m_pointerCaptureLostHandler,
                [this](auto const& sender, auto const&) { m_pressed = false; Recompute(sender, true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(), m_pointerCanceledHandler,
                [this](auto const& sender, auto const&) { m_pressed = false; Recompute(sender, true); });

            self->RegisterPropertyChangedCallback(Self::GlassBrushProperty(),
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

            if (m_baseline.active && get_abi(m_baseline.brush) != get_abi(brush)) RestoreOptics(m_baseline);
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
                if (!m_baseline.active) CaptureOptics(brush, m_baseline);
                else ApplySnapshot(m_baseline);
                auto const focusedBoost = m_focused ? implementation::LiquidGlassInteraction::GetFocusedTintBoost(owner) : 0.0;
                auto const pressedBoost = m_pressed ? implementation::LiquidGlassInteraction::GetPressedTintBoost(owner) : 0.0;
                brush.TintOpacity(std::clamp(m_baseline.tintOpacity + std::max(focusedBoost, pressedBoost), 0.0, 1.0));
                if (animate) AnimateOpticsTransition(owner, brush, from);
            }

            auto scale = m_focused
                ? implementation::LiquidGlassInteraction::GetFocusedScale(owner)
                : implementation::LiquidGlassInteraction::GetRestScale(owner);
            if (m_pressed) scale *= .99;
            scale = std::clamp(scale, .25, 4.0);
            if (animate)
            {
                AnimateElementScale(owner, element, scale, scale, implementation::LiquidGlassInteraction::GetMotionDuration(owner));
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

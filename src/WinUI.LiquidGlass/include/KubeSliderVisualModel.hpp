#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>

#include "ChildSurfaceInteraction.hpp"
#include "MotionAnimation.hpp"
#include "PressOpticsHelper.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self>
    class KubeSliderVisualModel
    {
    public:
        KubeSliderVisualModel()
        {
            auto self = static_cast<Self*>(this);
            ConfigureResources();

            self->Loaded([this](auto const&, auto const&)
            {
                m_loaded = true;
                RestorePendingOptics();
                RefreshVisual();
            });
            self->Unloaded([this](auto const&, auto const&)
            {
                m_loaded = false;
                DetachInitialLayoutSync();
                m_pointerField.Detach(false);
                if (m_scaleTimer)
                {
                    m_scaleTimer.Stop();
                    m_scaleTimer = nullptr;
                }
                m_currentScale = kRestScale;
                m_targetScale = kRestScale;
                m_scaleVelocity = 0.0;
                m_opticsInitialized = false;
                m_refractionVelocity = 0.0;
                m_tintVelocity = 0.0;
                m_thumb = nullptr;
                m_surface = nullptr;
                m_visualHost = nullptr;
                m_templateHost = nullptr;
                m_track = nullptr;
                m_decrease = nullptr;
                m_progressVisual = nullptr;
                m_progressGeometry = nullptr;
                m_progressShape = nullptr;
                m_pressed = false;
            });

            self->RegisterPropertyChangedCallback(Self::GlassBrushProperty(), [this](auto const&, auto const&)
            {
                if (!m_loaded) return;
                auto self = static_cast<Self*>(this);
                auto current = self->GlassBrush();
                if (m_pressOptics.active &&
                    (!current || !m_pressOptics.brush || get_abi(current) != get_abi(m_pressOptics.brush)))
                {
                    m_pressOptics = {};
                    m_pressed = false;
                }
                m_opticsInitialized = false;
                ApplyBrush();
                SyncDynamicOpticsFromBrush();
                ApplyInteractionState(false);
                RefreshPointerField();
            });
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Slider::OrientationProperty(),
                [this](auto const&, auto const&) { if (m_loaded) RefreshVisual(); });
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Slider::IsDirectionReversedProperty(),
                [this](auto const&, auto const&) { if (m_loaded) RefreshVisual(); });

            auto updateValue = [this](auto const&, auto const&)
            {
                if (!m_loaded) return;
                UpdateProgressVisual();
                // The glass lens is a sibling overlay, so its position can follow Value
                // immediately instead of waiting for the native Thumb's next layout pass.
                UpdateLensPosition(DisplayRatio(NormalizedValue()));
            };
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Primitives::RangeBase::ValueProperty(), updateValue);
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Primitives::RangeBase::MinimumProperty(), updateValue);
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Primitives::RangeBase::MaximumProperty(), updateValue);

            self->SizeChanged([this](auto const&, auto const&)
            {
                if (!m_loaded) return;
                UpdateProgressVisual();
                UpdateLensPosition(DisplayRatio(NormalizedValue()));
            });

            auto bind = [self](auto routedEvent, Windows::Foundation::IInspectable& storage, auto&& callback)
            {
                using Handler = Microsoft::UI::Xaml::Input::PointerEventHandler;
                storage = winrt::box_value<Handler>({ std::forward<decltype(callback)>(callback) });
                self->AddHandler(routedEvent, storage, true);
            };
            bind(Microsoft::UI::Xaml::UIElement::PointerPressedEvent(), m_pointerPressedHandler,
                [this](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args) { BeginPress(args); });
            bind(Microsoft::UI::Xaml::UIElement::PointerReleasedEvent(), m_pointerReleasedHandler,
                [this](auto const&, auto const&) { EndPress(true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCaptureLostEvent(), m_pointerCaptureLostHandler,
                [this](auto const&, auto const&) { EndPress(true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(), m_pointerCanceledHandler,
                [this](auto const&, auto const&) { EndPress(true); });
        }

        void RefreshVisual()
        {
            if (!m_loaded) return;
            ResolveTemplateParts();
            ApplyBrush();
            SyncDynamicOpticsFromBrush();
            UpdateProgressVisual();
            UpdateLensPosition(DisplayRatio(NormalizedValue()));
            ApplyInteractionState(false);
            RefreshPointerField();
        }

        void RefreshPointerFieldConfiguration()
        {
            // Slider uses Kube's authored global displacement field only. PointerField
            // remains a local specular reveal and does not add a second refractive field.
            m_pointerField.SetConfigurationScales(0.0, 0.0, 0.0);
        }

    private:
        static constexpr double kSemanticThumbExtent = 18.0;
        static constexpr double kVisualWidth = 90.0;
        static constexpr double kVisualHeight = 60.0;
        using Clock = std::chrono::steady_clock;
        static constexpr double kRestScale = 0.6;
        static constexpr double kScaleStiffness = 2000.0;
        static constexpr double kScaleDamping = 80.0;
        // Kube Slider.tsx uses an explicit 2000/80 spring for geometry and white
        // body opacity, but scaleRatio uses Motion's default spring (100/10).
        static constexpr double kRefractionStiffness = 100.0;
        static constexpr double kRefractionDamping = 10.0;
        static constexpr double kBodyOpacityStiffness = 2000.0;
        static constexpr double kBodyOpacityDamping = 80.0;
        static constexpr auto kScaleInterval = std::chrono::milliseconds{ 16 };

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

        void WriteDynamicOptics()
        {
            if (!m_loaded || !m_opticsInitialized) return;
            auto self = static_cast<Self*>(this);
            auto brush = self->GlassBrush();
            auto material = brush ? brush.Material() : WinUI::Composition::Hlsl::LiquidGlassMaterial{ nullptr };
            auto effect = material ? material.EffectBrush() : WinUI::Composition::Hlsl::HlslEffectBrush{ nullptr };
            if (!effect) return;

            try
            {
                effect.SetFloat(L"RefractionStrength", static_cast<float>(std::clamp(m_currentRefraction, 0.0, 128.0)));
                effect.SetFloat(L"TintOpacity", static_cast<float>(std::clamp(m_currentTintOpacity, 0.0, 1.0)));
            }
            catch (winrt::hresult_error const& error)
            {
                if (error.code() != winrt::hresult{ RO_E_CLOSED }) throw;
            }
        }

        void SyncDynamicOpticsFromBrush()
        {
            if (!m_loaded || m_pressOptics.active) return;
            auto self = static_cast<Self*>(this);
            auto brush = self->GlassBrush();
            if (!brush) return;

            m_currentRefraction = brush.RefractionStrength();
            m_targetRefraction = m_currentRefraction;
            m_refractionVelocity = 0.0;
            m_currentTintOpacity = brush.TintOpacity();
            m_targetTintOpacity = m_currentTintOpacity;
            m_tintVelocity = 0.0;
            m_opticsInitialized = true;
            WriteDynamicOptics();
        }

        static void RestoreSliderOptics(OpticsSnapshot& state)
        {
            if (!state.active || !state.brush) return;
            state.brush.RefractionStrength(state.refraction);
            state.brush.DispersionStrength(state.dispersion);
            state.brush.TintOpacity(state.tintOpacity);
            state.brush.HighlightStrength(state.highlight);
            state.brush.InnerShadowStrength(state.innerShadow);
            state.brush = nullptr;
            state.owner = nullptr;
            state.active = false;
        }

        void EnterSliderPressedOptics(
            Microsoft::UI::Xaml::DependencyObject const& owner,
            WinUI::Composition::Hlsl::LiquidGlassBrush const& brush)
        {
            if (m_pressOptics.active || !owner || !brush) return;

            CaptureOptics(brush, m_pressOptics);
            m_pressOptics.owner = owner;

            if (!m_opticsInitialized)
            {
                m_currentRefraction = m_pressOptics.refraction;
                m_targetRefraction = m_currentRefraction;
                m_refractionVelocity = 0.0;
                m_currentTintOpacity = m_pressOptics.tintOpacity;
                m_targetTintOpacity = m_currentTintOpacity;
                m_tintVelocity = 0.0;
                m_opticsInitialized = true;
            }

            m_targetRefraction = std::clamp(
                m_pressOptics.refraction * std::clamp(
                    implementation::LiquidGlassInteraction::GetPressedRefractionMultiplier(owner), 0.0, 4.0) +
                std::clamp(implementation::LiquidGlassInteraction::GetPressedRefractionBoost(owner), -128.0, 128.0),
                0.0,
                128.0);
            m_targetTintOpacity = std::clamp(
                m_pressOptics.tintOpacity + std::clamp(
                    implementation::LiquidGlassInteraction::GetPressedTintBoost(owner), -1.0, 1.0),
                0.0,
                1.0);

            // Synchronize DependencyProperty endpoints immediately so reconnect/retemplate
            // rebuilds the intended state. Then put the current presentation values back;
            // the continuous springs own what is visible between endpoints.
            brush.RefractionStrength(m_targetRefraction);
            brush.TintOpacity(m_targetTintOpacity);
            WriteDynamicOptics();
        }

        void LeaveSliderPressedOptics(
            Microsoft::UI::Xaml::DependencyObject const& owner)
        {
            if (!m_pressOptics.active || !m_pressOptics.brush) return;

            auto brush = m_pressOptics.brush;
            m_targetRefraction = m_pressOptics.refraction;
            m_targetTintOpacity = m_pressOptics.tintOpacity;

            // Restore the authored DP endpoints first, then retain the current presentation
            // values so rapid press/release reverses velocity instead of jumping.
            brush.RefractionStrength(m_pressOptics.refraction);
            brush.DispersionStrength(m_pressOptics.dispersion);
            brush.TintOpacity(m_pressOptics.tintOpacity);
            brush.HighlightStrength(m_pressOptics.highlight);
            brush.InnerShadowStrength(m_pressOptics.innerShadow);
            m_pressOptics.brush = nullptr;
            m_pressOptics.owner = nullptr;
            m_pressOptics.active = false;
            WriteDynamicOptics();
        }

        void ConfigureResources()
        {
            auto self = static_cast<Self*>(this);
            auto const track = SolidBrush(0x66, 0x89, 0x89, 0x8f);
            auto const transparent = SolidBrush(0x00, 0x00, 0x00, 0x00);

            self->Background(track);
            self->Foreground(transparent);
            self->CornerRadius({ 7.0, 7.0, 7.0, 7.0 });

            auto resources = self->Resources();
            auto insert = [&resources](wchar_t const* key, Windows::Foundation::IInspectable const& resourceValue)
            {
                if (resourceValue) resources.Insert(box_value(hstring{ key }), resourceValue);
            };
            insert(L"SliderThumbBackground", transparent);
            insert(L"SliderThumbBackgroundPointerOver", transparent);
            insert(L"SliderThumbBackgroundPressed", transparent);
            insert(L"SliderThumbBackgroundDisabled", transparent);
            insert(L"SliderOuterThumbBackground", transparent);
            insert(L"SliderThumbBorderBrush", transparent);
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
            resources.Insert(box_value(L"SliderHorizontalThumbWidth"), box_value(kSemanticThumbExtent));
            resources.Insert(box_value(L"SliderHorizontalThumbHeight"), box_value(kSemanticThumbExtent));
            resources.Insert(box_value(L"SliderVerticalThumbWidth"), box_value(kSemanticThumbExtent));
            resources.Insert(box_value(L"SliderVerticalThumbHeight"), box_value(kSemanticThumbExtent));
        }

        bool HorizontalVisualReversed() const
        {
            auto self = static_cast<Self const*>(this);
            auto reversed = self->IsDirectionReversed();
            if (self->FlowDirection() == Microsoft::UI::Xaml::FlowDirection::RightToLeft)
                reversed = !reversed;
            return reversed;
        }

        static void StepSpring(
            double target,
            double stiffness,
            double damping,
            double dt,
            double& value,
            double& velocity)
        {
            auto const steps = std::max(1, static_cast<int>(std::ceil(dt / (1.0 / 120.0))));
            auto const h = dt / static_cast<double>(steps);
            for (int i = 0; i < steps; ++i)
            {
                auto const acceleration = stiffness * (target - value) - damping * velocity;
                velocity += acceleration * h;
                value += velocity * h;
            }
        }

        void EnsureScaleTimer()
        {
            if (!m_loaded) return;
            auto self = static_cast<Self*>(this);
            if (!m_scaleTimer)
            {
                m_scaleTimer = self->DispatcherQueue().CreateTimer();
                m_scaleTimer.Interval(kScaleInterval);
                m_scaleTimer.IsRepeating(true);
                m_scaleTimer.Tick([this](auto const&, auto const&) { TickScaleDynamics(); });
            }
            if (!m_scaleTimer.IsRunning())
            {
                m_lastScaleTick = Clock::now();
                m_scaleTimer.Start();
            }
        }

        void TickScaleDynamics()
        {
            if (!m_loaded || !m_surface || !m_visualHost)
            {
                if (m_scaleTimer) m_scaleTimer.Stop();
                return;
            }

            auto const now = Clock::now();
            auto dt = std::chrono::duration<double>(now - m_lastScaleTick).count();
            m_lastScaleTick = now;
            dt = std::clamp(dt, 1.0 / 240.0, 1.0 / 30.0);

            StepSpring(
                m_targetScale,
                kScaleStiffness,
                kScaleDamping,
                dt,
                m_currentScale,
                m_scaleVelocity);
            if (m_opticsInitialized)
            {
                StepSpring(
                    m_targetRefraction,
                    kRefractionStiffness,
                    kRefractionDamping,
                    dt,
                    m_currentRefraction,
                    m_refractionVelocity);
                StepSpring(
                    m_targetTintOpacity,
                    kBodyOpacityStiffness,
                    kBodyOpacityDamping,
                    dt,
                    m_currentTintOpacity,
                    m_tintVelocity);
            }

            // Kube's three active quantities intentionally do NOT share one animation:
            // scale and white body use 2000/80, while refraction scaleRatio uses 100/10.
            SetElementScale(m_visualHost, m_currentScale, m_currentScale);
            WriteDynamicOptics();

            auto const scaleSettled =
                std::abs(m_currentScale - m_targetScale) < .0005 &&
                std::abs(m_scaleVelocity) < .005;
            auto const opticsSettled = !m_opticsInitialized ||
                ((std::abs(m_currentRefraction - m_targetRefraction) < .001 &&
                  std::abs(m_refractionVelocity) < .01) &&
                 (std::abs(m_currentTintOpacity - m_targetTintOpacity) < .0005 &&
                  std::abs(m_tintVelocity) < .005));
            if (scaleSettled && opticsSettled)
            {
                m_currentScale = m_targetScale;
                m_scaleVelocity = 0.0;
                SetElementScale(m_visualHost, m_currentScale, m_currentScale);

                if (m_opticsInitialized)
                {
                    m_currentRefraction = m_targetRefraction;
                    m_refractionVelocity = 0.0;
                    m_currentTintOpacity = m_targetTintOpacity;
                    m_tintVelocity = 0.0;
                    WriteDynamicOptics();
                }
                if (m_scaleTimer) m_scaleTimer.Stop();
            }
        }

        void SetScaleTarget(double scale, bool animate)
        {
            m_targetScale = scale;
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!animate || !owner || !MotionAnimationsEnabled(owner))
            {
                m_currentScale = scale;
                m_scaleVelocity = 0.0;
                if (m_visualHost) SetElementScale(m_visualHost, scale, scale);
                return;
            }
            EnsureScaleTimer();
        }

        void DetachInitialLayoutSync()
        {
            if (m_templateHost && m_initialLayoutHooked)
                m_templateHost.LayoutUpdated(m_initialLayoutToken);
            m_initialLayoutHooked = false;
        }

        void ArmInitialLayoutSync()
        {
            if (!m_templateHost || m_initialLayoutHooked) return;
            m_initialLayoutHooked = true;
            m_initialLayoutToken = m_templateHost.LayoutUpdated([this](auto const&, auto const&)
            {
                if (!m_loaded || !m_track || !m_surface || !m_templateHost) return;

                auto self = static_cast<Self*>(this);
                auto const horizontal = self->Orientation() == Microsoft::UI::Xaml::Controls::Orientation::Horizontal;
                auto const extent = horizontal ? m_track.ActualWidth() : m_track.ActualHeight();
                if (!(extent > 0.0)) return;

                UpdateProgressVisual();
                UpdateLensPosition(DisplayRatio(NormalizedValue()));
                ApplyInteractionState(false);
                RefreshPointerField();
                DetachInitialLayoutSync();
            });
        }

        void ResolveTemplateParts()
        {
            auto self = static_cast<Self*>(this);
            auto root = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!root) return;

            auto const horizontal = self->Orientation() == Microsoft::UI::Xaml::Controls::Orientation::Horizontal;
            auto const thumbName = horizontal ? L"HorizontalThumb" : L"VerticalThumb";
            auto const trackName = horizontal ? L"HorizontalTrackRect" : L"VerticalTrackRect";
            auto const decreaseName = horizontal ? L"HorizontalDecreaseRect" : L"VerticalDecreaseRect";
            auto const templateName = horizontal ? L"HorizontalTemplate" : L"VerticalTemplate";

            auto thumb = FindNamedDescendant(root, thumbName)
                .try_as<Microsoft::UI::Xaml::Controls::Primitives::Thumb>();
            if (!thumb) thumb = FindFirstDescendant<Microsoft::UI::Xaml::Controls::Primitives::Thumb>(root);
            auto track = FindNamedDescendant(root, trackName).try_as<Microsoft::UI::Xaml::Shapes::Rectangle>();
            auto decrease = FindNamedDescendant(root, decreaseName).try_as<Microsoft::UI::Xaml::Shapes::Rectangle>();
            auto templateHost = FindNamedDescendant(root, templateName)
                .try_as<Microsoft::UI::Xaml::Controls::Grid>();
            if (!thumb || !track || !templateHost) return;

            bool const thumbChanged = !m_thumb || get_abi(m_thumb) != get_abi(thumb);
            bool const trackChanged = !m_track || get_abi(m_track) != get_abi(track);
            bool const hostChanged = !m_templateHost || get_abi(m_templateHost) != get_abi(templateHost);

            m_thumb = thumb;
            m_track = track;
            m_decrease = decrease;

            if (thumbChanged)
            {
                // Keep the real WinUI Slider contract intact. The native Thumb remains
                // exactly 18x18 and owns all value/layout/input/UIA mechanics.
                thumb.Width(kSemanticThumbExtent);
                thumb.Height(kSemanticThumbExtent);
                thumb.Margin({ 0.0, 0.0, 0.0, 0.0 });
                // The semantic Thumb must remain fully interactive, but none of the
                // stock WinUI thumb visuals should render behind the sibling glass lens.
                // Opacity does not disable hit testing or native Slider mechanics.
                thumb.Opacity(0.0);
                if (auto inner = FindNamedDescendant(thumb, L"SliderInnerThumb")
                    .try_as<Microsoft::UI::Xaml::Shapes::Ellipse>())
                {
                    inner.Visibility(Microsoft::UI::Xaml::Visibility::Collapsed);
                }
            }

            // Keep the semantic Thumb at 18x18, but mirror the successful Switch
            // architecture for the visual lens:
            //
            //   transform host (scale/translation)
            //       -> fixed 90x60 optical Border (LiquidGlassBrush)
            //
            // This ordering matters. Scaling the same XAML element that owns a
            // XamlCompositionBrushBase changes the brush/backdrop materialization domain.
            // Kube paints backdrop-filter first and applies CSS transform afterwards;
            // the parent wrapper gives Composition the same explicit ordering.
            if (hostChanged || !m_surface || !m_visualHost)
            {
                if (hostChanged) DetachInitialLayoutSync();
                m_templateHost = templateHost;

                auto visualHost = FindNamedDescendant(templateHost, L"LiquidGlassSliderVisualHost")
                    .try_as<Microsoft::UI::Xaml::Controls::Grid>();
                if (!visualHost)
                {
                    visualHost = Microsoft::UI::Xaml::Controls::Grid{};
                    visualHost.Name(L"LiquidGlassSliderVisualHost");
                    visualHost.Width(horizontal ? kVisualWidth : kVisualHeight);
                    visualHost.Height(horizontal ? kVisualHeight : kVisualWidth);
                    visualHost.HorizontalAlignment(Microsoft::UI::Xaml::HorizontalAlignment::Left);
                    visualHost.VerticalAlignment(Microsoft::UI::Xaml::VerticalAlignment::Top);
                    visualHost.IsHitTestVisible(false);
                    visualHost.Opacity(0.0);
                    Microsoft::UI::Xaml::Controls::Grid::SetRow(visualHost, 0);
                    Microsoft::UI::Xaml::Controls::Grid::SetRowSpan(visualHost, 3);
                    Microsoft::UI::Xaml::Controls::Grid::SetColumn(visualHost, 0);
                    Microsoft::UI::Xaml::Controls::Grid::SetColumnSpan(visualHost, 3);
                    Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetIsTranslationEnabled(visualHost, true);
                    templateHost.Children().Append(visualHost);
                }

                auto surface = FindNamedDescendant(visualHost, L"LiquidGlassSliderSurface")
                    .try_as<Microsoft::UI::Xaml::Controls::Border>();
                if (!surface)
                {
                    surface = Microsoft::UI::Xaml::Controls::Border{};
                    surface.Name(L"LiquidGlassSliderSurface");
                    surface.Width(horizontal ? kVisualWidth : kVisualHeight);
                    surface.Height(horizontal ? kVisualHeight : kVisualWidth);
                    surface.HorizontalAlignment(Microsoft::UI::Xaml::HorizontalAlignment::Left);
                    surface.VerticalAlignment(Microsoft::UI::Xaml::VerticalAlignment::Top);
                    surface.IsHitTestVisible(false);
                    surface.CornerRadius({ 30.0, 30.0, 30.0, 30.0 });
                    visualHost.Children().Append(surface);
                }

                m_visualHost = visualHost;
                m_surface = surface;
                SetElementScale(m_visualHost, m_currentScale, m_currentScale);
            }

            if (trackChanged)
            {
                track.Fill(SolidBrush(0x66, 0x89, 0x89, 0x8f));
                if (decrease) decrease.Opacity(0.0);
                BuildProgressVisual();
            }
            else if (decrease && decrease.Opacity() != 0.0)
            {
                decrease.Opacity(0.0);
            }

            ArmInitialLayoutSync();
        }

        void BuildProgressVisual()
        {
            if (!m_track) return;
            auto hostVisual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(m_track);
            auto compositor = hostVisual.Compositor();
            m_progressGeometry = compositor.CreateRoundedRectangleGeometry();
            auto shape = compositor.CreateSpriteShape(m_progressGeometry);
            shape.FillBrush(compositor.CreateColorBrush(Windows::UI::Color{ 0xff, 0x03, 0x77, 0xf7 }));
            m_progressShape = shape;
            m_progressVisual = compositor.CreateShapeVisual();
            m_progressVisual.Shapes().Append(shape);
            Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetElementChildVisual(m_track, m_progressVisual);
        }

        double NormalizedValue() const
        {
            auto self = static_cast<Self const*>(this);
            auto const min = self->Minimum();
            auto const max = self->Maximum();
            if (!(max > min)) return 0.0;
            return std::clamp((self->Value() - min) / (max - min), 0.0, 1.0);
        }

        double DisplayRatio(double logicalRatio) const
        {
            auto self = static_cast<Self const*>(this);
            if (self->Orientation() == Microsoft::UI::Xaml::Controls::Orientation::Horizontal)
                return HorizontalVisualReversed() ? 1.0 - logicalRatio : logicalRatio;
            return self->IsDirectionReversed() ? logicalRatio : 1.0 - logicalRatio;
        }

        void UpdateProgressVisual()
        {
            if (!m_track || !m_progressVisual || !m_progressGeometry || !m_progressShape) return;

            auto self = static_cast<Self*>(this);
            auto const horizontal = self->Orientation() == Microsoft::UI::Xaml::Controls::Orientation::Horizontal;
            auto const ratio = NormalizedValue();
            auto const width = std::max(0.0, m_track.ActualWidth());
            auto const height = std::max(0.0, m_track.ActualHeight());
            if (width <= 0.0 || height <= 0.0) return;

            m_progressVisual.Size({ static_cast<float>(width), static_cast<float>(height) });
            m_progressVisual.Opacity(ratio > 1e-6 ? 1.0f : 0.0f);
            if (horizontal)
            {
                auto const progressWidth = width * ratio;
                m_progressGeometry.Size({ static_cast<float>(progressWidth), static_cast<float>(height) });
                auto const radius = static_cast<float>(height * .5);
                m_progressGeometry.CornerRadius({ radius, radius });
                auto const startX = HorizontalVisualReversed() ? width - progressWidth : 0.0;
                m_progressShape.Offset({ static_cast<float>(startX), 0.0f });
            }
            else
            {
                auto const progressHeight = height * ratio;
                m_progressGeometry.Size({ static_cast<float>(width), static_cast<float>(progressHeight) });
                auto const radius = static_cast<float>(width * .5);
                m_progressGeometry.CornerRadius({ radius, radius });
                auto const startY = self->IsDirectionReversed() ? 0.0 : height - progressHeight;
                m_progressShape.Offset({ 0.0f, static_cast<float>(startY) });
            }
        }

        void UpdateLensPosition(double displayRatio)
        {
            if (!m_surface || !m_visualHost || !m_track || !m_templateHost) return;

            auto self = static_cast<Self*>(this);
            auto const horizontal = self->Orientation() == Microsoft::UI::Xaml::Controls::Orientation::Horizontal;
            auto const authoredPrimary = horizontal ? kVisualWidth : kVisualHeight;
            auto const authoredCross = horizontal ? kVisualHeight : kVisualWidth;
            auto const visualExtent = authoredPrimary * kRestScale;
            auto const trackExtent = horizontal ? m_track.ActualWidth() : m_track.ActualHeight();
            if (trackExtent <= 0.0) return;

            double trackOriginPrimary = 0.0;
            double trackCenterCross = authoredCross * .5;
            try
            {
                auto transform = m_track.TransformToVisual(m_templateHost);
                auto origin = transform.TransformPoint({ 0.0f, 0.0f });
                trackOriginPrimary = horizontal ? origin.X : origin.Y;
                trackCenterCross = horizontal
                    ? origin.Y + m_track.ActualHeight() * .5
                    : origin.X + m_track.ActualWidth() * .5;
            }
            catch (...)
            {
                // Keep a newly-created lens hidden until the template has a valid
                // track-to-host transform. LayoutUpdated/SizeChanged will retry.
                return;
            }

            // Kube constrains the lens center using its rendered rest footprint (54 DIPs),
            // not the 18-DIP native input Thumb. Keep that visual mapping while the native
            // Slider remains the sole semantic/value owner.
            auto const halfVisual = visualExtent * .5;
            auto const usable = std::max(0.0, trackExtent - visualExtent);
            auto const desiredCenter = trackOriginPrimary +
                halfVisual + std::clamp(displayRatio, 0.0, 1.0) * usable;
            auto const primaryTranslation = desiredCenter - authoredPrimary * .5;
            auto const crossTranslation = trackCenterCross - authoredCross * .5;

            auto translation = m_visualHost.Translation();
            auto const newPrimary = static_cast<float>(primaryTranslation);
            auto const newCross = static_cast<float>(crossTranslation);
            constexpr float newZ = 0.0f;

            if (horizontal)
            {
                translation.x = newPrimary;
                translation.y = newCross;
            }
            else
            {
                translation.x = newCross;
                translation.y = newPrimary;
            }
            translation.z = newZ;
            m_visualHost.Translation(translation);
            if (m_visualHost.Opacity() != 1.0) m_visualHost.Opacity(1.0);
        }

        void ApplyBrush()
        {
            auto self = static_cast<Self*>(this);
            auto brush = self->GlassBrush();
            if (brush)
            {
                brush.CornerRadius(30.0);
                brush.BezelWidth(16.0);
            }
            if (m_surface)
            {
                auto desired = brush
                    ? brush.as<Microsoft::UI::Xaml::Media::Brush>()
                    : Microsoft::UI::Xaml::Media::Brush{ nullptr };
                auto current = m_surface.Background();
                if ((!current && desired) || (current && !desired) ||
                    (current && desired && get_abi(current) != get_abi(desired)))
                {
                    m_surface.Background(desired);
                }
            }
        }

        void RestorePendingOptics()
        {
            if (!m_pressOptics.active) return;
            auto self = static_cast<Self*>(this);
            auto brush = self->GlassBrush();
            if (brush && m_pressOptics.brush && get_abi(brush) == get_abi(m_pressOptics.brush))
                RestoreSliderOptics(m_pressOptics);
            else
                m_pressOptics = {};
            m_opticsInitialized = false;
        }

        void BeginPress(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_loaded || m_pressed) return;
            m_pressed = true;
            ApplyInteractionState(true);
            // Router updates are move-driven. Seed the field from PointerPressed as well
            // so touch/stationary-pointer activation receives the same refractive response.
            m_pointerField.UpdateFromPointer(args);
        }

        void EndPress(bool animate)
        {
            if (!m_loaded)
            {
                m_pressed = false;
                return;
            }
            if (!m_pressed && !m_pressOptics.active) return;
            m_pressed = false;
            ApplyInteractionState(animate);
        }

        void ApplyInteractionState(bool animate)
        {
            if (!m_surface) ResolveTemplateParts();
            if (!m_surface) return;
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!owner) return;

            auto const targetScale = m_pressed ? 1.0 : kRestScale;
            SetScaleTarget(targetScale, animate);

            if (m_pressed)
            {
                if (!m_pressOptics.active)
                    EnterSliderPressedOptics(owner, self->GlassBrush());
            }
            else
            {
                LeaveSliderPressedOptics(owner);
            }

            if (animate && MotionAnimationsEnabled(owner))
                EnsureScaleTimer();
            else if (m_opticsInitialized)
            {
                m_currentRefraction = m_targetRefraction;
                m_refractionVelocity = 0.0;
                m_currentTintOpacity = m_targetTintOpacity;
                m_tintVelocity = 0.0;
                WriteDynamicOptics();
            }

            m_pointerField.SetConfigurationScales(0.0, 0.0, 0.0);
            UpdateLensPosition(DisplayRatio(NormalizedValue()));
        }

        void RefreshPointerField()
        {
            auto self = static_cast<Self*>(this);
            auto target = m_surface.try_as<Microsoft::UI::Xaml::FrameworkElement>();
            auto xamlRoot = self->XamlRoot();
            if (!target || !xamlRoot)
            {
                m_pointerField.Detach(false);
                return;
            }
            auto weak = self->get_weak();
            m_pointerField.Attach(xamlRoot, target, [weak]() -> WinUI::Composition::Hlsl::LiquidGlassBrush
            {
                if (auto owner = weak.get()) return owner->GlassBrush();
                return nullptr;
            });
            m_pointerField.SetConfigurationScales(0.0, 0.0, 0.0);
        }

        PointerFieldSurface m_pointerField;
        Microsoft::UI::Xaml::Controls::Primitives::Thumb m_thumb{ nullptr };
        Microsoft::UI::Xaml::Controls::Border m_surface{ nullptr };
        Microsoft::UI::Xaml::Controls::Grid m_visualHost{ nullptr };
        Microsoft::UI::Xaml::Controls::Grid m_templateHost{ nullptr };
        Microsoft::UI::Xaml::Shapes::Rectangle m_track{ nullptr };
        Microsoft::UI::Xaml::Shapes::Rectangle m_decrease{ nullptr };
        Microsoft::UI::Composition::ShapeVisual m_progressVisual{ nullptr };
        Microsoft::UI::Composition::CompositionRoundedRectangleGeometry m_progressGeometry{ nullptr };
        Microsoft::UI::Composition::CompositionSpriteShape m_progressShape{ nullptr };
        Microsoft::UI::Dispatching::DispatcherQueueTimer m_scaleTimer{ nullptr };
        Clock::time_point m_lastScaleTick{};
        winrt::event_token m_initialLayoutToken{};
        double m_currentScale{ kRestScale };
        double m_targetScale{ kRestScale };
        double m_scaleVelocity{};
        double m_currentRefraction{ 9.6 };
        double m_targetRefraction{ 9.6 };
        double m_refractionVelocity{};
        double m_currentTintOpacity{ 1.0 };
        double m_targetTintOpacity{ 1.0 };
        double m_tintVelocity{};
        Windows::Foundation::IInspectable m_pointerPressedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerReleasedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCaptureLostHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCanceledHandler{ nullptr };
        OpticsSnapshot m_pressOptics;
        bool m_initialLayoutHooked{};
        bool m_loaded{};
        bool m_pressed{};
        bool m_opticsInitialized{};
    };
}

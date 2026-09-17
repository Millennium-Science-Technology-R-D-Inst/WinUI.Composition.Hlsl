#pragma once

#include <algorithm>
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
                m_pointerField.Detach(false);
                m_thumb = nullptr;
                m_surface = nullptr;
                m_track = nullptr;
                m_decrease = nullptr;
                m_progressVisual = nullptr;
                m_progressGeometry = nullptr;
                m_progressShape = nullptr;
                m_pressed = false;
            });

            auto refresh = [this](auto const&, auto const&)
            {
                if (m_loaded) RefreshVisual();
            };
            self->RegisterPropertyChangedCallback(Self::GlassBrushProperty(), refresh);
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Slider::OrientationProperty(), refresh);
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Slider::IsDirectionReversedProperty(), refresh);
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Primitives::RangeBase::ValueProperty(),
                [this](auto const&, auto const&) { if (m_loaded) UpdateValueVisuals(); });
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Primitives::RangeBase::MinimumProperty(),
                [this](auto const&, auto const&) { if (m_loaded) UpdateValueVisuals(); });
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Primitives::RangeBase::MaximumProperty(),
                [this](auto const&, auto const&) { if (m_loaded) UpdateValueVisuals(); });

            self->LayoutUpdated([this](auto const&, auto const&)
            {
                if (!m_loaded) return;
                ResolveTemplateParts();
                UpdateValueVisuals();
            });

            auto bind = [self](auto routedEvent, Windows::Foundation::IInspectable& storage, auto&& callback)
            {
                using Handler = Microsoft::UI::Xaml::Input::PointerEventHandler;
                storage = winrt::box_value<Handler>({ std::forward<decltype(callback)>(callback) });
                self->AddHandler(routedEvent, storage, true);
            };
            bind(Microsoft::UI::Xaml::UIElement::PointerPressedEvent(), m_pointerPressedHandler,
                [this](auto const&, auto const&) { BeginPress(); });
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
            UpdateValueVisuals();
            ApplyInteractionState(false);
            RefreshPointerField();
        }

    private:
        static constexpr double kSemanticThumbExtent = 18.0;
        static constexpr double kVisualWidth = 90.0;
        static constexpr double kVisualHeight = 60.0;
        static constexpr double kRestScale = 0.6;
        static constexpr double kRestElevation = 6.0;
        static constexpr double kPressedElevation = 10.0;
        static constexpr double kScaleDampingRatio = 0.8944271909999159;
        static constexpr double kScalePeriodMs = 140.49629462081452;

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
            auto const track = SolidBrush(0x66, 0x89, 0x89, 0x8f);
            auto const transparent = SolidBrush(0x00, 0x00, 0x00, 0x00);
            auto const stroke = SolidBrush(0x33, 0xff, 0xff, 0xff);

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

        void ResolveTemplateParts()
        {
            auto self = static_cast<Self*>(this);
            auto root = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!root) return;

            auto const horizontal = self->Orientation() == Microsoft::UI::Xaml::Controls::Orientation::Horizontal;
            auto const thumbName = horizontal ? L"HorizontalThumb" : L"VerticalThumb";
            auto const trackName = horizontal ? L"HorizontalTrackRect" : L"VerticalTrackRect";
            auto const decreaseName = horizontal ? L"HorizontalDecreaseRect" : L"VerticalDecreaseRect";

            auto thumb = FindNamedDescendant(root, thumbName)
                .try_as<Microsoft::UI::Xaml::Controls::Primitives::Thumb>();
            if (!thumb) thumb = FindFirstDescendant<Microsoft::UI::Xaml::Controls::Primitives::Thumb>(root);
            auto track = FindNamedDescendant(root, trackName).try_as<Microsoft::UI::Xaml::Shapes::Rectangle>();
            auto decrease = FindNamedDescendant(root, decreaseName).try_as<Microsoft::UI::Xaml::Shapes::Rectangle>();
            if (!thumb || !track) return;

            thumb.Width(kSemanticThumbExtent);
            thumb.Height(kSemanticThumbExtent);
            thumb.Margin({ 0.0, 0.0, 0.0, 0.0 });
            m_thumb = thumb;

            auto surface = FindFirstDescendant<Microsoft::UI::Xaml::Controls::Border>(thumb);
            m_surface = surface;
            if (surface)
            {
                surface.Width(horizontal ? kVisualWidth : kVisualHeight);
                surface.Height(horizontal ? kVisualHeight : kVisualWidth);
                surface.HorizontalAlignment(Microsoft::UI::Xaml::HorizontalAlignment::Center);
                surface.VerticalAlignment(Microsoft::UI::Xaml::VerticalAlignment::Center);
                surface.Margin({ 0.0, 0.0, 0.0, 0.0 });
                surface.CornerRadius({ 30.0, 30.0, 30.0, 30.0 });
                surface.BorderBrush(SolidBrush(0x33, 0xff, 0xff, 0xff));
                surface.BorderThickness({ 1.0, 1.0, 1.0, 1.0 });
                Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetIsTranslationEnabled(surface, true);
                if (!surface.Shadow()) surface.Shadow(Microsoft::UI::Xaml::Media::ThemeShadow{});
            }

            if (auto inner = FindNamedDescendant(thumb, L"SliderInnerThumb")
                .try_as<Microsoft::UI::Xaml::Shapes::Ellipse>())
            {
                inner.Visibility(Microsoft::UI::Xaml::Visibility::Collapsed);
            }

            track.Fill(SolidBrush(0x66, 0x89, 0x89, 0x8f));
            if (decrease) decrease.Opacity(0.0);

            bool const trackChanged = !m_track || get_abi(m_track) != get_abi(track);
            m_track = track;
            m_decrease = decrease;
            if (trackChanged) BuildProgressVisual();
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

        void UpdateValueVisuals()
        {
            if (!m_track || !m_progressVisual || !m_progressGeometry || !m_progressShape) return;

            auto self = static_cast<Self*>(this);
            auto const horizontal = self->Orientation() == Microsoft::UI::Xaml::Controls::Orientation::Horizontal;
            auto const ratio = NormalizedValue();
            auto const displayRatio = DisplayRatio(ratio);
            auto const width = std::max(0.0, m_track.ActualWidth());
            auto const height = std::max(0.0, m_track.ActualHeight());
            if (width <= 0.0 || height <= 0.0) return;

            m_progressVisual.Size({ static_cast<float>(width), static_cast<float>(height) });
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

            UpdateLensPosition(displayRatio);
        }

        void UpdateLensPosition(double displayRatio)
        {
            if (!m_surface) return;
            auto const restVisualExtent = kVisualWidth * kRestScale;
            auto const correction = (0.5 - displayRatio) * (restVisualExtent - kSemanticThumbExtent);
            auto translation = m_surface.Translation();
            auto self = static_cast<Self*>(this);
            if (self->Orientation() == Microsoft::UI::Xaml::Controls::Orientation::Horizontal)
            {
                translation.x = static_cast<float>(correction);
                translation.y = 0.0f;
            }
            else
            {
                translation.x = 0.0f;
                translation.y = static_cast<float>(correction);
            }
            translation.z = static_cast<float>(m_pressed ? kPressedElevation : kRestElevation);
            m_surface.Translation(translation);
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
                m_surface.Background(brush
                    ? brush.as<Microsoft::UI::Xaml::Media::Brush>()
                    : Microsoft::UI::Xaml::Media::Brush{ nullptr });
            }
        }

        void RestorePendingOptics()
        {
            if (!m_pressOptics.active) return;
            auto self = static_cast<Self*>(this);
            auto brush = self->GlassBrush();
            if (brush && m_pressOptics.brush && get_abi(brush) == get_abi(m_pressOptics.brush))
                RestoreOptics(m_pressOptics);
            else
                m_pressOptics = {};
        }

        void BeginPress()
        {
            if (!m_loaded || m_pressed) return;
            m_pressed = true;
            ApplyInteractionState(true);
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
            if (animate)
            {
                AnimateElementScaleSpring(
                    owner,
                    m_surface,
                    targetScale,
                    targetScale,
                    kScaleDampingRatio,
                    kScalePeriodMs);
            }
            else
            {
                SetElementScale(m_surface, targetScale, targetScale);
            }

            if (m_pressed)
            {
                if (!m_pressOptics.active)
                    EnterPressedOptics(owner, self->GlassBrush(), m_pressOptics);
            }
            else
            {
                LeavePressedOptics(owner, m_pressOptics);
            }
            UpdateValueVisuals();
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
        }

        PointerFieldSurface m_pointerField;
        Microsoft::UI::Xaml::Controls::Primitives::Thumb m_thumb{ nullptr };
        Microsoft::UI::Xaml::Controls::Border m_surface{ nullptr };
        Microsoft::UI::Xaml::Shapes::Rectangle m_track{ nullptr };
        Microsoft::UI::Xaml::Shapes::Rectangle m_decrease{ nullptr };
        Microsoft::UI::Composition::ShapeVisual m_progressVisual{ nullptr };
        Microsoft::UI::Composition::CompositionRoundedRectangleGeometry m_progressGeometry{ nullptr };
        Microsoft::UI::Composition::CompositionSpriteShape m_progressShape{ nullptr };
        Windows::Foundation::IInspectable m_pointerPressedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerReleasedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCaptureLostHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCanceledHandler{ nullptr };
        OpticsSnapshot m_pressOptics;
        bool m_loaded{};
        bool m_pressed{};
    };
}

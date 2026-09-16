#pragma once

#include "PointerFieldHelper.hpp"
#include "MotionAnimation.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    inline Microsoft::UI::Xaml::DependencyObject FindNamedDescendant(
        Microsoft::UI::Xaml::DependencyObject const& root,
        std::wstring_view name)
    {
        if (!root) return nullptr;
        if (auto element = root.try_as<Microsoft::UI::Xaml::FrameworkElement>();
            element && element.Name() == name)
            return root;

        auto const count = Microsoft::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(root);
        for (int32_t index = 0; index < count; ++index)
        {
            if (auto result = FindNamedDescendant(
                Microsoft::UI::Xaml::Media::VisualTreeHelper::GetChild(root, index), name))
                return result;
        }
        return nullptr;
    }

    template<typename T>
    T FindFirstDescendant(Microsoft::UI::Xaml::DependencyObject const& root)
    {
        if (!root) return nullptr;
        auto const count = Microsoft::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(root);
        for (int32_t index = 0; index < count; ++index)
        {
            auto child = Microsoft::UI::Xaml::Media::VisualTreeHelper::GetChild(root, index);
            if (auto result = child.try_as<T>()) return result;
            if (auto result = FindFirstDescendant<T>(child)) return result;
        }
        return nullptr;
    }

    class PointerFieldSurface
    {
    public:
        using Clock = PointerFieldRouter::Clock;
        using Brush = WinUI::Composition::Hlsl::LiquidGlassBrush;
        using BrushGetter = std::function<Brush()>;

        PointerFieldSurface() = default;
        PointerFieldSurface(PointerFieldSurface const&) = delete;
        PointerFieldSurface& operator=(PointerFieldSurface const&) = delete;
        ~PointerFieldSurface() { Detach(false); }

        void Attach(
            Microsoft::UI::Xaml::XamlRoot const& xamlRoot,
            Microsoft::UI::Xaml::FrameworkElement const& target,
            BrushGetter brushGetter)
        {
            if (!xamlRoot || !target || !brushGetter)
            {
                Detach();
                return;
            }

            if (m_target && m_xamlRoot &&
                get_abi(m_target) == get_abi(target) &&
                get_abi(m_xamlRoot) == get_abi(xamlRoot))
            {
                m_brushGetter = std::move(brushGetter);
                m_configurationDirty = true;
                return;
            }

            Detach();
            m_xamlRoot = xamlRoot;
            m_target = target;
            m_brushGetter = std::move(brushGetter);
            m_sizeChangedToken = m_target.SizeChanged([this](auto const&, auto const&)
            {
                m_configurationDirty = true;
            });

            m_router = PointerFieldRouter::For(xamlRoot);
            if (!m_router)
            {
                Detach();
                return;
            }

            m_registrationId = m_router->Register(
                [this](Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args, Clock::time_point now)
                {
                    Update(args, now);
                },
                [this] { DeactivateTrackedMaterial(); });
            if (!m_registrationId) Detach();
        }

        void Detach(bool deactivate = true)
        {
            if (m_router && m_registrationId) m_router->Unregister(m_registrationId);
            m_registrationId = 0;
            m_router.reset();
            if (m_target && m_sizeChangedToken.value) m_target.SizeChanged(m_sizeChangedToken);
            m_sizeChangedToken = {};
            m_target = nullptr;
            m_xamlRoot = nullptr;
            m_brushGetter = {};
            if (deactivate)
                DeactivateTrackedMaterial();
            else
                ClearTrackedMaterial();
        }

        void InvalidateBrush()
        {
            DeactivateTrackedMaterial();
            m_configurationDirty = true;
        }

    private:
        static constexpr double kHoverRangeDips = 28.0;

        static void DeactivateEffect(WinUI::Composition::Hlsl::HlslEffectBrush const& effect)
        {
            if (!effect) return;
            try
            {
                effect.SetFloat(L"PointerActive", 0.0f);
                effect.SetFloat(L"PointerVelocityX", 0.0f);
                effect.SetFloat(L"PointerVelocityY", 0.0f);
            }
            catch (winrt::hresult_error const& error)
            {
                if (error.code() != winrt::hresult{ RO_E_CLOSED }) throw;
            }
        }

        void ClearTrackedMaterial() noexcept
        {
            m_trackingMaterial = nullptr;
            m_active = false;
            m_lastPointValid = false;
        }

        void TrackMaterial(WinUI::Composition::Hlsl::LiquidGlassMaterial const& material)
        {
            if (m_trackingMaterial && (!material || get_abi(m_trackingMaterial) != get_abi(material)))
            {
                DeactivateEffect(m_trackingMaterial.EffectBrush());
            }
            if (!material)
            {
                ClearTrackedMaterial();
                return;
            }
            if (!m_trackingMaterial || get_abi(m_trackingMaterial) != get_abi(material))
            {
                m_trackingMaterial = material;
                m_configurationDirty = true;
                m_lastPointValid = false;
            }
        }

        void Configure(
            Brush const& brush,
            WinUI::Composition::Hlsl::HlslEffectBrush const& effect,
            double width,
            double height)
        {
            if (!brush || !effect) return;
            auto const maxExtent = std::max(width, height);
            if (maxExtent <= 1e-4) return;
            auto const interactionRadiusDips = std::clamp(maxExtent * 0.65, 56.0, 180.0);
            auto const refraction = static_cast<float>(std::clamp(brush.RefractionStrength() * 0.24, 2.0, 8.0));
            auto const highlight = static_cast<float>(std::clamp(brush.HighlightStrength() * 0.55, 0.12, 0.40));
            effect.SetFloat(L"PointerInteractionRadius", static_cast<float>(interactionRadiusDips / maxExtent));
            effect.SetFloat(L"PointerInteractionStrength", 1.0f);
            effect.SetFloat(L"PointerHoverRange", static_cast<float>(kHoverRangeDips / maxExtent));
            effect.SetFloat(L"PointerRefractionStrength", refraction);
            effect.SetFloat(L"PointerHighlightStrength", highlight);
            effect.SetFloat(L"PointerMotionRefractionStrength", 5.0f);
            m_configurationDirty = false;
        }

        void Update(
            Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args,
            Clock::time_point now)
        {
            if (!m_target || !m_brushGetter) return;
            auto brush = m_brushGetter();
            auto material = brush ? brush.Material() : WinUI::Composition::Hlsl::LiquidGlassMaterial{ nullptr };
            TrackMaterial(material);
            if (!brush || !material) return;
            auto effect = material.EffectBrush();
            if (!effect) return;

            auto const width = m_target.ActualWidth();
            auto const height = m_target.ActualHeight();
            if (width <= 0.0 || height <= 0.0)
            {
                SetInactive(effect);
                return;
            }

            auto const point = args.GetCurrentPoint(m_target).Position();
            auto const active =
                point.X >= -kHoverRangeDips && point.X <= width + kHoverRangeDips &&
                point.Y >= -kHoverRangeDips && point.Y <= height + kHoverRangeDips;
            if (!active)
            {
                SetInactive(effect);
                return;
            }
            if (m_configurationDirty) Configure(brush, effect, width, height);

            float velocityX = 0.0f;
            float velocityY = 0.0f;
            if (m_lastPointValid)
            {
                auto const elapsed = std::chrono::duration<double>(now - m_lastTime).count();
                if (elapsed > 1e-4 && elapsed < 0.25)
                {
                    velocityX = static_cast<float>(std::clamp(((point.X - m_lastPoint.X) / width) / elapsed, -100.0, 100.0));
                    velocityY = static_cast<float>(std::clamp(((point.Y - m_lastPoint.Y) / height) / elapsed, -100.0, 100.0));
                }
            }

            effect.SetFloat(L"PointerX", static_cast<float>(point.X / width));
            effect.SetFloat(L"PointerY", static_cast<float>(point.Y / height));
            effect.SetFloat(L"PointerVelocityX", velocityX);
            effect.SetFloat(L"PointerVelocityY", velocityY);
            effect.SetFloat(L"PointerActive", 1.0f);
            m_lastPoint = point;
            m_lastTime = now;
            m_lastPointValid = true;
            m_active = true;
        }

        void SetInactive(WinUI::Composition::Hlsl::HlslEffectBrush const& effect)
        {
            if (m_active) DeactivateEffect(effect);
            m_active = false;
            m_lastPointValid = false;
        }

        void DeactivateTrackedMaterial()
        {
            if (m_trackingMaterial) DeactivateEffect(m_trackingMaterial.EffectBrush());
            ClearTrackedMaterial();
        }

        std::shared_ptr<PointerFieldRouter> m_router;
        Microsoft::UI::Xaml::XamlRoot m_xamlRoot{ nullptr };
        Microsoft::UI::Xaml::FrameworkElement m_target{ nullptr };
        BrushGetter m_brushGetter;
        event_token m_sizeChangedToken{};
        WinUI::Composition::Hlsl::LiquidGlassMaterial m_trackingMaterial{ nullptr };
        Windows::Foundation::Point m_lastPoint{};
        Clock::time_point m_lastTime{};
        std::uint64_t m_registrationId{};
        bool m_configurationDirty{ true };
        bool m_lastPointValid{};
        bool m_active{};
    };

    template<typename Self>
    class SliderPointerFieldHelper
    {
    public:
        SliderPointerFieldHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const&, auto const&) { RefreshPointerFieldTarget(); });
            self->Unloaded([this](auto const&, auto const&) { m_pointerField.Detach(false); });
            self->RegisterPropertyChangedCallback(Self::GlassBrushProperty(), [this](auto const&, auto const&) { m_pointerField.InvalidateBrush(); });
            self->RegisterPropertyChangedCallback(Microsoft::UI::Xaml::Controls::Slider::OrientationProperty(), [this](auto const&, auto const&) { RefreshPointerFieldTarget(); });
        }

        void RefreshPointerFieldTarget()
        {
            auto self = static_cast<Self*>(this);
            auto root = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto xamlRoot = self->XamlRoot();
            if (!root || !xamlRoot)
            {
                m_pointerField.Detach();
                return;
            }

            auto const name = self->Orientation() == Microsoft::UI::Xaml::Controls::Orientation::Horizontal
                ? L"HorizontalThumb" : L"VerticalThumb";
            auto thumb = FindNamedDescendant(root, name).try_as<Microsoft::UI::Xaml::Controls::Primitives::Thumb>();
            if (!thumb) thumb = FindFirstDescendant<Microsoft::UI::Xaml::Controls::Primitives::Thumb>(root);
            auto target = thumb.try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!target)
            {
                m_pointerField.Detach();
                return;
            }

            auto weak = self->get_weak();
            m_pointerField.Attach(xamlRoot, target, [weak]() -> WinUI::Composition::Hlsl::LiquidGlassBrush
            {
                if (auto owner = weak.get()) return owner->GlassBrush();
                return nullptr;
            });
        }

    private:
        PointerFieldSurface m_pointerField;
    };

    template<typename Self>
    class ToggleSwitchInteractionHelper
    {
    public:
        ToggleSwitchInteractionHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const&, auto const&) { RefreshInteractionTarget(); });
            self->Unloaded([this](auto const&, auto const&)
            {
                EndDrag(false, true);
                m_pointerField.Detach(false);
                m_knob = nullptr;
            });
            self->RegisterPropertyChangedCallback(Self::GlassBrushProperty(), [this](auto const&, auto const&) { m_pointerField.InvalidateBrush(); });
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Primitives::ToggleButton::IsCheckedProperty(),
                [this](auto const&, auto const&) { if (!m_dragging) SyncSemanticPosition(true); });

            auto bind = [self](auto routedEvent, Windows::Foundation::IInspectable& storage, auto&& callback)
            {
                using Handler = Microsoft::UI::Xaml::Input::PointerEventHandler;
                storage = winrt::box_value<Handler>({ std::forward<decltype(callback)>(callback) });
                self->AddHandler(routedEvent, storage, true);
            };
            bind(Microsoft::UI::Xaml::UIElement::PointerPressedEvent(), m_pointerPressedHandler,
                [this](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args) { BeginDrag(args); });
            bind(Microsoft::UI::Xaml::UIElement::PointerMovedEvent(), m_pointerMovedHandler,
                [this](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args) { UpdateDrag(args); });
            bind(Microsoft::UI::Xaml::UIElement::PointerReleasedEvent(), m_pointerReleasedHandler,
                [this](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args) { EndDragFromRelease(args); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCaptureLostEvent(), m_pointerCaptureLostHandler,
                [this](auto const&, auto const&) { EndDrag(true, true); });
            bind(Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(), m_pointerCanceledHandler,
                [this](auto const&, auto const&) { EndDrag(true, true); });

            self->Click([this](auto const&, auto const&) { m_dragOverrideArmed = false; });
        }

        void RefreshInteractionTarget()
        {
            auto self = static_cast<Self*>(this);
            auto root = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto xamlRoot = self->XamlRoot();
            if (!root || !xamlRoot)
            {
                m_pointerField.Detach();
                m_knob = nullptr;
                return;
            }

            m_knob = FindNamedDescendant(root, L"SwitchKnob").try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!m_knob)
            {
                m_pointerField.Detach();
                return;
            }

            Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetIsTranslationEnabled(m_knob, true);
            if (!m_dragging)
            {
                SetElementTranslation(m_knob, TranslationForRatio(SemanticRatio(self->IsChecked())));
                auto const scale = std::clamp(implementation::LiquidGlassInteraction::GetRestScale(root), .25, 4.0);
                SetElementScale(m_knob, scale, scale);
            }

            auto weak = self->get_weak();
            m_pointerField.Attach(xamlRoot, m_knob, [weak]() -> WinUI::Composition::Hlsl::LiquidGlassBrush
            {
                if (auto owner = weak.get()) return owner->GlassBrush();
                return nullptr;
            });
        }

        bool TryHandleToggle()
        {
            if (!m_dragOverrideArmed) return false;
            auto self = static_cast<Self*>(this);
            auto const targetChecked = std::clamp(m_visualRatio, 0.0, 1.0) >= 0.5;
            auto current = self->IsChecked();
            m_dragOverrideArmed = false;
            m_visualRatio = targetChecked ? 1.0 : 0.0;
            if (!current || current.Value() != targetChecked)
                self->IsChecked(box_value(targetChecked).as<Windows::Foundation::IReference<bool>>());
            return true;
        }

    private:
        static constexpr double kTravelDips = 57.9;
        static constexpr double kOverscrollDamping = 22.0;
        static constexpr double kDragThresholdDips = 4.0;

        static double SemanticRatio(Windows::Foundation::IReference<bool> const& value)
        {
            if (!value) return 0.5;
            return value.Value() ? 1.0 : 0.0;
        }

        static Windows::Foundation::Numerics::float3 TranslationForRatio(double ratio)
        {
            return { static_cast<float>(ratio * kTravelDips), 0.0f, 0.0f };
        }

        void SyncSemanticPosition(bool animate)
        {
            if (!m_knob) return;
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!owner) return;
            auto const target = TranslationForRatio(SemanticRatio(self->IsChecked()));
            if (animate)
                AnimateElementTranslation(owner, m_knob, target, implementation::LiquidGlassInteraction::GetMotionDuration(owner));
            else
                SetElementTranslation(m_knob, target);
        }

        void BeginDrag(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (m_dragging) return;
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = self->template try_as<Microsoft::UI::Xaml::UIElement>();
            auto frameworkElement = self->template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!owner || !element || !frameworkElement) return;
            if (!m_knob) RefreshInteractionTarget();
            if (!m_knob) return;

            auto xamlRoot = frameworkElement.XamlRoot();
            m_coordinateRoot = xamlRoot ? xamlRoot.Content() : Microsoft::UI::Xaml::UIElement{ nullptr };
            if (!m_coordinateRoot) m_coordinateRoot = element;
            auto const point = args.GetCurrentPoint(m_coordinateRoot);
            m_pointerId = point.PointerId();
            m_dragStart = point.Position();
            m_baseRatio = SemanticRatio(self->IsChecked());
            m_visualRatio = m_baseRatio;
            m_dragOverrideArmed = false;
            m_dragging = true;
            element.CapturePointer(args.Pointer());
            SetElementTranslation(m_knob, TranslationForRatio(m_baseRatio));

            auto const pressedScale = std::clamp(
                implementation::LiquidGlassInteraction::GetPressedScale(owner), .25, 4.0);
            AnimateElementScale(owner, m_knob, pressedScale, implementation::LiquidGlassInteraction::GetMotionDuration(owner));
        }

        void UpdateDrag(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!m_dragging || !m_coordinateRoot || !m_knob) return;
            auto const point = args.GetCurrentPoint(m_coordinateRoot);
            if (point.PointerId() != m_pointerId) return;
            auto self = static_cast<Self*>(this);
            auto const direction = self->FlowDirection() == Microsoft::UI::Xaml::FlowDirection::RightToLeft ? -1.0 : 1.0;
            auto const delta = static_cast<double>(point.Position().X - m_dragStart.X) * direction;
            if (std::abs(delta) >= kDragThresholdDips) m_dragOverrideArmed = true;

            auto ratio = m_baseRatio + delta / kTravelDips;
            if (ratio < 0.0) ratio /= kOverscrollDamping;
            else if (ratio > 1.0) ratio = 1.0 + (ratio - 1.0) / kOverscrollDamping;
            m_visualRatio = ratio;
            SetElementTranslation(m_knob, TranslationForRatio(ratio));
        }

        void EndDragFromRelease(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
        {
            // If ButtonBase already invoked OnToggle, TryHandleToggle has committed and
            // cleared the override. A captured release outside the control may not produce a
            // native click, so commit the same ratio here when the override is still armed.
            if (m_dragging && m_dragOverrideArmed)
            {
                auto self = static_cast<Self*>(this);
                auto const targetChecked = std::clamp(m_visualRatio, 0.0, 1.0) >= 0.5;
                auto current = self->IsChecked();
                m_dragOverrideArmed = false;
                if (!current || current.Value() != targetChecked)
                    self->IsChecked(box_value(targetChecked).as<Windows::Foundation::IReference<bool>>());
            }
            EndDrag(true, false);
        }

        void EndDrag(bool animate, bool clearOverride)
        {
            if (!m_dragging)
            {
                if (clearOverride) m_dragOverrideArmed = false;
                return;
            }

            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto semanticRatio = SemanticRatio(self->IsChecked());
            auto targetRatio = m_dragOverrideArmed
                ? (std::clamp(m_visualRatio, 0.0, 1.0) >= 0.5 ? 1.0 : 0.0)
                : semanticRatio;
            if (clearOverride)
            {
                m_dragOverrideArmed = false;
                targetRatio = semanticRatio;
            }

            if (m_knob)
            {
                auto const target = TranslationForRatio(targetRatio);
                if (animate && owner)
                    AnimateElementTranslation(owner, m_knob, target, implementation::LiquidGlassInteraction::GetMotionDuration(owner));
                else
                    SetElementTranslation(m_knob, target);

                if (owner)
                {
                    auto const restScale = std::clamp(implementation::LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
                    AnimateElementScale(owner, m_knob, restScale, implementation::LiquidGlassInteraction::GetMotionDuration(owner));
                }
            }

            if (auto element = self->template try_as<Microsoft::UI::Xaml::UIElement>())
                element.ReleasePointerCaptures();
            m_coordinateRoot = nullptr;
            m_pointerId = 0;
            m_dragging = false;
            if (clearOverride) m_dragOverrideArmed = false;
        }

        PointerFieldSurface m_pointerField;
        Microsoft::UI::Xaml::FrameworkElement m_knob{ nullptr };
        Microsoft::UI::Xaml::UIElement m_coordinateRoot{ nullptr };
        Windows::Foundation::IInspectable m_pointerPressedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerMovedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerReleasedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCaptureLostHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCanceledHandler{ nullptr };
        Windows::Foundation::Point m_dragStart{};
        double m_baseRatio{};
        double m_visualRatio{};
        uint32_t m_pointerId{};
        bool m_dragging{};
        bool m_dragOverrideArmed{};
    };
}

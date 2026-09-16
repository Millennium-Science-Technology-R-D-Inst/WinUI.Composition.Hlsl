#pragma once

#include "ChildSurfaceInteraction.hpp"
#include "MotionAnimation.hpp"
#include "PressOpticsHelper.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self>
    class SliderDragMotionHelper
    {
    public:
        SliderDragMotionHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const&, auto const&) { RefreshInteractionTarget(); });
            self->Unloaded([this](auto const&, auto const&) { ClearForTeardown(); });
            self->RegisterPropertyChangedCallback(
                Microsoft::UI::Xaml::Controls::Slider::OrientationProperty(),
                [this](auto const&, auto const&) { RefreshInteractionTarget(); });
            self->RegisterPropertyChangedCallback(Self::GlassBrushProperty(), [this](auto const&, auto const&)
            {
                if (m_dragOptics.active) RestoreOptics(m_dragOptics);
                RefreshInteractionTarget();
            });
        }

        void RefreshInteractionTarget()
        {
            auto self = static_cast<Self*>(this);
            auto root = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!root) return;

            auto const horizontal = self->Orientation() == Microsoft::UI::Xaml::Controls::Orientation::Horizontal;
            auto const thumbName = horizontal ? L"HorizontalThumb" : L"VerticalThumb";
            auto thumb = FindNamedDescendant(root, thumbName).try_as<Microsoft::UI::Xaml::Controls::Primitives::Thumb>();
            if (!thumb) thumb = FindFirstDescendant<Microsoft::UI::Xaml::Controls::Primitives::Thumb>(root);
            if (!thumb) return;

            auto surface = FindFirstDescendant<Microsoft::UI::Xaml::Controls::Border>(thumb);
            if (!surface) return;

            if (!m_thumb || get_abi(m_thumb) != get_abi(thumb))
            {
                DetachThumbHandlers();
                m_thumb = thumb;
                auto weak = self->get_weak();
                m_dragStartedToken = m_thumb.DragStarted([weak](auto const&, auto const&)
                {
                    if (auto owner = weak.get())
                        static_cast<SliderDragMotionHelper<Self>*>(owner.get())->BeginDrag();
                });
                m_dragCompletedToken = m_thumb.DragCompleted([weak](auto const&, auto const&)
                {
                    if (auto owner = weak.get())
                        static_cast<SliderDragMotionHelper<Self>*>(owner.get())->EndDrag(true);
                });
            }

            m_surface = surface;
            if (!m_dragging)
            {
                auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
                if (!owner) return;
                auto const scale = std::clamp(
                    implementation::LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
                SetElementScale(m_surface, scale, scale);
            }
        }

    private:
        void BeginDrag()
        {
            if (m_dragging) return;
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!owner) return;
            if (!m_surface) RefreshInteractionTarget();
            if (!m_surface) return;

            m_dragging = true;
            auto const scale = std::clamp(
                implementation::LiquidGlassInteraction::GetPressedScale(owner), .25, 4.0);
            AnimateElementScale(
                owner,
                m_surface,
                scale,
                scale,
                implementation::LiquidGlassInteraction::GetMotionDuration(owner));
            EnterPressedOptics(owner, self->GlassBrush(), m_dragOptics);
        }

        void EndDrag(bool animate)
        {
            if (!m_dragging) return;
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            m_dragging = false;
            if (!owner)
            {
                m_dragOptics = {};
                return;
            }

            if (m_surface)
            {
                auto const scale = std::clamp(
                    implementation::LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
                if (animate)
                {
                    AnimateElementScale(
                        owner,
                        m_surface,
                        scale,
                        scale,
                        implementation::LiquidGlassInteraction::GetMotionDuration(owner));
                }
                else
                {
                    SetElementScale(m_surface, scale, scale);
                }
            }
            LeavePressedOptics(owner, m_dragOptics);
        }

        void DetachThumbHandlers()
        {
            if (m_thumb)
            {
                if (m_dragStartedToken.value) m_thumb.DragStarted(m_dragStartedToken);
                if (m_dragCompletedToken.value) m_thumb.DragCompleted(m_dragCompletedToken);
            }
            m_dragStartedToken = {};
            m_dragCompletedToken = {};
            m_thumb = nullptr;
        }

        void ClearForTeardown()
        {
            // XAML teardown is intentionally no-write. The compositor/effect may already be
            // closed, so simply drop transient state instead of restoring visual properties.
            DetachThumbHandlers();
            m_surface = nullptr;
            m_dragOptics = {};
            m_dragging = false;
        }

        Microsoft::UI::Xaml::Controls::Primitives::Thumb m_thumb{ nullptr };
        Microsoft::UI::Xaml::Controls::Border m_surface{ nullptr };
        event_token m_dragStartedToken{};
        event_token m_dragCompletedToken{};
        OpticsSnapshot m_dragOptics;
        bool m_dragging{};
    };
}

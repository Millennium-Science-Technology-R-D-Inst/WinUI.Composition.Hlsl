#pragma once

#include "ChildSurfaceInteraction.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self>
    class SwitchElevationHelper
    {
    public:
        SwitchElevationHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const&, auto const&)
            {
                m_loaded = true;
                RefreshSurface();
                ApplyElevation();
            });
            self->Unloaded([this](auto const&, auto const&)
            {
                // XAML/compositor teardown is deliberately no-write. The next Loaded
                // resolves the new template surface and restores authored elevation.
                m_loaded = false;
                m_surface = nullptr;
            });
        }

        void RefreshElevationSurface()
        {
            if (!m_loaded) return;
            // OnApplyTemplate may replace the Border while this helper is still loaded.
            // Drop the old reference before resolving the new template part.
            m_surface = nullptr;
            RefreshSurface();
            ApplyElevation();
        }

    private:
        // Kube keeps the outer "0 4px 22px rgba(0,0,0,.1)" shadow constant.
        // Press feedback belongs to scale, body opacity/refraction, and the inset pair;
        // changing ThemeShadow elevation creates a second moving outer layer.
        static constexpr float kElevation = 8.0f;

        void RefreshSurface()
        {
            auto self = static_cast<Self*>(this);
            auto root = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!root) return;

            auto surface = FindNamedDescendant(root, L"SwitchKnobSurface")
                .try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!surface)
            {
                m_surface = nullptr;
                return;
            }

            if (!m_surface || get_abi(m_surface) != get_abi(surface))
            {
                m_surface = surface;
                Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::SetIsTranslationEnabled(m_surface, true);
            }
        }

        void ApplyElevation()
        {
            if (!m_loaded) return;
            if (!m_surface) RefreshSurface();
            if (!m_surface) return;

            auto current = m_surface.Translation();
            current.z = kElevation;
            m_surface.Translation(current);
        }

        Microsoft::UI::Xaml::FrameworkElement m_surface{ nullptr };
        bool m_loaded{};
    };
}

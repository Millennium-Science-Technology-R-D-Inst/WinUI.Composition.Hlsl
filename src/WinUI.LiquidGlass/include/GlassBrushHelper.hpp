#pragma once

#include <algorithm>
#include <cmath>
#include <type_traits>

#include "EnsureDependencyProperty.hpp"
#include "ResourceDictionaryLoader.hpp"

namespace winrt::WinUI::LiquidGlass::implementation
{
    struct LiquidGlassSlider;
}

namespace winrt::WinUI::LiquidGlass::detail
{
    inline void EnsureLiquidGlassInputResources()
    {
        // Keep the sealed native input templates available before a wrapper creates its
        // child control. Each resource tracks success independently so a later dictionary
        // failure cannot cause an already merged dictionary to be appended twice on retry.
        static bool nativeInputsLoaded{};
        static bool passwordInputLoaded{};

        if (!nativeInputsLoaded)
        {
            nativeInputsLoaded = EnsureMergedResourceDictionary(
                L"ms-appx:///WinUI.LiquidGlass/Themes/NativeInputs.xaml");
        }
        if (!passwordInputLoaded)
        {
            passwordInputLoaded = EnsureMergedResourceDictionary(
                L"ms-appx:///WinUI.LiquidGlass/Themes/PasswordInput.xaml");
        }
    }

    template<typename Derived>
    struct GlassBrushHelper : EnsureDependencyProperty<Derived>
    {
        using Brush = WinUI::Composition::Hlsl::LiquidGlassBrush;
        static constexpr bool AutoRasterizationScale =
            !std::is_same_v<Derived, implementation::LiquidGlassSlider>;

        GlassBrushHelper()
        {
            // The component keeps its native-input templates in shared dictionaries.
            // Load them before a derived constructor assigns DefaultStyleKey or creates
            // a sealed native child such as PasswordBox.
            EnsureLiquidGlassInputResources();

            if constexpr (AutoRasterizationScale)
            {
                auto self = static_cast<Derived*>(this);
                self->Loaded([this](auto const&, auto const&)
                {
                    AttachXamlRoot();
                    SyncRasterizationScale();
                });
                self->Unloaded([this](auto const&, auto const&)
                {
                    DetachXamlRoot();
                });
            }
        }

        static void EnsureDependencyProperties()
        {
            (void)GlassBrushProperty();
        }

        static Microsoft::UI::Xaml::DependencyProperty GlassBrushProperty()
        {
            namespace Xaml = Microsoft::UI::Xaml;

            static auto const property = Xaml::DependencyProperty::Register(
                L"GlassBrush",
                xaml_typename<Brush>(),
                xaml_typename<typename Derived::class_type>(),
                Xaml::PropertyMetadata{
                    Windows::Foundation::IInspectable{ nullptr },
                    Xaml::PropertyChangedCallback{
                        [](Xaml::DependencyObject const& object, Xaml::DependencyPropertyChangedEventArgs const& args)
                        {
                            auto self = EnsureDependencyProperty<Derived>::GetSelf(object);
                            auto const brush = args.NewValue().try_as<Brush>();

                            // Keep scale synchronization outside the virtual-by-convention
                            // ApplyGlassBrush hook. PasswordBox/SearchBox replace that hook to
                            // paint an inner native control; a brush swapped after Loaded must
                            // still inherit the current XamlRoot scale immediately.
                            auto helper = static_cast<GlassBrushHelper<Derived>*>(self);
                            helper->PrepareGlassBrush(brush);
                            self->ApplyGlassBrush(brush);
                        } } });
            return property;
        }

        Brush GlassBrush() const
        {
            return static_cast<Derived const*>(this)->GetValue(GlassBrushProperty()).try_as<Brush>();
        }

        void GlassBrush(Brush const& value)
        {
            static_cast<Derived*>(this)->SetValue(GlassBrushProperty(), value);
        }

        void ApplyGlassBrush(Brush const& value)
        {
            m_glassBrush = value;
            static_cast<Derived*>(this)->Background(
                value ? value.as<Microsoft::UI::Xaml::Media::Brush>() : Microsoft::UI::Xaml::Media::Brush{ nullptr });
        }

    protected:
        Brush m_glassBrush{ nullptr };

    private:
        void PrepareGlassBrush(Brush const& value)
        {
            m_glassBrush = value;
            if constexpr (AutoRasterizationScale)
            {
                AttachXamlRoot();
                SyncRasterizationScale();
            }
        }

        void AttachXamlRoot()
        {
            if constexpr (AutoRasterizationScale)
            {
                auto self = static_cast<Derived*>(this);
                auto root = self->XamlRoot();
                if (root && m_xamlRoot && get_abi(root) == get_abi(m_xamlRoot))
                {
                    return;
                }

                DetachXamlRoot();
                m_xamlRoot = root;
                if (m_xamlRoot)
                {
                    m_xamlRootChangedRevoker = m_xamlRoot.Changed(auto_revoke, [this](auto const&, auto const&)
                    {
                        SyncRasterizationScale();
                    });
                }
            }
        }

        void DetachXamlRoot() noexcept
        {
            if constexpr (AutoRasterizationScale)
            {
                try
                {
                    m_xamlRootChangedRevoker.revoke();
                }
                catch (...)
                {
                }
                m_xamlRoot = nullptr;
            }
        }

        void SyncRasterizationScale()
        {
            if constexpr (AutoRasterizationScale)
            {
                auto self = static_cast<Derived*>(this);
                auto root = self->XamlRoot();
                auto brush = GlassBrush();
                if (!root || !brush)
                {
                    return;
                }

                auto const scale = std::max(root.RasterizationScale(), 0.01);
                if (std::abs(brush.RasterizationScale() - scale) > 1e-6)
                {
                    brush.RasterizationScale(scale);
                }
            }
        }

        Microsoft::UI::Xaml::XamlRoot m_xamlRoot{ nullptr };
        Microsoft::UI::Xaml::XamlRoot::Changed_revoker m_xamlRootChangedRevoker{};
    };
}

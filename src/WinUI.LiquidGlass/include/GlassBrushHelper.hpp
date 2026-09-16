#pragma once

#include "EnsureDependencyProperty.hpp"
#include "ResourceDictionaryLoader.hpp"

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

        GlassBrushHelper()
        {
            // The component keeps its native-input templates in shared dictionaries.
            // Load them before a derived constructor assigns DefaultStyleKey or creates
            // a sealed native child such as PasswordBox.
            EnsureLiquidGlassInputResources();
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
                            self->ApplyGlassBrush(args.NewValue().try_as<Brush>());
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
    };
}

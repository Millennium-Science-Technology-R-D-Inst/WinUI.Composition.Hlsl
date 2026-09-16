#pragma once

#include "EnsureDependencyProperty.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    inline void EnsureLiquidGlassInputResources()
    {
        // Do not use a function-local `static bool = [] { ... }()` here. If a control is
        // constructed before Application::Current() is available, that pattern permanently
        // caches `false` and the input templates can never be loaded later in the process.
        // XAML controls are UI-thread-affine, so this small one-time guard needs no locking.
        static bool loaded{};
        if (loaded)
        {
            return;
        }

        namespace Xaml = Microsoft::UI::Xaml;
        auto app = Xaml::Application::Current();
        if (!app)
        {
            return;
        }

        auto dictionaries = app.Resources().MergedDictionaries();
        for (auto const* uri : {
            L"ms-appx:///WinUI.LiquidGlass/Themes/NativeInputs.xaml",
            L"ms-appx:///WinUI.LiquidGlass/Themes/PasswordInput.xaml" })
        {
            Xaml::ResourceDictionary dictionary;
            dictionary.Source(Windows::Foundation::Uri{ uri });
            dictionaries.Append(dictionary);
        }
        loaded = true;
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

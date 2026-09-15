#pragma once

#include "EnsureDependencyProperty.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Derived>
    struct GlassBrushHelper : EnsureDependencyProperty<Derived>
    {
        using Brush = WinUI::Composition::Hlsl::LiquidGlassBrush;

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

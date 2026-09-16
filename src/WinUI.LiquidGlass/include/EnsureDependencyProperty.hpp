#pragma once

namespace winrt::WinUI::LiquidGlass::detail
{
    template <typename Derived>
    struct EnsureDependencyProperty
    {
        EnsureDependencyProperty()
        {
            Derived::EnsureDependencyProperties();
        }

        static Derived* GetSelf(winrt::Microsoft::UI::Xaml::DependencyObject const& object)
        {
            return winrt::get_self<Derived>(object.as<typename Derived::class_type>());
        }
    };
}

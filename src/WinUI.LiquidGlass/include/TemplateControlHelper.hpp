#pragma once

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self, bool UseXamlResource = true>
    struct XamlResourceHelper
    {
        XamlResourceHelper()
        {
            if constexpr (!UseXamlResource)
            {
                return;
            }
            else if constexpr (requires { Self::ResourceUri; })
            {
                if constexpr (requires(Self* value) { value->DefaultStyleResourceUri(winrt::Windows::Foundation::Uri{ Self::ResourceUri }); })
                {
                    static_cast<Self*>(this)->DefaultStyleResourceUri(
                        winrt::Windows::Foundation::Uri{ Self::ResourceUri });
                }
                else
                {
                    [[maybe_unused]] static bool resourceLoaded = []
                    {
                        winrt::Microsoft::UI::Xaml::ResourceDictionary dictionary;
                        dictionary.Source(winrt::Windows::Foundation::Uri{ Self::ResourceUri });
                        winrt::Microsoft::UI::Xaml::Application::Current()
                            .Resources()
                            .MergedDictionaries()
                            .Append(dictionary);
                        return true;
                    }();
                }
            }
            else
            {
                static_assert(!sizeof(Self), "Templated control is missing ResourceUri.");
            }
        }
    };

    template<typename Self, bool UseXamlResource = true>
    struct TemplateControlHelper : XamlResourceHelper<Self, UseXamlResource>
    {
        TemplateControlHelper()
        {
            using ProjectionType = typename Self::class_type;
            static_cast<Self*>(this)->DefaultStyleKey(
                winrt::box_value(winrt::xaml_typename<ProjectionType>()));
        }
    };
}

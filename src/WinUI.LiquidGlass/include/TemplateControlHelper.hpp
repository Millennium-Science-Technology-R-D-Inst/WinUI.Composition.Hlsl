#pragma once

#include "ResourceDictionaryLoader.hpp"

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
                    // Older WinUI projections may not expose DefaultStyleResourceUri.
                    // Metadata/type activation can happen before Application::Current(), so
                    // cache only a successful merge and retry a previous miss later.
                    static bool resourceLoaded{};
                    if (!resourceLoaded)
                    {
                        resourceLoaded = EnsureMergedResourceDictionary(Self::ResourceUri);
                    }
                }
            }
            else
            {
                static_assert(!sizeof(Self), "Templated control is missing ResourceUri.");
            }
        }
    };

    // DefaultStyleKey is protected on Control. A sibling CRTP base cannot legally
    // invoke it for every WinUI inheritance shape (notably ToggleButton-derived
    // controls), so concrete implementation constructors set the key themselves.
    template<typename Self, bool UseXamlResource = true>
    struct TemplateControlHelper : XamlResourceHelper<Self, UseXamlResource>
    {
        TemplateControlHelper() = default;
    };
}

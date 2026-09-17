#pragma once

namespace winrt::WinUI::LiquidGlass::detail
{
    // Adds a XAML dictionary to Application.Resources exactly once. Construction can run
    // during metadata activation before Application::Current() exists, so a miss is not
    // cached and callers can retry later. Scanning the existing merge list also makes a
    // partially completed earlier load idempotent instead of appending duplicate dictionaries.
    inline bool EnsureMergedResourceDictionary(std::wstring_view uri)
    {
        namespace Xaml = Microsoft::UI::Xaml;

        auto app = Xaml::Application::Current();
        if (!app) return false;

        auto dictionaries = app.Resources().MergedDictionaries();
        auto const targetUri = hstring{ uri };
        for (auto const& existing : dictionaries)
        {
            auto source = existing.Source();
            if (source && source.AbsoluteUri() == targetUri)
            {
                return true;
            }
        }

        Xaml::ResourceDictionary dictionary;
        dictionary.Source(Windows::Foundation::Uri{ targetUri });
        dictionaries.Append(dictionary);
        return true;
    }
}

#pragma once

#include "LiquidGlassControls.h"
#include "LiquidGlassFloatingPanel.g.h"
#include "LiquidGlassSearchBox.g.h"

namespace winrt::WinUI::LiquidGlass::implementation
{
    struct LiquidGlassFloatingPanel :
        LiquidGlassFloatingPanelT<LiquidGlassFloatingPanel>,
        detail::GlassBrushHelper<LiquidGlassFloatingPanel>,
        detail::PointerLightHelper<LiquidGlassFloatingPanel>
    {
        LiquidGlassFloatingPanel();
    };

    struct LiquidGlassSearchBox :
        LiquidGlassSearchBoxT<LiquidGlassSearchBox>,
        detail::GlassBrushHelper<LiquidGlassSearchBox>,
        detail::PointerLightHelper<LiquidGlassSearchBox>,
        detail::FocusOpticsHelper<LiquidGlassSearchBox>
    {
        LiquidGlassSearchBox();

        hstring Text() const;
        void Text(hstring const& value);
        hstring PlaceholderText() const;
        void PlaceholderText(hstring const& value);
        Windows::Foundation::IInspectable ItemsSource() const;
        void ItemsSource(Windows::Foundation::IInspectable const& value);
        Microsoft::UI::Xaml::DataTemplate ItemTemplate() const;
        void ItemTemplate(Microsoft::UI::Xaml::DataTemplate const& value);
        bool IsSuggestionListOpen() const;
        void IsSuggestionListOpen(bool value);
        bool AutoMaximizeSuggestionArea() const;
        void AutoMaximizeSuggestionArea(bool value);
        double MaxSuggestionListHeight() const;
        void MaxSuggestionListHeight(double value);
        bool UpdateTextOnSelect() const;
        void UpdateTextOnSelect(bool value);
        hstring TextMemberPath() const;
        void TextMemberPath(hstring const& value);
        Windows::Foundation::IInspectable Header() const;
        void Header(Windows::Foundation::IInspectable const& value);
        Microsoft::UI::Xaml::Controls::IconElement QueryIcon() const;
        void QueryIcon(Microsoft::UI::Xaml::Controls::IconElement const& value);
        Microsoft::UI::Xaml::Controls::AutoSuggestBox InnerAutoSuggestBox() const;

        event_token SuggestionChosen(
            Windows::Foundation::TypedEventHandler<
                Microsoft::UI::Xaml::Controls::AutoSuggestBox,
                Microsoft::UI::Xaml::Controls::AutoSuggestBoxSuggestionChosenEventArgs> const& handler);
        void SuggestionChosen(event_token const& token) noexcept;
        event_token TextChanged(
            Windows::Foundation::TypedEventHandler<
                Microsoft::UI::Xaml::Controls::AutoSuggestBox,
                Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs> const& handler);
        void TextChanged(event_token const& token) noexcept;
        event_token QuerySubmitted(
            Windows::Foundation::TypedEventHandler<
                Microsoft::UI::Xaml::Controls::AutoSuggestBox,
                Microsoft::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs> const& handler);
        void QuerySubmitted(event_token const& token) noexcept;

        // C++ implementation hook used by GlassBrushHelper; it is not projected by the IDL.
        void ApplyGlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value);

    private:
        Microsoft::UI::Xaml::Controls::AutoSuggestBox m_autoSuggestBox{ nullptr };
    };
}

namespace winrt::WinUI::LiquidGlass::factory_implementation
{
    struct LiquidGlassFloatingPanel : LiquidGlassFloatingPanelT<LiquidGlassFloatingPanel, implementation::LiquidGlassFloatingPanel> {};
    struct LiquidGlassSearchBox : LiquidGlassSearchBoxT<LiquidGlassSearchBox, implementation::LiquidGlassSearchBox> {};
}

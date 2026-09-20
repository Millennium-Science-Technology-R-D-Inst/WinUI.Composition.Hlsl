#pragma once

#include "LiquidGlassControls.h"
#include "LiquidGlassFloatingPanel.g.h"
#include "LiquidGlassSearchBox.g.h"

namespace winrt::WinUI::LiquidGlass::implementation
{
    struct LiquidGlassFloatingPanel :
        LiquidGlassFloatingPanelT<LiquidGlassFloatingPanel>,
        detail::GlassBrushHelper<LiquidGlassFloatingPanel>,
        detail::PointerLightHelper<LiquidGlassFloatingPanel>,
        detail::PointerFieldHelper<LiquidGlassFloatingPanel>
    {
        LiquidGlassFloatingPanel();
    };

    struct LiquidGlassSearchBox :
        LiquidGlassSearchBoxT<LiquidGlassSearchBox>,
        detail::GlassBrushHelper<LiquidGlassSearchBox>,
        detail::PointerLightHelper<LiquidGlassSearchBox>,
        detail::PointerFieldHelper<LiquidGlassSearchBox>,
        detail::SearchMotionHelper<LiquidGlassSearchBox>,
        detail::MotionDefaults<LiquidGlassSearchBox, detail::MotionProfile::Search>
    {
        LiquidGlassSearchBox();

        static void EnsureDependencyProperties();

        hstring Text() const;
        void Text(hstring const& value);
        static Microsoft::UI::Xaml::DependencyProperty TextProperty();

        hstring PlaceholderText() const;
        void PlaceholderText(hstring const& value);
        static Microsoft::UI::Xaml::DependencyProperty PlaceholderTextProperty();

        Windows::Foundation::IInspectable ItemsSource() const;
        void ItemsSource(Windows::Foundation::IInspectable const& value);
        static Microsoft::UI::Xaml::DependencyProperty ItemsSourceProperty();

        Microsoft::UI::Xaml::DataTemplate ItemTemplate() const;
        void ItemTemplate(Microsoft::UI::Xaml::DataTemplate const& value);
        static Microsoft::UI::Xaml::DependencyProperty ItemTemplateProperty();

        bool IsSuggestionListOpen() const;
        void IsSuggestionListOpen(bool value);
        static Microsoft::UI::Xaml::DependencyProperty IsSuggestionListOpenProperty();

        bool AutoMaximizeSuggestionArea() const;
        void AutoMaximizeSuggestionArea(bool value);
        static Microsoft::UI::Xaml::DependencyProperty AutoMaximizeSuggestionAreaProperty();

        double MaxSuggestionListHeight() const;
        void MaxSuggestionListHeight(double value);
        static Microsoft::UI::Xaml::DependencyProperty MaxSuggestionListHeightProperty();

        bool UpdateTextOnSelect() const;
        void UpdateTextOnSelect(bool value);
        static Microsoft::UI::Xaml::DependencyProperty UpdateTextOnSelectProperty();

        hstring TextMemberPath() const;
        void TextMemberPath(hstring const& value);
        static Microsoft::UI::Xaml::DependencyProperty TextMemberPathProperty();

        Windows::Foundation::IInspectable Header() const;
        void Header(Windows::Foundation::IInspectable const& value);
        static Microsoft::UI::Xaml::DependencyProperty HeaderProperty();

        Microsoft::UI::Xaml::Style TextBoxStyle() const;
        void TextBoxStyle(Microsoft::UI::Xaml::Style const& value);
        static Microsoft::UI::Xaml::DependencyProperty TextBoxStyleProperty();

        Microsoft::UI::Xaml::Controls::IconElement QueryIcon() const;
        void QueryIcon(Microsoft::UI::Xaml::Controls::IconElement const& value);
        static Microsoft::UI::Xaml::DependencyProperty QueryIconProperty();

        Microsoft::UI::Xaml::Controls::LightDismissOverlayMode LightDismissOverlayMode() const;
        void LightDismissOverlayMode(Microsoft::UI::Xaml::Controls::LightDismissOverlayMode value);
        static Microsoft::UI::Xaml::DependencyProperty LightDismissOverlayModeProperty();

        Windows::Foundation::IInspectable Description() const;
        void Description(Windows::Foundation::IInspectable const& value);
        static Microsoft::UI::Xaml::DependencyProperty DescriptionProperty();

        Microsoft::UI::Xaml::Controls::ControlHeaderPlacement HeaderPlacement() const;
        void HeaderPlacement(Microsoft::UI::Xaml::Controls::ControlHeaderPlacement value);
        static Microsoft::UI::Xaml::DependencyProperty HeaderPlacementProperty();

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

        void ApplyGlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value);

    private:
        template<typename T>
        static Microsoft::UI::Xaml::DependencyProperty RegisterForwardedProperty(
            wchar_t const* name,
            Windows::Foundation::IInspectable const& defaultValue);

        static Microsoft::UI::Xaml::PropertyChangedCallback ForwardedPropertyChangedCallback();
        static Microsoft::UI::Xaml::DependencyProperty InnerPropertyFor(Microsoft::UI::Xaml::DependencyProperty const& outerProperty);

        void OnForwardedPropertyChanged(Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const& args);
        void ForwardPropertyToInner(
            Microsoft::UI::Xaml::DependencyProperty const& outerProperty,
            Windows::Foundation::IInspectable const& value);
        void MirrorPropertyFromInner(
            Microsoft::UI::Xaml::DependencyProperty const& outerProperty,
            Windows::Foundation::IInspectable const& value);
        void AttachInnerPropertyMirrors();

        Microsoft::UI::Xaml::Controls::AutoSuggestBox m_autoSuggestBox{ nullptr };
        bool m_syncingToInner{};
        bool m_syncingFromInner{};
    };
}

namespace winrt::WinUI::LiquidGlass::factory_implementation
{
    struct LiquidGlassFloatingPanel : LiquidGlassFloatingPanelT<LiquidGlassFloatingPanel, implementation::LiquidGlassFloatingPanel> {};
    struct LiquidGlassSearchBox : LiquidGlassSearchBoxT<LiquidGlassSearchBox, implementation::LiquidGlassSearchBox> {};
}

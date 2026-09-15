#include "pch.h"
#include "winrt_module_imports.h"
#include "LiquidGlassSpecializedControls.h"

#if __has_include("LiquidGlassFloatingPanel.g.cpp")
#include "LiquidGlassFloatingPanel.g.cpp"
#include "LiquidGlassSearchBox.g.cpp"
#endif

namespace winrt::WinUI::LiquidGlass::implementation
{
    namespace Xaml = Microsoft::UI::Xaml;
    namespace Controls = Xaml::Controls;
    namespace Media = Xaml::Media;

    namespace
    {
        using Brush = WinUI::Composition::Hlsl::LiquidGlassBrush;

        Media::Brush AsMediaBrush(Brush const& brush)
        {
            return brush ? brush.as<Media::Brush>() : Media::Brush{ nullptr };
        }

        void EnsureSpecializedResources()
        {
            [[maybe_unused]] static bool loaded = []
            {
                Xaml::ResourceDictionary dictionary;
                dictionary.Source(Windows::Foundation::Uri{
                    L"ms-appx:///WinUI.LiquidGlass/Themes/Specialized.xaml" });
                Xaml::Application::Current().Resources().MergedDictionaries().Append(dictionary);
                return true;
            }();
        }
    }

    LiquidGlassFloatingPanel::LiquidGlassFloatingPanel()
    {
        EnsureSpecializedResources();
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(WinUI::LiquidGlass::LiquidGlassPresets::CreateBrush(
            WinUI::LiquidGlass::LiquidGlassPreset::FloatingPanel));
    }

    void LiquidGlassSearchBox::ApplyGlassBrush(Brush const& value)
    {
        m_glassBrush = value;

        // The wrapper owns the glass surface. Keeping the sealed AutoSuggestBox itself
        // transparent prevents its stock rectangular background from covering the rounded
        // liquid-glass geometry while retaining native text editing, suggestions and UIA.
        Background(AsMediaBrush(value));
        if (m_autoSuggestBox)
        {
            m_autoSuggestBox.Background(Media::Brush{ nullptr });
            m_autoSuggestBox.BorderBrush(Media::Brush{ nullptr });
            m_autoSuggestBox.BorderThickness({ 0.0, 0.0, 0.0, 0.0 });
        }
    }

    LiquidGlassSearchBox::LiquidGlassSearchBox()
    {
        EnsureSpecializedResources();
        DefaultStyleKey(box_value(xaml_typename<class_type>()));

        m_autoSuggestBox = Controls::AutoSuggestBox{};
        m_autoSuggestBox.HorizontalAlignment(Xaml::HorizontalAlignment::Stretch);
        m_autoSuggestBox.VerticalAlignment(Xaml::VerticalAlignment::Center);
        m_autoSuggestBox.Background(Media::Brush{ nullptr });
        m_autoSuggestBox.BorderBrush(Media::Brush{ nullptr });
        m_autoSuggestBox.BorderThickness({ 0.0, 0.0, 0.0, 0.0 });
        HorizontalContentAlignment(Xaml::HorizontalAlignment::Stretch);
        VerticalContentAlignment(Xaml::VerticalAlignment::Center);
        IsTabStop(false);
        Content(m_autoSuggestBox);

        GlassBrush(WinUI::LiquidGlass::LiquidGlassPresets::CreateBrush(
            WinUI::LiquidGlass::LiquidGlassPreset::SearchBox));
        SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.99));
        SetValue(LiquidGlassInteraction::FocusedScaleProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::FocusedTintBoostProperty(), box_value(.10));
        SetValue(LiquidGlassInteraction::FocusedContrastMultiplierProperty(), box_value(1.04));
        SetValue(LiquidGlassInteraction::FocusedRefractionMultiplierProperty(), box_value(1.06));
        SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(140.0));
    }

    hstring LiquidGlassSearchBox::Text() const { return m_autoSuggestBox ? m_autoSuggestBox.Text() : hstring{}; }
    void LiquidGlassSearchBox::Text(hstring const& value) { if (m_autoSuggestBox) m_autoSuggestBox.Text(value); }
    hstring LiquidGlassSearchBox::PlaceholderText() const { return m_autoSuggestBox ? m_autoSuggestBox.PlaceholderText() : hstring{}; }
    void LiquidGlassSearchBox::PlaceholderText(hstring const& value) { if (m_autoSuggestBox) m_autoSuggestBox.PlaceholderText(value); }
    Windows::Foundation::IInspectable LiquidGlassSearchBox::ItemsSource() const { return m_autoSuggestBox ? m_autoSuggestBox.ItemsSource() : nullptr; }
    void LiquidGlassSearchBox::ItemsSource(Windows::Foundation::IInspectable const& value) { if (m_autoSuggestBox) m_autoSuggestBox.ItemsSource(value); }
    Xaml::DataTemplate LiquidGlassSearchBox::ItemTemplate() const { return m_autoSuggestBox ? m_autoSuggestBox.ItemTemplate() : Xaml::DataTemplate{ nullptr }; }
    void LiquidGlassSearchBox::ItemTemplate(Xaml::DataTemplate const& value) { if (m_autoSuggestBox) m_autoSuggestBox.ItemTemplate(value); }
    bool LiquidGlassSearchBox::IsSuggestionListOpen() const { return m_autoSuggestBox && m_autoSuggestBox.IsSuggestionListOpen(); }
    void LiquidGlassSearchBox::IsSuggestionListOpen(bool value) { if (m_autoSuggestBox) m_autoSuggestBox.IsSuggestionListOpen(value); }
    bool LiquidGlassSearchBox::AutoMaximizeSuggestionArea() const { return m_autoSuggestBox && m_autoSuggestBox.AutoMaximizeSuggestionArea(); }
    void LiquidGlassSearchBox::AutoMaximizeSuggestionArea(bool value) { if (m_autoSuggestBox) m_autoSuggestBox.AutoMaximizeSuggestionArea(value); }
    double LiquidGlassSearchBox::MaxSuggestionListHeight() const { return m_autoSuggestBox ? m_autoSuggestBox.MaxSuggestionListHeight() : 0.0; }
    void LiquidGlassSearchBox::MaxSuggestionListHeight(double value) { if (m_autoSuggestBox) m_autoSuggestBox.MaxSuggestionListHeight(value); }
    bool LiquidGlassSearchBox::UpdateTextOnSelect() const { return m_autoSuggestBox && m_autoSuggestBox.UpdateTextOnSelect(); }
    void LiquidGlassSearchBox::UpdateTextOnSelect(bool value) { if (m_autoSuggestBox) m_autoSuggestBox.UpdateTextOnSelect(value); }
    hstring LiquidGlassSearchBox::TextMemberPath() const { return m_autoSuggestBox ? m_autoSuggestBox.TextMemberPath() : hstring{}; }
    void LiquidGlassSearchBox::TextMemberPath(hstring const& value) { if (m_autoSuggestBox) m_autoSuggestBox.TextMemberPath(value); }
    Windows::Foundation::IInspectable LiquidGlassSearchBox::Header() const { return m_autoSuggestBox ? m_autoSuggestBox.Header() : nullptr; }
    void LiquidGlassSearchBox::Header(Windows::Foundation::IInspectable const& value) { if (m_autoSuggestBox) m_autoSuggestBox.Header(value); }
    Controls::IconElement LiquidGlassSearchBox::QueryIcon() const { return m_autoSuggestBox ? m_autoSuggestBox.QueryIcon() : Controls::IconElement{ nullptr }; }
    void LiquidGlassSearchBox::QueryIcon(Controls::IconElement const& value) { if (m_autoSuggestBox) m_autoSuggestBox.QueryIcon(value); }
    Controls::AutoSuggestBox LiquidGlassSearchBox::InnerAutoSuggestBox() const { return m_autoSuggestBox; }

    event_token LiquidGlassSearchBox::SuggestionChosen(
        Windows::Foundation::TypedEventHandler<Controls::AutoSuggestBox, Controls::AutoSuggestBoxSuggestionChosenEventArgs> const& handler)
    {
        return m_autoSuggestBox.SuggestionChosen(handler);
    }

    void LiquidGlassSearchBox::SuggestionChosen(event_token const& token) noexcept
    {
        if (m_autoSuggestBox) m_autoSuggestBox.SuggestionChosen(token);
    }

    event_token LiquidGlassSearchBox::TextChanged(
        Windows::Foundation::TypedEventHandler<Controls::AutoSuggestBox, Controls::AutoSuggestBoxTextChangedEventArgs> const& handler)
    {
        return m_autoSuggestBox.TextChanged(handler);
    }

    void LiquidGlassSearchBox::TextChanged(event_token const& token) noexcept
    {
        if (m_autoSuggestBox) m_autoSuggestBox.TextChanged(token);
    }

    event_token LiquidGlassSearchBox::QuerySubmitted(
        Windows::Foundation::TypedEventHandler<Controls::AutoSuggestBox, Controls::AutoSuggestBoxQuerySubmittedEventArgs> const& handler)
    {
        return m_autoSuggestBox.QuerySubmitted(handler);
    }

    void LiquidGlassSearchBox::QuerySubmitted(event_token const& token) noexcept
    {
        if (m_autoSuggestBox) m_autoSuggestBox.QuerySubmitted(token);
    }
}

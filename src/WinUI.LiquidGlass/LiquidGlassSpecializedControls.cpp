#include "pch.h"
#include <limits>
#include "winrt_module_imports.h"
#include "LiquidGlassSpecializedControls.h"
#include "include/ResourceDictionaryLoader.hpp"

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

        struct SyncFlagGuard
        {
            explicit SyncFlagGuard(bool& value) : m_value(value) { m_value = true; }
            ~SyncFlagGuard() { m_value = false; }

            SyncFlagGuard(SyncFlagGuard const&) = delete;
            SyncFlagGuard& operator=(SyncFlagGuard const&) = delete;

        private:
            bool& m_value;
        };

        Media::Brush AsMediaBrush(Brush const& brush)
        {
            return brush ? brush.as<Media::Brush>() : Media::Brush{ nullptr };
        }

        void EnsureSpecializedResources()
        {
            static bool loaded{};
            if (!loaded)
            {
                loaded = detail::EnsureMergedResourceDictionary(
                    L"ms-appx:///WinUI.LiquidGlass/Themes/Specialized.xaml");
            }
        }

        Xaml::Style SpecializedStyle(wchar_t const* key)
        {
            auto app = Xaml::Application::Current();
            if (!app) return nullptr;
            auto resources = app.Resources();
            auto boxedKey = box_value(hstring{ key });
            return resources.HasKey(boxedKey)
                ? resources.Lookup(boxedKey).try_as<Xaml::Style>()
                : Xaml::Style{ nullptr };
        }
    }

    LiquidGlassFloatingPanel::LiquidGlassFloatingPanel()
    {
        EnsureSpecializedResources();
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(WinUI::LiquidGlass::LiquidGlassPresets::CreateBrush(
            WinUI::LiquidGlass::LiquidGlassPreset::FloatingPanel));
    }

    Xaml::PropertyChangedCallback LiquidGlassSearchBox::ForwardedPropertyChangedCallback()
    {
        return Xaml::PropertyChangedCallback{
            [](Xaml::DependencyObject const& object, Xaml::DependencyPropertyChangedEventArgs const& args)
            {
                auto self = detail::EnsureDependencyProperty<LiquidGlassSearchBox>::GetSelf(object);
                self->OnForwardedPropertyChanged(args);
            }
        };
    }

    template<typename T>
    Xaml::DependencyProperty LiquidGlassSearchBox::RegisterForwardedProperty(
        wchar_t const* name,
        Windows::Foundation::IInspectable const& defaultValue)
    {
        return Xaml::DependencyProperty::Register(
            name,
            xaml_typename<T>(),
            xaml_typename<class_type>(),
            Xaml::PropertyMetadata{ defaultValue, ForwardedPropertyChangedCallback() });
    }

    void LiquidGlassSearchBox::EnsureDependencyProperties()
    {
        (void)GlassBrushProperty();
        (void)MaxSuggestionListHeightProperty();
        (void)IsSuggestionListOpenProperty();
        (void)TextMemberPathProperty();
        (void)TextProperty();
        (void)UpdateTextOnSelectProperty();
        (void)PlaceholderTextProperty();
        (void)HeaderProperty();
        (void)AutoMaximizeSuggestionAreaProperty();
        (void)TextBoxStyleProperty();
        (void)QueryIconProperty();
        (void)LightDismissOverlayModeProperty();
        (void)DescriptionProperty();
        (void)HeaderPlacementProperty();
        (void)ItemsSourceProperty();
        (void)ItemTemplateProperty();
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::MaxSuggestionListHeightProperty()
    {
        static auto const property = RegisterForwardedProperty<double>(
            L"MaxSuggestionListHeight",
            box_value(std::numeric_limits<double>::infinity()));
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::IsSuggestionListOpenProperty()
    {
        static auto const property = RegisterForwardedProperty<bool>(L"IsSuggestionListOpen", box_value(false));
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::TextMemberPathProperty()
    {
        static auto const property = RegisterForwardedProperty<hstring>(L"TextMemberPath", box_value(hstring{}));
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::TextProperty()
    {
        static auto const property = RegisterForwardedProperty<hstring>(L"Text", box_value(hstring{}));
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::UpdateTextOnSelectProperty()
    {
        static auto const property = RegisterForwardedProperty<bool>(L"UpdateTextOnSelect", box_value(true));
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::PlaceholderTextProperty()
    {
        static auto const property = RegisterForwardedProperty<hstring>(L"PlaceholderText", box_value(hstring{}));
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::HeaderProperty()
    {
        static auto const property = RegisterForwardedProperty<Windows::Foundation::IInspectable>(
            L"Header",
            Windows::Foundation::IInspectable{ nullptr });
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::AutoMaximizeSuggestionAreaProperty()
    {
        static auto const property = RegisterForwardedProperty<bool>(L"AutoMaximizeSuggestionArea", box_value(true));
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::TextBoxStyleProperty()
    {
        static auto const property = RegisterForwardedProperty<Xaml::Style>(
            L"TextBoxStyle",
            Windows::Foundation::IInspectable{ nullptr });
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::QueryIconProperty()
    {
        static auto const property = RegisterForwardedProperty<Controls::IconElement>(
            L"QueryIcon",
            Windows::Foundation::IInspectable{ nullptr });
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::LightDismissOverlayModeProperty()
    {
        static auto const property = RegisterForwardedProperty<Controls::LightDismissOverlayMode>(
            L"LightDismissOverlayMode",
            box_value(Controls::LightDismissOverlayMode::Auto));
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::DescriptionProperty()
    {
        static auto const property = RegisterForwardedProperty<Windows::Foundation::IInspectable>(
            L"Description",
            Windows::Foundation::IInspectable{ nullptr });
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::HeaderPlacementProperty()
    {
        static auto const property = RegisterForwardedProperty<Controls::ControlHeaderPlacement>(
            L"HeaderPlacement",
            box_value(Controls::ControlHeaderPlacement::Top));
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::ItemsSourceProperty()
    {
        static auto const property = RegisterForwardedProperty<Windows::Foundation::IInspectable>(
            L"ItemsSource",
            Windows::Foundation::IInspectable{ nullptr });
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::ItemTemplateProperty()
    {
        static auto const property = RegisterForwardedProperty<Xaml::DataTemplate>(
            L"ItemTemplate",
            Windows::Foundation::IInspectable{ nullptr });
        return property;
    }

    Xaml::DependencyProperty LiquidGlassSearchBox::InnerPropertyFor(Xaml::DependencyProperty const& outerProperty)
    {
        if (outerProperty == MaxSuggestionListHeightProperty()) return Controls::AutoSuggestBox::MaxSuggestionListHeightProperty();
        if (outerProperty == IsSuggestionListOpenProperty()) return Controls::AutoSuggestBox::IsSuggestionListOpenProperty();
        if (outerProperty == TextMemberPathProperty()) return Controls::AutoSuggestBox::TextMemberPathProperty();
        if (outerProperty == TextProperty()) return Controls::AutoSuggestBox::TextProperty();
        if (outerProperty == UpdateTextOnSelectProperty()) return Controls::AutoSuggestBox::UpdateTextOnSelectProperty();
        if (outerProperty == PlaceholderTextProperty()) return Controls::AutoSuggestBox::PlaceholderTextProperty();
        if (outerProperty == HeaderProperty()) return Controls::AutoSuggestBox::HeaderProperty();
        if (outerProperty == AutoMaximizeSuggestionAreaProperty()) return Controls::AutoSuggestBox::AutoMaximizeSuggestionAreaProperty();
        if (outerProperty == TextBoxStyleProperty()) return Controls::AutoSuggestBox::TextBoxStyleProperty();
        if (outerProperty == QueryIconProperty()) return Controls::AutoSuggestBox::QueryIconProperty();
        if (outerProperty == LightDismissOverlayModeProperty()) return Controls::AutoSuggestBox::LightDismissOverlayModeProperty();
        if (outerProperty == DescriptionProperty()) return Controls::AutoSuggestBox::DescriptionProperty();
        if (outerProperty == HeaderPlacementProperty()) return Controls::AutoSuggestBox::HeaderPlacementProperty();
        if (outerProperty == ItemsSourceProperty()) return Controls::ItemsControl::ItemsSourceProperty();
        if (outerProperty == ItemTemplateProperty()) return Controls::ItemsControl::ItemTemplateProperty();
        return nullptr;
    }

    void LiquidGlassSearchBox::OnForwardedPropertyChanged(Xaml::DependencyPropertyChangedEventArgs const& args)
    {
        ForwardPropertyToInner(args.Property(), args.NewValue());
    }

    void LiquidGlassSearchBox::ForwardPropertyToInner(
        Xaml::DependencyProperty const& outerProperty,
        Windows::Foundation::IInspectable const& value)
    {
        if (!m_autoSuggestBox || m_syncingFromInner) return;

        auto const innerProperty = InnerPropertyFor(outerProperty);
        if (!innerProperty) return;

        SyncFlagGuard guard{ m_syncingToInner };

        // Null is the public default for TextBoxStyle. Internally, null means "use the
        // LiquidGlass transparent editor style" so the stock TextBox background never
        // obscures the glass surface.
        if (outerProperty == TextBoxStyleProperty() && !value)
        {
            if (auto textBoxStyle = SpecializedStyle(L"LiquidGlassAutoSuggestBoxTextBoxStyle"))
            {
                m_autoSuggestBox.SetValue(innerProperty, textBoxStyle);
                return;
            }
        }

        m_autoSuggestBox.SetValue(innerProperty, value);
    }

    void LiquidGlassSearchBox::MirrorPropertyFromInner(
        Xaml::DependencyProperty const& outerProperty,
        Windows::Foundation::IInspectable const& value)
    {
        if (m_syncingToInner) return;

        SyncFlagGuard guard{ m_syncingFromInner };
        SetValue(outerProperty, value);
    }

    void LiquidGlassSearchBox::AttachInnerPropertyMirrors()
    {
        auto weak = get_weak();
        auto mirror = [this, weak](Xaml::DependencyProperty const& outerProperty)
        {
            auto const innerProperty = InnerPropertyFor(outerProperty);
            if (!innerProperty) return;

            (void)m_autoSuggestBox.RegisterPropertyChangedCallback(
                innerProperty,
                [weak, outerProperty](
                    Xaml::DependencyObject const& sender,
                    Xaml::DependencyProperty const& changedProperty)
                {
                    if (auto self = weak.get())
                    {
                        self->MirrorPropertyFromInner(outerProperty, sender.GetValue(changedProperty));
                    }
                });
        };

        mirror(MaxSuggestionListHeightProperty());
        mirror(IsSuggestionListOpenProperty());
        mirror(TextMemberPathProperty());
        mirror(TextProperty());
        mirror(UpdateTextOnSelectProperty());
        mirror(PlaceholderTextProperty());
        mirror(HeaderProperty());
        mirror(AutoMaximizeSuggestionAreaProperty());
        mirror(TextBoxStyleProperty());
        mirror(QueryIconProperty());
        mirror(LightDismissOverlayModeProperty());
        mirror(DescriptionProperty());
        mirror(HeaderPlacementProperty());
        mirror(ItemsSourceProperty());
        mirror(ItemTemplateProperty());
    }

    void LiquidGlassSearchBox::ApplyGlassBrush(Brush const& value)
    {
        m_glassBrush = value;
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
        if (auto textBoxStyle = SpecializedStyle(L"LiquidGlassAutoSuggestBoxTextBoxStyle"))
        {
            m_autoSuggestBox.TextBoxStyle(textBoxStyle);
        }

        AttachInnerPropertyMirrors();

        HorizontalContentAlignment(Xaml::HorizontalAlignment::Stretch);
        VerticalContentAlignment(Xaml::VerticalAlignment::Center);
        IsTabStop(false);
        Content(m_autoSuggestBox);

        GlassBrush(WinUI::LiquidGlass::LiquidGlassPresets::CreateBrush(
            WinUI::LiquidGlass::LiquidGlassPreset::SearchBox));

        // 0.8 idle -> 1.0 focused; pointer-down multiplies the current scale by 0.99.
        // Native focus feedback is intentionally faster than the browser demo so text input
        // never feels delayed behind keyboard focus acquisition.
        SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.8));
        SetValue(LiquidGlassInteraction::FocusedScaleProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::FocusedTintBoostProperty(), box_value(.15));
        SetValue(LiquidGlassInteraction::PressedTintBoostProperty(), box_value(.25));
        SetValue(LiquidGlassInteraction::FocusedContrastMultiplierProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::FocusedRefractionMultiplierProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(100.0));
        SetValue(LiquidGlassInteraction::OpticsTransitionDurationProperty(), box_value(90.0));
    }

    double LiquidGlassSearchBox::MaxSuggestionListHeight() const
    {
        return unbox_value_or<double>(GetValue(MaxSuggestionListHeightProperty()), std::numeric_limits<double>::infinity());
    }

    void LiquidGlassSearchBox::MaxSuggestionListHeight(double value)
    {
        SetValue(MaxSuggestionListHeightProperty(), box_value(value));
    }

    bool LiquidGlassSearchBox::IsSuggestionListOpen() const
    {
        return unbox_value_or<bool>(GetValue(IsSuggestionListOpenProperty()), false);
    }

    void LiquidGlassSearchBox::IsSuggestionListOpen(bool value)
    {
        SetValue(IsSuggestionListOpenProperty(), box_value(value));
    }

    hstring LiquidGlassSearchBox::TextMemberPath() const
    {
        return unbox_value_or<hstring>(GetValue(TextMemberPathProperty()), hstring{});
    }

    void LiquidGlassSearchBox::TextMemberPath(hstring const& value)
    {
        SetValue(TextMemberPathProperty(), box_value(value));
    }

    hstring LiquidGlassSearchBox::Text() const
    {
        return unbox_value_or<hstring>(GetValue(TextProperty()), hstring{});
    }

    void LiquidGlassSearchBox::Text(hstring const& value)
    {
        SetValue(TextProperty(), box_value(value));
    }

    bool LiquidGlassSearchBox::UpdateTextOnSelect() const
    {
        return unbox_value_or<bool>(GetValue(UpdateTextOnSelectProperty()), true);
    }

    void LiquidGlassSearchBox::UpdateTextOnSelect(bool value)
    {
        SetValue(UpdateTextOnSelectProperty(), box_value(value));
    }

    hstring LiquidGlassSearchBox::PlaceholderText() const
    {
        return unbox_value_or<hstring>(GetValue(PlaceholderTextProperty()), hstring{});
    }

    void LiquidGlassSearchBox::PlaceholderText(hstring const& value)
    {
        SetValue(PlaceholderTextProperty(), box_value(value));
    }

    Windows::Foundation::IInspectable LiquidGlassSearchBox::Header() const
    {
        return GetValue(HeaderProperty());
    }

    void LiquidGlassSearchBox::Header(Windows::Foundation::IInspectable const& value)
    {
        SetValue(HeaderProperty(), value);
    }

    bool LiquidGlassSearchBox::AutoMaximizeSuggestionArea() const
    {
        return unbox_value_or<bool>(GetValue(AutoMaximizeSuggestionAreaProperty()), true);
    }

    void LiquidGlassSearchBox::AutoMaximizeSuggestionArea(bool value)
    {
        SetValue(AutoMaximizeSuggestionAreaProperty(), box_value(value));
    }

    Xaml::Style LiquidGlassSearchBox::TextBoxStyle() const
    {
        return GetValue(TextBoxStyleProperty()).try_as<Xaml::Style>();
    }

    void LiquidGlassSearchBox::TextBoxStyle(Xaml::Style const& value)
    {
        SetValue(TextBoxStyleProperty(), value);
    }

    Controls::IconElement LiquidGlassSearchBox::QueryIcon() const
    {
        return GetValue(QueryIconProperty()).try_as<Controls::IconElement>();
    }

    void LiquidGlassSearchBox::QueryIcon(Controls::IconElement const& value)
    {
        SetValue(QueryIconProperty(), value);
    }

    Controls::LightDismissOverlayMode LiquidGlassSearchBox::LightDismissOverlayMode() const
    {
        return unbox_value_or<Controls::LightDismissOverlayMode>(
            GetValue(LightDismissOverlayModeProperty()),
            Controls::LightDismissOverlayMode::Auto);
    }

    void LiquidGlassSearchBox::LightDismissOverlayMode(Controls::LightDismissOverlayMode value)
    {
        SetValue(LightDismissOverlayModeProperty(), box_value(value));
    }

    Windows::Foundation::IInspectable LiquidGlassSearchBox::Description() const
    {
        return GetValue(DescriptionProperty());
    }

    void LiquidGlassSearchBox::Description(Windows::Foundation::IInspectable const& value)
    {
        SetValue(DescriptionProperty(), value);
    }

    Controls::ControlHeaderPlacement LiquidGlassSearchBox::HeaderPlacement() const
    {
        return unbox_value_or<Controls::ControlHeaderPlacement>(
            GetValue(HeaderPlacementProperty()),
            Controls::ControlHeaderPlacement::Top);
    }

    void LiquidGlassSearchBox::HeaderPlacement(Controls::ControlHeaderPlacement value)
    {
        SetValue(HeaderPlacementProperty(), box_value(value));
    }

    Windows::Foundation::IInspectable LiquidGlassSearchBox::ItemsSource() const
    {
        return GetValue(ItemsSourceProperty());
    }

    void LiquidGlassSearchBox::ItemsSource(Windows::Foundation::IInspectable const& value)
    {
        SetValue(ItemsSourceProperty(), value);
    }

    Xaml::DataTemplate LiquidGlassSearchBox::ItemTemplate() const
    {
        return GetValue(ItemTemplateProperty()).try_as<Xaml::DataTemplate>();
    }

    void LiquidGlassSearchBox::ItemTemplate(Xaml::DataTemplate const& value)
    {
        SetValue(ItemTemplateProperty(), value);
    }

    Controls::AutoSuggestBox LiquidGlassSearchBox::InnerAutoSuggestBox() const
    {
        return m_autoSuggestBox;
    }

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

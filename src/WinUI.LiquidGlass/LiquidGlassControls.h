#pragma once

#include "winrt_module_imports.h"
#include "include/EnsureDependencyProperty.hpp"
#include "include/TemplateControlHelper.hpp"
#include "include/PointerLightHelper.hpp"
#include "include/PressOpticsHelper.hpp"

#include "LiquidGlassCard.g.h"
#include "LiquidGlassMagnifier.g.h"
#include "LiquidGlassButton.g.h"
#include "LiquidGlassToggleButton.g.h"
#include "LiquidGlassHyperlinkButton.g.h"
#include "LiquidGlassCheckBox.g.h"
#include "LiquidGlassRadioButton.g.h"
#include "LiquidGlassSlider.g.h"
#include "LiquidGlassTextBox.g.h"
#include "LiquidGlassPasswordBox.g.h"
#include "LiquidGlassComboBox.g.h"
#include "LiquidGlassToggleSwitch.g.h"

namespace winrt::WinUI::LiquidGlass::detail
{
    inline constexpr wchar_t ThemeResourceUri[] =
        L"ms-appx:///WinUI.LiquidGlass/Themes/Generic.xaml";
}

namespace winrt::WinUI::LiquidGlass::implementation
{
#define WINUI_LIQUID_GLASS_COMMON_MEMBERS(Type) \
        static void EnsureDependencyProperties(); \
        static Microsoft::UI::Xaml::DependencyProperty GlassBrushProperty(); \
        WinUI::Composition::Hlsl::LiquidGlassBrush GlassBrush() const; \
        void GlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value); \
    private: \
        void ApplyGlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value); \
        static void OnGlassBrushChanged( \
            Microsoft::UI::Xaml::DependencyObject const& object, \
            Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const& args); \
        WinUI::Composition::Hlsl::LiquidGlassBrush m_glassBrush{ nullptr };

#define WINUI_LIQUID_GLASS_STYLED_DECLARATION(Type) \
    struct Type : Type##T<Type>, \
        detail::EnsureDependencyProperty<Type>, \
        detail::TemplateControlHelper<Type>, \
        detail::PointerLightHelper<Type> \
    { \
        constexpr static auto ResourceUri = detail::ThemeResourceUri; \
        Type(); \
        WINUI_LIQUID_GLASS_COMMON_MEMBERS(Type) \
    };

#define WINUI_LIQUID_GLASS_STYLED_INTERACTIVE_DECLARATION(Type) \
    struct Type : Type##T<Type>, \
        detail::EnsureDependencyProperty<Type>, \
        detail::TemplateControlHelper<Type>, \
        detail::PointerLightHelper<Type>, \
        detail::PressOpticsHelper<Type> \
    { \
        constexpr static auto ResourceUri = detail::ThemeResourceUri; \
        Type(); \
        WINUI_LIQUID_GLASS_COMMON_MEMBERS(Type) \
    };

#define WINUI_LIQUID_GLASS_PLAIN_DECLARATION(Type) \
    struct Type : Type##T<Type>, \
        detail::EnsureDependencyProperty<Type>, \
        detail::PointerLightHelper<Type> \
    { \
        Type(); \
        WINUI_LIQUID_GLASS_COMMON_MEMBERS(Type) \
    };

#define WINUI_LIQUID_GLASS_PLAIN_INTERACTIVE_DECLARATION(Type) \
    struct Type : Type##T<Type>, \
        detail::EnsureDependencyProperty<Type>, \
        detail::PointerLightHelper<Type>, \
        detail::PressOpticsHelper<Type> \
    { \
        Type(); \
        WINUI_LIQUID_GLASS_COMMON_MEMBERS(Type) \
    };

    WINUI_LIQUID_GLASS_STYLED_DECLARATION(LiquidGlassCard)
    WINUI_LIQUID_GLASS_STYLED_INTERACTIVE_DECLARATION(LiquidGlassButton)
    WINUI_LIQUID_GLASS_STYLED_INTERACTIVE_DECLARATION(LiquidGlassToggleButton)
    WINUI_LIQUID_GLASS_STYLED_INTERACTIVE_DECLARATION(LiquidGlassHyperlinkButton)

    struct LiquidGlassMagnifier :
        LiquidGlassMagnifierT<LiquidGlassMagnifier>,
        detail::EnsureDependencyProperty<LiquidGlassMagnifier>,
        detail::TemplateControlHelper<LiquidGlassMagnifier>,
        detail::PointerLightHelper<LiquidGlassMagnifier>
    {
        constexpr static auto ResourceUri = detail::ThemeResourceUri;
        LiquidGlassMagnifier();

        static void EnsureDependencyProperties();
        static Microsoft::UI::Xaml::DependencyProperty GlassBrushProperty();
        WinUI::Composition::Hlsl::LiquidGlassBrush GlassBrush() const;
        void GlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value);

    private:
        void ApplyGlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value);
        static void OnGlassBrushChanged(
            Microsoft::UI::Xaml::DependencyObject const& object,
            Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const& args);

        WinUI::Composition::Hlsl::LiquidGlassBrush m_glassBrush{ nullptr };
        Microsoft::UI::Xaml::Media::CompositeTransform m_dragTransform{ nullptr };
        Windows::Foundation::Point m_lastPointer{};
        uint32_t m_activePointerId{};
        bool m_dragging{};
    };

    WINUI_LIQUID_GLASS_PLAIN_INTERACTIVE_DECLARATION(LiquidGlassCheckBox)
    WINUI_LIQUID_GLASS_PLAIN_INTERACTIVE_DECLARATION(LiquidGlassRadioButton)

    struct LiquidGlassSlider :
        LiquidGlassSliderT<LiquidGlassSlider>,
        detail::EnsureDependencyProperty<LiquidGlassSlider>,
        detail::PointerLightHelper<LiquidGlassSlider>
    {
        LiquidGlassSlider();

        static void EnsureDependencyProperties();
        static Microsoft::UI::Xaml::DependencyProperty GlassBrushProperty();
        WinUI::Composition::Hlsl::LiquidGlassBrush GlassBrush() const;
        void GlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value);

    private:
        void ApplyGlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value);
        static void OnGlassBrushChanged(
            Microsoft::UI::Xaml::DependencyObject const& object,
            Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const& args);

        WinUI::Composition::Hlsl::LiquidGlassBrush m_glassBrush{ nullptr };
        bool m_interactionsWired{};
    };

    WINUI_LIQUID_GLASS_PLAIN_DECLARATION(LiquidGlassTextBox)

    struct LiquidGlassPasswordBox :
        LiquidGlassPasswordBoxT<LiquidGlassPasswordBox>,
        detail::EnsureDependencyProperty<LiquidGlassPasswordBox>,
        detail::PointerLightHelper<LiquidGlassPasswordBox>
    {
        LiquidGlassPasswordBox();

        hstring PlaceholderText() const;
        void PlaceholderText(hstring const& value);
        hstring Password() const;
        void Password(hstring const& value);

        static void EnsureDependencyProperties();
        static Microsoft::UI::Xaml::DependencyProperty GlassBrushProperty();
        WinUI::Composition::Hlsl::LiquidGlassBrush GlassBrush() const;
        void GlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value);

    private:
        void ApplyGlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value);
        static void OnGlassBrushChanged(
            Microsoft::UI::Xaml::DependencyObject const& object,
            Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const& args);

        WinUI::Composition::Hlsl::LiquidGlassBrush m_glassBrush{ nullptr };
        Microsoft::UI::Xaml::Controls::PasswordBox m_passwordBox{ nullptr };
    };

    WINUI_LIQUID_GLASS_PLAIN_DECLARATION(LiquidGlassComboBox)

    struct LiquidGlassToggleSwitch :
        LiquidGlassToggleSwitchT<LiquidGlassToggleSwitch>,
        detail::EnsureDependencyProperty<LiquidGlassToggleSwitch>,
        detail::TemplateControlHelper<LiquidGlassToggleSwitch>,
        detail::PointerLightHelper<LiquidGlassToggleSwitch>
    {
        constexpr static auto ResourceUri = detail::ThemeResourceUri;
        LiquidGlassToggleSwitch();

        Windows::Foundation::IInspectable Header() const;
        void Header(Windows::Foundation::IInspectable const& value);
        bool IsOn() const;
        void IsOn(bool value);

        static void EnsureDependencyProperties();
        static Microsoft::UI::Xaml::DependencyProperty GlassBrushProperty();
        WinUI::Composition::Hlsl::LiquidGlassBrush GlassBrush() const;
        void GlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value);

    private:
        void ApplyGlassBrush(WinUI::Composition::Hlsl::LiquidGlassBrush const& value);
        static void OnGlassBrushChanged(
            Microsoft::UI::Xaml::DependencyObject const& object,
            Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const& args);

        WinUI::Composition::Hlsl::LiquidGlassBrush m_glassBrush{ nullptr };
        Windows::Foundation::IInspectable m_header{ nullptr };
        bool m_interactionsWired{};
    };

#undef WINUI_LIQUID_GLASS_PLAIN_INTERACTIVE_DECLARATION
#undef WINUI_LIQUID_GLASS_PLAIN_DECLARATION
#undef WINUI_LIQUID_GLASS_STYLED_INTERACTIVE_DECLARATION
#undef WINUI_LIQUID_GLASS_STYLED_DECLARATION
#undef WINUI_LIQUID_GLASS_COMMON_MEMBERS
}

namespace winrt::WinUI::LiquidGlass::factory_implementation
{
#define WINUI_LIQUID_GLASS_FACTORY(Type) \
    struct Type : Type##T<Type, implementation::Type> {};

    WINUI_LIQUID_GLASS_FACTORY(LiquidGlassCard)
    WINUI_LIQUID_GLASS_FACTORY(LiquidGlassMagnifier)
    WINUI_LIQUID_GLASS_FACTORY(LiquidGlassButton)
    WINUI_LIQUID_GLASS_FACTORY(LiquidGlassToggleButton)
    WINUI_LIQUID_GLASS_FACTORY(LiquidGlassHyperlinkButton)
    WINUI_LIQUID_GLASS_FACTORY(LiquidGlassCheckBox)
    WINUI_LIQUID_GLASS_FACTORY(LiquidGlassRadioButton)
    WINUI_LIQUID_GLASS_FACTORY(LiquidGlassSlider)
    WINUI_LIQUID_GLASS_FACTORY(LiquidGlassTextBox)
    WINUI_LIQUID_GLASS_FACTORY(LiquidGlassPasswordBox)
    WINUI_LIQUID_GLASS_FACTORY(LiquidGlassComboBox)
    WINUI_LIQUID_GLASS_FACTORY(LiquidGlassToggleSwitch)

#undef WINUI_LIQUID_GLASS_FACTORY
}

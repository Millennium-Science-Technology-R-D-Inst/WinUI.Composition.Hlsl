#pragma once
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/WinUI.Composition.Hlsl.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.h>

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

namespace winrt::WinUI::LiquidGlass::implementation
{
#define WINUI_LIQUID_GLASS_CONTROL_DECLARATION(Type) \
    struct Type : Type##T<Type> \
    { \
        Type(); \
        WinUI::Composition::Hlsl::LiquidGlassBrush GlassBrush() const; \
    private: \
        WinUI::Composition::Hlsl::LiquidGlassBrush m_glassBrush{ nullptr }; \
        bool m_interactionsWired{}; \
    };

    WINUI_LIQUID_GLASS_CONTROL_DECLARATION(LiquidGlassCard)
    WINUI_LIQUID_GLASS_CONTROL_DECLARATION(LiquidGlassButton)
    WINUI_LIQUID_GLASS_CONTROL_DECLARATION(LiquidGlassToggleButton)
    WINUI_LIQUID_GLASS_CONTROL_DECLARATION(LiquidGlassHyperlinkButton)
    WINUI_LIQUID_GLASS_CONTROL_DECLARATION(LiquidGlassCheckBox)
    WINUI_LIQUID_GLASS_CONTROL_DECLARATION(LiquidGlassRadioButton)
    WINUI_LIQUID_GLASS_CONTROL_DECLARATION(LiquidGlassSlider)
    WINUI_LIQUID_GLASS_CONTROL_DECLARATION(LiquidGlassTextBox)
    WINUI_LIQUID_GLASS_CONTROL_DECLARATION(LiquidGlassPasswordBox)
    WINUI_LIQUID_GLASS_CONTROL_DECLARATION(LiquidGlassComboBox)
    WINUI_LIQUID_GLASS_CONTROL_DECLARATION(LiquidGlassToggleSwitch)

#undef WINUI_LIQUID_GLASS_CONTROL_DECLARATION

    struct LiquidGlassMagnifier : LiquidGlassMagnifierT<LiquidGlassMagnifier>
    {
        LiquidGlassMagnifier();
        WinUI::Composition::Hlsl::LiquidGlassBrush GlassBrush() const;

    private:
        WinUI::Composition::Hlsl::LiquidGlassBrush m_glassBrush{ nullptr };
        Microsoft::UI::Xaml::Media::CompositeTransform m_dragTransform{ nullptr };
        Windows::Foundation::Point m_lastPointer{};
        uint32_t m_activePointerId{};
        bool m_dragging{};
    };
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

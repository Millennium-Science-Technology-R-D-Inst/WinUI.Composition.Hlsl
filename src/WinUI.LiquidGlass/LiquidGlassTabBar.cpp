#include "pch.h"
#include "winrt_module_imports.h"
#include "LiquidGlassControls.h"

#if __has_include("LiquidGlassTabBarItem.g.cpp")
#include "LiquidGlassTabBarItem.g.cpp"
#include "LiquidGlassTabBar.g.cpp"
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

        void EnsureTabBarResources()
        {
            [[maybe_unused]] static bool loaded = []
            {
                Xaml::ResourceDictionary dictionary;
                dictionary.Source(Windows::Foundation::Uri{
                    L"ms-appx:///WinUI.LiquidGlass/Themes/TabBar.xaml" });
                Xaml::Application::Current().Resources().MergedDictionaries().Append(dictionary);
                return true;
            }();
        }
    }

#define TAB_GLASS_DP(Type) \
    void Type::EnsureDependencyProperties() { (void)GlassBrushProperty(); } \
    Xaml::DependencyProperty Type::GlassBrushProperty() \
    { \
        static auto property = Xaml::DependencyProperty::Register( \
            L"GlassBrush", xaml_typename<Brush>(), xaml_typename<class_type>(), \
            Xaml::PropertyMetadata{ Windows::Foundation::IInspectable{ nullptr }, \
                Xaml::PropertyChangedCallback{ OnGlassBrushChanged } }); \
        return property; \
    } \
    Brush Type::GlassBrush() const { return GetValue(GlassBrushProperty()).try_as<Brush>(); } \
    void Type::GlassBrush(Brush const& value) { SetValue(GlassBrushProperty(), value); } \
    void Type::ApplyGlassBrush(Brush const& value) \
    { \
        m_glassBrush = value; \
        Background(AsMediaBrush(value)); \
    } \
    void Type::OnGlassBrushChanged( \
        Xaml::DependencyObject const& object, Xaml::DependencyPropertyChangedEventArgs const& args) \
    { \
        detail::EnsureDependencyProperty<Type>::GetSelf(object)->ApplyGlassBrush(args.NewValue().try_as<Brush>()); \
    }

    TAB_GLASS_DP(LiquidGlassTabBarItem)

    LiquidGlassTabBarItem::LiquidGlassTabBarItem()
    {
        EnsureTabBarResources();
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(WinUI::LiquidGlass::LiquidGlassPresets::CreateBrush(
            WinUI::LiquidGlass::LiquidGlassPreset::TabBarItem));
        UseSystemFocusVisuals(true);
    }

    TAB_GLASS_DP(LiquidGlassTabBar)

    LiquidGlassTabBar::LiquidGlassTabBar()
    {
        EnsureTabBarResources();
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(WinUI::LiquidGlass::LiquidGlassPresets::CreateBrush(
            WinUI::LiquidGlass::LiquidGlassPreset::TabBar));

        SelectionMode(Controls::ListViewSelectionMode::Single);
        IsMultiSelectCheckBoxEnabled(false);
        SingleSelectionFollowsFocus(false);

        // One configuration point controls generated item containers through the
        // attached-property visual-tree fallback in LiquidGlassInteraction.
        SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::PointerOverScaleProperty(), box_value(1.035));
        SetValue(LiquidGlassInteraction::PressedScaleProperty(), box_value(.96));
        SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(135.0));
        SetValue(LiquidGlassInteraction::ElasticityProperty(), box_value(.12));
        SetValue(LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(1.12));
        SetValue(LiquidGlassInteraction::PressedDispersionMultiplierProperty(), box_value(1.08));
        SetValue(LiquidGlassInteraction::PressedSaturationMultiplierProperty(), box_value(1.03));
        SetValue(LiquidGlassInteraction::PressedContrastMultiplierProperty(), box_value(1.03));
        SetValue(LiquidGlassInteraction::PressedTintBoostProperty(), box_value(.06));
        SetValue(LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.12));
    }

    Xaml::DependencyObject LiquidGlassTabBar::GetContainerForItemOverride()
    {
        return winrt::make<LiquidGlassTabBarItem>();
    }

    bool LiquidGlassTabBar::IsItemItsOwnContainerOverride(Windows::Foundation::IInspectable const& item)
    {
        return static_cast<bool>(item.try_as<WinUI::LiquidGlass::LiquidGlassTabBarItem>());
    }

#undef TAB_GLASS_DP
}

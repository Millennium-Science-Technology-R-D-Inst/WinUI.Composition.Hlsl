#include "pch.h"
#include "winrt_module_imports.h"
#include "LiquidGlassControls.h"
#include "include/ResourceDictionaryLoader.hpp"

#if __has_include("LiquidGlassTabBarItem.g.cpp")
#include "LiquidGlassTabBarItem.g.cpp"
#include "LiquidGlassTabBar.g.cpp"
#endif

namespace winrt::WinUI::LiquidGlass::implementation
{
	namespace Xaml = Microsoft::UI::Xaml;
	namespace Controls = Xaml::Controls;

	namespace
	{
		void EnsureTabBarResources()
		{
			// A first construction attempt can happen before Application::Current() is
			// available (for example during metadata/type activation). Cache only success;
			// the shared loader also detects an already merged dictionary before appending.
			static bool loaded{};
			if (!loaded)
			{
				loaded = detail::EnsureMergedResourceDictionary(
					L"ms-appx:///WinUI.LiquidGlass/Themes/TabBar.xaml");
			}
		}
	}

	LiquidGlassTabBarItem::LiquidGlassTabBarItem()
	{
		EnsureTabBarResources();
		DefaultStyleKey(box_value(xaml_typename<class_type>()));
		GlassBrush(WinUI::LiquidGlass::LiquidGlassPresets::CreateBrush(
			WinUI::LiquidGlass::LiquidGlassPreset::TabBarItem));
		UseSystemFocusVisuals(true);
	}

	LiquidGlassTabBar::LiquidGlassTabBar()
	{
		EnsureTabBarResources();
		DefaultStyleKey(box_value(xaml_typename<class_type>()));
		GlassBrush(WinUI::LiquidGlass::LiquidGlassPresets::CreateBrush(
			WinUI::LiquidGlass::LiquidGlassPreset::TabBar));

		SelectionMode(Controls::ListViewSelectionMode::Single);
		IsMultiSelectCheckBoxEnabled(false);
		SingleSelectionFollowsFocus(false);
		// CompactControlMotionDefaults<Selector> owns the interaction profile on this
		// parent. Generated item containers resolve those attached values through the
		// existing visual-tree fallback, preserving one configuration point.
	}

	Xaml::DependencyObject LiquidGlassTabBar::GetContainerForItemOverride()
	{
		return winrt::make<LiquidGlassTabBarItem>();
	}

	bool LiquidGlassTabBar::IsItemItsOwnContainerOverride(Windows::Foundation::IInspectable const& item)
	{
		return static_cast<bool>(item.try_as<WinUI::LiquidGlass::LiquidGlassTabBarItem>());
	}
}

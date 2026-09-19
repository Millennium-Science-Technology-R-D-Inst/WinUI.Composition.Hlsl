#pragma once

#include "NavigationWindow.g.h"
#include "OverviewPage.xaml.h"
#include "SharedBrushPage.xaml.h"
#include "IsolatedBrushPage.xaml.h"
#include "DetachPage.xaml.h"
#include <winrt/Microsoft.UI.Dispatching.h>

namespace winrt::WUILiquidGlassDemo_Hlsl::implementation
{
	struct NavigationWindow : NavigationWindowT<NavigationWindow>
	{
		NavigationWindow();
		void InitializeComponent();

		void OnNavigationSelectionChanged(
			winrt::Windows::Foundation::IInspectable const& sender,
			winrt::Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const& args);
		void OnMaterialToggled(
			winrt::Windows::Foundation::IInspectable const& sender,
			winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
		void OnStressClick(
			winrt::Windows::Foundation::IInspectable const& sender,
			winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

	private:
		void NavigateTo(winrt::hstring const& tag);
		void SetMaterialEnabled(bool enabled);
		void StopStress();

		winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer m_stressTimer{ nullptr };
		uint32_t m_stressIndex{};
		uint32_t m_stressRemaining{};
		bool m_materialEnabled{};
	};
}

namespace winrt::WUILiquidGlassDemo_Hlsl::factory_implementation
{
	struct NavigationWindow : NavigationWindowT<NavigationWindow, implementation::NavigationWindow>
	{
	};
}

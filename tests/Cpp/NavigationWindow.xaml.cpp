#include "XamlWorkaround.h"
#include "NavigationWindow.xaml.h"
#if __has_include("NavigationWindow.g.cpp")
#include "NavigationWindow.g.cpp"
#endif

import winrt.WinUI.Composition.Hlsl;

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::WUILiquidGlassDemo_Hlsl::implementation
{
	NavigationWindow::NavigationWindow()
	{
	}

	void NavigationWindow::InitializeComponent()
	{
		NavigationWindowT::InitializeComponent();
		MaterialToggle().IsOn(false);
		RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(0));
	}

	void NavigationWindow::OnNavigationSelectionChanged(IInspectable const&, NavigationViewSelectionChangedEventArgs const& args)
	{
		if (auto item = args.SelectedItem().try_as<NavigationViewItem>())
		{
			NavigateTo(item.Tag().as<hstring>());
		}
	}

	void NavigationWindow::NavigateTo(hstring const& tag)
	{
		if (tag == L"overview") ContentFrame().Navigate(xaml_typename<winrt::WUILiquidGlassDemo_Hlsl::OverviewPage>());
		else if (tag == L"shared") ContentFrame().Navigate(xaml_typename<winrt::WUILiquidGlassDemo_Hlsl::SharedBrushPage>());
		else if (tag == L"isolated") ContentFrame().Navigate(xaml_typename<winrt::WUILiquidGlassDemo_Hlsl::IsolatedBrushPage>());
		else if (tag == L"detach") ContentFrame().Navigate(xaml_typename<winrt::WUILiquidGlassDemo_Hlsl::DetachPage>());
	}

	void NavigationWindow::SetMaterialEnabled(bool enabled)
	{
		m_materialEnabled = enabled;
		if (auto brush = Application::Current().Resources().Lookup(box_value(L"SharedGlassBrush")).try_as<winrt::WinUI::Composition::Hlsl::LiquidGlassBrush>())
		{
			brush.IsEnabled(enabled);
		}
	}

	void NavigationWindow::OnMaterialToggled(IInspectable const&, RoutedEventArgs const&)
	{
		SetMaterialEnabled(MaterialToggle().IsOn());
	}

	void NavigationWindow::OnStressClick(IInspectable const&, RoutedEventArgs const&)
	{
		StopStress();
		m_stressIndex = 0;
		m_stressRemaining = 40;
		m_stressTimer = DispatcherQueue().CreateTimer();
		m_stressTimer.Interval(std::chrono::milliseconds(120));
		m_stressTimer.Tick([weak = get_weak()](auto const&, auto const&)
		{
			if (auto self = weak.get())
			{
				if (!self->m_stressRemaining)
				{
					self->StopStress();
					return;
				}
				self->RootNavigation().SelectedItem(self->RootNavigation().MenuItems().GetAt(self->m_stressIndex++ % 4));
				--self->m_stressRemaining;
			}
		});
		m_stressTimer.Start();
	}

	void NavigationWindow::StopStress()
	{
		if (m_stressTimer)
		{
			m_stressTimer.Stop();
			m_stressTimer = nullptr;
		}
	}
}

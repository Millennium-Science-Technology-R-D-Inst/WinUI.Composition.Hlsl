#pragma once
#include "App.xaml.g.h"
namespace winrt::HlslCppConsumer::implementation
{
	struct App : AppT<App>
	{
		App();
		void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
	private:
		Microsoft::UI::Xaml::Window m_window{ nullptr };
		Microsoft::UI::Dispatching::DispatcherQueueTimer m_timer{ nullptr };
		Microsoft::UI::Xaml::Controls::Border m_border{ nullptr };
	};
}

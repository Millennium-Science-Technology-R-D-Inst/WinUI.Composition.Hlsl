#pragma once

#include "DetachPage.g.h"

namespace winrt::WUILiquidGlassDemo_Hlsl::implementation
{
	struct DetachPage : DetachPageT<DetachPage>
	{
		DetachPage();
		void OnDetachClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
	};
}

namespace winrt::WUILiquidGlassDemo_Hlsl::factory_implementation
{
	struct DetachPage : DetachPageT<DetachPage, implementation::DetachPage>
	{
	};
}

#pragma once
#include "OverviewPage.g.h"

namespace winrt::WUILiquidGlassDemo_Hlsl::implementation
{
	struct OverviewPage : OverviewPageT<OverviewPage>
	{
		OverviewPage() = default;
	};
}

namespace winrt::WUILiquidGlassDemo_Hlsl::factory_implementation
{
	struct OverviewPage : OverviewPageT<OverviewPage, implementation::OverviewPage>
	{
	};
}

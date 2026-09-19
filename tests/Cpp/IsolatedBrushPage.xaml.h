#pragma once
#include "IsolatedBrushPage.g.h"

namespace winrt::WUILiquidGlassDemo_Hlsl::implementation
{
	struct IsolatedBrushPage : IsolatedBrushPageT<IsolatedBrushPage>
	{
		IsolatedBrushPage() = default;
	};
}

namespace winrt::WUILiquidGlassDemo_Hlsl::factory_implementation
{
	struct IsolatedBrushPage : IsolatedBrushPageT<IsolatedBrushPage, implementation::IsolatedBrushPage>
	{
	};
}

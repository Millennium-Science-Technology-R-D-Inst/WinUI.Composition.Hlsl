#pragma once
#include "SharedBrushPage.g.h"

namespace winrt::WUILiquidGlassDemo_Hlsl::implementation
{
	struct SharedBrushPage : SharedBrushPageT<SharedBrushPage>
	{
		SharedBrushPage() = default;
	};
}

namespace winrt::WUILiquidGlassDemo_Hlsl::factory_implementation
{
	struct SharedBrushPage : SharedBrushPageT<SharedBrushPage, implementation::SharedBrushPage>
	{
	};
}

#include "XamlWorkaround.h"
#include "DetachPage.xaml.h"
#if __has_include("DetachPage.g.cpp")
#include "DetachPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::WUILiquidGlassDemo_Hlsl::implementation
{
	DetachPage::DetachPage()
	{
	}

	void DetachPage::OnDetachClick(IInspectable const&, RoutedEventArgs const&)
	{
		auto content = DetachHost().Content();
		if (!content)
		{
			return;
		}

		DetachHost().Content(nullptr);
		StatusText().Text(L"Detached; waiting for dispatcher reattach...");
		auto weak = get_weak();
		if (auto queue = DispatcherQueue(); queue && queue.TryEnqueue([weak, content]
		{
			if (auto self = weak.get())
			{
				self->DetachHost().Content(content);
				self->StatusText().Text(L"Reattached successfully.");
			}
		}))
		{
			return;
		}

		DetachHost().Content(content);
		StatusText().Text(L"Reattached synchronously.");
	}
}

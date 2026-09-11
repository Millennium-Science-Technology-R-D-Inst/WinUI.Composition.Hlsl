#pragma once
import winrt.Microsoft.UI.Composition;
import winrt.Microsoft.UI.Xaml.Media;

namespace hlsl::xaml
{
	// Synchronous UI-thread lifecycle. Materials implement only pipeline construction and release.
	// No UWP view-scoped settings subscriptions or auxiliary native windows are required.
	template<typename D>
	class XamlHlslBrushBase
	{
	public:
		void Connect(D& owner)
		{
			if (m_connected)return;
			m_connected=true;
			Update(owner);
		}

		void Disconnect(D& owner) noexcept
		{
			m_connected=false;
			try
			{
				auto previous=owner.CompositionBrush();
				owner.CompositionBrush(nullptr);
				owner.ReleasePipeline();
				if (previous)previous.Close();
			}
		catch (winrt::hresult_error const&)
		{
		}
		}

		void Update(D& owner)
		{
			if (!m_connected)return;
			if (m_updating)
			{
				m_pending=true; return;
			}
			m_updating=true;
			struct Reset
			{
				bool& flag; ~Reset()
				{
					flag=false;
				}
			} reset{ m_updating };
			do
			{
				m_pending=false;
				auto compositor=winrt::Microsoft::UI::Xaml::Media::CompositionTarget::GetCompositorForCurrentThread();
				winrt::Microsoft::UI::Composition::CompositionBrush next{ nullptr };
				try
				{
					winrt::Microsoft::UI::Composition::CompositionCapabilities capabilities;
					if (owner.IsEnabled() && capabilities.AreEffectsSupported())
						next=owner.BuildPipeline(compositor);
					else
						owner.ReleasePipeline();
				}
			catch (winrt::hresult_error const&)
			{
				owner.ReleasePipeline();
				}
				if (!next)next=compositor.CreateColorBrush(owner.FallbackColor());
				auto previous=owner.CompositionBrush();
				if (previous != next)
				{
					owner.CompositionBrush(next);
					if (previous)previous.Close();
				}
			}
			while (m_pending && m_connected);
		}
	private:
		bool m_connected{};
		bool m_updating{};
		bool m_pending{};
	};
}

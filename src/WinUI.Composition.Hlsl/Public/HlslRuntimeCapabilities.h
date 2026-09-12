#pragma once
#ifndef WINRT_IMPORT_MODULE
#define WINRT_IMPORT_MODULE
#endif
#include "HlslRuntimeCapabilities.g.h"

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct HlslRuntimeCapabilities : HlslRuntimeCapabilitiesT<HlslRuntimeCapabilities>
	{
		HlslRuntimeCapabilities(
			Hlsl::HlslNativeArchitecture architecture,
			Hlsl::HlslRuntimeSupportLevel supportLevel,
			bool nativeAdapterAvailable,
			bool supportsGraphNodes,
			bool supportsMaterializedGraphs) noexcept :
			m_architecture(architecture),
			m_supportLevel(supportLevel),
			m_nativeAdapterAvailable(nativeAdapterAvailable),
			m_supportsGraphNodes(supportsGraphNodes),
			m_supportsMaterializedGraphs(supportsMaterializedGraphs)
		{
		}

		Hlsl::HlslNativeArchitecture Architecture() const noexcept { return m_architecture; }
		Hlsl::HlslRuntimeSupportLevel SupportLevel() const noexcept { return m_supportLevel; }
		bool NativeAdapterAvailable() const noexcept { return m_nativeAdapterAvailable; }
		bool SupportsCompositionGraphNodes() const noexcept { return m_supportsGraphNodes; }
		bool SupportsMaterializedGraphs() const noexcept { return m_supportsMaterializedGraphs; }
		bool SupportsAsyncCompilation() const noexcept { return true; }
		bool UsesPrivateCompositionAbi() const noexcept { return true; }

	private:
		Hlsl::HlslNativeArchitecture m_architecture{ Hlsl::HlslNativeArchitecture::Unknown };
		Hlsl::HlslRuntimeSupportLevel m_supportLevel{ Hlsl::HlslRuntimeSupportLevel::Unsupported };
		bool m_nativeAdapterAvailable{};
		bool m_supportsGraphNodes{};
		bool m_supportsMaterializedGraphs{};
	};
}

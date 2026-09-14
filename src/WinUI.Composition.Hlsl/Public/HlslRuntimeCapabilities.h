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
			bool supportsMaterializedGraphs,
			bool supportsLinkedMultiSource,
			bool supportsMaterializedMultiSource,
			bool supportsMultipleCustomNodes,
			bool supportsNativeNodesAfterCustom,
			bool supportsVectorProperties,
			bool supportsMatrixProperties) noexcept :
			m_architecture(architecture),
			m_supportLevel(supportLevel),
			m_nativeAdapterAvailable(nativeAdapterAvailable),
			m_supportsGraphNodes(supportsGraphNodes),
			m_supportsMaterializedGraphs(supportsMaterializedGraphs),
			m_supportsLinkedMultiSource(supportsLinkedMultiSource),
			m_supportsMaterializedMultiSource(supportsMaterializedMultiSource),
			m_supportsMultipleCustomNodes(supportsMultipleCustomNodes),
			m_supportsNativeNodesAfterCustom(supportsNativeNodesAfterCustom),
			m_supportsVectorProperties(supportsVectorProperties),
			m_supportsMatrixProperties(supportsMatrixProperties)
		{
		}

		Hlsl::HlslNativeArchitecture Architecture() const noexcept { return m_architecture; }
		Hlsl::HlslRuntimeSupportLevel SupportLevel() const noexcept { return m_supportLevel; }
		bool NativeAdapterAvailable() const noexcept { return m_nativeAdapterAvailable; }
		bool SupportsCompositionGraphNodes() const noexcept { return m_supportsGraphNodes; }
		bool SupportsMaterializedGraphs() const noexcept { return m_supportsMaterializedGraphs; }
		bool SupportsLinkedMultiSource() const noexcept { return m_supportsLinkedMultiSource; }
		bool SupportsMaterializedMultiSource() const noexcept { return m_supportsMaterializedMultiSource; }
		bool SupportsMultipleCustomNodes() const noexcept { return m_supportsMultipleCustomNodes; }
		bool SupportsNativeNodesAfterCustom() const noexcept { return m_supportsNativeNodesAfterCustom; }
		bool SupportsVectorProperties() const noexcept { return m_supportsVectorProperties; }
		bool SupportsMatrixProperties() const noexcept { return m_supportsMatrixProperties; }
		bool SupportsAsyncCompilation() const noexcept { return true; }
		bool UsesPrivateCompositionAbi() const noexcept { return true; }

	private:
		Hlsl::HlslNativeArchitecture m_architecture{ Hlsl::HlslNativeArchitecture::Unknown };
		Hlsl::HlslRuntimeSupportLevel m_supportLevel{ Hlsl::HlslRuntimeSupportLevel::Unsupported };
		bool m_nativeAdapterAvailable{};
		bool m_supportsGraphNodes{};
		bool m_supportsMaterializedGraphs{};
		bool m_supportsLinkedMultiSource{};
		bool m_supportsMaterializedMultiSource{};
		bool m_supportsMultipleCustomNodes{};
		bool m_supportsNativeNodesAfterCustom{};
		bool m_supportsVectorProperties{};
		bool m_supportsMatrixProperties{};
	};
}

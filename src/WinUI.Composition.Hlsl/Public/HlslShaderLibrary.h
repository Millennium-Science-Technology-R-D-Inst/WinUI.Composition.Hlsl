#pragma once
#ifndef WINRT_IMPORT_MODULE
#define WINRT_IMPORT_MODULE
#endif
#include "HlslShaderLibrary.g.h"

import std;
import winrt.Windows.Foundation;
import winrt.Windows.Storage;
import winrt.Windows.Storage.Streams;

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct HlslShaderLibrary : HlslShaderLibraryT<HlslShaderLibrary>
	{
		HlslShaderLibrary(
			std::vector<std::uint8_t> bytecode,
			Hlsl::HlslShaderProfile profile,
			Hlsl::HlslEffectKind effectKind) :
			m_bytecode(std::move(bytecode)),
			m_profile(profile),
			m_effectKind(effectKind)
		{
		}

		static Hlsl::HlslShaderLibrary Create(
			Windows::Storage::Streams::IBuffer const& bytecode,
			Hlsl::HlslShaderProfile profile);

		static Hlsl::HlslShaderLibrary CreateFromByteArray(
			winrt::array_view<std::uint8_t const> bytecode,
			Hlsl::HlslShaderProfile profile);

		// Build-time <HlslCompositionShader> output and HlslCompiler output embed a
		// reserved metadata export. This overload reconstructs Kind/Profile directly
		// from those bytes, so native .g.h consumers do not repeat project metadata.
		static Hlsl::HlslShaderLibrary CreateFromGeneratedByteArray(
			winrt::array_view<std::uint8_t const> bytecode);

		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> LoadFromFileAsync(
			Windows::Storage::StorageFile const& file,
			Hlsl::HlslShaderProfile profile);

		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> LoadFromApplicationUriAsync(
			Windows::Foundation::Uri const& uri,
			Hlsl::HlslShaderProfile profile);

		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> LoadGeneratedFromFileAsync(
			Windows::Storage::StorageFile const& file);

		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> LoadGeneratedFromApplicationUriAsync(
			Windows::Foundation::Uri const& uri);

		Hlsl::HlslShaderProfile Profile() const noexcept { return m_profile; }
		Hlsl::HlslEffectKind EffectKind() const noexcept { return m_effectKind; }
		Windows::Storage::Streams::IBuffer Bytecode() const;
		std::vector<std::uint8_t> const& BytecodeBytes() const noexcept { return m_bytecode; }
		void ValidateForEffect(Hlsl::HlslEffectKind kind, std::span<std::wstring const> propertyNames) const;

	private:
		std::vector<std::uint8_t> m_bytecode;
		Hlsl::HlslShaderProfile m_profile{};
		Hlsl::HlslEffectKind m_effectKind{};
	};
}

namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct HlslShaderLibrary : HlslShaderLibraryT<HlslShaderLibrary, implementation::HlslShaderLibrary>
	{
	};
}

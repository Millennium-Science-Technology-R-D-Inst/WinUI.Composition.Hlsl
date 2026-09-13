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
		HlslShaderLibrary(std::vector<std::uint8_t> bytecode, Hlsl::HlslShaderProfile profile) :
			m_bytecode(std::move(bytecode)), m_profile(profile)
		{
		}

		static Hlsl::HlslShaderLibrary Create(
			Windows::Storage::Streams::IBuffer const& bytecode,
			Hlsl::HlslShaderProfile profile);

		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> LoadFromFileAsync(
			Windows::Storage::StorageFile const& file,
			Hlsl::HlslShaderProfile profile);

		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> LoadFromApplicationUriAsync(
			Windows::Foundation::Uri const& uri,
			Hlsl::HlslShaderProfile profile);

		Hlsl::HlslShaderProfile Profile() const noexcept { return m_profile; }
		Windows::Storage::Streams::IBuffer Bytecode() const;
		std::vector<std::uint8_t> const& BytecodeBytes() const noexcept { return m_bytecode; }
		void ValidateForEffect(Hlsl::HlslEffectKind kind, std::span<std::wstring const> propertyNames) const;

	private:
		std::vector<std::uint8_t> m_bytecode;
		Hlsl::HlslShaderProfile m_profile{};
	};
}

namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct HlslShaderLibrary : HlslShaderLibraryT<HlslShaderLibrary, implementation::HlslShaderLibrary>
	{
	};
}

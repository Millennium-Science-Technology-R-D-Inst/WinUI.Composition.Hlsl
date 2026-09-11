#pragma once
#ifndef WINRT_IMPORT_MODULE
#define WINRT_IMPORT_MODULE
#endif
#include "HlslShaderLibrary.g.h"

import std;

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

		Hlsl::HlslShaderProfile Profile() const noexcept { return m_profile; }
		std::vector<std::uint8_t> const& Bytecode() const noexcept { return m_bytecode; }
		void ValidateForEffect(bool sampler, std::span<std::wstring const> propertyNames) const;

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

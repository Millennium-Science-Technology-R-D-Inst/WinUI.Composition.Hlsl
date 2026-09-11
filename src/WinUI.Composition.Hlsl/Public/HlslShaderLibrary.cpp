#include <robuffer.h>
#include "HlslShaderLibrary.h"
#include "HlslShaderLibrary.g.cpp"

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	Hlsl::HlslShaderLibrary HlslShaderLibrary::Create(
		Windows::Storage::Streams::IBuffer const& bytecode,
		Hlsl::HlslShaderProfile profile)
	{
		if (!bytecode || bytecode.Length() < 4 || bytecode.Length() > 16 * 1024 * 1024)
		{
			throw hresult_invalid_argument(L"Shader bytecode must contain a DXBC library no larger than 16 MiB.");
		}

		auto access = bytecode.as<::Windows::Storage::Streams::IBufferByteAccess>();
		byte* data{};
		check_hresult(access->Buffer(&data));
		if (!data || memcmp(data, "DXBC", 4) != 0)
		{
			throw hresult_invalid_argument(L"Shader bytecode is not a DXBC container.");
		}

		std::vector<std::uint8_t> owned(data, data + bytecode.Length());
		return make<HlslShaderLibrary>(std::move(owned), profile);
	}
}

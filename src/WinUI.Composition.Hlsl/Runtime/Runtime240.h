#pragma once
#include <bcrypt.h>
#include <array>
#include <vector>
#pragma comment(lib, "bcrypt.lib")
namespace HlslComposition::Runtime240
{
	// Audited x64 InteractiveExperiences 2.1.6, supplied by Windows App SDK 2.4.0.

	inline void ValidateModule(HMODULE module, char const* expected)
	{
#if !defined(_M_X64)
		winrt::throw_hresult(HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED));
#endif
		wchar_t path[32768]{};
		auto length = GetModuleFileNameW(module, path, ARRAYSIZE(path));
		if (!length || length == ARRAYSIZE(path)) winrt::throw_last_error();
		winrt::handle file{ CreateFileW(path,GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr) };
		if (file.get() == INVALID_HANDLE_VALUE) winrt::throw_last_error();
		LARGE_INTEGER size{}; winrt::check_bool(GetFileSizeEx(file.get(), &size));
		if (size.QuadPart <= 0 || size.QuadPart > 64 * 1024 * 1024) winrt::throw_hresult(E_FAIL);
		std::vector<unsigned char> data(static_cast<size_t>(size.QuadPart));
		DWORD read{}; winrt::check_bool(ReadFile(file.get(), data.data(), static_cast<DWORD>(data.size()), &read, nullptr));
		if (read != data.size()) winrt::throw_hresult(E_FAIL);
		std::array<unsigned char, 32> digest{};
		auto status = BCryptHash(BCRYPT_SHA256_ALG_HANDLE, nullptr, 0, data.data(), read, digest.data(), static_cast<ULONG>(digest.size()));
		if (status < 0) winrt::throw_hresult(E_FAIL);
		constexpr char digits[]="0123456789abcdef";
		char actual[65]{};
		for (size_t i=0; i < digest.size(); ++i)
		{
			actual[i * 2]=digits[digest[i] >> 4]; actual[i * 2 + 1]=digits[digest[i] & 15];
		}
		if (strcmp(actual, expected) != 0)
			throw winrt::hresult_error(HRESULT_FROM_WIN32(ERROR_REVISION_MISMATCH),
									   L"HLSL Composition: this runtime binary has not been validated for the 2.4.0 adapter.");
	}
	inline void Validate(HMODULE effects)
	{
		ValidateModule(effects, "a9a53e453e432fa4a2be4d9edaf8054d56e7ef2b7ea07ad1561905854a843050");
		ValidateModule(GetModuleHandleW(L"dwmcorei.dll"), "c8fee14add00b80759375b642db0f9935ffb422d5bdee694e512ccd02a79090d");
		ValidateModule(GetModuleHandleW(L"dcompi.dll"), "e8a7f36de274c83309b9142ad78ecb91ca1feb9465d5a0feedbecdf560c02c78");
	}
}


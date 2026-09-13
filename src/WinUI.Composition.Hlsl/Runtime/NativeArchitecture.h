#pragma once

// Instruction encoding used only after RuntimeImage has located EffectType::FromGuid.
// No Windows App SDK version, module hash, or fixed RVA is part of this adapter.
namespace HlslNativeAbi
{
#if defined(_M_IX86)
	constexpr WORD Machine = IMAGE_FILE_MACHINE_I386;
	constexpr size_t PatchSize = 5;
#elif defined(_M_ARM64)
	constexpr WORD Machine = IMAGE_FILE_MACHINE_ARM64;
	constexpr size_t PatchSize = 16;
#else
	constexpr WORD Machine = IMAGE_FILE_MACHINE_AMD64;
	constexpr size_t PatchSize = 15;
#endif

	inline std::array<std::uint8_t, PatchSize> MakeEntryPatch(void* target, void* replacement)
	{
		std::array<std::uint8_t, PatchSize> patch{};
#if defined(_M_IX86)
		patch[0] = 0xe9;
		auto displacement = static_cast<std::uint32_t>(
			reinterpret_cast<std::uintptr_t>(replacement) -
			reinterpret_cast<std::uintptr_t>(target) - PatchSize);
		memcpy(patch.data() + 1, &displacement, sizeof(displacement));
#elif defined(_M_ARM64)
		(void)target;
		constexpr std::uint32_t instructions[]{
			0x58000050, // ldr x16, PC+8
			0xd61f0200, // br x16
		};
		memcpy(patch.data(), instructions, sizeof(instructions));
		memcpy(patch.data() + sizeof(instructions), &replacement, sizeof(replacement));
#else
		(void)target;
		patch = { 0x48,0xb8,0,0,0,0,0,0,0,0,0xff,0xe0,0x90,0x90,0x90 };
		memcpy(patch.data() + 2, &replacement, sizeof(replacement));
#endif
		return patch;
	}

	// The runtime patch path must fail closed. In particular, PatchSlot used to
	// silently skip an IAT/delay-IAT entry when VirtualProtect failed, leaving the
	// adapter only partially installed and deferring the error to a Composition
	// worker. These wrappers preserve the Win32 BOOL signature used by the existing
	// patch code but convert failures into immediate HRESULT exceptions. Defining
	// the macros only after the wrappers keeps the ::Win32 calls inside them intact.
	inline BOOL CheckedVirtualProtect(
		LPVOID address,
		SIZE_T size,
		DWORD newProtect,
		PDWORD oldProtect)
	{
		if (::VirtualProtect(address, size, newProtect, oldProtect))
		{
			return TRUE;
		}
		auto const error = ::GetLastError();
		winrt::throw_hresult(HRESULT_FROM_WIN32(error ? error : ERROR_ACCESS_DENIED));
	}

	inline BOOL CheckedFlushInstructionCache(
		HANDLE process,
		LPCVOID address,
		SIZE_T size)
	{
		if (::FlushInstructionCache(process, address, size))
		{
			return TRUE;
		}
		auto const error = ::GetLastError();
		winrt::throw_hresult(HRESULT_FROM_WIN32(error ? error : ERROR_WRITE_FAULT));
	}
}

// CustomEffectRuntime.cpp includes this header after Windows headers and winrt_base.
// All subsequent patch-site calls therefore fail immediately instead of treating a
// protection/cache-flush failure as a successful hook installation.
#define VirtualProtect HlslNativeAbi::CheckedVirtualProtect
#define FlushInstructionCache HlslNativeAbi::CheckedFlushInstructionCache

#pragma once
#include <cstring>
#include <vector>
#include <d2d1effects.h>
namespace HlslComposition
{
	struct NativeEntrypoints
	{
		uintptr_t fromGuid{}, table{}, getBounds{}, calcInputBounds{}, updater{};
		size_t effectCount{};
	};
	class RuntimeImage
	{
		struct Section
		{
			uint8_t* begin; size_t size; DWORD flags;
		};
		uint8_t* m_base;
		std::vector<Section> m_sections;
	public:
		explicit RuntimeImage(HMODULE module) :m_base(reinterpret_cast<uint8_t*>(module))
		{
			if (!module) winrt::throw_hresult(E_INVALIDARG);
			auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(m_base);
			auto nt=reinterpret_cast<IMAGE_NT_HEADERS*>(m_base + dos->e_lfanew);
			if (dos->e_magic != IMAGE_DOS_SIGNATURE || nt->Signature != IMAGE_NT_SIGNATURE ||
				nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) Fail();
			for (auto section=IMAGE_FIRST_SECTION(nt); section < IMAGE_FIRST_SECTION(nt) + nt->FileHeader.NumberOfSections; ++section)
				m_sections.push_back({ m_base + section->VirtualAddress,section->Misc.VirtualSize,section->Characteristics });
		}

		[[noreturn]] static void Fail()
		{
			throw winrt::hresult_error(HRESULT_FROM_WIN32(ERROR_REVISION_MISMATCH),
									   L"HLSL Composition: native ABI fingerprint is missing or ambiguous.");
		}

		bool Contains(void const* pointer, size_t size, DWORD flags) const
		{
			auto address=reinterpret_cast<uintptr_t>(pointer);
			for (auto const& s : m_sections)
			{
				auto start=reinterpret_cast<uintptr_t>(s.begin);
				if ((s.flags & flags) == flags && address >= start && address - start <= s.size && size <= s.size - (address - start)) return true;
			}
			return false;
		}

		uint8_t* Unique(std::initializer_list<int> pattern, DWORD flags) const
		{
			uint8_t* found{};
			for (auto const& s : m_sections)
			{
				if ((s.flags & flags) != flags || s.size < pattern.size()) continue;
				for (size_t i=0; i <= s.size - pattern.size(); ++i)
				{
					size_t j=0;
					for (auto byte : pattern)
					{
						if (byte >= 0 && s.begin[i + j] != byte) break; ++j;
					}
					if (j == pattern.size())
					{
						if (found) Fail(); found=s.begin + i;
					}
				}
			}
			if (!found) Fail();
			return found;
		}

		uintptr_t Rva(void const* pointer) const
		{
			return reinterpret_cast<uintptr_t>(pointer) - reinterpret_cast<uintptr_t>(m_base);
		}

		NativeEntrypoints Resolve() const
		{
			constexpr DWORD code=IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;
			// Match the GUID lookup loop, not merely a common function prologue.
			// -1 represents relocation-dependent bytes. No module-relative address is embedded.
			auto lookup=Unique({
				0x48,0x89,0x5c,0x24,0x08,0x48,0x89,0x74,0x24,0x10,0x48,0x89,0x7c,0x24,0x18,
				0x41,0x56,0x48,0x83,0xec,0x20,0x4c,0x8b,0xf1,
				0x48,0x8d,0x3d,-1,-1,-1,-1,0x33,0xdb,0x48,0x8b,0x37,0x48,0x8b,0xce,
				0x48,0x8b,0x06,0x48,0x8b,0x40,0x08,0xe8,-1,-1,-1,-1,
				0x48,0x8b,0xc8,0x49,0x8b,0x06,0x48,0x3b,0x01,0x75,0x0a,
				0x49,0x8b,0x46,0x08,0x48,0x3b,0x41,0x08,0x74,0x24,0xff,0xc3,
				0x48,0x83,0xc7,0x08,0x83,0xfb,-1,0x72,0xce
							   }, code);
			int32_t displacement{};
			memcpy(&displacement, lookup + 27, 4);
			auto table = reinterpret_cast<void**>(lookup + 31 + displacement);
			auto count = static_cast<size_t>(lookup[80]);
			if (count < 1 || count>128 || !Contains(table, count * sizeof(void*), IMAGE_SCN_MEM_READ))
				Fail();
			void** neutral{};
			for (size_t i=0; i < count; ++i)
			{
				auto object=table[i];
				if (!Contains(object, sizeof(void*), IMAGE_SCN_MEM_READ)) Fail();
				auto vt=*reinterpret_cast<void***>(object);
				if (!Contains(vt, 22 * sizeof(void*), IMAGE_SCN_MEM_READ)) Fail();
				for (size_t slot=0; slot < 22; ++slot) if (!Contains(vt[slot], 1, code)) Fail();
				auto getGuid=reinterpret_cast<GUID const* (*)(void*)>(vt[1]);
				auto guid=getGuid(object);
				if (!Contains(guid, sizeof(GUID), IMAGE_SCN_MEM_READ)) Fail();
				if (IsEqualGUID(*guid, CLSID_D2D1ColorMatrix)) neutral=vt;
			}
			if (!neutral) Fail();
			auto copy=Unique({ 0x4d,0x8b,0xc8,0x48,0x8b,0xc2,0x44,0x8b,0x41,0x1c,0x8b,0x51,0x10,
				0x49,0xc1,0xe0,0x02,0x48,0x03,0x10,0x49,0x8b,0x09,0xe9
							 }, code);
			void** updater{};
			for (auto const& s : m_sections)
			{
				if (!(s.flags & IMAGE_SCN_MEM_READ) || (s.flags & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_WRITE))) continue;
				for (size_t i=0; i + 4 * sizeof(void*) <= s.size; i+=sizeof(void*))
				{
					auto candidate = reinterpret_cast<void**>(s.begin + i);
					if (candidate[2] != copy || candidate[0] != candidate[1] ||
						!Contains(candidate[0], 1, code) || !Contains(candidate[3], 1, code)) continue;
					if (updater) Fail();
					updater=candidate;
				}
			}
			if (!updater) Fail();
			return { Rva(lookup),Rva(table),Rva(neutral[15]),Rva(neutral[16]),Rva(updater),count };
		}
	};
}

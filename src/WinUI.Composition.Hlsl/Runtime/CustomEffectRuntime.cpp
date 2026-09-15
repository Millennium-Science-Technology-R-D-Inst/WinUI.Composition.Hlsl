module;
#include <unknwn.h>
#include <Windows.h>
#include <guiddef.h>
#include <windows.graphics.effects.interop.h>
#include <d2d1effects.h>
#include <d2d1_1.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>

module WinUI.Composition.Hlsl.CustomEffectRuntime;

import std;
import winrt_base;
import winrt.Windows.Foundation;
import winrt.Windows.Graphics.Effects;
import winrt.Microsoft.UI.Composition;

#if defined(_M_IX86)
#define HLSL_CALLBACK __stdcall
#define HLSL_MEMBER __thiscall
#define HLSL_METHOD(name) name##Thiscall
#define HLSL_THUNK(name) __declspec(naked) void name##Thiscall() { __asm pop eax __asm push ecx __asm push eax __asm jmp name }
#else
#define HLSL_CALLBACK __fastcall
#define HLSL_MEMBER __fastcall
#define HLSL_METHOD(name) name
#define HLSL_THUNK(name)
#endif

#include "NativeArchitecture.h"

#pragma once

// The native registry owns every byte to which the synthetic EffectType points.
struct OwnedEffectDefinition
{
	CustomEffectRuntime::CustomEffectDefinition value{};
	std::wstring effectName;
	std::string fragmentName, shaderSource, shaderFunctionName, materializationName, descriptorKey;
	std::vector<unsigned char> shaderBytecode;
	std::vector<CustomEffectRuntime::SourceDescriptor> sources;
	std::vector<std::wstring> sourceNames, propertyNames;
	std::vector<CustomEffectRuntime::PropertyDescriptor> properties;
	std::vector<CustomEffectRuntime::NativePropertyMetadata> metadata;
	std::vector<std::string> shaderNames;
	std::vector<CustomEffectRuntime::ConstantBufferPropertyMapping> mappings;
	std::vector<uint16_t> arguments;
	std::vector<unsigned char> constants;

	explicit OwnedEffectDefinition(CustomEffectRuntime::CustomEffectDefinition const& input) :value(input)
	{
		auto const hasSource = input.shaderSource && input.shaderSourceSize;
		auto const hasBytecode = input.shaderBytecode && input.shaderBytecodeSize;
		if (!input.effectName || !input.fragmentName || hasSource == hasBytecode || !input.shaderFunctionName ||
			input.sourceCount == 0 || input.sourceCount > 16 || !input.sources || input.propertyCount > 64 ||
			(input.propertyCount && !input.properties) ||
			(input.nativePropertyMetadataCount && !input.nativePropertyMetadata) ||
			(input.shaderArgumentCount && !input.shaderArguments) ||
			(input.constantBufferPropertyCount && !input.constantBufferProperties) ||
			(input.constantBufferSize && !input.constantBufferInitialValue) ||
			(input.inputMode == CustomEffectRuntime::CustomEffectInputMode::MaterializedTexture && input.sourceCount != 1))
			throw winrt::hresult_invalid_argument(L"Invalid native effect definition.");
		effectName = input.effectName; fragmentName = input.fragmentName;
		if (hasSource) shaderSource.assign(input.shaderSource, input.shaderSourceSize);
		if (hasBytecode)
		{
			auto begin = static_cast<unsigned char const*>(input.shaderBytecode);
			shaderBytecode.assign(begin, begin + input.shaderBytecodeSize);
		}
		shaderFunctionName = input.shaderFunctionName;
		materializationName = input.materializationShaderFunctionName ? input.materializationShaderFunctionName : "";
		descriptorKey = input.descriptorKey ? input.descriptorKey : ""; value.descriptorKey = descriptorKey.c_str();
		value.effectName = effectName.c_str(); value.fragmentName = fragmentName.c_str();
		value.shaderSource = shaderSource.empty() ? nullptr : shaderSource.data();
		value.shaderSourceSize = shaderSource.size();
		value.shaderBytecode = shaderBytecode.empty() ? nullptr : shaderBytecode.data();
		value.shaderBytecodeSize = shaderBytecode.size();
		value.shaderFunctionName = shaderFunctionName.c_str();
		value.materializationShaderFunctionName = materializationName.empty() ? nullptr : materializationName.c_str();
		sources.assign(input.sources, input.sources + input.sourceCount);
		sourceNames.resize(sources.size());
		for (std::size_t i = 0; i < sources.size(); ++i)
		{
			if (!sources[i].name)
				throw winrt::hresult_invalid_argument(L"Invalid native effect source.");
			sourceNames[i] = sources[i].name; sources[i].name = sourceNames[i].c_str();
		}
		if (input.propertyCount)properties.assign(input.properties, input.properties + input.propertyCount);
		propertyNames.resize(properties.size());
		for (std::size_t i = 0; i < properties.size(); ++i)
		{
			propertyNames[i] = properties[i].publicName; properties[i].publicName = propertyNames[i].c_str();
			if (properties[i].getDefaultValue)
			{
				winrt::Windows::Foundation::IPropertyValue initial{ nullptr };
				winrt::check_hresult(properties[i].getDefaultValue(reinterpret_cast<ABI::Windows::Foundation::IPropertyValue**>(winrt::put_abi(initial))));
				properties[i].initialScalar = initial.GetSingle(); properties[i].getDefaultValue = nullptr;
			}
		}
		if (input.nativePropertyMetadataCount)
		{
			auto begin = static_cast<CustomEffectRuntime::NativePropertyMetadata const*>(input.nativePropertyMetadata);
			metadata.assign(begin, begin + input.nativePropertyMetadataCount);
		}
		shaderNames.resize(metadata.size());
		for (std::size_t i = 0; i < metadata.size(); ++i)
		{
			shaderNames[i] = metadata[i].shaderName; metadata[i].shaderName = shaderNames[i].c_str();
		}
		if (input.constantBufferPropertyCount)mappings.assign(input.constantBufferProperties, input.constantBufferProperties + input.constantBufferPropertyCount);
		if (input.shaderArgumentCount)arguments.assign(input.shaderArguments, input.shaderArguments + input.shaderArgumentCount);
		if (input.constantBufferSize)
		{
			auto begin = static_cast<unsigned char const*>(input.constantBufferInitialValue);
			constants.assign(begin, begin + input.constantBufferSize);
		}
		value.sources = sources.data(); value.properties = properties.data(); value.nativePropertyMetadata = metadata.data();
		value.constantBufferProperties = mappings.data(); value.shaderArguments = arguments.data();
		value.constantBufferInitialValue = constants.empty() ? nullptr : constants.data();
	}

	bool Equivalent(OwnedEffectDefinition const& other) const
	{
		auto const& b = other.value;
		if (descriptorKey != other.descriptorKey || effectName != other.effectName || fragmentName != other.fragmentName ||
			shaderSource != other.shaderSource || shaderBytecode != other.shaderBytecode ||
			shaderFunctionName != other.shaderFunctionName || materializationName != other.materializationName ||
			sourceNames != other.sourceNames || propertyNames != other.propertyNames || shaderNames != other.shaderNames ||
			arguments != other.arguments || constants != other.constants ||
			value.linkingArgType != b.linkingArgType || value.shaderProfileVersion != b.shaderProfileVersion ||
			value.propertiesStructSize != b.propertiesStructSize ||
			value.inputMode != b.inputMode || value.graphPolicy != b.graphPolicy ||
			metadata.size() != other.metadata.size() || mappings.size() != other.mappings.size())return false;
		for (size_t i = 0; i < sources.size(); ++i)
			if (sources[i].kind != other.sources[i].kind ||
				sources[i].requiresSamplerData != other.sources[i].requiresSamplerData ||
				sources[i].requiresSamplerDataExt != other.sources[i].requiresSamplerDataExt)return false;
		for (size_t i = 0; i < properties.size(); ++i)
			if (properties[i].index != other.properties[i].index || properties[i].mapping != other.properties[i].mapping ||
				properties[i].initialScalar != other.properties[i].initialScalar)return false;
		for (size_t i = 0; i < metadata.size(); ++i)
			if (metadata[i].propertyOffset != other.metadata[i].propertyOffset ||
				metadata[i].expressionType != other.metadata[i].expressionType ||
				metadata[i].propertyType != other.metadata[i].propertyType ||
				metadata[i].valueCount != other.metadata[i].valueCount ||
				metadata[i].validator != other.metadata[i].validator)return false;
		for (size_t i = 0; i < mappings.size(); ++i)
			if (mappings[i].propertyIndex != other.mappings[i].propertyIndex ||
				mappings[i].constantBufferOffset != other.mappings[i].constantBufferOffset)return false;
		return true;
	}
};


#pragma once
namespace HlslComposition
{
	struct NativeEntrypoints
	{
		std::uintptr_t fromGuid{}, table{}, getBounds{}, calcInputBounds{}, updater{};
		std::size_t effectCount{};
	};
	class RuntimeImage
	{
		struct Section
		{
			std::uint8_t* begin; std::size_t size; DWORD flags;
		};
		std::uint8_t* m_base;
		std::vector<Section> m_sections;
	public:
		explicit RuntimeImage(HMODULE module) :m_base(reinterpret_cast<std::uint8_t*>(module))
		{
			if (!module) winrt::throw_hresult(E_INVALIDARG);
			auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(m_base);
			auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(m_base + dos->e_lfanew);
			if (dos->e_magic != IMAGE_DOS_SIGNATURE || nt->Signature != IMAGE_NT_SIGNATURE ||
				nt->FileHeader.Machine != HlslNativeAbi::Machine) Fail();
			for (auto section = IMAGE_FIRST_SECTION(nt); section < IMAGE_FIRST_SECTION(nt) + nt->FileHeader.NumberOfSections; ++section)
				m_sections.push_back({ m_base + section->VirtualAddress,section->Misc.VirtualSize,section->Characteristics });
		}

		[[noreturn]] static void Fail()
		{
			throw winrt::hresult_error(E_FAIL,
									   L"HLSL Composition could not resolve the required native effect entrypoints.");
		}

		bool Contains(void const* pointer, std::size_t size, DWORD flags) const
		{
			auto address = reinterpret_cast<std::uintptr_t>(pointer);
			for (auto const& s : m_sections)
			{
				auto start = reinterpret_cast<std::uintptr_t>(s.begin);
				if ((s.flags & flags) == flags && address >= start && address - start <= s.size && size <= s.size - (address - start)) return true;
			}
			return false;
		}

		std::uint8_t* Unique(std::initializer_list<int> pattern, DWORD flags) const
		{
			std::uint8_t* found{};
			for (auto const& s : m_sections)
			{
				if ((s.flags & flags) != flags || s.size < pattern.size()) continue;
				for (std::size_t i = 0; i <= s.size - pattern.size(); ++i)
				{
					std::size_t j = 0;
					for (auto byte : pattern)
					{
						if (byte >= 0 && s.begin[i + j] != byte) break; ++j;
					}
					if (j == pattern.size())
					{
						if (found) Fail(); found = s.begin + i;
					}
				}
			}
			if (!found) Fail();
			return found;
		}

		std::uintptr_t Rva(void const* pointer) const
		{
			return reinterpret_cast<std::uintptr_t>(pointer) - reinterpret_cast<std::uintptr_t>(m_base);
		}

		NativeEntrypoints Resolve() const
		{
			constexpr DWORD code = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;
			std::uint8_t* lookup{};
			void** table{};
#if defined(_M_IX86)
			// PDB: Microsoft::UI::Composition::EffectType::FromGuid. The absolute
			// table operand is relocated by the loader and is therefore decoded here.
			lookup = Unique({
				0x8b,0xff,0x55,0x8b,0xec,0x51,0x53,0x56,0x57,0x89,0x4d,-1,0x33,0xff,
				0x8b,0x9f,-1,-1,-1,-1,0x8b,0x03,0x8b,0x70,0x04
							}, code);
			memcpy(&table, lookup + 16, sizeof(table));
#elif defined(_M_ARM64)
			// Same function on ARM64. Decode its ADRP+ADD pair rather than retaining
			// the RVA produced by one Windows App SDK build.
			lookup = Unique({
				0x7f,0x23,0x03,0xd5,
				-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
				0xfd,0x03,0x00,0x91,
				-1,-1,-1,-1,-1,-1,-1,-1,
				0xf4,0x03,0x00,0xaa,0x15,0x00,0x80,0x52,0xd3,0x5a,0x75,0xf8
							}, code);
			std::uint32_t adrp{}, add{};
			memcpy(&adrp, lookup + 20, sizeof(adrp));
			memcpy(&add, lookup + 24, sizeof(add));
			int64_t pageOffset = static_cast<int64_t>(
				(((adrp >> 5) & 0x7ffff) << 2) | ((adrp >> 29) & 3));
			if (pageOffset & (1 << 20)) pageOffset -= (1 << 21);
			auto page = reinterpret_cast<std::uintptr_t>(lookup) & ~std::uintptr_t{ 0xfff };
			auto immediate = static_cast<std::uintptr_t>((add >> 10) & 0xfff);
			if (add & (1 << 22)) immediate <<= 12;
			auto tableAddress = static_cast<std::intptr_t>(page) + pageOffset * 4096 +
				static_cast<std::intptr_t>(immediate);
			table = reinterpret_cast<void**>(tableAddress);
#else
			// Match the GUID lookup loop, not merely a common function prologue.
			// -1 represents relocation-dependent bytes. No module-relative address is embedded.
			lookup = Unique({
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
			table = reinterpret_cast<void**>(lookup + 31 + displacement);
#endif

			// Discover the table length from valid EffectType objects. FromGuid's
			// loop bound is compiler-specific; the object/vtable/GUID shape is the ABI.
			std::size_t count{};
			void** neutral{};
			for (; count < 128; ++count)
			{
				if (!Contains(table + count, sizeof(void*), IMAGE_SCN_MEM_READ)) break;
				auto object = table[count];
				if (!Contains(object, sizeof(void*), IMAGE_SCN_MEM_READ)) break;
				auto vt = *reinterpret_cast<void***>(object);
				if (!Contains(vt, 22 * sizeof(void*), IMAGE_SCN_MEM_READ)) break;
				bool valid = true;
				for (std::size_t slot = 0; slot < 22; ++slot)
					valid = valid && Contains(vt[slot], 1, code);
				if (!valid) break;
				auto getGuid = reinterpret_cast<GUID const* (HLSL_MEMBER*)(void*)>(vt[1]);
				auto guid = getGuid(object);
				if (!Contains(guid, sizeof(GUID), IMAGE_SCN_MEM_READ)) break;
				if (IsEqualGUID(*guid, CLSID_D2D1ColorMatrix)) neutral = vt;
			}
			if (!count || !neutral) Fail();

			std::uint8_t* copy{};
#if defined(_M_IX86)
			// PDB: std::_Func_impl_no_alloc_<...DirectPropertyUpdater...>::_Do_call.
			copy = Unique({
				0x8b,0xff,0x55,0x8b,0xec,0x8b,0x41,0x14,0x8b,0x49,0x08,0xc1,0xe0,0x02,
				0x50,0x8b,0x45,0x08,0x03,0x08,0x8b,0x45,0x0c,0x51,0xff,0x30,0xe8,
				-1,-1,-1,-1,0x83,0xc4,0x0c,0x5d,0xc2,0x08,0x00
						  }, code);
#elif defined(_M_ARM64)
			copy = Unique({
				0x0a,0x1c,0x40,0xb9,0xeb,0x03,0x02,0xaa,0x09,0x10,0x40,0xb9,
				0x28,0x00,0x40,0xf9,0x60,0x01,0x40,0xf9,0x42,0x7d,0x7e,0xd3,
				0x01,0x41,0x29,0x8b,-1,-1,-1,-1
						  }, code);
#else
			copy = Unique({ 0x4d,0x8b,0xc8,0x48,0x8b,0xc2,0x44,0x8b,0x41,0x1c,0x8b,0x51,0x10,
				0x49,0xc1,0xe0,0x02,0x48,0x03,0x10,0x49,0x8b,0x09,0xe9
						  }, code);
#endif
			void** updater{};
			for (auto const& s : m_sections)
			{
				// x86 wuceffectsi merges RTTI/vtables into its read-only .text section.
				// Executability is therefore not a reason to skip a data candidate;
				// writable sections remain excluded.
				if (!(s.flags & IMAGE_SCN_MEM_READ) || (s.flags & IMAGE_SCN_MEM_WRITE)) continue;
				for (size_t i = 0; i + 4 * sizeof(void*) <= s.size; i += sizeof(void*))
				{
					auto candidate = reinterpret_cast<void**>(s.begin + i);
					if (candidate[2] != copy || candidate[0] != candidate[1] ||
						!Contains(candidate[0], 1, code) || !Contains(candidate[3], 1, code)) continue;
					// MSVC may emit more than one identical std::function vtable for the
					// same inline updater. They share the same _Do_call ABI; retain the
					// first complete table instead of treating COMDAT duplication as an
					// incompatible runtime.
					if (!updater) updater = candidate;
				}
			}
			if (!updater) Fail();
			return { Rva(lookup),Rva(table),Rva(neutral[15]),Rva(neutral[16]),Rva(updater),count };
		}
	};
}



using namespace winrt;
using namespace Microsoft::UI::Composition;
using namespace Windows::Graphics::Effects;

namespace
{
	// EffectNode indices address property blobs; subgraph indices address shader
	// bodies and intermediate outputs. The native traversal determines both maps.

	// This file is a deliberately narrow compatibility layer for one WinAppSDK /
	// Windows composition build. The public WinRT object returned by CreateEffect()
	// looks like a normal IGraphicsEffect, but wuceffectsi/dwmcorei only accept a
	// fixed internal EffectType/ICompiledEffect ABI. The runtime below supplies the
	// missing private EffectType for our GUIDs, intercepts CompileEffectDescription,
	// and returns an ICompiledEffect-shaped object whose vectors and vtables match
	// the native layout observed in reverse engineering.
	//
	// High-level flow:
	//   1. RuntimeGraphicsEffect exposes a private effect GUID to WinUI.
	//   2. EffectType::FromGuid is patched so wuceffectsi accepts that GUID.
	//   3. CompileEffectDescription is patched in dcompi/dwmcorei import tables.
	//   4. When the flattened graph contains our EffectType, the detour supplies its
	//      shader body while retaining native compilation for materialized inputs.
	//   5. DWM consumes CompiledResult through the ICompiledEffect vtable below.
	//
	// The RVAs are not symbolic API contracts. They must be treated as build-specific
	// offsets and guarded by byte-pattern checks where code patching is involved.
	std::uintptr_t kEffectTypeFromGuidRva{};
	std::uintptr_t kEffectTypeTableRva{};
	std::uintptr_t kEffectTypeGetBoundsRva{};
	std::uintptr_t kEffectTypeCalcInputBoundsRva{};
	std::uintptr_t kDirectPropertyUpdaterFunctionVtableRva{};
	std::size_t kEffectTypeCount{};
	// Reverse engineering shows EffectType virtual calls stop at slot 21
	// (+0xa8, GetEffectOpacityRelation) in this WinAppSDK build. Slot 22+
	// is not part of the callable ABI we need to model for private GUIDs.
	constexpr std::size_t kEffectTypeVtableSlotCount = 22;
	constexpr std::size_t kFromGuidPatchSize = HlslNativeAbi::PatchSize;
	constexpr std::uint32_t kCompiledEffectSubgraphOutputFlag = 0x8;

	struct RuntimeEffectEntry;

	// Private EffectType objects are not COM objects. wuceffectsi treats the first
	// pointer as a native C++ vtable and calls fixed slots directly. The entry back
	// pointer gives every slot access to the declarative CustomEffectDefinition.
	struct RuntimeEffectType
	{
		void** vtable;
		RuntimeEffectEntry* entry;
	};

	// One registered effect definition owns one native-shaped EffectType instance.
	// The shader blob is compiled lazily because the effect may be registered before
	// any CompositionEffectFactory asks DWM to materialize the graph.
	struct RuntimeEffectEntry
	{
		explicit RuntimeEffectEntry(CustomEffectRuntime::CustomEffectDefinition const& value) :
			owned(value), definition(&owned.value)
		{
			effectType.vtable = effectTypeVtable;
			effectType.entry = this;
		}

		OwnedEffectDefinition owned;
		CustomEffectRuntime::CustomEffectDefinition const* definition{};
		winrt::com_ptr<ID3DBlob> shaderBlob;
		std::once_flag shaderOnce;
		void* effectTypeVtable[kEffectTypeVtableSlotCount]{};
		RuntimeEffectType effectType{};
		RuntimeEffectEntry* next{};
	};

	// ABI returned by ICompiledEffect::GetSubgraphShaderLinkingBody (native size_t fields). dwmcorei copies
	// this POD by value, then feeds bytecodeData/functionName/argData into its shader
	// linker. The static_assert pins the reverse-engineered struct size so accidental
	// field changes fail at compile time instead of corrupting DWM reads.
	struct ShaderLinkingBody
	{
		std::size_t argCount;
		void const* argData;
		std::size_t bytecodeSize;
		void const* bytecodeData;
		char const* functionName;
		std::uint32_t constantBufferSize;
		std::uint16_t linkingArgType;
		// D3DShaderProfileVersion (1 byte). Misnamed historically as
		// hasCustomSamplers because wuceffectsi almost always wrote 1 here.
		std::uint8_t shaderProfileVersion;
		std::uint8_t padding;
	};

	static_assert(sizeof(ShaderLinkingBody) == (sizeof(void*) == 8 ? 48 : 28));

	// Mirrors CompiledEffectSubgraph::InputBindings: an input either maps to a named
	// brush input (isSubgraphOutput=false) or to a previously emitted subgraph output
	// (isSubgraphOutput=true). DWM uses this to build fragment inputs in
	// CBrushRenderingGraphBuilder::AddEffectBrush.
	struct InputBinding
	{
		std::uint32_t inputIndex;
		bool isSubgraphOutput;
		std::uint8_t padding[3];
	};

	static_assert(sizeof(InputBinding) == 8);

	// Four bytes copied from native EffectGenerator::SurfaceData. DWM currently
	// observes byte 2 for samplerData and byte 3 for samplerDataExt requirements.
	// The other bytes are kept so the vector stride matches native compiled graphs.
	struct SurfaceData
	{
		std::uint8_t data[4];
	};

	static_assert(sizeof(SurfaceData) == 4);
	static_assert(sizeof(CustomEffectRuntime::NativePropertyMetadata) == (sizeof(void*) == 8 ? 32 : 24));
	static_assert(offsetof(CustomEffectRuntime::NativePropertyMetadata, propertyOffset) == (sizeof(void*) == 8 ? 8 : 4));
	static_assert(offsetof(CustomEffectRuntime::NativePropertyMetadata, propertyType) == (sizeof(void*) == 8 ? 16 : 12));
	static_assert(offsetof(CustomEffectRuntime::NativePropertyMetadata, valueCount) == (sizeof(void*) == 8 ? 20 : 16));

	// Native property animation does not call our WinRT IGraphicsEffect again after
	// factory creation. wuceffectsi stores std::function-like updater callables in
	// the compiled subgraph; EffectInstance invokes them when CompositionPropertySet
	// values change. These structures model the inline-storage callable shape used
	// by the built-in DirectPropertyUpdater path.
	struct NativeFunctionStorage
	{
		std::uint8_t inlineStorage[sizeof(void*) == 8 ? 56 : 36];
		void* callable;
	};

	static_assert(sizeof(NativeFunctionStorage) == (sizeof(void*) == 8 ? 64 : 40));

	struct NativePropertyUpdaterCallable
	{
		void** vtable;
		CustomEffectRuntime::NativePropertyMetadata metadata;
	};

	static_assert(sizeof(NativePropertyUpdaterCallable) == (sizeof(void*) == 8 ? 40 : 28));

	struct ConstantBufferUpdater
	{
		uint32_t nodeIndex;
		uint32_t constantBufferOffset;
		NativeFunctionStorage update;
	};

	static_assert(sizeof(ConstantBufferUpdater) == (sizeof(void*) == 8 ? 72 : 48));
	static_assert(offsetof(ConstantBufferUpdater, update) == 8);
	static_assert(offsetof(ConstantBufferUpdater, update.callable) == (sizeof(void*) == 8 ? 64 : 44));

	// Native CompiledEffectSubgraph layout: 136 bytes on x64/ARM64, 72 on x86. DWM indexes this array directly through
	// ICompiledEffect methods, but wuceffectsi later also reads vector ranges for
	// constant buffer creation. The offsets therefore matter as much as the getters.
	struct CompiledSubgraph
	{
		uint32_t flags;
		uint16_t linkingArgType;
		uint16_t padding0;
		void* shaderArgumentBegin;
		void* shaderArgumentEnd;
		void* shaderArgumentCapacity;
		void* shaderSource;
		void* constantBufferUpdaterBegin;
		void* constantBufferUpdaterEnd;
		void* constantBufferUpdaterCapacity;
		void* constantBufferInitialBegin;
		void* constantBufferInitialEnd;
		void* constantBufferInitialCapacity;
		void* surfaceDataBegin;
		void* surfaceDataEnd;
		void* surfaceDataCapacity;
		void* inputBindingBegin;
		void* inputBindingEnd;
		void* inputBindingCapacity;
	};

	static_assert(sizeof(CompiledSubgraph) == (sizeof(void*) == 8 ? 136 : 72));
	static_assert(offsetof(CompiledSubgraph, constantBufferUpdaterBegin) == (sizeof(void*) == 8 ? 40 : 24));
	static_assert(offsetof(CompiledSubgraph, constantBufferInitialBegin) == (sizeof(void*) == 8 ? 64 : 36));
	static_assert(offsetof(CompiledSubgraph, inputBindingBegin) == (sizeof(void*) == 8 ? 112 : 60));

	// Synthetic ICompiledEffect object returned by DetourCompileEffectDescription.
	// It is COM-like enough for AddRef/Release and has the native vector fields at
	// the offsets EffectInstance expects. Do not wrap this in another object: DWM
	// already stores this pointer inside its own compilation task result wrapper.
	struct CompiledResult
	{
		void** vtable;
		volatile long refCount;
#if !defined(_M_IX86)
		std::uint32_t padding;
#endif
		CompiledSubgraph* subgraphBegin;
		CompiledSubgraph* subgraphEnd;
		CompiledSubgraph* subgraphCapacity;

		RuntimeEffectEntry* entry;
		CompiledResult* nativeBacking;
		std::uint32_t mainSubgraphIndex;
		struct CustomBody
		{
			std::uint32_t subgraphIndex;
			RuntimeEffectEntry* entry;
		};
		CustomBody* customBodyBegin;
		CustomBody* customBodyEnd;
		std::uint8_t* ownedSubgraphs;
	};

	static_assert(offsetof(CompiledResult, subgraphBegin) == (sizeof(void*) == 8 ? 16 : 8));
	static_assert(offsetof(CompiledResult, entry) == (sizeof(void*) == 8 ? 40 : 20));

	// Import patch records are name-aware because delay import thunks cannot always
	// be matched by function address before the delay loader resolves them.
	struct ImportPatch
	{
		char const* name;
		void* original;
		void* replacement;
	};

	RuntimeEffectEntry* g_effects{};
	std::mutex g_registryMutex;
	HMODULE g_wuceffectsiModule{};
	std::once_flag g_hookOnce;

	using CompileEffectDescriptionFn = HRESULT(__stdcall*)(void*, void**);
	CompileEffectDescriptionFn g_originalCompileEffectDescription{};

	bool SameGuid(GUID const& left, GUID const& right)
	{
		// Avoid relying on operator== or platform helpers here so the GUID comparison
		// stays valid for both WinRT GUID values and the raw GUID pointers returned
		// from native EffectType vtable slots.
		return left.Data1 == right.Data1 &&
			left.Data2 == right.Data2 &&
			left.Data3 == right.Data3 &&
			memcmp(left.Data4, right.Data4, sizeof(left.Data4)) == 0;
	}

	RuntimeEffectEntry* FindEntryByGuidLocked(GUID const& id)
	{
		// The registry is tiny and only mutated during effect construction, so a
		// linked list is enough. The caller holds g_registryMutex; keeping locking
		// outside avoids re-entering this helper from EffectType slots.
		for (auto* entry = g_effects; entry; entry = entry->next)
		{
			if (SameGuid(entry->definition->id, id))
			{
				return entry;
			}
		}

		return nullptr;
	}

	RuntimeEffectEntry* FindEntryByEffectTypeLocked(void* effectType)
	{
		// FlattenedEffectGraph stores EffectNode::m_effectType as a raw pointer to
		// the native EffectType. Comparing pointer identity is the most reliable way
		// to decide whether a graph node belongs to this runtime.
		for (auto* entry = g_effects; entry; entry = entry->next)
		{
			if (effectType == &entry->effectType)
			{
				return entry;
			}
		}

		return nullptr;
	}

	char const* GetShaderLibraryProfile(uint8_t shaderProfileVersion)
	{
		// Must stay paired with ShaderLinkingBody::shaderProfileVersion / the
		// D3DShaderProfileVersion byte DWM reads at body+0x2E:
		//   0 -> ps_4_0_level_9_1  (lib_4_0_level_9_1_ps_only)
		//   1 -> ps_4_0_level_9_3  (lib_4_0_level_9_3_ps_only)  [wuceffectsi default]
		//   2 -> ps_4_0            (lib_4_0)
		// There is no ps_5_0 / lib_5_0 path in dwmcorei: Link only knows those three
		// targets, and GetFragmentsModuleNoRef only loads three prebuilt modules.
		// A lib_5_0 blob can D3DLoadModule but fails at ID3D11Linker::Link when mixed
		// with 4.0-class fragment helpers (E_FAIL).
		switch (shaderProfileVersion)
		{
			case CustomEffectRuntime::kShaderProfileLevel91:
				return "lib_4_0_level_9_1_ps_only";
			case CustomEffectRuntime::kShaderProfilePs40:
				return "lib_4_0";
			case CustomEffectRuntime::kShaderProfileLevel93:
			default:
				return "lib_4_0_level_9_3_ps_only";
		}
	}

	void CompileShaderLibrary(
		char const* source,
		size_t sourceSize,
		uint8_t shaderProfileVersion,
		ID3DBlob** shaderBlob)
	{
		// Compile a shader-linking *library* module, not a standalone pixel shader.
		// dwmcorei!LoadShaderBody does D3DLoadModule + CreateInstance(functionName);
		// a plain ps_4_0 blob is not a loadable module.
		//
		// wuceffectsi!EffectGenerator::BuildCompiledEffectSubgraph hardcodes
		// "lib_4_0_level_9_3_ps_only" and flags 0x8800 (STRICTNESS | OPTIMIZATION_LEVEL3).
		// Profile 2 effects instead use "lib_4_0" so the bytecode matches
		// CShaderLinkingGraphBuilder::Link's "ps_4_0" target and fragment module 2.
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
		char const* profile = GetShaderLibraryProfile(shaderProfileVersion);

		winrt::com_ptr<ID3DBlob> errors;
		auto result = D3DCompile(
			source,
			sourceSize,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			profile,
			flags,
			0,
			shaderBlob,
			errors.put());
		if (FAILED(result))
		{
			std::string message = "HLSL compilation failed.";
			if (errors) message.assign(static_cast<char const*>(errors->GetBufferPointer()), errors->GetBufferSize());
			while (!message.empty() && message.back() == '\0')message.pop_back();
			throw hresult_error(result, to_hstring(message));
		}
	}

	void EnsureShader(RuntimeEffectEntry* entry)
	{
		auto const& definition = *entry->definition;
		if (definition.shaderBytecode && definition.shaderBytecodeSize)
		{
			winrt::com_ptr<ID3D11LibraryReflection> reflection;
			check_hresult(D3DReflectLibrary(
				definition.shaderBytecode,
				definition.shaderBytecodeSize,
				IID_ID3D11LibraryReflection,
				reflection.put_void()));
			D3D11_LIBRARY_DESC library{};
			check_hresult(reflection->GetDesc(&library));
			bool hasEntryPoint{};
			for (uint32_t index = 0; index < library.FunctionCount; ++index)
			{
				D3D11_FUNCTION_DESC function{};
				check_hresult(reflection->GetFunctionByIndex(index)->GetDesc(&function));
				if (function.Name && strcmp(function.Name, definition.shaderFunctionName) == 0)
				{
					hasEntryPoint = true;
					break;
				}
			}
			if (!hasEntryPoint)
			{
				throw hresult_invalid_argument(L"The DXBC library does not export the required shader function.");
			}
			return;
		}

		// Shader compilation is intentionally tied to the registered effect entry,
		// not to a brush instance. CompositionEffectFactory creation can occur on a
		// worker path and multiple brushes may share the same CustomEffectDefinition,
		// so call_once prevents duplicate D3DCompile work and keeps the bytecode
		// pointer stable for every compiled graph result.
		std::call_once(entry->shaderOnce, [entry]
					   {
						   auto const& definition = *entry->definition;
						   CompileShaderLibrary(
							   definition.shaderSource,
							   definition.shaderSourceSize,
							   definition.shaderProfileVersion,
							   entry->shaderBlob.put());
					   });
	}

	char const* HLSL_CALLBACK EffectType_GetShaderFragmentName(RuntimeEffectType* self)
	{
		// Used by wuceffectsi for diagnostics/hash names and by generated shader
		// naming. It is not the HLSL entrypoint; GetSubgraphShaderLinkingBody
		// provides that later through ShaderLinkingBody::functionName.
		return self->entry->definition->fragmentName;
	}

	GUID const* HLSL_CALLBACK EffectType_GetGuid(RuntimeEffectType* self)
	{
		// EffectType::FromGuid callers expect this slot to return stable storage.
		// Returning the address inside CustomEffectDefinition avoids temporary GUID
		// lifetime problems while preserving per-effect private GUID identity.
		return &self->entry->definition->id;
	}

	bool HLSL_CALLBACK EffectType_IsValidInputCount(RuntimeEffectType* self, uint32_t sourceCount)
	{
		// Traverser rejects the graph before CompileEffectDescription if the source
		// count does not match the EffectType metadata. Keep this strict so the
		// synthetic compiled graph shape cannot disagree with the WinRT wrapper.
		return sourceCount == self->entry->definition->sourceCount;
	}

	bool HLSL_CALLBACK EffectType_IsValidInputType(RuntimeEffectType*, uint32_t inputType)
	{
		// Native EffectType uses small input-type enums. Zero is the "null input"
		// case; all non-null source forms accepted by IGraphicsEffectD2D1Interop are
		// allowed here and resolved later by VisitEffectInputs.
		return inputType != 0;
	}

	uint32_t HLSL_CALLBACK EffectType_GetPropertiesStructSize(RuntimeEffectType* self)
	{
		// Traverser allocates/copies the default property blob using this size before
		// native metadata is consulted. It must match the struct layout used by the
		// effect-specific default-value provider and constant-buffer mappings.
		return self->entry->definition->propertiesStructSize;
	}

	uint32_t HLSL_CALLBACK EffectType_GetEffectSamplingBehavior(RuntimeEffectType*)
	{
		// Neutral/default sampling behavior. Custom sampler details are not exposed
		// from this slot; DWM later asks ICompiledEffect for samplerData flags and
		// shader-linking arguments per subgraph input.
		return 0;
	}

	bool HLSL_CALLBACK EffectType_ReturnFalse(RuntimeEffectType*)
	{
		// Several EffectType slots are boolean feature probes. The custom runtime
		// opts out of those native special cases unless a slot is modeled explicitly,
		// because an accidental true changes graph simplification or bounds behavior.
		return false;
	}

	bool HLSL_CALLBACK EffectType_RequiresSourceFlattening(RuntimeEffectType* self)
	{
		// wuceffectsi!Traverser uses EffectType slot 5 as the native source-flattening
		// gate: when it is true, named inputs are first wrapped in
		// CSingleInputCompositeEffect and become their own EffectSubgraph. The native
		// compiler preserves the resulting boundaries in CompileMaterializedGraph.
		(void)self;
		return true;
	}

	bool HLSL_CALLBACK EffectType_ReturnTrue(RuntimeEffectType*)
	{
		// Slot 12 is observed as a positive capability bit for generated shader
		// effects in this build. Keeping it true matches the native generated-effect
		// path while the other feature-probe slots remain false.
		return true;
	}

	bool HLSL_CALLBACK EffectType_IsInputTransform(RuntimeEffectType*, uint32_t* mode)
	{
		// Input-transform effects such as AffineTransform2D change bounds and source
		// coordinate propagation. Custom glass/blur effects here are ordinary render
		// effects, so report false and leave transform mode at zero.
		if (mode)
		{
			*mode = 0;
		}

		return false;
	}

	bool HLSL_CALLBACK EffectType_IsIntersectionCombinator(RuntimeEffectType*, void const*)
	{
		// Intersection/combinator effects alter how source bounds are merged. Custom
		// sampler effects here consume one already-resolved source surface, so they
		// must not participate in that built-in combinator path.
		return false;
	}

	bool HLSL_CALLBACK EffectType_IsNoOp(RuntimeEffectType*, uint32_t, void const*)
	{
		// Never let wuceffectsi elide a private effect as a no-op. Even a passthrough
		// shader is useful as a probe because it proves the synthetic compile result
		// and shader-linking path are the code that actually ran.
		return false;
	}

	uint32_t HLSL_CALLBACK EffectType_GetEffectOpacityRelation(RuntimeEffectType*, void const*)
	{
		// Report the neutral opacity relation. DWM can still blend the final brush
		// normally, but this avoids claiming built-in opacity preservation rules that
		// may not hold for arbitrary custom shader code.
		return 0;
	}

	void HLSL_CALLBACK EffectType_GetPropertiesMetadata(
		RuntimeEffectType* self,
		uint32_t* count,
		void const** metadata)
	{
		// EffectGenerator and EffectInstance both depend on native property metadata:
		// property name, byte offset, scalar/vector type, and value count. The public
		// WinRT property mapping alone is not enough for animated CompositionBrush
		// properties because DWM needs constant-buffer updater descriptors.
		auto const& definition = *self->entry->definition;
		if (count)
		{
			*count = definition.nativePropertyMetadataCount;
		}

		if (metadata)
		{
			*metadata = definition.nativePropertyMetadata;
		}
	}

	void HLSL_CALLBACK EffectType_Validate(RuntimeEffectType*, void const*)
	{
		// Built-in effects validate property ranges here. This runtime validates
		// structural metadata while building ConstantBufferUpdater records; per-effect
		// range validation can be added through NativePropertyMetadata::validator
		// once the native validator ABI is modeled.
	}

	thread_local void* g_nativePassthroughType{};

	struct NativeEffectNode
	{
		void* effectType;
		void* inputs;
		uint32_t index;
		uint32_t inputCount;
		void* properties;
		void* propertyMappings;
	};
	static_assert(sizeof(NativeEffectNode) == (sizeof(void*) == 8 ? 40 : 24));
	static_assert(offsetof(NativeEffectNode, properties) == (sizeof(void*) == 8 ? 24 : 16));

	void HLSL_CALLBACK EffectType_GenerateCode(RuntimeEffectType*, void const* node, void* generator, char const* name)
	{
		// Generate a disposable native body so the native compiler can retain the
		// upstream passes and bindings. Only this isolated body's shader is replaced.
		check_pointer(g_nativePassthroughType);
		auto passthroughNode = *static_cast<NativeEffectNode const*>(node);
		uint32_t mode = D2D1_COMPOSITE_MODE_SOURCE_OVER;
		passthroughNode.effectType = g_nativePassthroughType;
		passthroughNode.properties = &mode;
		auto vtable = *static_cast<void***>(g_nativePassthroughType);
		using Generate = void(HLSL_MEMBER*)(void*, void const*, void*, char const*);
		reinterpret_cast<Generate>(vtable[20])(g_nativePassthroughType, &passthroughNode, generator, name);
	}

	HLSL_THUNK(EffectType_GetShaderFragmentName)
		HLSL_THUNK(EffectType_GetGuid)
		HLSL_THUNK(EffectType_IsValidInputCount)
		HLSL_THUNK(EffectType_IsValidInputType)
		HLSL_THUNK(EffectType_GetPropertiesStructSize)
		HLSL_THUNK(EffectType_GetEffectSamplingBehavior)
		HLSL_THUNK(EffectType_ReturnFalse)
		HLSL_THUNK(EffectType_RequiresSourceFlattening)
		HLSL_THUNK(EffectType_ReturnTrue)
		HLSL_THUNK(EffectType_IsInputTransform)
		HLSL_THUNK(EffectType_IsIntersectionCombinator)
		HLSL_THUNK(EffectType_IsNoOp)
		HLSL_THUNK(EffectType_GetEffectOpacityRelation)
		HLSL_THUNK(EffectType_GetPropertiesMetadata)
		HLSL_THUNK(EffectType_Validate)
		HLSL_THUNK(EffectType_GenerateCode)

		void InitializeEffectType(RuntimeEffectEntry* entry, HMODULE wuceffectsi)
	{
		auto const base = reinterpret_cast<uint8_t*>(wuceffectsi);
		auto* vtable = entry->effectTypeVtable;

		std::fill(vtable, vtable + kEffectTypeVtableSlotCount, nullptr);

		// EffectType slots are not COM methods, and several folded tiny functions have
		// different meanings depending on their slot. These 22 entries cover every
		// observed EffectType virtual call from traversal, flattening, hashing, opacity
		// propagation, and generator code in this wuceffectsi build. The neutral native
		// bounds helpers are reused because their ABI includes struct-return details.
		vtable[0] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_GetShaderFragmentName));
		vtable[1] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_GetGuid));
		vtable[2] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_GetEffectSamplingBehavior));
		vtable[3] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_IsValidInputCount));
		vtable[4] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_IsValidInputType));
		vtable[5] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_RequiresSourceFlattening));
		vtable[6] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_IsInputTransform));
		vtable[7] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_ReturnFalse));
		vtable[8] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_ReturnFalse));
		vtable[9] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_ReturnFalse));
		vtable[10] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_ReturnFalse));
		vtable[11] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_ReturnFalse));
		vtable[12] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_ReturnTrue));
		vtable[13] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_IsIntersectionCombinator));
		vtable[14] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_IsNoOp));
		vtable[15] = base + kEffectTypeGetBoundsRva;
		vtable[16] = base + kEffectTypeCalcInputBoundsRva;
		vtable[17] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_GetPropertiesStructSize));
		vtable[18] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_GetPropertiesMetadata));
		vtable[19] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_Validate));
		vtable[20] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_GenerateCode));
		vtable[21] = reinterpret_cast<void*>(HLSL_METHOD(EffectType_GetEffectOpacityRelation));
	}

	void InitializeAllEffectTypes(HMODULE wuceffectsi)
	{
		// RegisterEffect may run before wuceffectsi.dll is loaded. Once the hook is
		// installed, every previously registered entry must receive a valid native
		// vtable before EffectType::FromGuid can return it.
		std::lock_guard<std::mutex> guard(g_registryMutex);
		for (auto* entry = g_effects; entry; entry = entry->next)
		{
			InitializeEffectType(entry, wuceffectsi);
		}
	}

	void* __fastcall DetourEffectTypeFromGuid(GUID const* guid)
	{
		// First give private runtime GUIDs a chance to resolve to synthetic
		// EffectType objects. This is what lets CreateEffectFactory accept a real
		// custom GUID instead of pretending to be ColorMatrix/GaussianBlur/etc.
		if (guid)
		{
			std::lock_guard<std::mutex> guard(g_registryMutex);
			if (auto* entry = FindEntryByGuidLocked(*guid))
			{
				return &entry->effectType;
			}
		}

		// For all built-in GUIDs, reproduce the native lookup by scanning the
		// wuceffectsi EffectType table and calling slot 1 (GetGuid). This keeps the
		// patch transparent for every effect this runtime does not own.
		auto const module = GetModuleHandleW(L"wuceffectsi.dll");
		if (!module || !guid)
		{
			return nullptr;
		}

		auto const base = reinterpret_cast<uint8_t*>(module);
		auto* table = reinterpret_cast<void**>(base + kEffectTypeTableRva);
		for (size_t index = 0; index < kEffectTypeCount; ++index)
		{
			auto* effectType = table[index];
			if (!effectType)
			{
				continue;
			}

			auto* vtable = *reinterpret_cast<void***>(effectType);
			auto const getGuid = reinterpret_cast<GUID const* (HLSL_MEMBER*)(void*)>(vtable[1]);
			auto const knownGuid = getGuid(effectType);
			if (knownGuid && SameGuid(*knownGuid, *guid))
			{
				return effectType;
			}
		}

		return nullptr;
	}

	void PatchEffectTypeFromGuid(HMODULE wuceffectsi)
	{
		// EffectType::FromGuid is called before CompileEffectDescription. If it
		// returns null, traversal fails with "Unsupported effect type" and our
		// compile detour never runs. Patching this function is therefore the first
		// gate that makes private GUIDs possible.
		auto* target = reinterpret_cast<uint8_t*>(wuceffectsi) + kEffectTypeFromGuidRva;
		auto patch = HlslNativeAbi::MakeEntryPatch(target, reinterpret_cast<void*>(DetourEffectTypeFromGuid));

		DWORD oldProtect{};
		check_bool(VirtualProtect(target, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect));
		memcpy(target, patch.data(), patch.size());
		FlushInstructionCache(GetCurrentProcess(), target, sizeof(patch));
		DWORD unused{};
		VirtualProtect(target, sizeof(patch), oldProtect, &unused);
	}

	ULONG AddRef(volatile long* refCount)
	{
		// Keep ref-count helpers tiny and ABI-neutral. The object is consumed across
		// native DWM/WUCEffectsI code paths, so interlocked operations are required
		// even though creation happens on the app side.
		return static_cast<ULONG>(InterlockedIncrement(refCount));
	}

	ULONG ReleaseRef(volatile long* refCount)
	{
		// Release may run on the DWM composition/effect worker path, not necessarily
		// on the UI thread that created the brush.
		return static_cast<ULONG>(InterlockedDecrement(refCount));
	}

	uint32_t PointerRangeByteSize(void const* begin, void const* end);
	bool UsesFlattenSourceSubgraph(CustomEffectRuntime::CustomEffectDefinition const& definition);
	uint32_t GetMainSubgraphIndex(CustomEffectRuntime::CustomEffectDefinition const& definition);

	ULONG HLSL_CALLBACK Wrapper_AddRef(CompiledResult* self)
	{
		// DWM treats ICompiledEffect as ref-counted even though this object is not a
		// C++/WinRT implements type. Keep the lifetime independent from the public
		// RuntimeGraphicsEffect object; factories can outlive the original wrapper.
		return AddRef(&self->refCount);
	}

	void DestroyCompiledResult(CompiledResult* self)
	{
		// Every vector field in CompiledSubgraph points to process-heap allocations
		// created by this file. Free each vector explicitly before freeing the outer
		// CompiledResult so native EffectInstance cannot observe dangling ranges.
		if (self->subgraphBegin)
		{
			for (auto* subgraph = self->subgraphBegin; subgraph != self->subgraphEnd; ++subgraph)
			{
				auto index = static_cast<size_t>(subgraph - self->subgraphBegin);
				if (self->nativeBacking && (!self->ownedSubgraphs || !self->ownedSubgraphs[index]))
					continue;
				if (subgraph->inputBindingBegin)
				{
					HeapFree(GetProcessHeap(), 0, subgraph->inputBindingBegin);
				}

				if (subgraph->shaderArgumentBegin)
				{
					HeapFree(GetProcessHeap(), 0, subgraph->shaderArgumentBegin);
				}

				if (subgraph->constantBufferInitialBegin)
				{
					HeapFree(GetProcessHeap(), 0, subgraph->constantBufferInitialBegin);
				}

				if (subgraph->constantBufferUpdaterBegin)
				{
					HeapFree(GetProcessHeap(), 0, subgraph->constantBufferUpdaterBegin);
				}

				if (subgraph->surfaceDataBegin)
				{
					HeapFree(GetProcessHeap(), 0, subgraph->surfaceDataBegin);
				}
			}

			HeapFree(GetProcessHeap(), 0, self->subgraphBegin);
		}
		if (self->customBodyBegin) HeapFree(GetProcessHeap(), 0, self->customBodyBegin);
		if (self->ownedSubgraphs) HeapFree(GetProcessHeap(), 0, self->ownedSubgraphs);
		if (self->nativeBacking)
		{
			using Release = ULONG(__stdcall*)(CompiledResult*);
			reinterpret_cast<Release>(self->nativeBacking->vtable[1])(self->nativeBacking);
		}

		HeapFree(GetProcessHeap(), 0, self);
	}

	ULONG HLSL_CALLBACK Wrapper_Release(CompiledResult* self)
	{
		// This is paired with Wrapper_AddRef rather than C++/WinRT lifetime support.
		// Native callers only know the first vtable pointer and the reference count
		// field; they never see a winrt::implements control block.
		auto const ref = ReleaseRef(&self->refCount);
		if (ref == 0)
		{
			DestroyCompiledResult(self);
		}

		return ref;
	}

	uint32_t HLSL_CALLBACK Wrapper_GetSubgraphCount(CompiledResult* self)
	{
		// DWM uses this count to size its SubgraphOutput array. wuceffectsi later
		// iterates the same subgraph vector for constant-buffer creation, so this
		// must match the actual [subgraphBegin, subgraphEnd) range.
		auto* subgraphBegin = self->subgraphBegin;
		auto* subgraphEnd = self->subgraphEnd;
		if (!subgraphBegin || !subgraphEnd || subgraphEnd < subgraphBegin)
		{
			return 0;
		}

		return static_cast<uint32_t>(subgraphEnd - subgraphBegin);
	}

	ShaderLinkingBody* HLSL_CALLBACK Wrapper_GetSubgraphShaderLinkingBody(
		CompiledResult* self,
		ShaderLinkingBody* body,
		uint32_t subgraphIndex)
	{
		// This is the most important ICompiledEffect method. It supplies DWM with a
		// loadable shader library, the exported HLSL function name, and the packed
		// shader-linking argument list (color input, uv, samplerData, samplerDataExt,
		// custom sampler result, etc.).
		if (subgraphIndex >= Wrapper_GetSubgraphCount(self))
		{
			check_hresult(E_INVALIDARG);
		}
		RuntimeEffectEntry* entry = self->entry;
		for (auto current = self->customBodyBegin; current && current != self->customBodyEnd; ++current)
			if (current->subgraphIndex == subgraphIndex) entry = current->entry;
		if (self->nativeBacking && !entry)
		{
			using GetBody = ShaderLinkingBody * (HLSL_MEMBER*)(CompiledResult*, ShaderLinkingBody*, uint32_t);
			return reinterpret_cast<GetBody>(self->nativeBacking->vtable[3])(self->nativeBacking, body, subgraphIndex);
		}
		check_pointer(entry);
		EnsureShader(entry);
		auto const& definition = *entry->definition;

		auto* subgraph = self->subgraphBegin + subgraphIndex;
		auto const isMainSubgraph = self->customBodyBegin || subgraphIndex == self->mainSubgraphIndex;
		auto const argCount = subgraph && subgraph->shaderArgumentBegin && subgraph->shaderArgumentEnd
			? static_cast<size_t>(
				(static_cast<uint8_t*>(subgraph->shaderArgumentEnd) -
				 static_cast<uint8_t*>(subgraph->shaderArgumentBegin)) /
				sizeof(uint16_t))
			: static_cast<size_t>(definition.shaderArgumentCount);

		body->argCount = argCount;
		body->argData = subgraph && subgraph->shaderArgumentBegin
			? subgraph->shaderArgumentBegin
			: definition.shaderArguments;
		if (definition.shaderBytecode && definition.shaderBytecodeSize)
		{
			body->bytecodeSize = definition.shaderBytecodeSize;
			body->bytecodeData = definition.shaderBytecode;
		}
		else
		{
			body->bytecodeSize = entry->shaderBlob->GetBufferSize();
			body->bytecodeData = entry->shaderBlob->GetBufferPointer();
		}
		body->functionName = isMainSubgraph ? definition.shaderFunctionName : definition.materializationShaderFunctionName;
		body->constantBufferSize = PointerRangeByteSize(
			subgraph->constantBufferInitialBegin,
			subgraph->constantBufferInitialEnd);
		body->linkingArgType = subgraph->linkingArgType;
		// Body+0x2E is D3DShaderProfileVersion (1 byte). LinkShader takes it from the
		// technique's main body and applies it to that entire link (target string +
		// GetFragmentsModuleNoRef). Scope is one CRenderingTechnique, not the brush
		// or visual tree: other techniques / brushes link independently after any
		// intermediate is materialized to a surface. All subgraphs of one definition
		// share one blob, so every body reports the same version — including
		// flatten/final helpers that may be pulled into the same span as 0x0500 deps.
		// wuceffectsi always writes 1; CCustomKernelEffect writes 0/1/2 from FL.
		body->shaderProfileVersion = definition.shaderProfileVersion;
		body->padding = 0;
		return body;
	}

	uint32_t HLSL_CALLBACK Wrapper_GetSubgraphInputCount(CompiledResult* self, uint32_t subgraphIndex)
	{
		// Input count is per subgraph, not per effect. A flatten/custom-sampler graph
		// has different input meanings at each subgraph: source brush, previous
		// intermediate, or final wrapper input.
		if (subgraphIndex >= Wrapper_GetSubgraphCount(self))
		{
			return 0;
		}

		auto const& subgraph = self->subgraphBegin[subgraphIndex];
		auto* inputBegin = static_cast<InputBinding*>(subgraph.inputBindingBegin);
		auto* inputEnd = static_cast<InputBinding*>(subgraph.inputBindingEnd);
		if (!inputBegin || !inputEnd || inputEnd < inputBegin)
		{
			return 0;
		}

		return static_cast<uint32_t>(inputEnd - inputBegin);
	}

	uint32_t HLSL_CALLBACK Wrapper_GetSubgraphFlags(CompiledResult* self, uint32_t subgraphIndex)
	{
		// Flag 0x8 is observed by CBrushRenderingGraphBuilder as "keep this subgraph
		// as a fragment output". For the LiquidGlass shape this prevents the custom
		// material from being rendered into the upstream blur's prescaled target.
		if (subgraphIndex >= Wrapper_GetSubgraphCount(self))
		{
			return 0;
		}

		return self->subgraphBegin[subgraphIndex].flags;
	}

	uint32_t HLSL_CALLBACK Wrapper_GetInputMapping(
		CompiledResult* self,
		uint32_t subgraphIndex,
		uint32_t inputIndex,
		bool* isSubgraphOutput)
	{
		// DWM asks this for each subgraph input. If isSubgraphOutput is false, the
		// returned index selects a named brush source. If true, it selects a previous
		// SubgraphOutput entry. This is how the synthetic graph expresses edges
		// between flattened/intermediate/custom/final subgraphs.
		if (subgraphIndex >= Wrapper_GetSubgraphCount(self))
		{
			check_hresult(E_INVALIDARG);
		}

		auto const& subgraph = self->subgraphBegin[subgraphIndex];
		auto* inputBegin = static_cast<InputBinding*>(subgraph.inputBindingBegin);
		auto* inputEnd = static_cast<InputBinding*>(subgraph.inputBindingEnd);
		if (!inputBegin || !inputEnd ||
			inputIndex >= static_cast<uint32_t>(inputEnd - inputBegin))
		{
			check_hresult(E_INVALIDARG);
		}

		auto const& binding = inputBegin[inputIndex];
		if (isSubgraphOutput)
		{
			*isSubgraphOutput = binding.isSubgraphOutput;
		}

		return binding.inputIndex;
	}

	bool HLSL_CALLBACK Wrapper_IsUVClampingRequired(
		CompiledResult* self,
		uint32_t subgraphIndex,
		uint32_t inputIndex,
		uint8_t* horizontalMode,
		uint8_t* verticalMode)
	{
		// Native callers reserve ONE byte for each edge mode. A uint32_t store
		// corrupts the caller's frame on x86 (and adjacent locals on 64-bit).
		SurfaceData data{};
		if (subgraphIndex < Wrapper_GetSubgraphCount(self))
		{
			auto const& subgraph = self->subgraphBegin[subgraphIndex];
			auto begin = static_cast<SurfaceData const*>(subgraph.surfaceDataBegin);
			auto end = static_cast<SurfaceData const*>(subgraph.surfaceDataEnd);
			if (begin && end && inputIndex < static_cast<size_t>(end - begin))
				data = begin[inputIndex];
		}
		if (horizontalMode) *horizontalMode = data.data[0];
		if (verticalMode) *verticalMode = data.data[1];
		return data.data[2] != 0;
	}

	bool HLSL_CALLBACK Wrapper_IsSamplerDataExtRequired(
		CompiledResult* self,
		uint32_t subgraphIndex,
		uint32_t inputIndex)
	{
		// samplerDataExt carries source/intermediate dimensions and texel-size data
		// for custom sampler bodies. The LiquidGlass shader uses it for refraction
		// offsets, while samplerData is used for the effective content rect.
		auto* subgraphBegin = self->subgraphBegin;
		auto* subgraphEnd = self->subgraphEnd;
		if (!subgraphBegin || !subgraphEnd || subgraphIndex >= static_cast<uint32_t>(subgraphEnd - subgraphBegin))
		{
			return false;
		}

		auto const& subgraph = subgraphBegin[subgraphIndex];
		auto* surfaceDataBegin = static_cast<SurfaceData*>(subgraph.surfaceDataBegin);
		auto* surfaceDataEnd = static_cast<SurfaceData*>(subgraph.surfaceDataEnd);
		if (!surfaceDataBegin || !surfaceDataEnd ||
			inputIndex >= static_cast<uint32_t>(surfaceDataEnd - surfaceDataBegin))
		{
			return false;
		}

		return surfaceDataBegin[inputIndex].data[3] != 0;
	}

	uint32_t HLSL_CALLBACK Wrapper_GetConstantBufferSize(CompiledResult* self, uint32_t subgraphIndex)
	{
		// EffectInstance allocates one constant buffer per flattened subgraph. Most
		// helper/flatten subgraphs intentionally return zero here; the main subgraph
		// exposes the effect definition's constant buffer range.
		if (subgraphIndex >= Wrapper_GetSubgraphCount(self))
		{
			return 0;
		}

		auto const& subgraph = self->subgraphBegin[subgraphIndex];
		return PointerRangeByteSize(
			subgraph.constantBufferInitialBegin,
			subgraph.constantBufferInitialEnd);
	}

	void const* HLSL_CALLBACK Wrapper_GetConstantBufferInitialValue(CompiledResult* self, uint32_t subgraphIndex)
	{
		// Native code copies this initial blob before applying direct property
		// updates. It must remain valid for the lifetime of the CompiledResult.
		if (subgraphIndex >= Wrapper_GetSubgraphCount(self))
		{
			return nullptr;
		}

		auto const& subgraph = self->subgraphBegin[subgraphIndex];
		return subgraph.constantBufferInitialBegin;
	}

	void* HLSL_CALLBACK Wrapper_ScalarDeletingDestructor(CompiledResult* self, uint32_t flags)
	{
		// MSVC scalar-deleting destructor slot. Some native cleanup paths call this
		// instead of Release when they believe they own the compiled effect directly;
		// honor the delete flag but otherwise leave ownership unchanged.
		if ((flags & 1) != 0)
		{
			DestroyCompiledResult(self);
		}

		return self;
	}

	void HLSL_CALLBACK Wrapper_FinalRelease(CompiledResult*)
	{
		// Native ICompiledEffect has a final-release-style slot after the deleting
		// destructor. The synthetic object has no secondary resources outside the
		// explicit vector ranges freed in DestroyCompiledResult, so this is a no-op.
	}

	// CompileEffectDescription must return the CompiledEffect-shaped object directly.
	// dwmcorei!Compile_WorkerThread already wraps that returned pointer in its own task
	// result object, and GetCompiledEffectNoRef returns the pointer stored in that DWM
	// wrapper at +0x20. Returning another app-defined outer wrapper here makes
	// wuceffectsi!EffectInstance read that wrapper's +0x10/+0x18 as an empty subgraph
	// vector, so this intentionally diverges from the earlier v3 note's extra-wrapper
	// wording for this WinUI3 build.
	HLSL_THUNK(Wrapper_GetSubgraphCount)
		HLSL_THUNK(Wrapper_GetSubgraphShaderLinkingBody)
		HLSL_THUNK(Wrapper_GetSubgraphInputCount)
		HLSL_THUNK(Wrapper_GetSubgraphFlags)
		HLSL_THUNK(Wrapper_GetInputMapping)
		HLSL_THUNK(Wrapper_IsUVClampingRequired)
		HLSL_THUNK(Wrapper_IsSamplerDataExtRequired)
		HLSL_THUNK(Wrapper_GetConstantBufferSize)
		HLSL_THUNK(Wrapper_GetConstantBufferInitialValue)
		HLSL_THUNK(Wrapper_ScalarDeletingDestructor)
		HLSL_THUNK(Wrapper_FinalRelease)

		void* g_wrapperVtable[] = {
			reinterpret_cast<void*>(Wrapper_AddRef),
			reinterpret_cast<void*>(Wrapper_Release),
			reinterpret_cast<void*>(HLSL_METHOD(Wrapper_GetSubgraphCount)),
			reinterpret_cast<void*>(HLSL_METHOD(Wrapper_GetSubgraphShaderLinkingBody)),
			reinterpret_cast<void*>(HLSL_METHOD(Wrapper_GetSubgraphInputCount)),
			reinterpret_cast<void*>(HLSL_METHOD(Wrapper_GetSubgraphFlags)),
			reinterpret_cast<void*>(HLSL_METHOD(Wrapper_GetInputMapping)),
			reinterpret_cast<void*>(HLSL_METHOD(Wrapper_IsUVClampingRequired)),
			reinterpret_cast<void*>(HLSL_METHOD(Wrapper_IsSamplerDataExtRequired)),
			reinterpret_cast<void*>(HLSL_METHOD(Wrapper_GetConstantBufferSize)),
			reinterpret_cast<void*>(HLSL_METHOD(Wrapper_GetConstantBufferInitialValue)),
			reinterpret_cast<void*>(HLSL_METHOD(Wrapper_ScalarDeletingDestructor)),
			reinterpret_cast<void*>(HLSL_METHOD(Wrapper_FinalRelease)),
	};

	void* AllocateBytes(size_t size)
	{
		// Match the native heap used by the rest of this synthetic object so cleanup
		// can be uniform in DestroyCompiledResult.
		if (!size)
		{
			return nullptr;
		}

		auto* memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
		check_pointer(memory);
		return memory;
	}

	uint32_t PointerRangeByteSize(void const* begin, void const* end)
	{
		// Native vectors are represented by begin/end pointers. Defensive validation
		// avoids unsigned wrap if a malformed definition leaves a range inverted.
		auto const beginAddress = reinterpret_cast<uintptr_t>(begin);
		auto const endAddress = reinterpret_cast<uintptr_t>(end);
		if (!beginAddress || !endAddress || endAddress < beginAddress)
		{
			return 0;
		}

		return static_cast<uint32_t>(endAddress - beginAddress);
	}

	bool UsesFlattenSourceSubgraph(CustomEffectRuntime::CustomEffectDefinition const& definition)
	{
		// Match EffectType slot 5 when selecting native materialized compilation.
		return definition.inputMode ==
			CustomEffectRuntime::CustomEffectInputMode::MaterializedTexture;
	}

	uint32_t GetMainSubgraphIndex(CustomEffectRuntime::CustomEffectDefinition const& definition)
	{
		// With no flatten stage, subgraph 0 is the effect. With source flattening,
		// subgraph 0 materializes the source and subgraph 1 is the custom sampler.
		return UsesFlattenSourceSubgraph(definition) ? 1u : 0u;
	}

	uint32_t GetCompiledSubgraphCount(CustomEffectRuntime::CustomEffectDefinition const& definition)
	{
		// This is only the standalone shader template. Materialized graphs borrow
		// native stages and relocate its main/output bodies using the traversed map;
		// they must never return this template as the whole compiled graph.
		return UsesFlattenSourceSubgraph(definition) ? 3u : 1u;
	}

	void InitializeDirectPropertyUpdater(
		ConstantBufferUpdater& updater,
		void** directUpdaterFunctionVtable,
		CustomEffectRuntime::NativePropertyMetadata const& metadata,
		uint32_t constantBufferOffset,
		uint32_t nodeIndex)
	{
		updater.nodeIndex = nodeIndex;
		updater.constantBufferOffset = constantBufferOffset;
		memset(&updater.update, 0, sizeof(updater.update));

		// wuceffectsi!DeclareShaderVariableForProperty uses this exact
		// std::function target for direct animatable properties. Reusing its vtable
		// keeps SetAnimatableProperty on the native EffectInstance path instead of
		// rebuilding brushes from XAML slider changes.
		auto* callable = reinterpret_cast<NativePropertyUpdaterCallable*>(updater.update.inlineStorage);
		callable->vtable = directUpdaterFunctionVtable;
		callable->metadata = metadata;
		updater.update.callable = callable;
	}

	void InitializeSubgraphInputs(
		CompiledSubgraph& subgraph,
		CustomEffectRuntime::SourceDescriptor const* sources,
		uint32_t sourceCount,
		bool mapFromSubgraphOutput,
		uint32_t mappedInputBase,
		bool copySamplerDataExtRequirements)
	{
		if (!sourceCount)
		{
			return;
		}

		// This fills two native vectors in parallel. inputBindings describes where
		// each logical source comes from; surfaceData describes which sampler helper
		// arguments DWM must make available for that input. The copySamplerData flag
		// lets final color-only wrapper subgraphs avoid requesting sampler metadata
		// they do not consume.
		auto* inputBindings = static_cast<InputBinding*>(
			AllocateBytes(sizeof(InputBinding) * sourceCount));
		auto* surfaceData = static_cast<SurfaceData*>(
			AllocateBytes(sizeof(SurfaceData) * sourceCount));
		for (uint32_t index = 0; index < sourceCount; ++index)
		{
			inputBindings[index].inputIndex = mappedInputBase + index;
			inputBindings[index].isSubgraphOutput = mapFromSubgraphOutput;

			// wuceffectsi copies EffectGenerator::SurfaceData bytes 4..7 into
			// CompiledEffectSubgraph::SurfaceData. This code-only path bypasses
			// that generator, so we synthesize the two bytes DWM later observes:
			// byte 2 drives IsUVClampingRequired and makes PopulateSamplerArguments
			// emit GetSamplerDataN, while byte 3 drives IsSamplerDataExtRequired
			// and emits GetSamplerDataExtN. This intentionally replaces the
			// earlier samplerDataExt-only shortcut because CCustomKernelEffect's
			// single-source model uses samplerData to recover the effective content
			// rect when a source has been materialized into a padded intermediate.
			surfaceData[index].data[2] =
				copySamplerDataExtRequirements && sources[index].requiresSamplerData ? 1 : 0;
			surfaceData[index].data[3] =
				copySamplerDataExtRequirements && sources[index].requiresSamplerDataExt ? 1 : 0;
		}

		subgraph.inputBindingBegin = inputBindings;
		subgraph.inputBindingEnd = inputBindings + sourceCount;
		subgraph.inputBindingCapacity = inputBindings + sourceCount;

		subgraph.surfaceDataBegin = surfaceData;
		subgraph.surfaceDataEnd = surfaceData + sourceCount;
		subgraph.surfaceDataCapacity = surfaceData + sourceCount;
	}

	void InitializeSubgraphShaderArguments(
		CompiledSubgraph& subgraph,
		uint16_t const* arguments,
		uint64_t argumentCount)
	{
		// Shader arguments are the compact numbers consumed by dwmcorei's shader
		// linker. They are not HLSL reflection data. Each value selects one linker
		// input kind such as color sample, uv, samplerData, samplerDataExt, or the
		// custom sampler result type.
		if (!argumentCount)
		{
			return;
		}

		auto* shaderArguments = static_cast<uint16_t*>(
			AllocateBytes(sizeof(uint16_t) * static_cast<size_t>(argumentCount)));
		memcpy(
			shaderArguments,
			arguments,
			sizeof(uint16_t) * static_cast<size_t>(argumentCount));

		subgraph.shaderArgumentBegin = shaderArguments;
		subgraph.shaderArgumentEnd = shaderArguments + argumentCount;
		subgraph.shaderArgumentCapacity = shaderArguments + argumentCount;
	}

	void* CreateCompiledResult(RuntimeEffectEntry* entry, uint32_t propertyNodeIndex)
	{
		// Build the native-shaped compiled graph DWM expects after
		// CompileEffectDescription. Nothing in this function is cosmetic: every
		// vector range is later read either through the wrapper vtable or directly by
		// wuceffectsi's EffectInstance.
		EnsureShader(entry);

		CompiledResult* result{};
		try
		{
			auto const& definition = *entry->definition;
			result = static_cast<CompiledResult*>(
				HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CompiledResult)));
			check_pointer(result);

			result->vtable = g_wrapperVtable;
			result->refCount = 1;
			result->entry = entry;

			if (UsesFlattenSourceSubgraph(definition))
			{
				// The flatten stage models the Traverser path used by source-materializing
				// effects such as GaussianBlur: first emit an ordinary color passthrough
				// subgraph, then let the custom sampler subgraph consume that intermediate
				// as a real surface. Without this extra subgraph dwmcorei records the
				// upstream effect as a child fragment and MakeShaderLinkingArgument emits
				// 0x0500 dependency output, which cannot provide texture/samplerDataExt.
				if (definition.sourceCount != 1 ||
					!definition.materializationShaderFunctionName ||
					definition.shaderArgumentCount == 0)
				{
					check_hresult(E_INVALIDARG);
				}
			}

			auto const subgraphCount = GetCompiledSubgraphCount(definition);
			auto* subgraph = static_cast<CompiledSubgraph*>(
				HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CompiledSubgraph) * subgraphCount));
			check_pointer(subgraph);

			result->subgraphBegin = subgraph;
			result->subgraphEnd = subgraph + subgraphCount;
			result->subgraphCapacity = subgraph + subgraphCount;

			auto const mainSubgraphIndex = GetMainSubgraphIndex(definition);
			result->mainSubgraphIndex = mainSubgraphIndex;
			auto& mainSubgraph = subgraph[mainSubgraphIndex];

			if (UsesFlattenSourceSubgraph(definition))
			{
				constexpr uint16_t flattenColorArgument = 0x0200;
				auto& flattenSubgraph = subgraph[0];
				InitializeSubgraphInputs(
					flattenSubgraph,
					definition.sources,
					definition.sourceCount,
					false,
					0,
					true);
				InitializeSubgraphShaderArguments(flattenSubgraph, &flattenColorArgument, 1);
				// Keep flatten materialized. The custom sampler body needs a real
				// surface argument from MakeShaderLinkingArgument; if this subgraph
				// is kept as a fragment output, dwmcorei rewrites the source to a
				// 0x0500 dependency argument instead of a Texture2D/samplerData pair.
				flattenSubgraph.flags = 0;
				flattenSubgraph.linkingArgType = 0;

				InitializeSubgraphInputs(
					mainSubgraph,
					definition.sources,
					definition.sourceCount,
					true,
					0,
					true);

				constexpr uint16_t finalColorArgument = 0x0200;
				auto& finalSubgraph = subgraph[2];
				InitializeSubgraphInputs(
					finalSubgraph,
					definition.sources,
					definition.sourceCount,
					true,
					1,
					false);
				InitializeSubgraphShaderArguments(finalSubgraph, &finalColorArgument, 1);
				finalSubgraph.flags = 0;
				finalSubgraph.linkingArgType = 0;
			}
			else
			{
				InitializeSubgraphInputs(
					mainSubgraph,
					definition.sources,
					definition.sourceCount,
					false,
					0,
					true);
			}

			InitializeSubgraphShaderArguments(
				mainSubgraph,
				definition.shaderArguments,
				definition.shaderArgumentCount);

			if (definition.constantBufferSize)
			{
				// Store the initial constant buffer only on the main shader subgraph.
				// Helper flatten/final wrapper subgraphs are color passthrough nodes
				// and must report no constant buffer to EffectInstance.
				auto* constantBuffer = static_cast<uint8_t*>(
					AllocateBytes(definition.constantBufferSize));
				if (definition.constantBufferInitialValue)
				{
					memcpy(
						constantBuffer,
						definition.constantBufferInitialValue,
						definition.constantBufferSize);
				}

				mainSubgraph.constantBufferInitialBegin = constantBuffer;
				mainSubgraph.constantBufferInitialEnd = constantBuffer + definition.constantBufferSize;
				mainSubgraph.constantBufferInitialCapacity = constantBuffer + definition.constantBufferSize;
			}

			if (definition.constantBufferPropertyCount)
			{
				// Animated CompositionBrush properties update the native constant
				// buffer through DirectPropertyUpdater callables. This validates that
				// every public property maps to a real scalar range inside the native
				// property struct and the HLSL constant buffer.
				check_pointer(definition.nativePropertyMetadata);
				check_pointer(definition.constantBufferProperties);

				auto* metadata = static_cast<CustomEffectRuntime::NativePropertyMetadata const*>(
					definition.nativePropertyMetadata);
				auto* module = g_wuceffectsiModule ? g_wuceffectsiModule : GetModuleHandleW(L"wuceffectsi.dll");
				check_pointer(module);
				auto* directUpdaterFunctionVtable =
					reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(module) + kDirectPropertyUpdaterFunctionVtableRva);

				auto* updaters = static_cast<ConstantBufferUpdater*>(
					AllocateBytes(sizeof(ConstantBufferUpdater) * definition.constantBufferPropertyCount));
				for (uint32_t index = 0; index < definition.constantBufferPropertyCount; ++index)
				{
					auto const& mapping = definition.constantBufferProperties[index];
					if (mapping.propertyIndex >= definition.nativePropertyMetadataCount)
					{
						check_hresult(E_INVALIDARG);
					}

					auto const& property = metadata[mapping.propertyIndex];
					auto const propertyBytes = property.valueCount * sizeof(float);
					if (property.propertyType != 8 ||
						property.propertyOffset + propertyBytes > definition.propertiesStructSize ||
						mapping.constantBufferOffset + propertyBytes > definition.constantBufferSize)
					{
						check_hresult(E_INVALIDARG);
					}

					InitializeDirectPropertyUpdater(
						updaters[index],
						directUpdaterFunctionVtable,
						property,
						mapping.constantBufferOffset,
						// EffectInstance reads the original graph's property blob here.
						propertyNodeIndex);
				}

				mainSubgraph.constantBufferUpdaterBegin = updaters;
				mainSubgraph.constantBufferUpdaterEnd = updaters + definition.constantBufferPropertyCount;
				mainSubgraph.constantBufferUpdaterCapacity = updaters + definition.constantBufferPropertyCount;
			}

			// DWM materializes non-final subgraphs with flags==0 via
			// CBrushRenderingGraphBuilder::CreateTechniqueForFragment. For the
			// flatten/custom-sampler shape, only the custom material subgraph
			// should stay linked into the final consumer fragment; otherwise the
			// LiquidGlass result is rendered into an upstream GaussianBlur
			// prescale target and then linearly enlarged back to the XAML rect.
			mainSubgraph.flags = UsesFlattenSourceSubgraph(definition)
				? kCompiledEffectSubgraphOutputFlag
				: 0;
			mainSubgraph.linkingArgType = definition.linkingArgType;
			return result;
		}
		catch (...)
		{
			if (result)
			{
				DestroyCompiledResult(result);
			}

			throw;
		}
	}

	struct EffectNodeView
	{
		void* address{};
		void* effectType{};
	};

	struct InspectedEffectGraph
	{
		std::vector<EffectNodeView> nodes;
	};

	struct CustomNode
	{
		uint32_t nodeIndex{};
		uint32_t subgraphIndex{ std::numeric_limits<uint32_t>::max() };
		RuntimeEffectEntry* entry{};
	};

	InspectedEffectGraph InspectEffectGraph(void* description)
	{
		// CompileEffectDescription is shared by all composition effects. Only return
		// a custom compiled result when the flattened graph actually contains one of
		// our synthetic EffectType pointers; otherwise forward to the original export.
		if (!description)
		{
			return {};
		}

		// CompileEffectDescription receives the IEffectDescriptionWithNames interface
		// pointer at FlattenedEffectGraph + two pointers, not the object base. Reversing the
		// export showed it subtracts 0x10 before invoking EffectGenerator::Compile, so
		// the detour must do the same when it inspects the node vector.
		auto* graph = static_cast<uint8_t*>(description) - 2 * sizeof(void*);
		auto* nodeBegin = *reinterpret_cast<void***>(graph + 6 * sizeof(void*));
		auto* nodeEnd = *reinterpret_cast<void***>(graph + 7 * sizeof(void*));
		auto const beginAddress = reinterpret_cast<uintptr_t>(nodeBegin);
		auto const endAddress = reinterpret_cast<uintptr_t>(nodeEnd);
		if (!nodeBegin || !nodeEnd || endAddress < beginAddress)
		{
			return {};
		}

		auto const nodeBytes = endAddress - beginAddress;
		if ((nodeBytes % sizeof(void*)) != 0)
		{
			return {};
		}

		auto const nodeCount = nodeBytes / sizeof(void*);
		if (nodeCount > 4096)
		{
			return {};
		}

		InspectedEffectGraph inspected;
		inspected.nodes.reserve(nodeCount);
		for (auto** current = nodeBegin; current != nodeEnd; ++current)
		{
			auto* node = *current;
			if (!node)
				throw hresult_invalid_argument(L"A flattened effect node is null.");
			inspected.nodes.push_back({ node, *reinterpret_cast<void**>(node) });
		}
		return inspected;
	}

	std::vector<CustomNode> FindCustomNodes(InspectedEffectGraph const& graph)
	{
		std::lock_guard<std::mutex> guard(g_registryMutex);
		std::vector<CustomNode> result;
		for (uint32_t index = 0; index < graph.nodes.size(); ++index)
		{
			if (auto* entry = FindEntryByEffectTypeLocked(graph.nodes[index].effectType))
				result.push_back({ index, std::numeric_limits<uint32_t>::max(), entry });
		}
		return result;
	}

	CompiledResult* CompileCustomGraph(
		void* description,
		InspectedEffectGraph const& graph,
		std::vector<CustomNode> customNodes)
	{
		void* compositeType{};
		for (auto const& node : graph.nodes)
		{
			using GetGuid = GUID const* (HLSL_MEMBER*)(void*);
			auto vtable = *static_cast<void***>(node.effectType);
			if (*reinterpret_cast<GetGuid>(vtable[1])(node.effectType) == CLSID_D2D1Composite)
				compositeType = node.effectType;
		}
		check_pointer(compositeType);

		// FlattenedEffectGraph owns a vector of EffectSubgraph pointers at +0x18.
		// Each EffectSubgraph starts with its vector of graph-global node indices.
		auto base = static_cast<uint8_t*>(description) - 2 * sizeof(void*);
		auto begin = *reinterpret_cast<uint8_t***>(base + 3 * sizeof(void*));
		auto end = *reinterpret_cast<uint8_t***>(base + 4 * sizeof(void*));
		if (!begin || end < begin || end - begin > 4096)
			throw hresult_invalid_argument(L"Invalid flattened subgraph range.");
		auto count = static_cast<uint32_t>(end - begin);
		for (uint32_t index = 0; index < count; ++index)
		{
			check_pointer(begin[index]);
			auto nodes = *reinterpret_cast<uint32_t**>(begin[index]);
			auto nodesEnd = *reinterpret_cast<uint32_t**>(begin[index] + sizeof(void*));
			if (!nodes || nodesEnd < nodes || static_cast<size_t>(nodesEnd - nodes) > graph.nodes.size())
				throw hresult_invalid_argument(L"Invalid flattened node range.");
			for (auto current = nodes; current != nodesEnd; ++current)
				if (*current >= graph.nodes.size())
					throw hresult_invalid_argument(L"A flattened subgraph refers to an invalid node.");
			for (auto& custom : customNodes)
			{
				if (std::find(nodes, nodesEnd, custom.nodeIndex) == nodesEnd) continue;
				if (nodesEnd - nodes != 1 || custom.subgraphIndex != std::numeric_limits<uint32_t>::max())
					throw hresult_not_implemented(L"Each custom shader must occupy one isolated compiled subgraph.");
				custom.subgraphIndex = index;
			}
		}
		for (auto const& custom : customNodes)
			if (custom.subgraphIndex == std::numeric_limits<uint32_t>::max())
				throw hresult_invalid_argument(L"A custom effect node has no compiled subgraph.");

		CompiledResult* native{};
		CompiledResult* merged{};
		try
		{
			struct CodegenScope
			{
				void* previous = g_nativePassthroughType;
				~CodegenScope()
				{
					g_nativePassthroughType = previous;
				}
			} scope;
			g_nativePassthroughType = compositeType;
			check_hresult(g_originalCompileEffectDescription(description, reinterpret_cast<void**>(&native)));
			if (!native || !native->subgraphBegin || native->subgraphEnd < native->subgraphBegin ||
				static_cast<size_t>(native->subgraphEnd - native->subgraphBegin) != count)
				throw hresult_invalid_argument(L"Native compiled subgraphs do not match the flattened graph.");
			merged = static_cast<CompiledResult*>(AllocateBytes(sizeof(CompiledResult)));
			merged->vtable = g_wrapperVtable;
			merged->refCount = 1;
			merged->nativeBacking = native;
			native = nullptr;
			merged->subgraphBegin = static_cast<CompiledSubgraph*>(AllocateBytes(sizeof(CompiledSubgraph) * count));
			merged->subgraphEnd = merged->subgraphBegin + count;
			merged->subgraphCapacity = merged->subgraphEnd;
			memcpy(merged->subgraphBegin, merged->nativeBacking->subgraphBegin, sizeof(CompiledSubgraph) * count);
			merged->customBodyBegin = static_cast<CompiledResult::CustomBody*>(
				AllocateBytes(sizeof(CompiledResult::CustomBody) * customNodes.size()));
			merged->customBodyEnd = merged->customBodyBegin + customNodes.size();
			merged->ownedSubgraphs = static_cast<uint8_t*>(AllocateBytes(count));

			for (size_t customIndex = 0; customIndex < customNodes.size(); ++customIndex)
			{
				auto const custom = customNodes[customIndex];
				auto shader = static_cast<CompiledResult*>(CreateCompiledResult(custom.entry, custom.nodeIndex));
				auto const shaderIndex = GetMainSubgraphIndex(*custom.entry->definition);
				auto& target = merged->subgraphBegin[custom.subgraphIndex];
				auto const& source = merged->nativeBacking->subgraphBegin[custom.subgraphIndex];
				target = shader->subgraphBegin[shaderIndex];
				shader->subgraphBegin[shaderIndex] = {};
				merged->ownedSubgraphs[custom.subgraphIndex] = 1;
				merged->customBodyBegin[customIndex] = { custom.subgraphIndex, custom.entry };

				if (!source.inputBindingBegin || !source.inputBindingEnd ||
					source.inputBindingEnd < source.inputBindingBegin)
					throw hresult_invalid_argument(L"Native custom shader inputs are malformed.");
				auto const inputCount = static_cast<size_t>(
					static_cast<InputBinding*>(source.inputBindingEnd) -
					static_cast<InputBinding*>(source.inputBindingBegin));
				if (inputCount != custom.entry->definition->sourceCount)
					throw hresult_invalid_argument(L"Native and custom shader input counts differ.");
				memcpy(target.inputBindingBegin, source.inputBindingBegin, inputCount * sizeof(InputBinding));

				auto modes = static_cast<SurfaceData const*>(source.surfaceDataBegin);
				auto targetData = static_cast<SurfaceData*>(target.surfaceDataBegin);
				auto modeCount = modes && source.surfaceDataEnd >= source.surfaceDataBegin
					? static_cast<size_t>(static_cast<SurfaceData const*>(source.surfaceDataEnd) - modes)
					: 0;
				for (size_t input = 0; targetData && input < inputCount; ++input)
				{
					if (input < modeCount)
					{
						targetData[input].data[0] = modes[input].data[0];
						targetData[input].data[1] = modes[input].data[1];
					}
				}

				// Add back or you'll keep meeting the onecoreuap\windows\dwm\dwmcore\rendering\brushrenderingeffect.cpp(185)\dwmcorei.dll!00007FFD49D74323: (caller: 00007FFD49C895A4) ReturnHr(4088) tid(5c4c) 80004005 Unspecified error
				// A custom pass is materialized. This gives later custom or native nodes a real surface and avoids cross-profile fragment linking.
				target.flags = 0;

				// AI piece of shit. WRONG!
				// Preserve the shader template's output/materialization policy. A
				// MaterializedTexture template marks its custom sampler as a fragment
				// output so geometry is evaluated at destination resolution instead of
				// inside an upstream effect's prescaled intermediate. LinkedColor
				// templates already carry flags == 0 and remain materialized as before.
				DestroyCompiledResult(shader);
			}
			return merged;
		}
		catch (...)
		{
			if (merged) DestroyCompiledResult(merged);
			if (native)
			{
				using Release = ULONG(__stdcall*)(CompiledResult*);
				reinterpret_cast<Release>(native->vtable[1])(native);
			}
			throw;
		}
	}
	HRESULT __stdcall DetourCompileEffectDescription(void* description, void** result)
	{
		// This detour is reached on DWM's effect compilation worker path after
		// wuceffectsi has already traversed and flattened the public IGraphicsEffect
		// graph. Replacing only the compile result keeps traversal, named inputs, and
		// animatable property enumeration on the normal WinUI path.
		if (!result)
		{
			return E_POINTER;
		}

		try
		{
			auto const inspected = InspectEffectGraph(description);
			auto customNodes = FindCustomNodes(inspected);
			if (!customNodes.empty())
			{
				*result = CompileCustomGraph(description, inspected, std::move(customNodes));
				return S_OK;
			}
		}
		catch (...)
		{
			*result = nullptr; return to_hresult();
		}
		return g_originalCompileEffectDescription(description, result);
	}

	bool IsTargetImport(char const* dllName)
	{
		// Restrict import patching to wuceffectsi.dll. The symbol name alone is not a
		// safe discriminator because other modules can export unrelated functions with
		// the same name or keep helper thunks in delay-load tables.
		return dllName && _stricmp(dllName, "wuceffectsi.dll") == 0;
	}

	void PatchSlot(void** slot, void* replacement)
	{
		// IAT/delay-IAT sections are normally read-only. Patch one pointer at a time
		// and restore the original protection immediately to reduce the blast radius
		// if another module maps the same page as executable/read-only import data.
		DWORD oldProtect{};
		if (VirtualProtect(slot, sizeof(void*), PAGE_READWRITE, &oldProtect))
		{
			*slot = replacement;
			FlushInstructionCache(GetCurrentProcess(), slot, sizeof(void*));
			DWORD unused{};
			VirtualProtect(slot, sizeof(void*), oldProtect, &unused);
		}
	}

	void PatchImport(HMODULE module, ImportPatch const* patches, size_t patchCount)
	{
		// Patch already-resolved imports. This covers modules that linked
		// wuceffectsi.dll normally and whose FirstThunk entries already contain the
		// resolved CompileEffectDescription address.
		auto* base = reinterpret_cast<uint8_t*>(module);
		auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
		if (dos->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return;
		}

		auto* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
		if (nt->Signature != IMAGE_NT_SIGNATURE)
		{
			return;
		}

		auto const& imports = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		if (!imports.VirtualAddress || !imports.Size)
		{
			return;
		}

		auto* descriptor = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base + imports.VirtualAddress);
		for (; descriptor->Name; ++descriptor)
		{
			if (!IsTargetImport(reinterpret_cast<char const*>(base + descriptor->Name)))
			{
				continue;
			}

			auto* thunk = reinterpret_cast<IMAGE_THUNK_DATA*>(base + descriptor->FirstThunk);
			for (; thunk->u1.Function; ++thunk)
			{
				auto** slot = reinterpret_cast<void**>(&thunk->u1.Function);
				for (size_t index = 0; index < patchCount; ++index)
				{
					if (*slot == patches[index].original)
					{
						PatchSlot(slot, patches[index].replacement);
						break;
					}
				}
			}
		}
	}

	bool IsImportByOrdinal(IMAGE_THUNK_DATA const& thunk)
	{
		// Delay import name thunks can encode ordinals. Those entries have no
		// IMAGE_IMPORT_BY_NAME payload, so trying to read Name would treat an ordinal
		// value as an RVA and walk invalid memory.
#ifdef _WIN64
		return IMAGE_SNAP_BY_ORDINAL64(thunk.u1.Ordinal);
#else
		return IMAGE_SNAP_BY_ORDINAL32(thunk.u1.Ordinal);
#endif
	}

	void PatchDelayImport(HMODULE module, ImportPatch const* patches, size_t patchCount)
	{
		// Patch delay-load imports before first use. dcompi.dll reaches
		// CompileEffectDescription through a delay import table, so scanning only the
		// normal import directory makes the detour appear installed while calls still
		// go to the original export.
		auto* base = reinterpret_cast<uint8_t*>(module);
		auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
		if (dos->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return;
		}

		auto* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
		if (nt->Signature != IMAGE_NT_SIGNATURE)
		{
			return;
		}

		auto const& delayImports = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT];
		if (!delayImports.VirtualAddress || !delayImports.Size)
		{
			return;
		}

		auto* descriptor = reinterpret_cast<IMAGE_DELAYLOAD_DESCRIPTOR*>(base + delayImports.VirtualAddress);
		for (; descriptor->DllNameRVA; ++descriptor)
		{
			if (!descriptor->Attributes.RvaBased)
			{
				continue;
			}

			if (!IsTargetImport(reinterpret_cast<char const*>(base + descriptor->DllNameRVA)))
			{
				continue;
			}

			// dcompi.dll delay-loads wuceffectsi.dll, so its calls are routed through the
			// .didat delay IAT instead of the normal import directory. Matching by import
			// name here is intentional: before delay resolution the slot is a helper thunk,
			// not GetProcAddress(wuceffectsi, name), so address comparison never reaches
			// VirtualProtect.
			auto* nameThunk = reinterpret_cast<IMAGE_THUNK_DATA*>(base + descriptor->ImportNameTableRVA);
			auto* addressThunk = reinterpret_cast<IMAGE_THUNK_DATA*>(base + descriptor->ImportAddressTableRVA);
			for (; nameThunk->u1.AddressOfData && addressThunk->u1.Function; ++nameThunk, ++addressThunk)
			{
				if (IsImportByOrdinal(*nameThunk))
				{
					continue;
				}

				auto const* importByName = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base + nameThunk->u1.AddressOfData);
				auto const* importName = reinterpret_cast<char const*>(importByName->Name);
				for (size_t index = 0; index < patchCount; ++index)
				{
					if (strcmp(importName, patches[index].name) == 0)
					{
						auto** slot = reinterpret_cast<void**>(&addressThunk->u1.Function);
						PatchSlot(slot, patches[index].replacement);
						break;
					}
				}
			}
		}
	}

	void InstallHook()
	{
		// Hook installation is process-wide and must be idempotent. It is triggered
		// lazily by CreateEffect so callers can keep using a normal WinUI shape:
		// create an effect description and pass it to CreateEffectFactory.
		std::call_once(g_hookOnce, []
					   {
						   LoadLibraryW(L"dwmcorei.dll");
						   auto module = LoadLibraryW(L"wuceffectsi.dll");
						   check_pointer(module);

						   auto original = reinterpret_cast<CompileEffectDescriptionFn>(GetProcAddress(module, "CompileEffectDescription"));
						   check_pointer(original);

						   // false check temp
						   // HlslComposition::Runtime240::Validate(module);
						   auto const resolved = HlslComposition::RuntimeImage(module).Resolve();
						   kEffectTypeFromGuidRva = resolved.fromGuid;
						   kEffectTypeTableRva = resolved.table;
						   kEffectTypeGetBoundsRva = resolved.getBounds;
						   kEffectTypeCalcInputBoundsRva = resolved.calcInputBounds;
						   kDirectPropertyUpdaterFunctionVtableRva = resolved.updater;
						   kEffectTypeCount = resolved.effectCount;
						   g_originalCompileEffectDescription = original;
						   g_wuceffectsiModule = module;

						   HMODULE owningModule{};
						   check_bool(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
														 reinterpret_cast<LPCWSTR>(&InstallHook), &owningModule));
						   InitializeAllEffectTypes(module);
						   PatchEffectTypeFromGuid(module);

						   ImportPatch const patches[] = {
							   {
								   "CompileEffectDescription",
								   reinterpret_cast<void*>(original),
								   reinterpret_cast<void*>(DetourCompileEffectDescription),
							   },
						   };

						   for (auto name : { L"dcompi.dll",L"dwmcorei.dll" })
						   {
							   auto caller = GetModuleHandleW(name);
							   check_pointer(caller);
							   PatchImport(caller, patches, ARRAYSIZE(patches));
							   PatchDelayImport(caller, patches, ARRAYSIZE(patches));
						   }
					   });
	}

	struct RuntimeGraphicsEffect :
		winrt::implements<
		RuntimeGraphicsEffect,
		IGraphicsEffect,
		IGraphicsEffectSource,
		ABI::Windows::Graphics::Effects::IGraphicsEffectD2D1Interop>
	{
		// This is the public WinRT-facing object. WinUI and wuceffectsi initially
		// see only standard IGraphicsEffectD2D1Interop methods: effect GUID,
		// property count/defaults, and source list. The private runtime state is
		// deliberately not exposed here; the detours recover it later from the GUID
		// and synthetic EffectType pointer.
		explicit RuntimeGraphicsEffect(
			CustomEffectRuntime::CustomEffectDefinition const* definition,
			std::span<IGraphicsEffectSource const> sources = {}) :
			m_definition(definition),
			m_name(definition->effectName)
		{
			m_sources.reserve(definition->sourceCount);
			if (!sources.empty())
			{
				if (sources.size() != definition->sourceCount)
					throw hresult_invalid_argument(L"The explicit source count does not match the shader definition.");
				for (auto const& source : sources)
				{
					if (!source) throw hresult_invalid_argument(L"A custom effect source is null.");
					m_sources.push_back(source);
				}
				return;
			}
			for (uint32_t index = 0; index < definition->sourceCount; ++index)
			{
				auto const& sourceDefinition = definition->sources[index];
				// wuceffectsi!Traverser::FindSourceFlatteningEffect matches source
				// objects by raw COM pointer identity. Returning a freshly constructed
				// CompositionEffectSourceParameter from each GetSource call makes the
				// pre-enumeration flatten wrapper undiscoverable in VisitEffectInputs.
				m_sources.push_back(CompositionEffectSourceParameter(sourceDefinition.name)
									.as<IGraphicsEffectSource>());
			}
		}

		hstring Name() const
		{
			// IGraphicsEffect::Name is still used by WinUI for property paths and
			// diagnostics even though the native compile path is redirected later.
			return m_name;
		}

		void Name(hstring const& value)
		{
			// Keep the public WinRT behavior normal: callers can rename the effect
			// instance before passing it to CreateEffectFactory, and property paths
			// should reflect that name.
			m_name = value;
		}

		HRESULT __stdcall GetEffectId(GUID* id) noexcept final
		{
			if (!id)
			{
				return E_POINTER;
			}

			// Unknown GUIDs are rejected by wuceffectsi!EffectType::FromGuid before the
			// compile hook runs. The runtime detour registers real EffectType objects for
			// every private GUID so custom effects do not masquerade as built-in D2D IDs.
			*id = m_definition->id;
			return S_OK;
		}

		HRESULT __stdcall GetNamedPropertyMapping(
			LPCWSTR name,
			UINT* index,
			ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING* mapping) noexcept final
		{
			// This mapping is consumed by Compositor::CreateEffectFactory when it
			// validates animatable property paths such as EffectName.PropertyName.
			// The returned index must match both NativePropertyMetadata and
			// ConstantBufferPropertyMapping because the compile detour later builds
			// native constant-buffer updater records from the same index.
			if (!name || !index || !mapping)
			{
				return E_POINTER;
			}

			for (uint32_t propertyIndex = 0; propertyIndex < m_definition->propertyCount; ++propertyIndex)
			{
				auto const& property = m_definition->properties[propertyIndex];
				if (property.publicName && wcscmp(name, property.publicName) == 0)
				{
					*index = property.index;
					*mapping = property.mapping;
					return S_OK;
				}
			}

			return E_INVALIDARG;
		}

		HRESULT __stdcall GetPropertyCount(UINT* count) noexcept final
		{
			if (!count)
			{
				return E_POINTER;
			}

			// This count is the public WinRT property count. It may be smaller than
			// the native metadata table if future effects add internal-only fields,
			// so callers must use the explicit mappings rather than assuming indexes
			// are interchangeable by accident.
			*count = m_definition->propertyCount;
			return S_OK;
		}

		HRESULT __stdcall GetProperty(UINT index, ABI::Windows::Foundation::IPropertyValue** value) noexcept final
		{
			// Default property values are still requested through the public
			// IGraphicsEffectD2D1Interop API during traversal. Native metadata only
			// handles the later DWM-side constant-buffer update path.
			if (!value)
			{
				return E_POINTER;
			}

			*value = nullptr;
			if (index >= m_definition->propertyCount)
			{
				return E_INVALIDARG;
			}

			auto const& property = m_definition->properties[index];

			if (property.getDefaultValue) return property.getDefaultValue(value);
			try
			{
				auto metadata = static_cast<CustomEffectRuntime::NativePropertyMetadata const*>(
					m_definition->nativePropertyMetadata);
				if (!metadata || property.index >= m_definition->nativePropertyMetadataCount ||
					!m_definition->constantBufferInitialValue)
					return E_INVALIDARG;
				auto const& native = metadata[property.index];
				auto values = static_cast<float const*>(m_definition->constantBufferInitialValue) +
					native.propertyOffset / sizeof(float);
				Windows::Foundation::IPropertyValue initial{ nullptr };
				if (native.valueCount == 1)
					initial = Windows::Foundation::PropertyValue::CreateSingle(values[0]).as<Windows::Foundation::IPropertyValue>();
				else
					initial = Windows::Foundation::PropertyValue::CreateSingleArray(
						array_view<float const>{ values, values + native.valueCount }).as<Windows::Foundation::IPropertyValue>();
				*value = reinterpret_cast<ABI::Windows::Foundation::IPropertyValue*>(detach_abi(initial));
				return S_OK;
			}
			catch (...)
			{
				return to_hresult();
			}
		}

		HRESULT __stdcall GetSource(
			UINT index,
			ABI::Windows::Graphics::Effects::IGraphicsEffectSource** source) noexcept final
		{
			// Return stable source COM identities. Source flattening relies on
			// pointer identity during EnumerateEffectSubgraphs/VisitEffectInputs;
			// creating a new CompositionEffectSourceParameter per call would make
			// the precomputed flatten wrapper impossible to find.
			if (!source)
			{
				return E_POINTER;
			}

			*source = nullptr;
			if (index >= m_definition->sourceCount)
			{
				return E_INVALIDARG;
			}

			try
			{
				void* abi{};
				copy_to_abi(m_sources[index], abi);
				*source = static_cast<ABI::Windows::Graphics::Effects::IGraphicsEffectSource*>(abi);
				return S_OK;
			}
			catch (...)
			{
				return to_hresult();
			}
		}

		HRESULT __stdcall GetSourceCount(UINT* count) noexcept final
		{
			if (!count)
			{
				return E_POINTER;
			}

			// Traverser enumerates this count before calling GetSource. It must match
			// the source descriptors used by InitializeSubgraphInputs, otherwise the
			// public graph and synthetic compiled graph describe different edges.
			*count = m_definition->sourceCount;
			return S_OK;
		}

	private:
		CustomEffectRuntime::CustomEffectDefinition const* m_definition{};
		hstring m_name;
		std::vector<IGraphicsEffectSource> m_sources;
	};
}

namespace CustomEffectRuntime
{
	void RegisterEffect(CustomEffectDefinition const& definition)
	{
		// RegisterEffect is intentionally cheap and idempotent. Multiple calls can
		// happen if the app creates the same effect description for several brushes;
		// all of them should share the same synthetic EffectType and shader blob.
		std::lock_guard<std::mutex> guard(g_registryMutex);
		if (auto* existing = FindEntryByGuidLocked(definition.id))
		{
			OwnedEffectDefinition candidate{ definition };
			if (!existing->owned.Equivalent(candidate)) throw hresult_invalid_argument(L"Effect GUID is already registered with a different shader definition.");
			return;
		}
		std::size_t registrations = 0;
		for (auto* current = g_effects; current; current = current->next)++registrations;
		if (registrations >= 1024) throw hresult_error(E_OUTOFMEMORY, L"The process has reached the limit of 1024 distinct HLSL definitions. Reuse deterministic descriptors.");

		auto* entry = new RuntimeEffectEntry(definition);
		if (g_wuceffectsiModule)
		{
			InitializeEffectType(entry, g_wuceffectsiModule);
		}

		entry->next = g_effects;
		g_effects = entry;
	}

	IGraphicsEffect CreateEffect(CustomEffectDefinition const& definition)
	{
		return CreateEffect(definition, std::span<IGraphicsEffectSource const>{});
	}

	IGraphicsEffect CreateEffect(
		CustomEffectDefinition const& definition,
		IGraphicsEffectSource const& source)
	{
		if (!source) return CreateEffect(definition);
		return CreateEffect(definition, std::span<IGraphicsEffectSource const>{ &source, 1 });
	}

	IGraphicsEffect CreateEffect(
		CustomEffectDefinition const& definition,
		std::span<IGraphicsEffectSource const> sources)
	{
		// The public shape must be the same shape WinUI expects from built-in effects:
		// an IGraphicsEffect that can be passed directly to Compositor::CreateEffectFactory.
		// Registration and hook installation live here only to make that object usable
		// before wuceffectsi resolves its private EffectType GUID.
		//
		// Callers should not need to know about the native compiled-result object.
		// That separation is what keeps effect definitions code-only: each effect
		// supplies metadata and HLSL source, while this runtime owns the fragile
		// build-specific ABI adaptation.
		RegisterEffect(definition);
		RuntimeEffectEntry* entry{};
		{
			std::lock_guard<std::mutex> guard(g_registryMutex); entry = FindEntryByGuidLocked(definition.id);
		}
		EnsureShader(entry);
		InstallHook();
		return make<RuntimeGraphicsEffect>(entry->definition, sources);
	}

}






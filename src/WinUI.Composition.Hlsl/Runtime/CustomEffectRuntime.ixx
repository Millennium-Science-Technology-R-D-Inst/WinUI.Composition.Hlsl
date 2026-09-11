module;
#include <unknwn.h>
#include <Windows.h>
#include <guiddef.h>
#include <windows.graphics.effects.interop.h>

export module WinUI.Composition.Hlsl.CustomEffectRuntime;

import std;
import winrt.Windows.Graphics.Effects;

export namespace CustomEffectRuntime
{
	enum class SourceKind
	{
		Backdrop
	};
	struct SourceDescriptor
	{
		wchar_t const* name; SourceKind kind; bool requiresSamplerData; bool requiresSamplerDataExt;
	};
	struct PropertyDescriptor
	{
		wchar_t const* publicName; std::uint32_t index; ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING mapping; HRESULT(*getDefaultValue)(ABI::Windows::Foundation::IPropertyValue** value); float initialScalar{};
	};
	struct NativePropertyMetadata
	{
		char const* shaderName; std::uint32_t propertyOffset; std::uint32_t expressionType; std::uint32_t propertyType; std::uint32_t valueCount; void* validator;
	};
	struct ConstantBufferPropertyMapping
	{
		std::uint32_t propertyIndex; std::uint32_t constantBufferOffset;
	};
	constexpr std::uint8_t kShaderProfileLevel91 = 0;
	constexpr std::uint8_t kShaderProfileLevel93 = 1;
	constexpr std::uint8_t kShaderProfilePs40 = 2;
	struct CustomEffectDefinition
	{
		GUID id; wchar_t const* effectName; char const* fragmentName;
		char const* shaderSource; size_t shaderSourceSize;
		void const* shaderBytecode; size_t shaderBytecodeSize; char const* shaderFunctionName;
		SourceDescriptor const* sources; std::uint32_t sourceCount;
		PropertyDescriptor const* properties; std::uint32_t propertyCount;
		void const* nativePropertyMetadata; std::uint32_t nativePropertyMetadataCount; std::uint32_t propertiesStructSize;
		ConstantBufferPropertyMapping const* constantBufferProperties; std::uint32_t constantBufferPropertyCount;
		std::uint16_t const* shaderArguments; std::uint64_t shaderArgumentCount; std::uint16_t linkingArgType; std::uint8_t shaderProfileVersion;
		std::uint32_t constantBufferSize; void const* constantBufferInitialValue;
		bool flattenSourceBeforeCustomSampler; char const* flattenShaderFunctionName; char const* descriptorKey{};
	};
	void RegisterEffect(CustomEffectDefinition const& definition);
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect(CustomEffectDefinition const& definition);
}

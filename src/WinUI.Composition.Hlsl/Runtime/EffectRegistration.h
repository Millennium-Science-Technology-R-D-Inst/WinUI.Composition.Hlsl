#pragma once
#include "CustomEffectRuntime.h"
#include <string>
#include <vector>
#include <cstring>

// The native registry owns every byte to which the synthetic EffectType points.
struct OwnedEffectDefinition
{
	CustomEffectRuntime::CustomEffectDefinition value{};
	std::wstring effectName;
	std::string fragmentName, shaderSource, shaderFunctionName, flattenName, descriptorKey;
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
		if (!input.effectName || !input.fragmentName || !input.shaderSource || !input.shaderFunctionName ||
			input.sourceCount != 1 || !input.sources || input.propertyCount > 64 ||
			(input.propertyCount && !input.properties) ||
			(input.nativePropertyMetadataCount && !input.nativePropertyMetadata) ||
			(input.shaderArgumentCount && !input.shaderArguments) ||
			(input.constantBufferPropertyCount && !input.constantBufferProperties) ||
			(input.constantBufferSize && !input.constantBufferInitialValue))
			throw winrt::hresult_invalid_argument(L"Invalid native effect definition.");
		effectName=input.effectName; fragmentName=input.fragmentName;
		shaderSource.assign(input.shaderSource, input.shaderSourceSize); shaderFunctionName=input.shaderFunctionName;
		flattenName=input.flattenShaderFunctionName ? input.flattenShaderFunctionName : "";
		descriptorKey=input.descriptorKey ? input.descriptorKey : ""; value.descriptorKey=descriptorKey.c_str();
		value.effectName=effectName.c_str(); value.fragmentName=fragmentName.c_str();
		value.shaderSource=shaderSource.data(); value.shaderFunctionName=shaderFunctionName.c_str();
		value.flattenShaderFunctionName=flattenName.empty() ? nullptr : flattenName.c_str();
		sources.assign(input.sources, input.sources + input.sourceCount);
		sourceNames.resize(sources.size());
		for (size_t i=0; i < sources.size(); ++i)
		{
			sourceNames[i]=sources[i].name; sources[i].name=sourceNames[i].c_str();
		}
		if (input.propertyCount)properties.assign(input.properties, input.properties + input.propertyCount);
		propertyNames.resize(properties.size());
		for (size_t i=0; i < properties.size(); ++i)
		{
			propertyNames[i]=properties[i].publicName; properties[i].publicName=propertyNames[i].c_str();
			if (properties[i].getDefaultValue)
			{
				winrt::Windows::Foundation::IPropertyValue initial{ nullptr };
				winrt::check_hresult(properties[i].getDefaultValue(reinterpret_cast<ABI::Windows::Foundation::IPropertyValue**>(winrt::put_abi(initial))));
				properties[i].initialScalar=initial.GetSingle(); properties[i].getDefaultValue=nullptr;
			}
		}
		if (input.nativePropertyMetadataCount)
		{
			auto begin=static_cast<CustomEffectRuntime::NativePropertyMetadata const*>(input.nativePropertyMetadata);
			metadata.assign(begin, begin + input.nativePropertyMetadataCount);
		}
		shaderNames.resize(metadata.size());
		for (size_t i=0; i < metadata.size(); ++i)
		{
			shaderNames[i]=metadata[i].shaderName; metadata[i].shaderName=shaderNames[i].c_str();
		}
		if (input.constantBufferPropertyCount)mappings.assign(input.constantBufferProperties, input.constantBufferProperties + input.constantBufferPropertyCount);
		if (input.shaderArgumentCount)arguments.assign(input.shaderArguments, input.shaderArguments + input.shaderArgumentCount);
		if (input.constantBufferSize)
		{
			auto begin=static_cast<unsigned char const*>(input.constantBufferInitialValue);
			constants.assign(begin, begin + input.constantBufferSize);
		}
		value.sources=sources.data(); value.properties=properties.data(); value.nativePropertyMetadata=metadata.data();
		value.constantBufferProperties=mappings.data(); value.shaderArguments=arguments.data();
		value.constantBufferInitialValue=constants.empty() ? nullptr : constants.data();
	}

	bool Equivalent(OwnedEffectDefinition const& other) const
	{
		auto const& b=other.value;
		if (descriptorKey != other.descriptorKey || effectName != other.effectName || fragmentName != other.fragmentName || shaderSource != other.shaderSource ||
			shaderFunctionName != other.shaderFunctionName || flattenName != other.flattenName ||
			sourceNames != other.sourceNames || propertyNames != other.propertyNames || shaderNames != other.shaderNames ||
			arguments != other.arguments || constants != other.constants ||
			value.linkingArgType != b.linkingArgType || value.shaderProfileVersion != b.shaderProfileVersion ||
			value.propertiesStructSize != b.propertiesStructSize ||
			value.flattenSourceBeforeCustomSampler != b.flattenSourceBeforeCustomSampler ||
			metadata.size() != other.metadata.size() || mappings.size() != other.mappings.size())return false;
		for (size_t i=0; i < sources.size(); ++i)
			if (sources[i].kind != other.sources[i].kind ||
				sources[i].requiresSamplerData != other.sources[i].requiresSamplerData ||
				sources[i].requiresSamplerDataExt != other.sources[i].requiresSamplerDataExt)return false;
		for (size_t i=0; i < properties.size(); ++i)
			if (properties[i].index != other.properties[i].index || properties[i].mapping != other.properties[i].mapping ||
				properties[i].initialScalar != other.properties[i].initialScalar)return false;
		for (size_t i=0; i < metadata.size(); ++i)
			if (metadata[i].propertyOffset != other.metadata[i].propertyOffset ||
				metadata[i].expressionType != other.metadata[i].expressionType ||
				metadata[i].propertyType != other.metadata[i].propertyType ||
				metadata[i].valueCount != other.metadata[i].valueCount ||
				metadata[i].validator != other.metadata[i].validator)return false;
		for (size_t i=0; i < mappings.size(); ++i)
			if (mappings[i].propertyIndex != other.mappings[i].propertyIndex ||
				mappings[i].constantBufferOffset != other.mappings[i].constantBufferOffset)return false;
		return true;
	}
};

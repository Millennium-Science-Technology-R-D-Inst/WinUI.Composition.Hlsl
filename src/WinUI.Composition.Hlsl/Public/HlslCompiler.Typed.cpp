#include "HlslCompiler.h"
#include "HlslProperty.h"
#include "HlslShaderLibrary.h"

import WinUI.Composition.Hlsl.EffectDef;
import WinUI.Composition.Hlsl.TypedPropertyAbi;

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	namespace
	{
		std::vector<hlsl::engine::Property> CopyTypedProperties(
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslProperty> const& properties)
		{
			std::vector<hlsl::engine::Property> result;
			if (!properties) return result;
			if (properties.Size() > 64)
				throw hresult_invalid_argument(L"At most 64 HLSL properties are supported.");

			result.reserve(properties.Size());
			for (auto const& projected : properties)
			{
				if (!projected) throw hresult_invalid_argument(L"A property descriptor is null.");
				auto property = get_self<HlslProperty>(projected);
				result.emplace_back(
					std::wstring(property->Name()),
					static_cast<hlsl::engine::PropertyType>(property->Type()),
					property->Values());
			}

			// Validate names/component counts/finiteness with the same rules used by
			// effect construction before source is handed to FXC.
			hlsl::engine::EffectDefinition definition;
			definition.shader = "typed-property-validation";
			definition.properties = result;
			hlsl::engine::Validate(definition);
			return result;
		}

		hstring DecorateTypedShader(
			hstring const& shader,
			std::span<hlsl::engine::Property const> properties)
		{
			auto source = hlsl::propertyabi::BuildDeclarations(properties);
			source += to_string(shader);
			return to_hstring(source);
		}

		void ValidateTypedResult(
			Hlsl::HlslShaderLibrary const& library,
			std::span<hlsl::engine::Property const> properties)
		{
			if (!library) throw hresult_invalid_argument(L"The compiled shader library is null.");
			auto implementation = get_self<HlslShaderLibrary>(library);
			hlsl::propertyabi::ValidateLibrary(implementation->BytecodeBytes(), properties);
		}

		Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> CompileTypedAsync(
			hstring const& shader,
			Hlsl::HlslEffectKind kind,
			Hlsl::HlslShaderProfile profile,
			std::uint32_t sourceCount,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslProperty> const& properties,
			Windows::Foundation::Collections::IVectorView<hstring> const& defines)
		{
			auto schema = CopyTypedProperties(properties);
			auto decorated = DecorateTypedShader(shader, schema);
			Hlsl::HlslShaderLibrary library{ nullptr };
			if (defines)
			{
				library = co_await HlslCompiler::CompileAdvancedWithDefinesAsync(
					decorated, kind, profile, sourceCount, defines);
			}
			else
			{
				library = co_await HlslCompiler::CompileAdvancedAsync(
					decorated, kind, profile, sourceCount);
			}
			ValidateTypedResult(library, schema);
			co_return library;
		}
	}

	Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> HlslCompiler::CompileWithTypedPropertiesAsync(
		hstring const& shader,
		Hlsl::HlslEffectKind kind,
		Hlsl::HlslShaderProfile profile,
		Windows::Foundation::Collections::IVectorView<Hlsl::HlslProperty> const& properties)
	{
		return CompileTypedAsync(shader, kind, profile, 1, properties, nullptr);
	}

	Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> HlslCompiler::CompileWithTypedPropertiesAndDefinesAsync(
		hstring const& shader,
		Hlsl::HlslEffectKind kind,
		Hlsl::HlslShaderProfile profile,
		Windows::Foundation::Collections::IVectorView<Hlsl::HlslProperty> const& properties,
		Windows::Foundation::Collections::IVectorView<hstring> const& defines)
	{
		return CompileTypedAsync(shader, kind, profile, 1, properties, defines);
	}

	Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> HlslCompiler::CompileAdvancedWithTypedPropertiesAsync(
		hstring const& shader,
		Hlsl::HlslEffectKind kind,
		Hlsl::HlslShaderProfile profile,
		std::uint32_t sourceCount,
		Windows::Foundation::Collections::IVectorView<Hlsl::HlslProperty> const& properties)
	{
		return CompileTypedAsync(shader, kind, profile, sourceCount, properties, nullptr);
	}

	Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> HlslCompiler::CompileAdvancedWithTypedPropertiesAndDefinesAsync(
		hstring const& shader,
		Hlsl::HlslEffectKind kind,
		Hlsl::HlslShaderProfile profile,
		std::uint32_t sourceCount,
		Windows::Foundation::Collections::IVectorView<Hlsl::HlslProperty> const& properties,
		Windows::Foundation::Collections::IVectorView<hstring> const& defines)
	{
		return CompileTypedAsync(shader, kind, profile, sourceCount, properties, defines);
	}
}

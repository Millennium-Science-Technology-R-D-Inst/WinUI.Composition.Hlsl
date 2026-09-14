module;
#include <d3dcompiler.h>
#include <d3d11shader.h>

export module WinUI.Composition.Hlsl.TypedPropertyAbi;

import std;
import winrt_base;
import WinUI.Composition.Hlsl.EffectDef;

namespace hlsl::propertyabi
{
	namespace
	{
		std::string PackOffset(std::uint32_t byteOffset)
		{
			std::string result = "packoffset(c" + std::to_string(byteOffset / 16);
			auto const component = (byteOffset % 16) / 4;
			if (component)
			{
				result += ".";
				result.push_back("xyzw"[component]);
			}
			result += ")";
			return result;
		}

		std::string MatrixBackingName(std::size_t propertyIndex, std::size_t chunk)
		{
			return "__wuchlsl_property_" + std::to_string(propertyIndex) + "_" + std::to_string(chunk);
		}

		std::size_t MatrixChunkCount(hlsl::engine::PropertyAbiSpec const& spec)
		{
			return (spec.valueCount + 3u) / 4u;
		}

		std::uint32_t MatrixChunkWidth(hlsl::engine::PropertyAbiSpec const& spec, std::size_t chunk)
		{
			auto const consumed = static_cast<std::uint32_t>(chunk) * 4u;
			return std::min(4u, spec.valueCount - consumed);
		}

		std::string FloatType(std::uint32_t width)
		{
			if (width == 1) return "float";
			return "float" + std::to_string(width);
		}

		std::string ComponentExpression(std::string const& variable, std::uint32_t component)
		{
			return variable + "." + std::string(1, "xyzw"[component]);
		}
	}

	export std::string BuildDeclarations(std::span<hlsl::engine::Property const> properties)
	{
		if (properties.empty()) return {};

		std::string declarations = "cbuffer UserConstants : register(b0) {\n";
		std::string aliases;
		hlsl::engine::PropertyLayoutCursor cursor{};
		for (std::size_t propertyIndex = 0; propertyIndex < properties.size(); ++propertyIndex)
		{
			auto const& property = properties[propertyIndex];
			auto const spec = hlsl::engine::GetPropertyAbiSpec(property.type);
			auto const layout = hlsl::engine::AppendPropertyLayout(cursor, property.type);
			if (!spec.matrix)
			{
				declarations += std::string(spec.hlslType) + " " + winrt::to_string(property.name) +
					" : " + PackOffset(layout.constantBufferOffset) + ";\n";
				continue;
			}

			// The native DirectPropertyUpdater copies matrix values as a contiguous
			// float array. HLSL's native matrix cbuffer layout inserts 16-byte row or
			// column strides, so expose packed float chunks and reconstruct the logical
			// matrix as a macro. This makes Matrix3x2 (24 bytes) and Matrix4x4 (64 bytes)
			// use exactly the byte representation delivered by Windows.Foundation.Numerics.
			for (std::size_t chunk = 0; chunk < MatrixChunkCount(spec); ++chunk)
			{
				auto const width = MatrixChunkWidth(spec, chunk);
				declarations += FloatType(width) + " " + MatrixBackingName(propertyIndex, chunk) +
					" : " + PackOffset(layout.constantBufferOffset + static_cast<std::uint32_t>(chunk) * 16u) + ";\n";
			}

			aliases += "#define " + winrt::to_string(property.name) + " " + std::string(spec.hlslType) + "(";
			for (std::uint32_t value = 0; value < spec.valueCount; ++value)
			{
				if (value) aliases += ",";
				auto const chunk = value / 4u;
				auto const component = value % 4u;
				aliases += ComponentExpression(MatrixBackingName(propertyIndex, chunk), component);
			}
			aliases += ")\n";
		}
		declarations += "};\n";
		declarations += aliases;
		return declarations;
	}

	namespace
	{
		winrt::com_ptr<ID3D11LibraryReflection> Reflect(std::span<std::uint8_t const> bytecode)
		{
			winrt::com_ptr<ID3D11LibraryReflection> reflection;
			winrt::check_hresult(D3DReflectLibrary(
				bytecode.data(), bytecode.size(), __uuidof(ID3D11LibraryReflection), reflection.put_void()));
			return reflection;
		}

		ID3D11FunctionReflection* FindFunction(ID3D11LibraryReflection* library, char const* name)
		{
			D3D11_LIBRARY_DESC libraryDesc{};
			winrt::check_hresult(library->GetDesc(&libraryDesc));
			for (UINT index = 0; index < libraryDesc.FunctionCount; ++index)
			{
				auto* function = library->GetFunctionByIndex(index);
				if (!function) continue;
				D3D11_FUNCTION_DESC functionDesc{};
				if (SUCCEEDED(function->GetDesc(&functionDesc)) && functionDesc.Name &&
					strcmp(functionDesc.Name, name) == 0)
				{
					return function;
				}
			}
			return nullptr;
		}

		bool MatchesScalarOrVector(D3D11_SHADER_TYPE_DESC const& reflected, std::uint32_t width)
		{
			if (reflected.Type != D3D_SVT_FLOAT || reflected.Elements > 1 || reflected.Members != 0 || reflected.Rows != 1)
				return false;
			if (width == 1)
				return reflected.Class == D3D_SVC_SCALAR && reflected.Columns == 1;
			return reflected.Class == D3D_SVC_VECTOR && reflected.Columns == width;
		}

		void ValidateVariable(
			ID3D11ShaderReflectionConstantBuffer* constants,
			std::string const& name,
			std::uint32_t expectedOffset,
			std::uint32_t expectedWidth)
		{
			auto* variable = constants->GetVariableByName(name.c_str());
			if (!variable)
				throw winrt::hresult_invalid_argument(L"The compiled UserConstants buffer is missing a typed-property backing variable.");
			D3D11_SHADER_VARIABLE_DESC variableDesc{};
			if (FAILED(variable->GetDesc(&variableDesc)) || variableDesc.StartOffset != expectedOffset ||
				variableDesc.Size != expectedWidth * sizeof(float))
			{
				throw winrt::hresult_invalid_argument(L"A compiled typed-property backing variable has the wrong offset or size.");
			}
			auto* type = variable->GetType();
			D3D11_SHADER_TYPE_DESC typeDesc{};
			if (!type || FAILED(type->GetDesc(&typeDesc)) || !MatchesScalarOrVector(typeDesc, expectedWidth))
				throw winrt::hresult_invalid_argument(L"A compiled typed-property backing variable has the wrong HLSL type.");
		}
	}

	export void ValidateLibrary(
		std::span<std::uint8_t const> bytecode,
		std::span<hlsl::engine::Property const> properties)
	{
		auto reflection = Reflect(bytecode);
		auto* function = FindFunction(reflection.get(), "PSBody");
		if (!function)
			throw winrt::hresult_invalid_argument(L"The compiled shader does not export PSBody.");

		auto* constants = function->GetConstantBufferByName("UserConstants");
		D3D11_SHADER_BUFFER_DESC bufferDesc{};
		bool hasConstants = constants && SUCCEEDED(constants->GetDesc(&bufferDesc));
		if (properties.empty())
		{
			if (hasConstants && bufferDesc.Variables != 0)
				throw winrt::hresult_invalid_argument(
					L"The compiled shader declares UserConstants properties, but no HlslProperty schema was supplied.");
			return;
		}
		if (!hasConstants)
			throw winrt::hresult_invalid_argument(
				L"Typed compiled shader properties require cbuffer UserConstants : register(b0).");

		D3D11_SHADER_INPUT_BIND_DESC binding{};
		if (FAILED(function->GetResourceBindingDescByName("UserConstants", &binding)) ||
			binding.Type != D3D_SIT_CBUFFER || binding.BindPoint != 0)
		{
			throw winrt::hresult_invalid_argument(L"UserConstants must be bound to constant-buffer register b0.");
		}

		std::size_t expectedVariables{};
		for (auto const& property : properties)
		{
			auto const spec = hlsl::engine::GetPropertyAbiSpec(property.type);
			expectedVariables += spec.matrix ? MatrixChunkCount(spec) : 1u;
		}
		if (bufferDesc.Variables != expectedVariables)
			throw winrt::hresult_invalid_argument(
				L"The compiled UserConstants variable count does not match the declared HlslProperty schema.");

		hlsl::engine::PropertyLayoutCursor cursor{};
		for (std::size_t propertyIndex = 0; propertyIndex < properties.size(); ++propertyIndex)
		{
			auto const& property = properties[propertyIndex];
			auto const spec = hlsl::engine::GetPropertyAbiSpec(property.type);
			auto const layout = hlsl::engine::AppendPropertyLayout(cursor, property.type);
			if (!spec.matrix)
			{
				ValidateVariable(
					constants,
					winrt::to_string(property.name),
					layout.constantBufferOffset,
					spec.valueCount);
				continue;
			}
			for (std::size_t chunk = 0; chunk < MatrixChunkCount(spec); ++chunk)
			{
				ValidateVariable(
					constants,
					MatrixBackingName(propertyIndex, chunk),
					layout.constantBufferOffset + static_cast<std::uint32_t>(chunk) * 16u,
					MatrixChunkWidth(spec, chunk));
			}
		}

		if (bufferDesc.Size != hlsl::engine::FinalConstantBufferSize(cursor))
			throw winrt::hresult_invalid_argument(
				L"Compiled UserConstants size does not match the declared typed-property ABI.");
	}
}

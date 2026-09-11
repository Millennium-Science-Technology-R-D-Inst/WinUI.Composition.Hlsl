#include <robuffer.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include "HlslShaderLibrary.h"
#include "HlslShaderLibrary.g.cpp"

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	namespace
	{
		com_ptr<ID3D11LibraryReflection> ReflectLibrary(std::span<std::uint8_t const> bytecode)
		{
			com_ptr<ID3D11LibraryReflection> reflection;
			check_hresult(D3DReflectLibrary(
				bytecode.data(),
				bytecode.size(),
				__uuidof(ID3D11LibraryReflection),
				reflection.put_void()));
			return reflection;
		}

		ID3D11FunctionReflection* FindFunction(ID3D11LibraryReflection* library, char const* name)
		{
			D3D11_LIBRARY_DESC libraryDesc{};
			check_hresult(library->GetDesc(&libraryDesc));
			for (UINT i = 0; i < libraryDesc.FunctionCount; ++i)
			{
				auto* function = library->GetFunctionByIndex(i);
				if (!function) continue;
				D3D11_FUNCTION_DESC functionDesc{};
				if (SUCCEEDED(function->GetDesc(&functionDesc)) && functionDesc.Name && strcmp(functionDesc.Name, name) == 0)
				{
					return function;
				}
			}
			return nullptr;
		}

		bool IsFloatVector(ID3D11FunctionParameterReflection* parameter, UINT columns)
		{
			if (!parameter) return false;
			D3D11_PARAMETER_DESC desc{};
			return SUCCEEDED(parameter->GetDesc(&desc)) &&
				desc.Type == D3D_SVT_FLOAT &&
				desc.Class == D3D_SVC_VECTOR &&
				desc.Rows == 1 &&
				desc.Columns == columns;
		}
	}

	Hlsl::HlslShaderLibrary HlslShaderLibrary::Create(
		Windows::Storage::Streams::IBuffer const& bytecode,
		Hlsl::HlslShaderProfile profile)
	{
		if (!bytecode || bytecode.Length() < 4 || bytecode.Length() > 16 * 1024 * 1024)
		{
			throw hresult_invalid_argument(L"Shader bytecode must contain a DXBC library no larger than 16 MiB.");
		}

		switch (profile)
		{
			case Hlsl::HlslShaderProfile::Level91:
			case Hlsl::HlslShaderProfile::Level93:
			case Hlsl::HlslShaderProfile::Pixel40:
				break;
			default:
				throw hresult_invalid_argument(L"Unknown HLSL shader profile.");
		}

		auto access = bytecode.as<::Windows::Storage::Streams::IBufferByteAccess>();
		byte* data{};
		check_hresult(access->Buffer(&data));
		if (!data || memcmp(data, "DXBC", 4) != 0)
		{
			throw hresult_invalid_argument(L"Shader bytecode is not a DXBC container.");
		}

		std::vector<std::uint8_t> owned(data, data + bytecode.Length());
		try
		{
			auto reflection = ReflectLibrary(owned);
			D3D11_LIBRARY_DESC desc{};
			check_hresult(reflection->GetDesc(&desc));
			if (desc.FunctionCount == 0)
			{
				throw hresult_invalid_argument(L"DXBC library does not contain any exported HLSL functions.");
			}
		}
		catch (hresult_invalid_argument const&)
		{
			throw;
		}
		catch (...)
		{
			throw hresult_invalid_argument(L"Shader bytecode is not a valid reflectable HLSL DXBC library.");
		}

		return make<HlslShaderLibrary>(std::move(owned), profile);
	}

	void HlslShaderLibrary::ValidateForEffect(bool sampler, std::span<std::wstring const> propertyNames) const
	{
		auto reflection = ReflectLibrary(m_bytecode);
		auto* function = FindFunction(reflection.get(), "PSBody");
		if (!function)
		{
			throw hresult_invalid_argument(L"The DXBC library does not export the required PSBody function.");
		}

		D3D11_FUNCTION_DESC functionDesc{};
		check_hresult(function->GetDesc(&functionDesc));
		auto const expectedParameterCount = sampler ? 2 : 1;
		if (!functionDesc.HasReturn || functionDesc.FunctionParameterCount != expectedParameterCount ||
			!IsFloatVector(function->GetFunctionParameter(-1), 4))
		{
			throw hresult_invalid_argument(
				sampler
					? L"Compiled sampler PSBody must have ABI float4 PSBody(float2 uv, float4 samplerDataExt)."
					: L"Compiled color PSBody must have ABI float4 PSBody(float4 color).");
		}

		if (sampler)
		{
			if (!IsFloatVector(function->GetFunctionParameter(0), 2) ||
				!IsFloatVector(function->GetFunctionParameter(1), 4))
			{
				throw hresult_invalid_argument(L"Compiled sampler PSBody parameter types do not match the public sampler ABI.");
			}
		}
		else if (!IsFloatVector(function->GetFunctionParameter(0), 4))
		{
			throw hresult_invalid_argument(L"Compiled color PSBody parameter type does not match the public color ABI.");
		}

		if (propertyNames.empty())
		{
			return;
		}

		auto* constants = function->GetConstantBufferByName("UserConstants");
		if (!constants)
		{
			throw hresult_invalid_argument(L"Compiled shader properties require cbuffer UserConstants : register(b0).");
		}
		D3D11_SHADER_BUFFER_DESC bufferDesc{};
		if (FAILED(constants->GetDesc(&bufferDesc)))
		{
			throw hresult_invalid_argument(L"Compiled shader properties require cbuffer UserConstants : register(b0).");
		}

		auto const expectedSize = static_cast<UINT>(((propertyNames.size() + 3) / 4) * 16);
		if (bufferDesc.Size != expectedSize)
		{
			throw hresult_invalid_argument(L"Compiled shader UserConstants size does not match the declared property ABI.");
		}

		for (size_t i = 0; i < propertyNames.size(); ++i)
		{
			auto name = to_string(hstring(propertyNames[i]));
			auto* variable = function->GetVariableByName(name.c_str());
			if (!variable)
			{
				throw hresult_invalid_argument(L"Compiled shader is missing a declared scalar property in UserConstants.");
			}
			D3D11_SHADER_VARIABLE_DESC variableDesc{};
			if (FAILED(variable->GetDesc(&variableDesc)) || variableDesc.StartOffset != i * sizeof(float) || variableDesc.Size != sizeof(float))
			{
				throw hresult_invalid_argument(L"Compiled shader scalar property layout does not match the declared property ABI.");
			}
		}
	}
}

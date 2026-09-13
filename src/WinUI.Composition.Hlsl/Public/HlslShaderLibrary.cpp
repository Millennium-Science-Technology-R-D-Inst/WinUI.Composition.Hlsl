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

		bool HasColorAbi(ID3D11FunctionReflection* function)
		{
			if (!function) return false;
			D3D11_FUNCTION_DESC desc{};
			return SUCCEEDED(function->GetDesc(&desc)) && desc.HasReturn && desc.FunctionParameterCount == 1 &&
				IsFloatVector(function->GetFunctionParameter(-1), 4) &&
				IsFloatVector(function->GetFunctionParameter(0), 4);
		}

		bool HasSamplerAbi(ID3D11FunctionReflection* function, bool materialized)
		{
			if (!function) return false;
			D3D11_FUNCTION_DESC desc{};
			if (FAILED(function->GetDesc(&desc)) || !desc.HasReturn ||
				desc.FunctionParameterCount != (materialized ? 3 : 2) ||
				!IsFloatVector(function->GetFunctionParameter(-1), 4) ||
				!IsFloatVector(function->GetFunctionParameter(0), 2) ||
				!IsFloatVector(function->GetFunctionParameter(1), 4))
			{
				return false;
			}
			return !materialized || IsFloatVector(function->GetFunctionParameter(2), 4);
		}

		Hlsl::HlslShaderLibrary CreateValidatedLibrary(
			std::span<std::uint8_t const> bytecode,
			Hlsl::HlslShaderProfile profile)
		{
			if (bytecode.size() < 4 || bytecode.size() > 16 * 1024 * 1024)
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

			if (memcmp(bytecode.data(), "DXBC", 4) != 0)
			{
				throw hresult_invalid_argument(L"Shader bytecode is not a DXBC container.");
			}

			std::vector<std::uint8_t> owned(bytecode.begin(), bytecode.end());
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
	}

	Hlsl::HlslShaderLibrary HlslShaderLibrary::Create(
		Windows::Storage::Streams::IBuffer const& bytecode,
		Hlsl::HlslShaderProfile profile)
	{
		if (!bytecode)
		{
			throw hresult_invalid_argument(L"The shader bytecode buffer is null.");
		}

		auto access = bytecode.as<::Windows::Storage::Streams::IBufferByteAccess>();
		byte* data{};
		check_hresult(access->Buffer(&data));
		if (!data && bytecode.Length() != 0)
		{
			throw hresult_invalid_argument(L"The shader bytecode buffer is not readable.");
		}

		return CreateValidatedLibrary(
			std::span<std::uint8_t const>{ reinterpret_cast<std::uint8_t const*>(data), bytecode.Length() },
			profile);
	}

	Hlsl::HlslShaderLibrary HlslShaderLibrary::CreateFromByteArray(
		winrt::array_view<std::uint8_t const> bytecode,
		Hlsl::HlslShaderProfile profile)
	{
		return CreateValidatedLibrary(
			std::span<std::uint8_t const>{ bytecode.data(), bytecode.size() },
			profile);
	}

	Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> HlslShaderLibrary::LoadFromFileAsync(
		Windows::Storage::StorageFile const& file,
		Hlsl::HlslShaderProfile profile)
	{
		if (!file)
		{
			throw hresult_invalid_argument(L"The shader file is null.");
		}
		auto buffer = co_await Windows::Storage::FileIO::ReadBufferAsync(file);
		co_return Create(buffer, profile);
	}

	Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> HlslShaderLibrary::LoadFromApplicationUriAsync(
		Windows::Foundation::Uri const& uri,
		Hlsl::HlslShaderProfile profile)
	{
		if (!uri)
		{
			throw hresult_invalid_argument(L"The shader URI is null.");
		}
		auto file = co_await Windows::Storage::StorageFile::GetFileFromApplicationUriAsync(uri);
		co_return co_await LoadFromFileAsync(file, profile);
	}

	Windows::Storage::Streams::IBuffer HlslShaderLibrary::Bytecode() const
	{
		auto buffer = Windows::Storage::Streams::Buffer(static_cast<uint32_t>(m_bytecode.size()));
		buffer.Length(static_cast<uint32_t>(m_bytecode.size()));
		auto access = buffer.as<::Windows::Storage::Streams::IBufferByteAccess>();
		byte* destination{};
		check_hresult(access->Buffer(&destination));
		if (!m_bytecode.empty())
		{
			memcpy(destination, m_bytecode.data(), m_bytecode.size());
		}
		return buffer;
	}

	void HlslShaderLibrary::ValidateForEffect(Hlsl::HlslEffectKind kind, std::span<std::wstring const> propertyNames) const
	{
		bool sampler{};
		bool materialized{};
		switch (kind)
		{
			case Hlsl::HlslEffectKind::Color: break;
			case Hlsl::HlslEffectKind::Sampler: sampler = true; break;
			case Hlsl::HlslEffectKind::MaterializedSampler: sampler = true; materialized = true; break;
			default: throw hresult_invalid_argument(L"Unknown HLSL effect kind.");
		}

		auto reflection = ReflectLibrary(m_bytecode);
		auto* function = FindFunction(reflection.get(), "PSBody");
		if (!function)
		{
			throw hresult_invalid_argument(L"The DXBC library does not export the required PSBody function.");
		}

		if (sampler)
		{
			static constexpr char const* requiredSamplerExports[] = {
				"PSBody", "PSBodyCC", "PSBodyCW", "PSBodyCM", "PSBodyWC", "PSBodyWW", "PSBodyWM",
				"PSBodyMC", "PSBodyMW", "PSBodyMM", "PSBodyC", "PSBodyW", "PSBodyM"
			};
			for (auto const* exportName : requiredSamplerExports)
			{
				if (!HasSamplerAbi(FindFunction(reflection.get(), exportName), materialized))
				{
					throw hresult_invalid_argument(materialized
						? L"Compiled materialized sampler libraries must export all PSBody edge-mode variants with ABI float4(float2 uv, float4 samplerDataExt, float4 samplerData)."
						: L"Compiled sampler libraries must export all PSBody edge-mode variants with ABI float4(float2 uv, float4 samplerDataExt).");
				}
			}
			if (materialized && !HasColorAbi(FindFunction(reflection.get(), "MaterializeColor")))
			{
				throw hresult_invalid_argument(L"Compiled materialized sampler libraries must export float4 MaterializeColor(float4 color).");
			}
		}
		else if (!HasColorAbi(function))
		{
			throw hresult_invalid_argument(L"Compiled color PSBody must have ABI float4 PSBody(float4 color).");
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

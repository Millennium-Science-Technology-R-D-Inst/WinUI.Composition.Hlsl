#include <robuffer.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include "HlslShaderLibrary.h"
#include "HlslShaderLibrary.g.cpp"

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	namespace
	{
		constexpr std::array<char const*, 13> RequiredSamplerExports{
			"PSBody", "PSBodyCC", "PSBodyCW", "PSBodyCM", "PSBodyWC", "PSBodyWW", "PSBodyWM",
			"PSBodyMC", "PSBodyMW", "PSBodyMM", "PSBodyC", "PSBodyW", "PSBodyM"
		};

		struct ReflectedEffectAbi
		{
			Hlsl::HlslEffectKind kind{};
			std::uint32_t sourceCount{};
		};

		struct EmbeddedMetadata
		{
			Hlsl::HlslEffectKind kind{};
			Hlsl::HlslShaderProfile profile{};
			std::optional<std::uint32_t> sourceCount;
		};

		void ValidateProfile(Hlsl::HlslShaderProfile profile)
		{
			switch (profile)
			{
				case Hlsl::HlslShaderProfile::Level91:
				case Hlsl::HlslShaderProfile::Level93:
				case Hlsl::HlslShaderProfile::Pixel40:
					return;
				default:
					throw hresult_invalid_argument(L"Unknown HLSL shader profile.");
			}
		}

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

		bool HasSingleColorAbi(ID3D11FunctionReflection* function)
		{
			if (!function) return false;
			D3D11_FUNCTION_DESC desc{};
			return SUCCEEDED(function->GetDesc(&desc)) && desc.HasReturn && desc.FunctionParameterCount == 1 &&
				IsFloatVector(function->GetFunctionParameter(-1), 4) &&
				IsFloatVector(function->GetFunctionParameter(0), 4);
		}

		std::optional<std::uint32_t> GetColorSourceCount(ID3D11FunctionReflection* function)
		{
			if (!function) return std::nullopt;
			D3D11_FUNCTION_DESC desc{};
			if (FAILED(function->GetDesc(&desc)) || !desc.HasReturn ||
				desc.FunctionParameterCount < 1 || desc.FunctionParameterCount > 16 ||
				!IsFloatVector(function->GetFunctionParameter(-1), 4))
			{
				return std::nullopt;
			}
			for (UINT index = 0; index < desc.FunctionParameterCount; ++index)
			{
				if (!IsFloatVector(function->GetFunctionParameter(index), 4))
				{
					return std::nullopt;
				}
			}
			return desc.FunctionParameterCount;
		}

		std::optional<std::uint32_t> GetSamplerSourceCount(
			ID3D11FunctionReflection* function,
			bool materialized)
		{
			if (!function) return std::nullopt;
			D3D11_FUNCTION_DESC desc{};
			if (FAILED(function->GetDesc(&desc)) || !desc.HasReturn ||
				!IsFloatVector(function->GetFunctionParameter(-1), 4))
			{
				return std::nullopt;
			}

			auto const parametersPerSource = materialized ? 3u : 2u;
			if (desc.FunctionParameterCount == 0 ||
				desc.FunctionParameterCount % parametersPerSource != 0)
			{
				return std::nullopt;
			}
			auto const sourceCount = desc.FunctionParameterCount / parametersPerSource;
			if (sourceCount < 1 || sourceCount > 16)
			{
				return std::nullopt;
			}

			for (UINT source = 0; source < sourceCount; ++source)
			{
				auto const base = source * parametersPerSource;
				if (!IsFloatVector(function->GetFunctionParameter(base), 2) ||
					!IsFloatVector(function->GetFunctionParameter(base + 1), 4) ||
					(materialized && !IsFloatVector(function->GetFunctionParameter(base + 2), 4)))
				{
					return std::nullopt;
				}
			}
			return sourceCount;
		}

		ReflectedEffectAbi InferEffectAbi(ID3D11LibraryReflection* reflection)
		{
			auto* body = FindFunction(reflection, "PSBody");
			if (!body)
			{
				throw hresult_invalid_argument(L"The DXBC library does not expose the required PSBody function.");
			}

			if (auto const colorSourceCount = GetColorSourceCount(body))
			{
				return { Hlsl::HlslEffectKind::Color, *colorSourceCount };
			}

			auto const materializedSourceCount = GetSamplerSourceCount(body, true);
			auto const samplerSourceCount = GetSamplerSourceCount(body, false);
			if (materializedSourceCount && samplerSourceCount)
			{
				throw hresult_invalid_argument(L"The DXBC library exposes an ambiguous sampler ABI.");
			}
			if (!materializedSourceCount && !samplerSourceCount)
			{
				throw hresult_invalid_argument(L"The DXBC library does not match a supported Composition HLSL ABI.");
			}

			auto const materialized = materializedSourceCount.has_value();
			auto const sourceCount = materialized ? *materializedSourceCount : *samplerSourceCount;
		for (auto const* exportName : RequiredSamplerExports)
			{
				auto const exportSourceCount = GetSamplerSourceCount(FindFunction(reflection, exportName), materialized);
				if (!exportSourceCount || *exportSourceCount != sourceCount)
				{
					throw hresult_invalid_argument(materialized
						? L"Compiled materialized sampler libraries must export all PSBody edge-mode variants with the same float2/float4/float4 source groups."
						: L"Compiled sampler libraries must export all PSBody edge-mode variants with the same float2/float4 source groups.");
				}
			}

			if (materialized)
			{
				if (sourceCount != 1)
				{
					throw hresult_invalid_argument(L"MaterializedSampler currently supports exactly one source.");
				}
				if (!HasSingleColorAbi(FindFunction(reflection, "MaterializeColor")))
				{
					throw hresult_invalid_argument(L"Compiled materialized sampler libraries must export float4 MaterializeColor(float4 color).");
				}
				return { Hlsl::HlslEffectKind::MaterializedSampler, sourceCount };
			}

			return { Hlsl::HlslEffectKind::Sampler, sourceCount };
		}

		std::optional<EmbeddedMetadata> FindEmbeddedMetadata(ID3D11LibraryReflection* reflection)
		{
			std::optional<EmbeddedMetadata> result;
			auto accept = [&](ID3D11FunctionReflection* marker, EmbeddedMetadata metadata)
				{
					if (!marker) return;
					if (!HasSingleColorAbi(marker))
					{
						throw hresult_invalid_argument(
							L"The DXBC library contains a malformed WinUI.Composition.Hlsl metadata marker.");
					}
					if (result)
					{
						throw hresult_invalid_argument(L"The DXBC library contains conflicting WinUI.Composition.Hlsl metadata markers.");
					}
					result = metadata;
				};

			for (std::uint32_t kind = 0; kind <= 2; ++kind)
			{
				for (std::uint32_t profile = 0; profile <= 2; ++profile)
				{
					for (std::uint32_t sourceCount = 1; sourceCount <= 16; ++sourceCount)
					{
						auto name = std::string("__WinUICompositionHlsl_Metadata_K") +
							std::to_string(kind) + "_P" + std::to_string(profile) +
							"_S" + std::to_string(sourceCount);
						accept(
							FindFunction(reflection, name.c_str()),
							EmbeddedMetadata{
								static_cast<Hlsl::HlslEffectKind>(kind),
								static_cast<Hlsl::HlslShaderProfile>(profile),
								sourceCount });
					}

					// Accept the original single-source K/P marker so generated DXBC from
					// pre-SourceCount package versions remains loadable. Its source count is
					// inferred from PSBody rather than trusted from metadata.
					auto legacyName = std::string("__WinUICompositionHlsl_Metadata_K") +
						std::to_string(kind) + "_P" + std::to_string(profile);
					accept(
						FindFunction(reflection, legacyName.c_str()),
						EmbeddedMetadata{
							static_cast<Hlsl::HlslEffectKind>(kind),
							static_cast<Hlsl::HlslShaderProfile>(profile),
							std::nullopt });
				}
			}
			return result;
		}

		Hlsl::HlslShaderLibrary CreateValidatedLibrary(
			std::span<std::uint8_t const> bytecode,
			std::optional<Hlsl::HlslShaderProfile> requestedProfile)
		{
			if (bytecode.size() < 4 || bytecode.size() > 16 * 1024 * 1024)
			{
				throw hresult_invalid_argument(L"Shader bytecode must contain a DXBC library no larger than 16 MiB.");
			}
			if (memcmp(bytecode.data(), "DXBC", 4) != 0)
			{
				throw hresult_invalid_argument(L"Shader bytecode is not a DXBC container.");
			}
			if (requestedProfile) ValidateProfile(*requestedProfile);

			std::vector<std::uint8_t> owned(bytecode.begin(), bytecode.end());
			try
			{
				auto reflection = ReflectLibrary(owned);
				D3D11_LIBRARY_DESC desc{};
				check_hresult(reflection->GetDesc(&desc));
				if (desc.FunctionCount == 0)
				{
					throw hresult_invalid_argument(L"DXBC library does not contain any reflected HLSL functions.");
				}

				auto const effectAbi = InferEffectAbi(reflection.get());
				auto const metadata = FindEmbeddedMetadata(reflection.get());
				if (metadata && metadata->kind != effectAbi.kind)
				{
					throw hresult_invalid_argument(L"Embedded shader metadata does not match the reflected Composition HLSL ABI.");
				}
				if (metadata && metadata->sourceCount && *metadata->sourceCount != effectAbi.sourceCount)
				{
					throw hresult_invalid_argument(L"Embedded shader source-count metadata does not match the reflected Composition HLSL ABI.");
				}

				Hlsl::HlslShaderProfile profile{};
				if (requestedProfile)
				{
					profile = *requestedProfile;
					if (metadata && metadata->profile != profile)
					{
						throw hresult_invalid_argument(L"The explicit HLSL shader profile does not match the embedded build metadata.");
					}
				}
				else
				{
					if (!metadata)
					{
						throw hresult_invalid_argument(
							L"The DXBC library does not contain WinUI.Composition.Hlsl build metadata. Use the overload that supplies an explicit shader profile for external or legacy bytecode.");
					}
					profile = metadata->profile;
				}

				return make<HlslShaderLibrary>(
					std::move(owned),
					profile,
					effectAbi.kind,
					effectAbi.sourceCount);
			}
			catch (hresult_invalid_argument const&)
			{
				throw;
			}
			catch (...)
			{
				throw hresult_invalid_argument(L"Shader bytecode is not a valid reflectable HLSL DXBC library.");
			}
		}

		Hlsl::HlslShaderLibrary CreateValidatedLibraryFromBuffer(
			Windows::Storage::Streams::IBuffer const& bytecode,
			std::optional<Hlsl::HlslShaderProfile> requestedProfile)
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
				requestedProfile);
		}
	}

	Hlsl::HlslShaderLibrary HlslShaderLibrary::Create(
		Windows::Storage::Streams::IBuffer const& bytecode,
		Hlsl::HlslShaderProfile profile)
	{
		return CreateValidatedLibraryFromBuffer(bytecode, profile);
	}

	Hlsl::HlslShaderLibrary HlslShaderLibrary::CreateFromByteArray(
		winrt::array_view<std::uint8_t const> bytecode,
		Hlsl::HlslShaderProfile profile)
	{
		return CreateValidatedLibrary(
			std::span<std::uint8_t const>{ bytecode.data(), bytecode.size() },
			profile);
	}

	Hlsl::HlslShaderLibrary HlslShaderLibrary::CreateFromGeneratedByteArray(
		winrt::array_view<std::uint8_t const> bytecode)
	{
		return CreateValidatedLibrary(
			std::span<std::uint8_t const>{ bytecode.data(), bytecode.size() },
			std::nullopt);
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
		co_return CreateValidatedLibraryFromBuffer(buffer, profile);
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

	Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> HlslShaderLibrary::LoadGeneratedFromFileAsync(
		Windows::Storage::StorageFile const& file)
	{
		if (!file)
		{
			throw hresult_invalid_argument(L"The shader file is null.");
		}
		auto buffer = co_await Windows::Storage::FileIO::ReadBufferAsync(file);
		co_return CreateValidatedLibraryFromBuffer(buffer, std::nullopt);
	}

	Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> HlslShaderLibrary::LoadGeneratedFromApplicationUriAsync(
		Windows::Foundation::Uri const& uri)
	{
		if (!uri)
		{
			throw hresult_invalid_argument(L"The shader URI is null.");
		}
		auto file = co_await Windows::Storage::StorageFile::GetFileFromApplicationUriAsync(uri);
		co_return co_await LoadGeneratedFromFileAsync(file);
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

	void HlslShaderLibrary::ValidateForEffect(
		Hlsl::HlslEffectKind kind,
		std::uint32_t sourceCount,
		std::span<std::wstring const> propertyNames) const
	{
		if (sourceCount == 0 || sourceCount > 16)
		{
			throw hresult_invalid_argument(L"Composition HLSL supports between 1 and 16 sources.");
		}
		auto reflection = ReflectLibrary(m_bytecode);
		auto const reflectedAbi = InferEffectAbi(reflection.get());
		if (reflectedAbi.kind != kind || reflectedAbi.kind != m_effectKind)
		{
			throw hresult_invalid_argument(L"The requested effect kind does not match the compiled Composition HLSL ABI.");
		}
		if (reflectedAbi.sourceCount != sourceCount || reflectedAbi.sourceCount != m_sourceCount)
		{
			throw hresult_invalid_argument(L"The requested source count does not match the compiled Composition HLSL ABI.");
		}

		if (propertyNames.empty())
		{
			return;
		}

		auto* function = FindFunction(reflection.get(), "PSBody");
		auto* constants = function ? function->GetConstantBufferByName("UserConstants") : nullptr;
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

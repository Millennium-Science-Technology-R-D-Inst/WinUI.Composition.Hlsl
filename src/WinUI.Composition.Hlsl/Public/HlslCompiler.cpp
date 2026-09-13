#include <d3dcompiler.h>
#include "HlslCompiler.h"
#include "HlslCompiler.g.cpp"
#include "HlslShaderLibrary.h"

import WinUI.Composition.Hlsl.ShaderSource;
import std;
import winrt_base;

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	namespace
	{
		struct EffectKindInfo
		{
			bool sampler{};
			bool materialized{};
		};

		struct MacroDefinition
		{
			std::string name;
			std::string value;
		};

		EffectKindInfo GetEffectKindInfo(Hlsl::HlslEffectKind kind)
		{
			switch (kind)
			{
				case Hlsl::HlslEffectKind::Color: return {};
				case Hlsl::HlslEffectKind::Sampler: return { true, false };
				case Hlsl::HlslEffectKind::MaterializedSampler: return { true, true };
				default: throw hresult_invalid_argument(L"Unknown HLSL effect kind.");
			}
		}

		char const* ShaderTarget(Hlsl::HlslShaderProfile profile)
		{
			switch (profile)
			{
				case Hlsl::HlslShaderProfile::Level91: return "lib_4_0_level_9_1_ps_only";
				case Hlsl::HlslShaderProfile::Level93: return "lib_4_0_level_9_3_ps_only";
				case Hlsl::HlslShaderProfile::Pixel40: return "lib_4_0";
				default: throw hresult_invalid_argument(L"Unknown HLSL shader profile.");
			}
		}

		bool IsIdentifier(std::string_view value)
		{
			if (value.empty() || value.size() > 128) return false;
			auto isAlpha = [](char c) noexcept
				{
					return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
				};
			auto isDigit = [](char c) noexcept { return c >= '0' && c <= '9'; };
			if (!isAlpha(value.front())) return false;
			for (char c : value)
			{
				if (!isAlpha(c) && !isDigit(c)) return false;
			}
			return true;
		}

		std::vector<MacroDefinition> CopyDefines(
			Windows::Foundation::Collections::IVectorView<hstring> const& definitions)
		{
			std::vector<MacroDefinition> result;
			if (!definitions) return result;
			if (definitions.Size() > 64)
			{
				throw hresult_invalid_argument(L"At most 64 HLSL preprocessor definitions are supported.");
			}
			result.reserve(definitions.Size());
			std::set<std::string> names;
			for (auto const& projected : definitions)
			{
				auto text = to_string(projected);
				if (text.empty() || text.size() > 1152 || text.find('\0') != std::string::npos)
				{
					throw hresult_invalid_argument(L"Each HLSL definition must be NAME or NAME=VALUE and no larger than 1152 bytes.");
				}
				auto const separator = text.find('=');
				auto name = text.substr(0, separator);
				auto value = separator == std::string::npos ? std::string{ "1" } : text.substr(separator + 1);
				if (!IsIdentifier(name) || value.size() > 1024 || !names.insert(name).second)
				{
					throw hresult_invalid_argument(L"HLSL definitions must use unique ASCII identifiers and values no larger than 1024 bytes.");
				}
				result.push_back({ std::move(name),std::move(value) });
			}
			return result;
		}

		std::vector<std::uint8_t> CompileLibrary(
			std::string const& source,
			Hlsl::HlslShaderProfile profile,
			std::span<MacroDefinition const> definitions)
		{
			// D3D_SHADER_MACRO is declared by the Windows SDK in the global namespace.
			// Qualify it explicitly because this implementation itself lives under winrt::.
			std::vector<::D3D_SHADER_MACRO> macros;
			if (!definitions.empty())
			{
				macros.reserve(definitions.size() + 1);
				for (auto const& definition : definitions)
				{
					macros.push_back({ definition.name.c_str(),definition.value.c_str() });
				}
				macros.push_back({ nullptr,nullptr });
			}

			com_ptr<ID3DBlob> bytecode;
			com_ptr<ID3DBlob> errors;
			auto const flags = D3DCOMPILE_ENABLE_STRICTNESS |
				D3DCOMPILE_OPTIMIZATION_LEVEL3 |
				D3DCOMPILE_WARNINGS_ARE_ERRORS;
			auto const result = D3DCompile(
				source.data(),
				source.size(),
				"UserShader.hlsl",
				macros.empty() ? nullptr : macros.data(),
				nullptr,
				nullptr,
				ShaderTarget(profile),
				flags,
				0,
				bytecode.put(),
				errors.put());
			if (FAILED(result))
			{
				if (errors && errors->GetBufferPointer() && errors->GetBufferSize())
				{
					auto text = std::string_view(
						static_cast<char const*>(errors->GetBufferPointer()),
						errors->GetBufferSize());
					while (!text.empty() && text.back() == '\0') text.remove_suffix(1);
					throw hresult_error(result, to_hstring(text));
				}
				throw_hresult(result);
			}
			auto begin = static_cast<std::uint8_t const*>(bytecode->GetBufferPointer());
			return { begin, begin + bytecode->GetBufferSize() };
		}

		struct CompileInput
		{
			std::string shader;
			std::string declarations;
			std::vector<std::wstring> propertyNames;
			std::vector<MacroDefinition> definitions;
			Hlsl::HlslEffectKind kind{};
			bool sampler{};
			bool materialized{};
			Hlsl::HlslShaderProfile profile{};
		};

		CompileInput PrepareInput(
			hstring const& shader,
			Hlsl::HlslEffectKind kind,
			Hlsl::HlslShaderProfile profile,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties,
			Windows::Foundation::Collections::IVectorView<hstring> const& definitions)
		{
			CompileInput input;
			input.shader = to_string(shader);
			if (input.shader.empty() || input.shader.size() > 1024 * 1024 || input.shader.find('\0') != std::string::npos)
			{
				throw hresult_invalid_argument(L"Shader source must be non-empty, contain no embedded NUL, and be no larger than 1 MiB.");
			}
			auto const kindInfo = GetEffectKindInfo(kind);
			input.kind = kind;
			input.sampler = kindInfo.sampler;
			input.materialized = kindInfo.materialized;
			(void)ShaderTarget(profile);
			input.profile = profile;
			input.definitions = CopyDefines(definitions);
			if (properties && properties.Size())
			{
				input.declarations = "cbuffer UserConstants : register(b0) {\n";
				input.propertyNames.reserve(properties.Size());
				for (auto const& property : properties)
				{
					if (!property) throw hresult_invalid_argument(L"A property descriptor is null.");
					auto name = std::wstring(property.Name());
					input.propertyNames.push_back(name);
					input.declarations += "float " + to_string(property.Name()) + ";\n";
				}
				input.declarations += "};\n";
			}
			return input;
		}

		Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> CompilePreparedAsync(CompileInput input)
		{
			co_await resume_background();
			auto source = hlsl::compiler::BuildPublicShaderSource(input.declarations, input.shader, input.sampler, input.materialized);
			auto bytes = CompileLibrary(source, input.profile, input.definitions);
			auto projected = make<HlslShaderLibrary>(std::move(bytes), input.profile);
			get_self<HlslShaderLibrary>(projected)->ValidateForEffect(input.kind, input.propertyNames);
			co_return projected;
		}
	}

	Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> HlslCompiler::CompileAsync(
		hstring const& shader,
		Hlsl::HlslEffectKind kind,
		Hlsl::HlslShaderProfile profile)
	{
		return CompilePreparedAsync(PrepareInput(shader, kind, profile, nullptr, nullptr));
	}

	Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> HlslCompiler::CompileWithDefinesAsync(
		hstring const& shader,
		Hlsl::HlslEffectKind kind,
		Hlsl::HlslShaderProfile profile,
		Windows::Foundation::Collections::IVectorView<hstring> const& definitions)
	{
		return CompilePreparedAsync(PrepareInput(shader, kind, profile, nullptr, definitions));
	}

	Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> HlslCompiler::CompileWithPropertiesAsync(
		hstring const& shader,
		Hlsl::HlslEffectKind kind,
		Hlsl::HlslShaderProfile profile,
		Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
	{
		return CompilePreparedAsync(PrepareInput(shader, kind, profile, properties, nullptr));
	}

	Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> HlslCompiler::CompileWithPropertiesAndDefinesAsync(
		hstring const& shader,
		Hlsl::HlslEffectKind kind,
		Hlsl::HlslShaderProfile profile,
		Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties,
		Windows::Foundation::Collections::IVectorView<hstring> const& definitions)
	{
		return CompilePreparedAsync(PrepareInput(shader, kind, profile, properties, definitions));
	}
}

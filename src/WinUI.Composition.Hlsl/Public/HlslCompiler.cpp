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
		bool IsSampler(Hlsl::HlslEffectKind kind)
		{
			switch (kind)
			{
				case Hlsl::HlslEffectKind::Color: return false;
				case Hlsl::HlslEffectKind::Sampler: return true;
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

		std::vector<std::uint8_t> CompileLibrary(std::string const& source, Hlsl::HlslShaderProfile profile)
		{
			com_ptr<ID3DBlob> bytecode;
			com_ptr<ID3DBlob> errors;
			auto const flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
			auto const result = D3DCompile(
				source.data(),
				source.size(),
				"UserShader.hlsl",
				nullptr,
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
			bool sampler{};
			Hlsl::HlslShaderProfile profile{};
		};

		CompileInput PrepareInput(
			hstring const& shader,
			Hlsl::HlslEffectKind kind,
			Hlsl::HlslShaderProfile profile,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
		{
			CompileInput input;
			input.shader = to_string(shader);
			if (input.shader.empty() || input.shader.size() > 1024 * 1024 || input.shader.find('\0') != std::string::npos)
			{
				throw hresult_invalid_argument(L"Shader source must be non-empty, contain no embedded NUL, and be no larger than 1 MiB.");
			}
			input.sampler = IsSampler(kind);
			(void)ShaderTarget(profile);
			input.profile = profile;
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
			auto source = hlsl::compiler::BuildPublicShaderSource(input.declarations, input.shader, input.sampler);
			auto bytes = CompileLibrary(source, input.profile);
			auto projected = make<HlslShaderLibrary>(std::move(bytes), input.profile);
			get_self<HlslShaderLibrary>(projected)->ValidateForEffect(input.sampler, input.propertyNames);
			co_return projected;
		}
	}

	Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> HlslCompiler::CompileAsync(
		hstring const& shader,
		Hlsl::HlslEffectKind kind,
		Hlsl::HlslShaderProfile profile)
	{
		return CompilePreparedAsync(PrepareInput(shader, kind, profile, nullptr));
	}

	Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> HlslCompiler::CompileWithPropertiesAsync(
		hstring const& shader,
		Hlsl::HlslEffectKind kind,
		Hlsl::HlslShaderProfile profile,
		Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
	{
		return CompilePreparedAsync(PrepareInput(shader, kind, profile, properties));
	}
}

module;

export module WinUI.Composition.Hlsl.ShaderSource;

import std;

export namespace hlsl::compiler
{
	inline constexpr std::array<std::string_view, 13> SamplerSuffixes{
		"", "CC", "CW", "CM", "WC", "WW", "WM", "MC", "MW", "MM", "C", "W", "M"
	};

	inline std::string BuildPublicShaderSource(
		std::string_view declarations,
		std::string_view userShader,
		bool sampler,
		bool materializedSampler = false,
		size_t sourceCount = 1)
	{
		std::string code;
		code.reserve(declarations.size() + userShader.size() + (sampler ? 4096u : 512u));
		if (sampler)
		{
			for (size_t input = 0; input < sourceCount; ++input)
				code += "Texture2D texture" + std::to_string(input) + "; SamplerState sampler" + std::to_string(input) + ";\n";
		}
		code.append(declarations);
		code += "#line 1 \"UserShader.hlsl\"\n";
		code.append(userShader);
		auto samplerParameters = [&](bool includeContentRect)
			{
				std::string parameters, arguments;
				for (size_t input = 0; input < sourceCount; ++input)
				{
					if (input)
					{
						parameters += ","; arguments += ",";
					}
					auto suffix = sourceCount == 1 ? std::string{} : std::to_string(input);
					parameters += "float2 uv" + suffix + ",float4 samplerDataExt" + suffix;
					arguments += "uv" + suffix + ",samplerDataExt" + suffix;
					if (includeContentRect)
					{
						parameters += ",float4 samplerData" + suffix;
						arguments += ",samplerData" + suffix;
					}
				}
				return std::pair{ std::move(parameters),std::move(arguments) };
			};
		if (materializedSampler)
		{
			auto const [parameters, arguments] = samplerParameters(true);
			code += "\n#line 1 \"WinUI.Composition.Hlsl.Generated.hlsl\"\nexport float4 MaterializeColor(float4 color){return color;}\n";
			for (auto suffix : SamplerSuffixes)
			{
				code += "#line 1 \"WinUI.Composition.Hlsl.Generated.hlsl\"\nexport float4 PSBody";
				code.append(suffix);
				code += "(" + parameters + "){return Shade(" + arguments + ");}\n";
			}
		}
		else if (sampler)
		{
			auto const [parameters, arguments] = samplerParameters(false);
			for (auto suffix : SamplerSuffixes)
			{
				code += "\n#line 1 \"WinUI.Composition.Hlsl.Generated.hlsl\"\nexport float4 PSBody";
				code.append(suffix);
				code += "(" + parameters + "){return Shade(" + arguments + ");}\n";
			}
		}
		else if (sourceCount > 1)
		{
			std::string parameters, arguments;
			for (size_t input = 0; input < sourceCount; ++input)
			{
				if (input)
				{
					parameters += ","; arguments += ",";
				}
				parameters += "float4 color" + std::to_string(input);
				arguments += "color" + std::to_string(input);
			}
			code += "\n#line 1 \"WinUI.Composition.Hlsl.Generated.hlsl\"\nexport float4 PSBody(" +
				parameters + "){return Shade(" + arguments + ");}\n";
		}
		return code;
	}
}

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
		bool materializedSampler = false)
	{
		std::string code;
		code.reserve(declarations.size() + userShader.size() + (sampler ? 3072u : 256u));
		if (sampler)
		{
			code += "Texture2D texture0; SamplerState sampler0;\n";
		}
		code.append(declarations);
		code += "#line 1 \"UserShader.hlsl\"\n";
		code.append(userShader);
		if (materializedSampler)
		{
			code += "\n#line 1 \"WinUI.Composition.Hlsl.Generated.hlsl\"\nexport float4 MaterializeColor(float4 color){return color;}\n";
			for (auto suffix : SamplerSuffixes)
			{
				code += "#line 1 \"WinUI.Composition.Hlsl.Generated.hlsl\"\nexport float4 PSBody";
				code.append(suffix);
				code += "(float2 uv,float4 samplerDataExt,float4 samplerData){return Shade(uv,samplerDataExt,samplerData);}\n";
			}
		}
		else if (sampler)
		{
			for (auto suffix : SamplerSuffixes)
			{
				code += "\n#line 1 \"WinUI.Composition.Hlsl.Generated.hlsl\"\nexport float4 PSBody";
				code.append(suffix);
				code += "(float2 uv,float4 samplerDataExt){return Shade(uv,samplerDataExt);}\n";
			}
		}
		return code;
	}
}

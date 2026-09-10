#include "pch.h"
#include "CustomLiquidGlassEffect.h"
#include "CustomEffectRuntime.h"

using namespace winrt;

namespace
{
	constexpr GUID kCustomLiquidGlassEffectId{ 0xc690ecdc, 0x9f2d, 0x46a8, { 0xa6, 0x54, 0x48, 0x0b, 0x3a, 0xf1, 0x12, 0x4e } };

	struct LiquidGlassConstants
	{
		float materialParams0[4];
		float materialParams1[4];
	};

	constexpr LiquidGlassConstants kInitialConstants{
		{ 0.0f, 1.5f, 36.0f, 12.0f },
		{ 0.85f, 1.0f, 1.2f, 1.0f },
	};

	static_assert(sizeof(LiquidGlassConstants) == 32);

	enum LiquidGlassPropertyIndex : uint32_t
	{
		BlurRadiusProperty = 0,
		RefractionStrengthProperty,
		CornerRadiusProperty,
		BorderThicknessProperty,
		HighlightStrengthProperty,
		DispersionStrengthProperty,
	};

	constexpr uint32_t kDCompositionExpressionTypeScalar = 18;
	constexpr uint32_t kPropertyTypeSingle = 8;
	constexpr uint32_t kBlurRadiusOffset = 0;
	constexpr uint32_t kBorderThicknessOffset = 4;
	constexpr uint32_t kCornerRadiusOffset = 8;
	constexpr uint32_t kRefractionStrengthOffset = 12;
	constexpr uint32_t kHighlightStrengthOffset = 16;
	constexpr uint32_t kDispersionStrengthOffset = 24;

	HRESULT CreateScalarProperty(
		float scalar,
		ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		if (!value)
		{
			return E_POINTER;
		}

		*value = nullptr;
		try
		{
			auto propertyValue = Windows::Foundation::PropertyValue::CreateSingle(scalar)
				.as<Windows::Foundation::IPropertyValue>();
			*value = reinterpret_cast<ABI::Windows::Foundation::IPropertyValue*>(
				detach_abi(propertyValue));
			return S_OK;
		}
		catch (...)
		{
			return to_hresult();
		}
	}

	HRESULT GetBlurRadiusDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams0[0], value);
	}

	HRESULT GetRefractionStrengthDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams0[3], value);
	}

	HRESULT GetCornerRadiusDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams0[2], value);
	}

	HRESULT GetBorderThicknessDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams0[1], value);
	}

	HRESULT GetHighlightStrengthDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams1[0], value);
	}

	HRESULT GetDispersionStrengthDefault(ABI::Windows::Foundation::IPropertyValue** value) noexcept
	{
		return CreateScalarProperty(kInitialConstants.materialParams1[2], value);
	}

	CustomEffectRuntime::PropertyDescriptor const kProperties[] = {
		{
			L"BlurRadius",
			BlurRadiusProperty,
			ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT,
			GetBlurRadiusDefault,
		},
		{
			L"RefractionStrength",
			RefractionStrengthProperty,
			ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT,
			GetRefractionStrengthDefault,
		},
		{
			L"CornerRadius",
			CornerRadiusProperty,
			ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT,
			GetCornerRadiusDefault,
		},
		{
			L"BorderThickness",
			BorderThicknessProperty,
			ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT,
			GetBorderThicknessDefault,
		},
		{
			L"HighlightStrength",
			HighlightStrengthProperty,
			ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT,
			GetHighlightStrengthDefault,
		},
		{
			L"DispersionStrength",
			DispersionStrengthProperty,
			ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT,
			GetDispersionStrengthDefault,
		},
	};

	CustomEffectRuntime::NativePropertyMetadata const kNativePropertyMetadata[] = {
		{ "BlurRadius", kBlurRadiusOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "RefractionStrength", kRefractionStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "CornerRadius", kCornerRadiusOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "BorderThickness", kBorderThicknessOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "HighlightStrength", kHighlightStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
		{ "DispersionStrength", kDispersionStrengthOffset, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
	};

	CustomEffectRuntime::ConstantBufferPropertyMapping const kConstantBufferProperties[] = {
		{ BlurRadiusProperty, kBlurRadiusOffset },
		{ RefractionStrengthProperty, kRefractionStrengthOffset },
		{ CornerRadiusProperty, kCornerRadiusOffset },
		{ BorderThicknessProperty, kBorderThicknessOffset },
		{ HighlightStrengthProperty, kHighlightStrengthOffset },
		{ DispersionStrengthProperty, kDispersionStrengthOffset },
	};

	char const kLiquidGlassShader[] = R"(
Texture2D texture0;
SamplerState sampler0;

cbuffer LiquidGlassConstants : register(b0)
{
    float4 MaterialParams0;
    float4 MaterialParams1;
};

float RoundedRectSdf(float2 p, float2 halfSize, float radius)
{
    float2 q = abs(p) - (halfSize - radius.xx);
    return length(max(q, 0.0f.xx)) + min(max(q.x, q.y), 0.0f) - radius;
}

float4 SampleTransmission(float2 uv, float2 texelSize, float blurRadius)
{
    // TODO: Replace this local placeholder with a real Dual Kawase blur pipeline.
    // DWM GaussianBlur is not a drop-in internal blur source here because its
    // external graph downsamples/pads intermediates; LiquidGlass needs stable
    // source UV/size semantics for shape, refraction, and border calculations.
    float2 stepSize = texelSize * max(blurRadius, 0.0f);
    float4 color = texture0.Sample(sampler0, uv) * 0.52f;
    color += texture0.Sample(sampler0, uv + float2( stepSize.x, 0.0f)) * 0.12f;
    color += texture0.Sample(sampler0, uv + float2(-stepSize.x, 0.0f)) * 0.12f;
    color += texture0.Sample(sampler0, uv + float2(0.0f,  stepSize.y)) * 0.12f;
    color += texture0.Sample(sampler0, uv + float2(0.0f, -stepSize.y)) * 0.12f;
    return color;
}

export float4 FlattenSource(float4 sample0)
{
    return sample0;
}

float4 LiquidGlassCore(float2 uv, float4 samplerDataExt, float4 samplerData)
{
    const float blurRadius = MaterialParams0.x;
    const float borderThickness = MaterialParams0.y;
    const float cornerRadius = MaterialParams0.z;
    const float refractionStrength = MaterialParams0.w;
    const float highlightStrength = MaterialParams1.x;
    const float edgeSoftness = MaterialParams1.y;
    const float dispersionStrength = MaterialParams1.z;
    const float materialOpacity = MaterialParams1.w;

    const float2 contentMin = min(samplerData.xy, samplerData.zw);
    const float2 contentMax = max(samplerData.xy, samplerData.zw);
    const float2 contentUvSizeRaw = contentMax - contentMin;
    const bool hasContentRect = all(contentUvSizeRaw > 1e-6f.xx);
    const float2 contentUvSize = hasContentRect ? contentUvSizeRaw : 1.0f.xx;
    // Do not saturate the content-space coordinate before the rounded-rect SDF.
    // GaussianBlur expands the source into padded intermediates; clamping that
    // padding back onto the content edge makes the material border stretch and
    // blur outward as radius grows. Letting coordinates go outside [0,1] keeps
    // the shape tied to DWM's effective content rect instead.
    const float2 localUv = hasContentRect ? ((uv - contentMin) / contentUvSize) : uv;
/*return float4(uv.x, 0.0f, 0.0f, 1.0f);
if (any(localUv < 0.0f.xx) || any(localUv > 1.0f.xx))
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f); // Debug: visualize localUv
}
return texture0.Sample(sampler0, localUv);*/

    const float2 localUvPixelStep = max(abs(ddx(localUv)) + abs(ddy(localUv)), 1e-6f.xx);
    // samplerDataExt.xy is the physical source/intermediate size. GaussianBlur can
    // grow that surface and DWM then linearly maps it back, so using ext as material
    // geometry makes corner radius scale with blur padding. Pixel derivatives are
    // evaluated after that mapping and recover the current output-space material
    // size without adding app-provided width/height properties.
    const float2 rectSize = max(1.0f.xx / localUvPixelStep, 1.0f.xx);
    const float2 texelSize = max(samplerDataExt.zw, 1e-6f.xx);
    const float2 localPosition = localUv * rectSize;
    const float2 halfRect = rectSize * 0.5f;
    const float2 local = localPosition - halfRect;
    const float clampedCornerRadius = min(cornerRadius, min(halfRect.x, halfRect.y));
    const float radius = max(clampedCornerRadius, 0.0f);
    const float sdf = RoundedRectSdf(local, halfRect, radius);
    const float feather = max(edgeSoftness, 1.0f);
    const float alpha = saturate((feather - sdf) / feather) * materialOpacity;
    if (alpha <= 0.0f)
    {
        return 0.0f.xxxx;
    }

    const float innerDistance = max(-sdf, 0.0f);
    const float halfMinSize = max(min(halfRect.x, halfRect.y), 1.0f);
    const float2 domeCoord = local / max(halfRect, 1.0f.xx);
    const float domeRadius = saturate(length(domeCoord));
    const float domeDepth = (1.0f - domeRadius) * halfMinSize;
    const float2 domeNormal = normalize(domeCoord + 1e-5f.xx);
    const float edgeFactor = 1.0f - saturate(innerDistance / max(radius, 1.0f));
    const float interiorFactor = saturate(domeDepth / halfMinSize);
    const float edgeDistance = max(borderThickness * 4.0f + feather * 2.0f, 1.0f);
    const float rimDistance = max(borderThickness * 2.0f + 1.0f, 1.0f);
    const float edgeIntensity = exp(-innerDistance / edgeDistance) * 0.85f;
    const float rimIntensity = exp(-innerDistance / rimDistance) * 0.25f;
    const float centerFade = 1.0f - smoothstep(
        halfMinSize * 0.08f,
        halfMinSize * 0.55f,
        domeDepth);
    const float refractionWeight = (edgeIntensity + rimIntensity) * centerFade;
    const float dispersionWeight = edgeIntensity * centerFade;

    const float2 refractUv = uv - domeNormal * texelSize * refractionStrength * refractionWeight;
    const float2 dispersionOffset = domeNormal * texelSize * dispersionStrength * dispersionWeight;

    float3 color = float3(
        SampleTransmission(refractUv - dispersionOffset, texelSize, blurRadius).r,
        SampleTransmission(refractUv, texelSize, blurRadius).g,
        SampleTransmission(refractUv + dispersionOffset, texelSize, blurRadius).b);
    color = lerp(color, 1.0f.xxx, 0.08f + interiorFactor * 0.06f);

    const float borderMask = 1.0f - smoothstep(borderThickness, borderThickness + feather, innerDistance);
    const float innerGlow = 1.0f - smoothstep(borderThickness * 2.0f, borderThickness * 6.0f + feather, innerDistance);
    const float domeHeight = sqrt(saturate(1.0f - dot(domeCoord, domeCoord)));
    const float3 surfaceNormal = normalize(float3(-domeCoord * 0.35f, 0.45f + domeHeight * 0.75f));
    const float3 lightDir = normalize(float3(-0.35f, -0.45f, 0.82f));
    const float specular = pow(saturate(dot(surfaceNormal, lightDir)), 18.0f) * (0.20f + edgeFactor * 0.50f);
    const float topSweep = pow(saturate(1.0f - localPosition.y / rectSize.y), 2.5f) * (0.15f + edgeFactor * 0.20f);

    color += (specular * 0.28f + topSweep * 0.12f + innerGlow * 0.10f) * highlightStrength;
    color = lerp(color, 1.0f.xxx, borderMask * 0.22f * highlightStrength);
    color = saturate(color);

    return float4(color * alpha, alpha);
}

// DWM appends sampler edge-mode suffixes for custom sampler bodies. The aliases
// preserve the code-only path while still using DWM's native custom sampler linker.
// These signatures intentionally include samplerData: DWM's single-source custom
// kernel ABI uses it to expose the effective content rect of expanded intermediates,
// which keeps the material shape stable when an upstream GaussianBlur changes padding.
export float4 PSBody(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyCC(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyCW(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyCM(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyWC(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyWW(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyWM(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyMC(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyMW(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyMM(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyC(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyW(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
export float4 PSBodyM(float2 uv, float4 samplerDataExt, float4 samplerData) { return LiquidGlassCore(uv, samplerDataExt, samplerData); }
)";

	constexpr uint16_t kBackdropUvArgument = 0x0100;
	constexpr uint16_t kBackdropSamplerDataExtArgument = 0x0400;
	constexpr uint16_t kBackdropSamplerDataArgument = 0x0300;
	constexpr uint16_t kBackdropCustomSamplerResult = 0x0200;

	CustomEffectRuntime::SourceDescriptor const kSources[] = {
		{ L"Backdrop", CustomEffectRuntime::SourceKind::Backdrop, true, true },
	};

	uint16_t const kShaderArguments[] = {
		kBackdropUvArgument,
		kBackdropSamplerDataExtArgument,
		kBackdropSamplerDataArgument,
	};

	CustomEffectRuntime::CustomEffectDefinition const kDefinition{
		kCustomLiquidGlassEffectId,
		CustomLiquidGlassEffect::EffectName,
		"CustomLiquidGlassEffect",
		kLiquidGlassShader,
		sizeof(kLiquidGlassShader) - 1,
		"PSBody",
		kSources,
		ARRAYSIZE(kSources),
		kProperties,
		ARRAYSIZE(kProperties),
		kNativePropertyMetadata,
		ARRAYSIZE(kNativePropertyMetadata),
		sizeof(LiquidGlassConstants),
		kConstantBufferProperties,
		ARRAYSIZE(kConstantBufferProperties),
		kShaderArguments,
		ARRAYSIZE(kShaderArguments),
		kBackdropCustomSamplerResult,
		// Profile 2: ps_4_0 + lib_4_0 (full SM4.0, not SM5). Verified runnable
		// for this brush because flatten materializes to an intermediate surface
		// and the glass technique links alone — not co-linked with wuceffectsi
		// profile-1 bodies. Still must keep body+0x2E and D3DCompile paired; see
		// CustomEffectRuntime.h (D3DShaderProfileVersion + isolation notes).
		CustomEffectRuntime::kShaderProfilePs40,
		sizeof(kInitialConstants),
		&kInitialConstants,
		true,
		"FlattenSource",
	};
}

namespace CustomLiquidGlassEffect
{
	hlsl::engine::Definition Description()
	{
		static auto value=[]
			{
				auto definition=std::make_shared<hlsl::engine::EffectDefinition>();
				definition->id=kCustomLiquidGlassEffectId; definition->sampler=true;
				definition->effectName=EffectName; definition->nativeTemplate=&kDefinition;
				definition->properties={
					{ L"BlurRadius",12,0,64 },{ L"RefractionStrength",24,0,128 },
					{ L"CornerRadius",12,0,512 },{ L"BorderThickness",1,0,32 },
					{ L"HighlightStrength",0.8f,0,4 },{ L"DispersionStrength",1.2f,0,16 } };
				return definition;
			}();
		return value;
	}
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect()
	{
		// This ports the WinUI2 glass material into the DWM custom sampler model.
		// The old app rendered an intermediate surface itself; this effect instead
		// asks DWM for UV plus samplerDataExt/samplerData so the shader can sample
		// the backdrop texture directly while using DWM's effective-content rect for
		// size-dependent material coordinates.
		return CustomEffectRuntime::CreateEffect(kDefinition);
	}

}


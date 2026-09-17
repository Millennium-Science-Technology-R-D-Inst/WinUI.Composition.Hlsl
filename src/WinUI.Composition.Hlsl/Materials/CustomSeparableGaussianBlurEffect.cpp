#include <Windows.h>
#include <windows.graphics.effects.interop.h>

#include "CustomSeparableGaussianBlurEffect.h"

import std;
import winrt.Windows.Graphics.Effects;
import WinUI.Composition.Hlsl.CustomEffectRuntime;
import WinUI.Composition.Hlsl.Shaders.SeparableGaussianBlur;

using namespace winrt;
using namespace Windows::Graphics::Effects;

namespace
{
	constexpr GUID kHorizontalEffectId{
		0x35e7864f, 0x2b58, 0x4c6d, { 0xa7, 0x4c, 0x6c, 0x54, 0x95, 0x2d, 0x11, 0x37 }
	};
	constexpr GUID kVerticalEffectId{
		0xd70b39a2, 0x86b1, 0x4d53, { 0x98, 0x7a, 0x2e, 0x7c, 0x43, 0xc4, 0x89, 0x1a }
	};

	constexpr std::size_t kKernelRadius = 20;
	constexpr std::size_t kMergedPairCount = 10;
	constexpr std::size_t kPairFloatCount = kMergedPairCount * 2;

	struct BlurConstants
	{
		float blurAmount;
		float padding0[3];
		float centerWeight;
		float padding1[3];
		float pairData[kPairFloatCount];
	};
	static_assert(sizeof(BlurConstants) == 112);

	BlurConstants BuildBlurConstants()
	{
		BlurConstants result{};
		constexpr float sigma = static_cast<float>(kKernelRadius) / 3.0f;
		std::array<float, kKernelRadius + 1> weights{};
		float totalWeight = 0.0f;

		for (std::size_t i = 0; i <= kKernelRadius; ++i)
		{
			auto const x = static_cast<float>(i);
			weights[i] = std::exp(-0.5f * x * x / (sigma * sigma));
			totalWeight += i == 0 ? weights[i] : weights[i] * 2.0f;
		}

		auto const inverseTotal = 1.0f / totalWeight;
		result.blurAmount = 0.0f;
		result.centerWeight = weights[0] * inverseTotal;

		for (std::size_t pair = 0; pair < kMergedPairCount; ++pair)
		{
			auto const i0 = pair * 2 + 1;
			auto const i1 = i0 + 1;
			auto const combinedWeight = weights[i0] + weights[i1];
			auto const mergedOffset =
				(static_cast<float>(i0) * weights[i0] + static_cast<float>(i1) * weights[i1]) /
				combinedWeight;
			result.pairData[pair * 2] = mergedOffset;
			result.pairData[pair * 2 + 1] = combinedWeight * inverseTotal;
		}

		return result;
	}

	BlurConstants const kInitialConstants = BuildBlurConstants();
	constexpr std::uint32_t kDCompositionExpressionTypeScalar = 18;
	constexpr std::uint32_t kPropertyTypeSingle = 8;
	constexpr std::uint16_t kUvArgument = 0x0100;
	constexpr std::uint16_t kSamplerDataExtArgument = 0x0400;
	constexpr std::uint16_t kCustomSamplerResult = 0x0200;

	CustomEffectRuntime::PropertyDescriptor const kProperties[] = {
		{ L"BlurAmount", 0, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, nullptr, 0.0f },
	};
	CustomEffectRuntime::NativePropertyMetadata const kMetadata[] = {
		{ "BlurAmount", 0, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
	};
	CustomEffectRuntime::ConstantBufferPropertyMapping const kMappings[] = {
		{ 0, 0 },
	};
	CustomEffectRuntime::SourceDescriptor const kSources[] = {
		{ L"Backdrop", CustomEffectRuntime::SourceKind::Backdrop, false, true },
	};
	std::uint16_t const kArguments[] = {
		kUvArgument,
		kSamplerDataExtArgument,
	};

	auto const kBlurShader = WinUI::Composition::Hlsl::Shaders::SeparableGaussianBlurShader();

	CustomEffectRuntime::CustomEffectDefinition const kHorizontalDefinition{
		kHorizontalEffectId,
		CustomSeparableGaussianBlurEffect::HorizontalEffectName,
		"LiquidGlassBlurHorizontal",
		nullptr,
		0,
		kBlurShader.data,
		kBlurShader.size,
		"BlurHorizontal",
		kSources,
		ARRAYSIZE(kSources),
		kProperties,
		ARRAYSIZE(kProperties),
		kMetadata,
		ARRAYSIZE(kMetadata),
		sizeof(float),
		kMappings,
		ARRAYSIZE(kMappings),
		kArguments,
		ARRAYSIZE(kArguments),
		kCustomSamplerResult,
		CustomEffectRuntime::kShaderProfilePs40,
		sizeof(kInitialConstants),
		&kInitialConstants,
		CustomEffectRuntime::CustomEffectInputMode::MaterializedTexture,
		CustomEffectRuntime::GraphLoweringPolicy::MaterializedInput,
		"FlattenSource",
	};

	CustomEffectRuntime::CustomEffectDefinition const kVerticalDefinition{
		kVerticalEffectId,
		CustomSeparableGaussianBlurEffect::VerticalEffectName,
		"LiquidGlassBlurVertical",
		nullptr,
		0,
		kBlurShader.data,
		kBlurShader.size,
		"BlurVertical",
		kSources,
		ARRAYSIZE(kSources),
		kProperties,
		ARRAYSIZE(kProperties),
		kMetadata,
		ARRAYSIZE(kMetadata),
		sizeof(float),
		kMappings,
		ARRAYSIZE(kMappings),
		kArguments,
		ARRAYSIZE(kArguments),
		kCustomSamplerResult,
		CustomEffectRuntime::kShaderProfilePs40,
		sizeof(kInitialConstants),
		&kInitialConstants,
		CustomEffectRuntime::CustomEffectInputMode::MaterializedTexture,
		CustomEffectRuntime::GraphLoweringPolicy::MaterializedInput,
		"FlattenSource",
	};
}

namespace CustomSeparableGaussianBlurEffect
{
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateHorizontalEffect(
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source)
	{
		if (!source) throw hresult_invalid_argument();
		return CustomEffectRuntime::CreateEffect(kHorizontalDefinition, source);
	}

	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateVerticalEffect(
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source)
	{
		if (!source) throw hresult_invalid_argument();
		return CustomEffectRuntime::CreateEffect(kVerticalDefinition, source);
	}
}

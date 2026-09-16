#include <Windows.h>
#include <windows.graphics.effects.interop.h>

#include "CustomKawaseBlurEffect.h"

import std;
import winrt.Windows.Graphics.Effects;
import WinUI.Composition.Hlsl.CustomEffectRuntime;
import WinUI.Composition.Hlsl.Shaders.KawaseBlur;

using namespace winrt;
using namespace Windows::Graphics::Effects;

namespace
{
	constexpr GUID kKawaseDownEffectId{
		0x1b764d31, 0xd8d8, 0x42fb, { 0xa4, 0x26, 0x50, 0xdb, 0xf4, 0xd9, 0x56, 0xc1 }
	};
	constexpr GUID kKawaseUpEffectId{
		0xf4914f3c, 0xa3fb, 0x43e5, { 0xb2, 0x57, 0xaf, 0x99, 0xcc, 0xf9, 0xb5, 0x75 }
	};
	constexpr GUID kKawaseResolveEffectId{
		0x362f0ad5, 0x6295, 0x4e48, { 0xb9, 0x3c, 0x4e, 0xef, 0x4b, 0x56, 0xe0, 0xa2 }
	};

	struct KawaseConstants
	{
		float amount;
		float padding[3];
	};
	static_assert(sizeof(KawaseConstants) == 16);

	constexpr KawaseConstants kSamplerInitial{ 1.0f, {} };
	constexpr KawaseConstants kResolveInitial{ 1.0f, {} };
	constexpr std::uint32_t kDCompositionExpressionTypeScalar = 18;
	constexpr std::uint32_t kPropertyTypeSingle = 8;
	constexpr std::uint16_t kUvArgument = 0x0100;
	constexpr std::uint16_t kColorArgument = 0x0200;
	constexpr std::uint16_t kSamplerDataArgument = 0x0300;
	constexpr std::uint16_t kSamplerDataExtArgument = 0x0400;
	constexpr std::uint16_t kCustomSamplerResult = 0x0200;

	CustomEffectRuntime::PropertyDescriptor const kSpreadProperties[] = {
		{ L"Spread", 0, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, nullptr, 1.0f },
	};
	CustomEffectRuntime::NativePropertyMetadata const kSpreadMetadata[] = {
		{ "Spread", 0, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
	};
	CustomEffectRuntime::ConstantBufferPropertyMapping const kSpreadMappings[] = {
		{ 0, 0 },
	};

	CustomEffectRuntime::PropertyDescriptor const kResolveProperties[] = {
		{ L"Mix", 0, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT, nullptr, 1.0f },
	};
	CustomEffectRuntime::NativePropertyMetadata const kResolveMetadata[] = {
		{ "Mix", 0, kDCompositionExpressionTypeScalar, kPropertyTypeSingle, 1, nullptr },
	};
	CustomEffectRuntime::ConstantBufferPropertyMapping const kResolveMappings[] = {
		{ 0, 0 },
	};

	CustomEffectRuntime::SourceDescriptor const kSamplerSources[] = {
		{ L"Source", CustomEffectRuntime::SourceKind::Backdrop, true, true },
	};
	CustomEffectRuntime::SourceDescriptor const kResolveSources[] = {
		{ L"RawSource", CustomEffectRuntime::SourceKind::Backdrop, false, false },
		{ L"BlurredSource", CustomEffectRuntime::SourceKind::Backdrop, false, false },
	};
	std::uint16_t const kSamplerArguments[] = {
		kUvArgument,
		kSamplerDataExtArgument,
		kSamplerDataArgument,
	};
	std::uint16_t const kResolveArguments[] = {
		kColorArgument,
		kColorArgument,
	};

	auto const kKawaseShader = WinUI::Composition::Hlsl::Shaders::KawaseBlurShader();

	CustomEffectRuntime::CustomEffectDefinition const kDownDefinition{
		kKawaseDownEffectId,
		L"KawaseDown",
		"KawaseDownEffect",
		nullptr,
		0,
		kKawaseShader.data,
		kKawaseShader.size,
		"KawaseDown",
		kSamplerSources,
		ARRAYSIZE(kSamplerSources),
		kSpreadProperties,
		ARRAYSIZE(kSpreadProperties),
		kSpreadMetadata,
		ARRAYSIZE(kSpreadMetadata),
		sizeof(float),
		kSpreadMappings,
		ARRAYSIZE(kSpreadMappings),
		kSamplerArguments,
		ARRAYSIZE(kSamplerArguments),
		kCustomSamplerResult,
		CustomEffectRuntime::kShaderProfilePs40,
		sizeof(kSamplerInitial),
		&kSamplerInitial,
		CustomEffectRuntime::CustomEffectInputMode::MaterializedTexture,
		CustomEffectRuntime::GraphLoweringPolicy::MaterializedInput,
		"MaterializeColor",
	};

	CustomEffectRuntime::CustomEffectDefinition const kUpDefinition{
		kKawaseUpEffectId,
		L"KawaseUp",
		"KawaseUpEffect",
		nullptr,
		0,
		kKawaseShader.data,
		kKawaseShader.size,
		"KawaseUp",
		kSamplerSources,
		ARRAYSIZE(kSamplerSources),
		kSpreadProperties,
		ARRAYSIZE(kSpreadProperties),
		kSpreadMetadata,
		ARRAYSIZE(kSpreadMetadata),
		sizeof(float),
		kSpreadMappings,
		ARRAYSIZE(kSpreadMappings),
		kSamplerArguments,
		ARRAYSIZE(kSamplerArguments),
		kCustomSamplerResult,
		CustomEffectRuntime::kShaderProfilePs40,
		sizeof(kSamplerInitial),
		&kSamplerInitial,
		CustomEffectRuntime::CustomEffectInputMode::MaterializedTexture,
		CustomEffectRuntime::GraphLoweringPolicy::MaterializedInput,
		"MaterializeColor",
	};

	CustomEffectRuntime::CustomEffectDefinition const kResolveDefinition{
		kKawaseResolveEffectId,
		CustomKawaseBlurEffect::ResolveEffectName,
		"KawaseResolveEffect",
		nullptr,
		0,
		kKawaseShader.data,
		kKawaseShader.size,
		"KawaseResolve",
		kResolveSources,
		ARRAYSIZE(kResolveSources),
		kResolveProperties,
		ARRAYSIZE(kResolveProperties),
		kResolveMetadata,
		ARRAYSIZE(kResolveMetadata),
		sizeof(float),
		kResolveMappings,
		ARRAYSIZE(kResolveMappings),
		kResolveArguments,
		ARRAYSIZE(kResolveArguments),
		0,
		CustomEffectRuntime::kShaderProfilePs40,
		sizeof(kResolveInitial),
		&kResolveInitial,
		CustomEffectRuntime::CustomEffectInputMode::LinkedColor,
		CustomEffectRuntime::GraphLoweringPolicy::SingleCustom,
		nullptr,
	};

	IGraphicsEffect RenameEffect(IGraphicsEffect const& effect, wchar_t const* effectName)
	{
		if (!effectName) throw hresult_invalid_argument();
		effect.Name(effectName);
		return effect;
	}
}

namespace CustomKawaseBlurEffect
{
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateDownEffect(
		wchar_t const* effectName,
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source)
	{
		if (!source) throw hresult_invalid_argument();
		return RenameEffect(CustomEffectRuntime::CreateEffect(kDownDefinition, source), effectName);
	}

	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateUpEffect(
		wchar_t const* effectName,
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source)
	{
		if (!source) throw hresult_invalid_argument();
		return RenameEffect(CustomEffectRuntime::CreateEffect(kUpDefinition, source), effectName);
	}

	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateResolveEffect(
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& rawSource,
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& blurredSource)
	{
		if (!rawSource || !blurredSource) throw hresult_invalid_argument();
		std::array<IGraphicsEffectSource, 2> sources{ rawSource, blurredSource };
		return CustomEffectRuntime::CreateEffect(
			kResolveDefinition,
			std::span<IGraphicsEffectSource const>{ sources.data(), sources.size() });
	}
}

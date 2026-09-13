#include <unknwn.h>
#include <weakreference.h>
#include <bcrypt.h>
#include <windows.graphics.effects.interop.h>
#include "EffectEngine.h"
#pragma comment(lib,"bcrypt.lib")

import WinUI.Composition.Hlsl.EffectDef;
import WinUI.Composition.Hlsl.CustomEffectRuntime;
import WinUI.Composition.Hlsl.ShaderSource;
import WinUI.Composition.Hlsl.TypedPropertyAbi;
import std;
import winrt_base;
import winrt.Windows.Foundation;

namespace hlsl::engine
{
	namespace
	{
		bool Identifier(std::wstring const& name)
		{
			if (name.empty() || name.size() > 128 || !(iswalpha(name[0]) || name[0] == L'_'))return false;
			for (auto c : name)if (c > 127 || !(iswalnum(c) || c == L'_'))return false;
			return true;
		}

		std::vector<std::wstring> SourceNames(EffectDefinition const& definition)
		{
			return definition.sourceNames.empty()
				? std::vector<std::wstring>{ definition.sourceName }
			: definition.sourceNames;
		}

		std::array<unsigned char, 32> Sha256(std::span<std::uint8_t const> bytes)
		{
			if (bytes.size() > std::numeric_limits<ULONG>::max())
				winrt::throw_hresult(E_INVALIDARG);
			std::array<unsigned char, 32> digest{};
			auto* mutableBytes = const_cast<std::uint8_t*>(bytes.data());
			auto status = BCryptHash(
				BCRYPT_SHA256_ALG_HANDLE,
				nullptr,
				0,
				reinterpret_cast<PUCHAR>(mutableBytes),
				static_cast<ULONG>(bytes.size()),
				digest.data(),
				static_cast<ULONG>(digest.size()));
			if (status < 0) winrt::throw_hresult(E_FAIL);
			return digest;
		}

		std::string Hex(std::span<unsigned char const> bytes)
		{
			static constexpr char digits[] = "0123456789abcdef";
			std::string result(bytes.size() * 2, '\0');
			for (size_t index = 0; index < bytes.size(); ++index)
			{
				result[index * 2] = digits[bytes[index] >> 4];
				result[index * 2 + 1] = digits[bytes[index] & 0x0f];
			}
			return result;
		}

		std::string PayloadFingerprint(EffectDefinition const& definition)
		{
			if (!definition.shaderBytecode.empty())
			{
				auto digest = Sha256(std::span<std::uint8_t const>{ definition.shaderBytecode.data(), definition.shaderBytecode.size() });
				return "dxbc:" + Hex(digest);
			}
			auto const* begin = reinterpret_cast<std::uint8_t const*>(definition.shader.data());
			auto digest = Sha256({ begin,definition.shader.size() });
			return "source:" + Hex(digest);
		}

		std::string Key(EffectDefinition const& definition)
		{
			std::string result = definition.materializedSampler ? "materialized-sampler-v2:" : (definition.sampler ? "sampler-v3:" : "color-v3:");
			auto append = [&](std::string const& value)
				{
					result += std::to_string(value.size()) + ":" + value;
				};
			append(PayloadFingerprint(definition));
			result += ":" + std::to_string(definition.shaderProfile);
			for (auto const& source : SourceNames(definition)) append(winrt::to_string(source));
			append(winrt::to_string(definition.effectName));
			for (auto const& p : definition.properties)
			{
				append(winrt::to_string(p.name));
				result += ":" + std::to_string(static_cast<uint32_t>(p.type));
				for (auto value : p.initial) result += ":" + std::to_string(std::bit_cast<uint32_t>(value));
				result += ":" + std::to_string(std::bit_cast<uint32_t>(p.minimum)) + ":" + std::to_string(std::bit_cast<uint32_t>(p.maximum));
			}
			return result;
		}
		struct Program
		{
			std::string code, key, declarations;
			std::vector<CustomEffectRuntime::PropertyDescriptor> properties;
			std::vector<CustomEffectRuntime::NativePropertyMetadata> metadata;
			std::vector<CustomEffectRuntime::ConstantBufferPropertyMapping> mappings;
			std::vector<std::string> names;
			std::vector<uint8_t> constants;
			std::vector<CustomEffectRuntime::SourceDescriptor> sources;
			std::vector<std::wstring> sourceNames;
			std::vector<uint16_t> arguments;
			CustomEffectRuntime::CustomEffectDefinition native{};
			explicit Program(EffectDefinition const& description) :key(Key(description))
			{
				sourceNames = SourceNames(description);
				if (!description.properties.empty())
				{
					declarations = hlsl::propertyabi::BuildDeclarations(description.properties);
					names.resize(description.properties.size());
					hlsl::engine::PropertyLayoutCursor layoutCursor{};
					for (size_t i = 0; i < description.properties.size(); ++i)
					{
						auto const& p = description.properties[i];
						auto const spec = GetPropertyAbiSpec(p.type);
						auto const layout = AppendPropertyLayout(layoutCursor, p.type);
						if (layout.propertyOffset != layout.constantBufferOffset)
							throw winrt::hresult_error(E_FAIL, L"Typed property and constant-buffer layouts diverged unexpectedly.");
						constants.resize(std::max<std::size_t>(
							constants.size(), layout.constantBufferOffset + layout.propertySize));
						memcpy(constants.data() + layout.constantBufferOffset, p.initial.data(), layout.propertySize);
						names[i] = winrt::to_string(p.name);
						properties.push_back({ p.name.c_str(),static_cast<uint32_t>(i),ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT,nullptr,p.initial[0] });
						metadata.push_back({ names[i].c_str(),layout.propertyOffset,spec.expressionType,8,spec.valueCount,nullptr });
						mappings.push_back({ static_cast<uint32_t>(i),layout.constantBufferOffset });
					}
					constants.resize(FinalConstantBufferSize(layoutCursor));
					native.propertiesStructSize = FinalPropertyStructSize(layoutCursor);
				}
				if (description.shaderBytecode.empty())
				{
					code = hlsl::compiler::BuildPublicShaderSource(
						declarations, description.shader, description.sampler,
						description.materializedSampler, sourceNames.size());
				}
				for (size_t index = 0; index < sourceNames.size(); ++index)
				{
					if (description.sampler)
					{
						arguments.push_back(0x0100);
						arguments.push_back(0x0400);
						if (description.materializedSampler) arguments.push_back(0x0300);
					}
					else
					{
						arguments.push_back(0x0200);
					}
					sources.push_back({
						sourceNames[index].c_str(),
						CustomEffectRuntime::SourceKind::Backdrop,
						description.materializedSampler,
						description.sampler });
				}
				native.descriptorKey = key.c_str(); native.id = description.id; native.effectName = description.effectName.c_str(); native.fragmentName = "AppHlslEffect";
				if (description.shaderBytecode.empty())
				{
					native.shaderSource = code.c_str(); native.shaderSourceSize = code.size();
				}
				else
				{
					native.shaderBytecode = description.shaderBytecode.data(); native.shaderBytecodeSize = description.shaderBytecode.size();
				}
				native.shaderFunctionName = "PSBody";
				native.sources = sources.data(); native.sourceCount = static_cast<uint32_t>(sources.size()); native.properties = properties.data(); native.propertyCount = static_cast<uint32_t>(properties.size());
				native.nativePropertyMetadata = metadata.data(); native.nativePropertyMetadataCount = static_cast<uint32_t>(metadata.size());
				if (description.properties.empty()) native.propertiesStructSize = 0;
				native.constantBufferProperties = mappings.data(); native.constantBufferPropertyCount = static_cast<uint32_t>(mappings.size());
				native.constantBufferSize = static_cast<uint32_t>(constants.size()); native.constantBufferInitialValue = constants.data();
				native.shaderArguments = arguments.data();
				native.shaderArgumentCount = arguments.size();
				native.linkingArgType = description.sampler ? 0x0200 : 0;
				native.shaderProfileVersion = description.shaderProfile;
				native.inputMode = description.materializedSampler
					? CustomEffectRuntime::CustomEffectInputMode::MaterializedTexture
					: CustomEffectRuntime::CustomEffectInputMode::LinkedColor;
				native.graphPolicy = description.materializedSampler
					? CustomEffectRuntime::GraphLoweringPolicy::MaterializedInput
					: CustomEffectRuntime::GraphLoweringPolicy::SingleCustom;
				native.materializationShaderFunctionName = description.materializedSampler ? "MaterializeColor" : nullptr;
			}
		};
		struct CachedFactory
		{
			winrt::weak_ref<winrt::Microsoft::UI::Composition::Compositor> compositor;
			winrt::weak_ref<winrt::Microsoft::UI::Composition::CompositionEffectFactory> factory;
			winrt::guid id;
		};
		thread_local std::vector<CachedFactory> factories;

		struct XamlBrush : winrt::Microsoft::UI::Xaml::Media::XamlCompositionBrushBaseT<XamlBrush>
		{
			winrt::Microsoft::UI::Composition::CompositionBrush value{ nullptr };
			explicit XamlBrush(winrt::Microsoft::UI::Composition::CompositionBrush const& brush) :value(brush)
			{
			}
			void OnConnected()
			{
				CompositionBrush(value);
			}
			void OnDisconnected()
			{
				CompositionBrush(nullptr);
			}
		};
	}
	void Validate(EffectDefinition const& definition)
	{
		auto const hasSource = !definition.shader.empty();
		auto const hasBytecode = !definition.shaderBytecode.empty();
		if (hasSource == hasBytecode || definition.shader.size() > 1024 * 1024 || definition.shader.find('\0') != std::string::npos ||
			definition.shaderBytecode.size() > 16 * 1024 * 1024)
			throw winrt::hresult_invalid_argument(L"Specify exactly one valid HLSL source or DXBC library payload.");
		if (definition.materializedSampler && !definition.sampler)
			throw winrt::hresult_invalid_argument(L"MaterializedTexture lowering is valid only for sampler effects.");
		auto sources = SourceNames(definition);
		if (sources.empty() || sources.size() > 16 || definition.properties.size() > 64)
			throw winrt::hresult_invalid_argument(L"An effect supports 1-16 sources and up to 64 properties.");
		std::set<std::wstring> names;
		for (auto const& source : sources)
			if (!Identifier(source) || !names.insert(source).second)
				throw winrt::hresult_invalid_argument(L"Invalid or duplicate source name.");
		names.clear();
		for (auto const& p : definition.properties)
		{
			auto const spec = GetPropertyAbiSpec(p.type);
			if (!Identifier(p.name) || !names.insert(p.name).second || p.name == L"PSBody" || p.name == L"Shade" ||
				p.initial.size() != spec.valueCount || !std::ranges::all_of(p.initial, [](float value)
																		  {
																			  return std::isfinite(value);
																		  }) ||
				!std::isfinite(p.minimum) || !std::isfinite(p.maximum) || p.minimum > p.maximum ||
																			  (p.type == PropertyType::Scalar && (p.initial[0] < p.minimum || p.initial[0] > p.maximum)))
				throw winrt::hresult_invalid_argument(L"Invalid or duplicate property definition.");
		}
	}
	winrt::guid DeriveId(EffectDefinition const& definition)
	{
		auto key = Key(definition);
		auto const* begin = reinterpret_cast<std::uint8_t const*>(key.data());
		auto digest = Sha256({ begin,key.size() });
		winrt::guid id{}; memcpy(&id, digest.data(), sizeof(id)); return id;
	}
	winrt::Windows::Graphics::Effects::IGraphicsEffect Compile(std::shared_ptr<EffectDefinition const> const& definition)
	{
		if (definition->nativeTemplate)return CustomEffectRuntime::CreateEffect(*definition->nativeTemplate);
		Validate(*definition);
		Program program{ *definition };
		return CustomEffectRuntime::CreateEffect(program.native);
	}
	winrt::Windows::Graphics::Effects::IGraphicsEffect Compile(
		std::shared_ptr<EffectDefinition const> const& definition,
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source)
	{
		if (!source) throw winrt::hresult_invalid_argument();
		return Compile(definition, std::span<winrt::Windows::Graphics::Effects::IGraphicsEffectSource const>{ &source, 1 });
	}
	winrt::Windows::Graphics::Effects::IGraphicsEffect Compile(
		std::shared_ptr<EffectDefinition const> const& definition,
		std::span<winrt::Windows::Graphics::Effects::IGraphicsEffectSource const> sources)
	{
		if (!definition || sources.empty())throw winrt::hresult_invalid_argument();
		if (definition->nativeTemplate)return CustomEffectRuntime::CreateEffect(*definition->nativeTemplate, sources);
		Validate(*definition);
		Program program{ *definition };
		return CustomEffectRuntime::CreateEffect(program.native, sources);
	}
	winrt::Microsoft::UI::Composition::CompositionEffectFactory GetFactory(winrt::Microsoft::UI::Composition::Compositor const& compositor, std::shared_ptr<EffectDefinition const> const& definition)
	{
		if (!compositor || !definition)throw winrt::hresult_invalid_argument();
		auto effect = Compile(definition);
		for (auto it = factories.begin(); it != factories.end();)
		{
			auto owner = it->compositor.get(); auto factory = it->factory.get();
			if (!owner || !factory)
			{
				it = factories.erase(it); continue;
			}
			if (owner == compositor && it->id == definition->id)return factory;
			++it;
		}
		auto paths = winrt::single_threaded_vector<winrt::hstring>();
		for (auto const& p : definition->properties) paths.Append(winrt::hstring{ definition->effectName + L"." + p.name });
		auto factory = compositor.CreateEffectFactory(effect, paths);
		if (compositor.try_as<::IWeakReferenceSource>() && factory.try_as<::IWeakReferenceSource>())
			factories.push_back({ winrt::make_weak(compositor),winrt::make_weak(factory),definition->id });
		return factory;
	}
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateColorEffect(winrt::guid const& id, std::string_view shader)
	{
		auto definition = std::make_shared<EffectDefinition>(); definition->id = id; definition->shader = shader; return Compile(definition);
	}
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateSamplerEffect(winrt::guid const& id, std::string_view shader)
	{
		auto definition = std::make_shared<EffectDefinition>(); definition->id = id; definition->shader = shader; definition->sampler = true; return Compile(definition);
	}
	winrt::Microsoft::UI::Composition::CompositionEffectBrush CreateBackdropBrush(winrt::Microsoft::UI::Composition::Compositor const& compositor, winrt::Windows::Graphics::Effects::IGraphicsEffect const& effect)
	{
		auto brush = compositor.CreateEffectFactory(effect).CreateBrush(); brush.SetSourceParameter(L"Backdrop", compositor.CreateBackdropBrush()); return brush;
	}
	winrt::Microsoft::UI::Xaml::Media::XamlCompositionBrushBase AsXamlBrush(winrt::Microsoft::UI::Composition::CompositionBrush const& brush)
	{
		if (!brush)throw winrt::hresult_invalid_argument(); return winrt::make<XamlBrush>(brush);
	}
}

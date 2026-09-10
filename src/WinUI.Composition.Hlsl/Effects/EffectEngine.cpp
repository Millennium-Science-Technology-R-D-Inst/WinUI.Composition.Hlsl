#include "pch.h"
#include "EffectEngine.h"
#include "EffectDefinition.h"
#include <bcrypt.h>
#include <weakreference.h>
#include <array>
#include <bit>
#include <cmath>
#include <set>
#include <sstream>
#pragma comment(lib,"bcrypt.lib")
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
		std::string Key(EffectDefinition const& definition)
		{
			std::string result=definition.sampler ? "sampler-v1:" : "color-v1:";
			auto append=[&](std::string const& value)
				{
					result+=std::to_string(value.size()) + ":" + value;
				};
			append(definition.shader); append(winrt::to_string(definition.sourceName)); append(winrt::to_string(definition.effectName));
			for (auto const& p : definition.properties)
			{
				append(winrt::to_string(p.name));
				result+=":" + std::to_string(std::bit_cast<uint32_t>(p.initial)) + ":" + std::to_string(std::bit_cast<uint32_t>(p.minimum)) + ":" + std::to_string(std::bit_cast<uint32_t>(p.maximum));
			}
			return result;
		}
		struct Program
		{
			std::string code, key;
			std::vector<CustomEffectRuntime::PropertyDescriptor> properties;
			std::vector<CustomEffectRuntime::NativePropertyMetadata> metadata;
			std::vector<CustomEffectRuntime::ConstantBufferPropertyMapping> mappings;
			std::vector<std::string> names;
			std::vector<float> constants;
			CustomEffectRuntime::SourceDescriptor source{};
			uint16_t arguments[2]{ 0x0200,0 };
			CustomEffectRuntime::CustomEffectDefinition native{};
			explicit Program(EffectDefinition const& description) :key(Key(description))
			{
				if (description.sampler)code="Texture2D texture0; SamplerState sampler0;\n";
				if (!description.properties.empty())
				{
					code+="cbuffer UserConstants : register(b0) {\n";
					names.resize(description.properties.size());
					constants.resize((description.properties.size() + 3) / 4 * 4);
					for (size_t i=0; i < description.properties.size(); ++i)
					{
						auto const& p=description.properties[i];
						names[i]=winrt::to_string(p.name); code+="float " + names[i] + ";\n";
						properties.push_back({ p.name.c_str(),static_cast<uint32_t>(i),ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT,nullptr,p.initial });
						constants[i]=p.initial;
						mappings.push_back({ static_cast<uint32_t>(i),static_cast<uint32_t>(i * 4) });
					}
					for (size_t i=0; i < names.size(); ++i)metadata.push_back({ names[i].c_str(),static_cast<uint32_t>(i * 4),18,8,1,nullptr });
					code+="};\n";
				}
				code+="#line 1 \"UserShader.hlsl\"\n" + description.shader;
				if (description.sampler)
				{
					for (auto suffix : { "","CC","CW","CM","WC","WW","WM","MC","MW","MM","C","W","M" })
						code+="\n#line 1 \"GeneratedShader.hlsl\"\nexport float4 PSBody" + std::string(suffix) + "(float2 uv,float4 info){return Shade(uv,info);}\n";
					arguments[0]=0x0100; arguments[1]=0x0400;
				}
				source={ description.sourceName.c_str(),CustomEffectRuntime::SourceKind::Backdrop,false,description.sampler };
				native.descriptorKey=key.c_str(); native.id=description.id; native.effectName=description.effectName.c_str(); native.fragmentName="AppHlslEffect";
				native.shaderSource=code.c_str(); native.shaderSourceSize=code.size(); native.shaderFunctionName="PSBody";
				native.sources=&source; native.sourceCount=1; native.properties=properties.data(); native.propertyCount=static_cast<uint32_t>(properties.size());
				native.nativePropertyMetadata=metadata.data(); native.nativePropertyMetadataCount=static_cast<uint32_t>(metadata.size());
				native.propertiesStructSize=static_cast<uint32_t>(constants.size() * 4);
				native.constantBufferProperties=mappings.data(); native.constantBufferPropertyCount=static_cast<uint32_t>(mappings.size());
				native.constantBufferSize=static_cast<uint32_t>(constants.size() * 4); native.constantBufferInitialValue=constants.data();
				native.shaderArguments=arguments; native.shaderArgumentCount=description.sampler ? 2 : 1; native.linkingArgType=description.sampler ? 0x0200 : 0;
				native.shaderProfileVersion=CustomEffectRuntime::kShaderProfileLevel93;
			}
		};
		struct CachedFactory
		{
			winrt::Microsoft::UI::Composition::Compositor compositor{ nullptr };
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
		if (definition.shader.empty() || definition.shader.size() > 1024 * 1024 || definition.shader.find('\0') != std::string::npos)
			throw winrt::hresult_invalid_argument(L"Shader source must contain 1 to 1048576 UTF-8 bytes without NUL.");
		if (!Identifier(definition.sourceName) || definition.properties.size() > 64)throw winrt::hresult_invalid_argument(L"Invalid source name or too many scalar properties.");
		std::set<std::wstring> names;
		for (auto const& p : definition.properties)
		{
			if (!Identifier(p.name) || !names.insert(p.name).second || p.name == L"texture0" || p.name == L"sampler0" || p.name == L"PSBody" || p.name == L"Shade" ||
				!std::isfinite(p.initial) || !std::isfinite(p.minimum) || !std::isfinite(p.maximum) || p.minimum > p.maximum || p.initial<p.minimum || p.initial>p.maximum)
				throw winrt::hresult_invalid_argument(L"Invalid or duplicate scalar property definition.");
		}
	}
	winrt::guid DeriveId(EffectDefinition const& definition)
	{
		auto key=Key(definition); std::array<unsigned char, 32> digest{};
		auto status=BCryptHash(BCRYPT_SHA256_ALG_HANDLE, nullptr, 0, reinterpret_cast<PUCHAR>(key.data()), static_cast<ULONG>(key.size()), digest.data(), 32);
		if (status < 0)winrt::throw_hresult(E_FAIL);
		winrt::guid id{}; memcpy(&id, digest.data(), sizeof(id)); return id;
	}
	winrt::Windows::Graphics::Effects::IGraphicsEffect Compile(Definition const& definition)
	{
		if (definition->nativeTemplate)return CustomEffectRuntime::CreateEffect(*definition->nativeTemplate);
		Validate(*definition);
		Program program{ *definition };
		return CustomEffectRuntime::CreateEffect(program.native);
	}
	winrt::Microsoft::UI::Composition::CompositionEffectFactory GetFactory(winrt::Microsoft::UI::Composition::Compositor const& compositor, Definition const& definition)
	{
		if (!compositor || !definition)throw winrt::hresult_invalid_argument();
		// Always register/validate before cache lookup, including explicit-GUID collisions.
		auto effect=Compile(definition);
		for (auto it=factories.begin(); it != factories.end();)
		{
			auto owner=it->compositor; auto factory=it->factory.get();
			if (!owner || !factory)
			{
				it=factories.erase(it); continue;
			}
			if (owner == compositor && it->id == definition->id)return factory;
			++it;
		}
		auto paths=winrt::single_threaded_vector<winrt::hstring>();
		for (auto const& p : definition->properties)paths.Append(definition->effectName + L"." + p.name);
		auto factory=compositor.CreateEffectFactory(effect, paths);
		if (factory.try_as<::IWeakReferenceSource>())
			factories.push_back({ compositor,winrt::make_weak(factory),definition->id });
		return factory;
	}
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateColorEffect(winrt::guid const& id, std::string_view shader)
	{
		auto definition=std::make_shared<EffectDefinition>(); definition->id=id; definition->shader=shader; return Compile(definition);
	}
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateSamplerEffect(winrt::guid const& id, std::string_view shader)
	{
		auto definition=std::make_shared<EffectDefinition>(); definition->id=id; definition->shader=shader; definition->sampler=true; return Compile(definition);
	}
	winrt::Microsoft::UI::Composition::CompositionEffectBrush CreateBackdropBrush(winrt::Microsoft::UI::Composition::Compositor const& compositor, winrt::Windows::Graphics::Effects::IGraphicsEffect const& effect)
	{
		auto brush=compositor.CreateEffectFactory(effect).CreateBrush(); brush.SetSourceParameter(L"Backdrop", compositor.CreateBackdropBrush()); return brush;
	}
	winrt::Microsoft::UI::Xaml::Media::XamlCompositionBrushBase AsXamlBrush(winrt::Microsoft::UI::Composition::CompositionBrush const& brush)
	{
		if (!brush)throw winrt::hresult_invalid_argument(); return winrt::make<XamlBrush>(brush);
	}
}

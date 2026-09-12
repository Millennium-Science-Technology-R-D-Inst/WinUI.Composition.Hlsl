#include "HlslEffect.h"
#include "HlslEffect.g.cpp"
#include "HlslShaderLibrary.h"
import WinUI.Composition.Hlsl.CustomEffectRuntime;
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	namespace
	{
		void AppendProperties(
			hlsl::engine::EffectDefinition& definition,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
		{
			if (!properties) return;
			for (auto const& property : properties)
			{
				if (!property) throw hresult_invalid_argument(L"A property descriptor is null.");
				definition.properties.push_back({ std::wstring(property.Name()),property.DefaultValue(),property.Minimum(),property.Maximum() });
			}
		}

		Hlsl::HlslEffect Describe(
			hstring const& shader,
			bool sampler,
			winrt::guid id,
			hstring const& sourceName,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties=nullptr)
		{
			auto definition=std::make_shared<hlsl::engine::EffectDefinition>();
			definition->shader=to_string(shader); definition->sampler=sampler; definition->sourceName=sourceName;
			AppendProperties(*definition, properties);
			hlsl::engine::Validate(*definition);
			definition->id=id == winrt::guid{} ? hlsl::engine::DeriveId(*definition) : id;
			return make<HlslEffect>(definition);
		}

		Hlsl::HlslEffect DescribeCompiled(
			Hlsl::HlslShaderLibrary const& shader,
			bool sampler,
			winrt::guid id,
			hstring const& sourceName=L"Backdrop",
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties=nullptr)
		{
			if (!shader) throw hresult_invalid_argument(L"The shader library is null.");
			auto library = get_self<HlslShaderLibrary>(shader);
			auto definition = std::make_shared<hlsl::engine::EffectDefinition>();
			definition->sampler = sampler;
			definition->sourceName = sourceName;
			definition->shaderBytecode = library->BytecodeBytes();
			switch (library->Profile())
			{
				case Hlsl::HlslShaderProfile::Level91: definition->shaderProfile = CustomEffectRuntime::kShaderProfileLevel91; break;
				case Hlsl::HlslShaderProfile::Level93: definition->shaderProfile = CustomEffectRuntime::kShaderProfileLevel93; break;
				case Hlsl::HlslShaderProfile::Pixel40: definition->shaderProfile = CustomEffectRuntime::kShaderProfilePs40; break;
				default: throw hresult_invalid_argument(L"Unknown shader profile.");
			}
			AppendProperties(*definition, properties);
			std::vector<std::wstring> propertyNames;
			propertyNames.reserve(definition->properties.size());
			for (auto const& property : definition->properties)
			{
				propertyNames.push_back(property.name);
			}
			library->ValidateForEffect(sampler, propertyNames);
			hlsl::engine::Validate(*definition);
			definition->id = id == winrt::guid{} ? hlsl::engine::DeriveId(*definition) : id;
			return make<HlslEffect>(definition);
		}
	}
	Hlsl::HlslEffect HlslEffect::CreateColor(winrt::guid const& id, hstring const& shader)
	{
		return Describe(shader, false, id, L"Backdrop");
	}
	Hlsl::HlslEffect HlslEffect::CreateSampler(winrt::guid const& id, hstring const& shader)
	{
		return Describe(shader, true, id, L"Backdrop");
	}
	Hlsl::HlslEffect HlslEffect::CreateColorTransform(hstring const& shader)
	{
		return Describe(shader, false, {}, L"Backdrop");
	}
	Hlsl::HlslEffect HlslEffect::CreateCustomSampler(hstring const& shader)
	{
		return Describe(shader, true, {}, L"Backdrop");
	}
	Hlsl::HlslEffect HlslEffect::CreateCompiledColor(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader)
	{
		return DescribeCompiled(shader, false, id);
	}
	Hlsl::HlslEffect HlslEffect::CreateCompiledSampler(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader)
	{
		return DescribeCompiled(shader, true, id);
	}
	Hlsl::HlslEffect HlslEffect::CreateColorWithProperties(hstring const& shader, hstring const& sourceName, Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
	{
		return Describe(shader, false, {}, sourceName, properties);
	}
	Hlsl::HlslEffect HlslEffect::CreateSamplerWithProperties(hstring const& shader, hstring const& sourceName, Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
	{
		return Describe(shader, true, {}, sourceName, properties);
	}
	Hlsl::HlslEffect HlslEffect::CreateCompiledColorWithProperties(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader, hstring const& sourceName, Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
	{
		return DescribeCompiled(shader, false, id, sourceName, properties);
	}
	Hlsl::HlslEffect HlslEffect::CreateCompiledSamplerWithProperties(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader, hstring const& sourceName, Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
	{
		return DescribeCompiled(shader, true, id, sourceName, properties);
	}
	Windows::Foundation::Collections::IVectorView<hstring> HlslEffect::PropertyNames() const
	{
		auto names = single_threaded_vector<hstring>();
		for (auto const& property : m_definition->properties)
		{
			names.Append(property.name);
		}
		return names.GetView();
	}
	Windows::Graphics::Effects::IGraphicsEffect HlslEffect::CreateGraphicsEffect() const
	{
		return hlsl::engine::Compile(m_definition);
	}
	Windows::Graphics::Effects::IGraphicsEffect HlslEffect::CreateGraphicsEffectWithSource(Windows::Graphics::Effects::IGraphicsEffectSource const& source) const
	{
		if (!source) throw hresult_invalid_argument(L"The graphics-effect source is null.");
		return hlsl::engine::Compile(m_definition, source);
	}
}

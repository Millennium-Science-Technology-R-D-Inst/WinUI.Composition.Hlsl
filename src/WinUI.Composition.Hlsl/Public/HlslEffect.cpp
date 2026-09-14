#include "HlslEffect.h"
#include "HlslEffect.g.cpp"
#include "HlslShaderLibrary.h"
#include "HlslProperty.h"
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

		void ApplyKind(hlsl::engine::EffectDefinition& definition, Hlsl::HlslEffectKind kind)
		{
			switch (kind)
			{
				case Hlsl::HlslEffectKind::Color:
					definition.sampler = false;
					definition.materializedSampler = false;
					break;
				case Hlsl::HlslEffectKind::Sampler:
					definition.sampler = true;
					definition.materializedSampler = false;
					break;
				case Hlsl::HlslEffectKind::MaterializedSampler:
					definition.sampler = true;
					definition.materializedSampler = true;
					break;
				default:
					throw hresult_invalid_argument(L"Unknown HLSL effect kind.");
			}
		}

		void ApplySources(hlsl::engine::EffectDefinition& definition,
						  Windows::Foundation::Collections::IVectorView<hstring> const& sources)
		{
			if (!sources || !sources.Size()) throw hresult_invalid_argument(L"At least one source name is required.");
			definition.sourceNames.reserve(sources.Size());
			for (auto const& source : sources) definition.sourceNames.emplace_back(source);
			definition.sourceName = definition.sourceNames.front();
			if (definition.materializedSampler && definition.sourceNames.size() != 1)
			{
				throw hresult_invalid_argument(L"MaterializedSampler currently supports exactly one source.");
			}
		}

		void AppendAdvancedProperties(hlsl::engine::EffectDefinition& definition,
									  Windows::Foundation::Collections::IVectorView<Hlsl::HlslProperty> const& properties)
		{
			if (!properties) return;
			for (auto const& projected : properties)
			{
				if (!projected) throw hresult_invalid_argument(L"A property descriptor is null.");
				auto property = get_self<HlslProperty>(projected);
				definition.properties.emplace_back(
					std::wstring(property->Name()),
					static_cast<hlsl::engine::PropertyType>(property->Type()),
					property->Values());
			}
		}

		Hlsl::HlslEffect Describe(
			hstring const& shader,
			Hlsl::HlslEffectKind kind,
			winrt::guid id,
			hstring const& sourceName,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties = nullptr)
		{
			auto definition = std::make_shared<hlsl::engine::EffectDefinition>();
			definition->shader = to_string(shader); definition->sourceName = sourceName;
			ApplyKind(*definition, kind);
			AppendProperties(*definition, properties);
			hlsl::engine::Validate(*definition);
			definition->id = id == winrt::guid{} ? hlsl::engine::DeriveId(*definition) : id;
			return make<HlslEffect>(definition);
		}

		Hlsl::HlslEffect DescribeCompiled(
			Hlsl::HlslShaderLibrary const& shader,
			Hlsl::HlslEffectKind kind,
			winrt::guid id,
			hstring const& sourceName = L"Backdrop",
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties = nullptr)
		{
			if (!shader) throw hresult_invalid_argument(L"The shader library is null.");
			auto library = get_self<HlslShaderLibrary>(shader);
			auto definition = std::make_shared<hlsl::engine::EffectDefinition>();
			ApplyKind(*definition, kind);
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
			library->ValidateForEffect(kind, 1, propertyNames);
			hlsl::engine::Validate(*definition);
			definition->id = id == winrt::guid{} ? hlsl::engine::DeriveId(*definition) : id;
			return make<HlslEffect>(definition);
		}
	}

	Hlsl::HlslEffect HlslEffect::CreateColor(winrt::guid const& id, hstring const& shader)
	{
		return Describe(shader, Hlsl::HlslEffectKind::Color, id, L"Backdrop");
	}
	Hlsl::HlslEffect HlslEffect::CreateSampler(winrt::guid const& id, hstring const& shader)
	{
		return Describe(shader, Hlsl::HlslEffectKind::Sampler, id, L"Backdrop");
	}
	Hlsl::HlslEffect HlslEffect::CreateMaterializedSampler(winrt::guid const& id, hstring const& shader)
	{
		return Describe(shader, Hlsl::HlslEffectKind::MaterializedSampler, id, L"Backdrop");
	}
	Hlsl::HlslEffect HlslEffect::CreateColorTransform(hstring const& shader)
	{
		return Describe(shader, Hlsl::HlslEffectKind::Color, {}, L"Backdrop");
	}
	Hlsl::HlslEffect HlslEffect::CreateCustomSampler(hstring const& shader)
	{
		return Describe(shader, Hlsl::HlslEffectKind::Sampler, {}, L"Backdrop");
	}
	Hlsl::HlslEffect HlslEffect::CreateCustomMaterializedSampler(hstring const& shader)
	{
		return Describe(shader, Hlsl::HlslEffectKind::MaterializedSampler, {}, L"Backdrop");
	}
	Hlsl::HlslEffect HlslEffect::CreateCompiled(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader)
	{
		if (!shader) throw hresult_invalid_argument(L"The shader library is null.");
		auto library = get_self<HlslShaderLibrary>(shader);
		return DescribeCompiled(shader, library->EffectKind(), id);
	}
	Hlsl::HlslEffect HlslEffect::CreateCompiledFromGeneratedByteArray(
		winrt::guid const& id,
		winrt::array_view<std::uint8_t const> bytecode)
	{
		auto library = HlslShaderLibrary::CreateFromGeneratedByteArray(bytecode);
		return CreateCompiled(id, library);
	}
	Hlsl::HlslEffect HlslEffect::CreateCompiledColor(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader)
	{
		return DescribeCompiled(shader, Hlsl::HlslEffectKind::Color, id);
	}
	Hlsl::HlslEffect HlslEffect::CreateCompiledSampler(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader)
	{
		return DescribeCompiled(shader, Hlsl::HlslEffectKind::Sampler, id);
	}
	Hlsl::HlslEffect HlslEffect::CreateCompiledMaterializedSampler(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader)
	{
		return DescribeCompiled(shader, Hlsl::HlslEffectKind::MaterializedSampler, id);
	}
	Hlsl::HlslEffect HlslEffect::CreateColorWithProperties(hstring const& shader, hstring const& sourceName, Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
	{
		return Describe(shader, Hlsl::HlslEffectKind::Color, {}, sourceName, properties);
	}
	Hlsl::HlslEffect HlslEffect::CreateSamplerWithProperties(hstring const& shader, hstring const& sourceName, Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
	{
		return Describe(shader, Hlsl::HlslEffectKind::Sampler, {}, sourceName, properties);
	}
	Hlsl::HlslEffect HlslEffect::CreateMaterializedSamplerWithProperties(hstring const& shader, hstring const& sourceName, Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
	{
		return Describe(shader, Hlsl::HlslEffectKind::MaterializedSampler, {}, sourceName, properties);
	}
	Hlsl::HlslEffect HlslEffect::CreateCompiledColorWithProperties(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader, hstring const& sourceName, Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
	{
		return DescribeCompiled(shader, Hlsl::HlslEffectKind::Color, id, sourceName, properties);
	}
	Hlsl::HlslEffect HlslEffect::CreateCompiledSamplerWithProperties(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader, hstring const& sourceName, Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
	{
		return DescribeCompiled(shader, Hlsl::HlslEffectKind::Sampler, id, sourceName, properties);
	}
	Hlsl::HlslEffect HlslEffect::CreateCompiledMaterializedSamplerWithProperties(winrt::guid const& id, Hlsl::HlslShaderLibrary const& shader, hstring const& sourceName, Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
	{
		return DescribeCompiled(shader, Hlsl::HlslEffectKind::MaterializedSampler, id, sourceName, properties);
	}
	Hlsl::HlslEffect HlslEffect::CreateAdvanced(
		hstring const& shader,
		Hlsl::HlslEffectKind kind,
		Windows::Foundation::Collections::IVectorView<hstring> const& sourceNames,
		Windows::Foundation::Collections::IVectorView<Hlsl::HlslProperty> const& properties)
	{
		auto definition = std::make_shared<hlsl::engine::EffectDefinition>();
		definition->shader = to_string(shader);
		ApplyKind(*definition, kind);
		ApplySources(*definition, sourceNames);
		AppendAdvancedProperties(*definition, properties);
		hlsl::engine::Validate(*definition);
		definition->id = hlsl::engine::DeriveId(*definition);
		return make<HlslEffect>(definition);
	}
	Hlsl::HlslEffect HlslEffect::CreateCompiledAdvanced(
		winrt::guid const& id,
		Hlsl::HlslShaderLibrary const& shader,
		Hlsl::HlslEffectKind kind,
		Windows::Foundation::Collections::IVectorView<hstring> const& sourceNames,
		Windows::Foundation::Collections::IVectorView<Hlsl::HlslProperty> const& properties)
	{
		if (!shader) throw hresult_invalid_argument(L"The shader library is null.");
		auto library = get_self<HlslShaderLibrary>(shader);
		auto definition = std::make_shared<hlsl::engine::EffectDefinition>();
		ApplyKind(*definition, kind);
		ApplySources(*definition, sourceNames);
		AppendAdvancedProperties(*definition, properties);
		definition->shaderBytecode = library->BytecodeBytes();
		definition->shaderProfile = static_cast<uint8_t>(library->Profile());
		library->ValidateForEffect(
			kind,
			static_cast<std::uint32_t>(definition->sourceNames.size()),
			{}
		);
		hlsl::engine::Validate(*definition);
		definition->id = id == winrt::guid{} ? hlsl::engine::DeriveId(*definition) : id;
		return make<HlslEffect>(definition);
	}
	Windows::Foundation::Collections::IVectorView<hstring> HlslEffect::PropertyNames() const
	{
		auto names = single_threaded_vector<hstring>();
		for (auto const& property : m_definition->properties)
		{
			names.Append(hstring{ property.name });
		}
		return names.GetView();
	}
	hstring HlslEffect::GetPropertyPath(hstring const& name) const
	{
		for (auto const& property : m_definition->properties)
		{
			if (name == property.name)
			{
				return hstring{ m_definition->effectName + L"." + property.name };
			}
		}
		throw hresult_invalid_argument(L"The float property is not declared by this effect.");
	}
	Windows::Foundation::Collections::IVectorView<hstring> HlslEffect::SourceNames() const
	{
		auto result = single_threaded_vector<hstring>();
		if (m_definition->sourceNames.empty()) result.Append(hstring{ m_definition->sourceName });
		else for (auto const& name : m_definition->sourceNames) result.Append(hstring{ name });
		return result.GetView();
	}
	Windows::Foundation::Collections::IVectorView<hstring> HlslEffect::GetAnimatablePropertyPaths() const
	{
		auto paths = single_threaded_vector<hstring>();
		for (auto const& property : m_definition->properties)
		{
			paths.Append(hstring{ m_definition->effectName + L"." + property.name });
		}
		return paths.GetView();
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
	Windows::Graphics::Effects::IGraphicsEffect HlslEffect::CreateGraphicsEffectWithSources(
		Windows::Foundation::Collections::IVectorView<Windows::Graphics::Effects::IGraphicsEffectSource> const& sources) const
	{
		if (!sources || !sources.Size()) throw hresult_invalid_argument(L"At least one graphics-effect source is required.");
		std::vector<Windows::Graphics::Effects::IGraphicsEffectSource> values;
		values.reserve(sources.Size());
		for (auto const& source : sources)
		{
			if (!source) throw hresult_invalid_argument(L"A graphics-effect source is null.");
			values.push_back(source);
		}
		return hlsl::engine::Compile(m_definition, values);
	}
}

#include "pch.h"
#include "HlslEffect.h"
#include "HlslEffect.g.cpp"
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	namespace
	{
		Hlsl::HlslEffect Describe(hstring const& shader, bool sampler, winrt::guid id, hstring const& sourceName,
								  Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties=nullptr)
		{
			auto definition=std::make_shared<hlsl::engine::EffectDefinition>();
			definition->shader=to_string(shader); definition->sampler=sampler; definition->sourceName=sourceName;
			if (properties) for (auto const& p : properties)
			{
				if (!p) throw hresult_invalid_argument(L"A property descriptor is null.");
				definition->properties.push_back({ std::wstring(p.Name()),p.DefaultValue(),p.Minimum(),p.Maximum() });
			}
			hlsl::engine::Validate(*definition);
			definition->id=id == winrt::guid{} ? hlsl::engine::DeriveId(*definition) : id;
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
	Hlsl::HlslEffect HlslEffect::CreateColorWithProperties(hstring const& shader, hstring const& sourceName, Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
	{
		return Describe(shader, false, {}, sourceName, properties);
	}
	Hlsl::HlslEffect HlslEffect::CreateSamplerWithProperties(hstring const& shader, hstring const& sourceName, Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties)
	{
		return Describe(shader, true, {}, sourceName, properties);
	}
}

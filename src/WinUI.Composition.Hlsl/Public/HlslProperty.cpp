#include "HlslProperty.h"
#include "HlslProperty.g.cpp"

import WinUI.Composition.Hlsl.EffectDef;

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	namespace
	{
		hlsl::engine::PropertyType ToNative(Hlsl::HlslPropertyType type)
		{
			return static_cast<hlsl::engine::PropertyType>(type);
		}
	}

	HlslProperty::HlslProperty(hstring const& name, Hlsl::HlslPropertyType type,
							   Windows::Foundation::Collections::IVectorView<float> const& defaultValue) :
		m_name(name), m_type(type)
	{
		// The private direct-property updater metadata has only been verified for
		// scalar values. Vector/matrix enum values and brush setter projections are
		// retained for ABI evolution, but accepting them here would make an
		// unverified private Composition path appear supported. Fail closed until
		// their native metadata/property-value contracts are validated end to end.
		if (type != Hlsl::HlslPropertyType::Scalar)
		{
			throw hresult_not_implemented(
				L"Vector and matrix HLSL properties are not yet validated by the private Composition property-updater ABI. Only Scalar is currently supported.");
		}
		if (!defaultValue) throw hresult_invalid_argument(L"A property default value is required.");
		m_values.reserve(defaultValue.Size());
		for (auto value : defaultValue) m_values.push_back(value);
		hlsl::engine::EffectDefinition definition;
		definition.shader = "validation";
		definition.properties.emplace_back(std::wstring(name), ToNative(type), m_values);
		hlsl::engine::Validate(definition);
	}

	Windows::Foundation::Collections::IVectorView<float> HlslProperty::DefaultValue() const
	{
		auto result = single_threaded_vector<float>();
		for (auto value : m_values) result.Append(value);
		return result.GetView();
	}
}

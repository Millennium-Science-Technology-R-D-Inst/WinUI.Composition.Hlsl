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
		if (!defaultValue) throw hresult_invalid_argument(L"A property default value is required.");
		m_values.reserve(defaultValue.Size());
		for (auto value : defaultValue) m_values.push_back(value);

		// Route public construction through the same definition validator used by
		// runtime effects. This checks identifier syntax, exact component counts for
		// scalar/vector/matrix types, and finite initial values before the descriptor
		// can reach private Composition metadata.
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

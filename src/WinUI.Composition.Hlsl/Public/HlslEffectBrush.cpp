#include "HlslEffectBrush.h"
#include "HlslEffectBrush.g.cpp"

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	hstring HlslEffectBrush::GetPropertyPath(hstring const& name) const
	{
		for (auto const& property : m_definition->properties)
		{
			if (name == property.name)
				return hstring{ m_definition->effectName + L"." + property.name };
		}
		throw hresult_invalid_argument(L"The property is not declared by this effect.");
	}

	void HlslEffectBrush::SetSource(hstring const& name, Microsoft::UI::Composition::CompositionBrush const& source)
	{
		auto valid = name == m_definition->sourceName ||
			std::ranges::any_of(m_definition->sourceNames, [&](auto const& value) { return name == value; });
		if (!valid || !source) throw hresult_invalid_argument(L"The source name is not declared by this effect.");
		if (source.Compositor() != m_brush.Compositor()) throw hresult_invalid_argument(L"The source belongs to another compositor.");
		m_brush.SetSourceParameter(name, source);
	}

	void HlslEffectBrush::SetFloat(hstring const& name, float value)
	{
		for (auto const& property : m_definition->properties)
		{
			if (name == property.name)
			{
				if (property.type != hlsl::engine::PropertyType::Scalar)
					throw hresult_invalid_argument(L"The property is not a scalar.");
				if (!std::isfinite(value) || value < property.minimum || value > property.maximum)
					throw hresult_invalid_argument(L"The value is outside the declared property range.");
				m_brush.Properties().InsertScalar(hstring{ m_definition->effectName + L"." + property.name }, value);
				return;
			}
		}
		throw hresult_invalid_argument(L"The scalar property is not declared by this effect.");
	}

	namespace
	{
		hlsl::engine::Property const& RequireProperty(
			std::shared_ptr<hlsl::engine::EffectDefinition const> const& definition,
			hstring const& name,
			hlsl::engine::PropertyType type)
		{
			for (auto const& property : definition->properties)
				if (name == property.name && property.type == type) return property;
			throw hresult_invalid_argument(L"The property name or type does not match the shader definition.");
		}
	}

	void HlslEffectBrush::SetVector2(hstring const& name, Windows::Foundation::Numerics::float2 const& value)
	{
		(void)RequireProperty(m_definition, name, hlsl::engine::PropertyType::Vector2);
		m_brush.Properties().InsertVector2(GetPropertyPath(name), value);
	}
	void HlslEffectBrush::SetVector3(hstring const& name, Windows::Foundation::Numerics::float3 const& value)
	{
		(void)RequireProperty(m_definition, name, hlsl::engine::PropertyType::Vector3);
		m_brush.Properties().InsertVector3(GetPropertyPath(name), value);
	}
	void HlslEffectBrush::SetVector4(hstring const& name, Windows::Foundation::Numerics::float4 const& value)
	{
		(void)RequireProperty(m_definition, name, hlsl::engine::PropertyType::Vector4);
		m_brush.Properties().InsertVector4(GetPropertyPath(name), value);
	}
	void HlslEffectBrush::SetMatrix3x2(hstring const& name, Windows::Foundation::Numerics::float3x2 const& value)
	{
		(void)RequireProperty(m_definition, name, hlsl::engine::PropertyType::Matrix3x2);
		m_brush.Properties().InsertMatrix3x2(GetPropertyPath(name), value);
	}
	void HlslEffectBrush::SetMatrix4x4(hstring const& name, Windows::Foundation::Numerics::float4x4 const& value)
	{
		(void)RequireProperty(m_definition, name, hlsl::engine::PropertyType::Matrix4x4);
		m_brush.Properties().InsertMatrix4x4(GetPropertyPath(name), value);
	}
}

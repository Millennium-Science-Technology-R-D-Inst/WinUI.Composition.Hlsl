#include "HlslEffectFactory.h"
#include "HlslEffectFactory.g.cpp"
#include "HlslEffectBrush.h"
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	Hlsl::HlslEffectBrush HlslEffectFactory::CreateBrush()
	{
		auto brush = make<implementation::HlslEffectBrush>(m_factory.CreateBrush(), m_definition);
		for (auto const& property : m_definition->properties)
		{
			auto name = hstring{ property.name };
			auto const* v = property.initial.data();
			switch (property.type)
			{
				case hlsl::engine::PropertyType::Scalar: brush.SetFloat(name, v[0]); break;
				case hlsl::engine::PropertyType::Vector2: brush.SetVector2(name, { v[0],v[1] }); break;
				case hlsl::engine::PropertyType::Vector3: brush.SetVector3(name, { v[0],v[1],v[2] }); break;
				case hlsl::engine::PropertyType::Vector4: brush.SetVector4(name, { v[0],v[1],v[2],v[3] }); break;
				case hlsl::engine::PropertyType::Matrix3x2: brush.SetMatrix3x2(name, { v[0],v[1],v[2],v[3],v[4],v[5] }); break;
				case hlsl::engine::PropertyType::Matrix4x4: brush.SetMatrix4x4(name, {
					v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7],v[8],v[9],v[10],v[11],v[12],v[13],v[14],v[15] }); break;
			}
		}
		return brush;
	}
}

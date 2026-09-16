module;
#include <Windows.h>
#include <cmath>
#include <d2d1effects.h>
#include <windows.graphics.effects.interop.h>

module WinUI.Composition.Hlsl.ScaleEffect;

import winrt.Windows.Foundation;
import winrt.Windows.Graphics.Effects;

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Effects;

namespace
{
	// D2D Scale is a valid Direct2D effect, but it is not one of the effect types
	// accepted by Microsoft.UI.Composition::Compositor::CreateEffectFactory.
	// Composition does support the 2D affine-transform effect, and its output bounds
	// follow the transform matrix, so use that supported primitive to establish the
	// Dual Kawase pyramid levels.
	constexpr GUID kAffineTransform2DEffectId{
		0x6aa97485, 0x6354, 0x4cfc, { 0x90, 0x8c, 0xe4, 0xa7, 0x4f, 0x62, 0xc9, 0x6c }
	};

	struct Effect :
		winrt::implements<
		Effect,
		IGraphicsEffect,
		IGraphicsEffectSource,
		ABI::Windows::Graphics::Effects::IGraphicsEffectD2D1Interop>
	{
		Effect(wchar_t const* effectName, IGraphicsEffectSource const& source, float scaleX, float scaleY) :
			m_name(effectName),
			m_source(source),
			m_scaleX(scaleX),
			m_scaleY(scaleY),
			m_borderMode(scaleX <= 1.0f && scaleY <= 1.0f ? D2D1_BORDER_MODE_HARD : D2D1_BORDER_MODE_SOFT)
		{
		}

		hstring Name() const { return m_name; }
		void Name(hstring const& value) { m_name = value; }

		HRESULT __stdcall GetEffectId(GUID* effectId) noexcept final
		{
			if (!effectId) return E_POINTER;
			*effectId = kAffineTransform2DEffectId;
			return S_OK;
		}

		HRESULT __stdcall GetNamedPropertyMapping(
			LPCWSTR name,
			UINT* index,
			ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING* mapping) noexcept final
		{
			if (!name || !index || !mapping) return E_POINTER;
			if (wcscmp(name, L"InterpolationMode") == 0)
				*index = D2D1_2DAFFINETRANSFORM_PROP_INTERPOLATION_MODE;
			else if (wcscmp(name, L"BorderMode") == 0)
				*index = D2D1_2DAFFINETRANSFORM_PROP_BORDER_MODE;
			else if (wcscmp(name, L"TransformMatrix") == 0)
				*index = D2D1_2DAFFINETRANSFORM_PROP_TRANSFORM_MATRIX;
			else if (wcscmp(name, L"Sharpness") == 0)
				*index = D2D1_2DAFFINETRANSFORM_PROP_SHARPNESS;
			else
				return E_INVALIDARG;
			*mapping = ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT;
			return S_OK;
		}

		HRESULT __stdcall GetPropertyCount(UINT* count) noexcept final
		{
			if (!count) return E_POINTER;
			*count = 4;
			return S_OK;
		}

		HRESULT __stdcall GetProperty(
			UINT index,
			ABI::Windows::Foundation::IPropertyValue** value) noexcept final
		{
			if (!value) return E_POINTER;
			*value = nullptr;
			try
			{
				IPropertyValue propertyValue{ nullptr };
				switch (index)
				{
				case D2D1_2DAFFINETRANSFORM_PROP_INTERPOLATION_MODE:
					propertyValue = PropertyValue::CreateUInt32(
						static_cast<uint32_t>(D2D1_2DAFFINETRANSFORM_INTERPOLATION_MODE_LINEAR)).as<IPropertyValue>();
					break;
				case D2D1_2DAFFINETRANSFORM_PROP_BORDER_MODE:
					propertyValue = PropertyValue::CreateUInt32(
						static_cast<uint32_t>(m_borderMode)).as<IPropertyValue>();
					break;
				case D2D1_2DAFFINETRANSFORM_PROP_TRANSFORM_MATRIX:
				{
					// D2D1_MATRIX_3X2_F in row-major field order:
					// _11, _12, _21, _22, _31, _32.
					float values[]{ m_scaleX, 0.0f, 0.0f, m_scaleY, 0.0f, 0.0f };
					propertyValue = PropertyValue::CreateSingleArray(values).as<IPropertyValue>();
					break;
				}
				case D2D1_2DAFFINETRANSFORM_PROP_SHARPNESS:
					propertyValue = PropertyValue::CreateSingle(0.0f).as<IPropertyValue>();
					break;
				default:
					return E_INVALIDARG;
				}
				*value = reinterpret_cast<ABI::Windows::Foundation::IPropertyValue*>(detach_abi(propertyValue));
				return S_OK;
			}
			catch (...)
			{
				return to_hresult();
			}
		}

		HRESULT __stdcall GetSource(
			UINT index,
			ABI::Windows::Graphics::Effects::IGraphicsEffectSource** source) noexcept final
		{
			if (!source) return E_POINTER;
			*source = nullptr;
			if (index != 0) return E_INVALIDARG;
			try
			{
				*source = reinterpret_cast<ABI::Windows::Graphics::Effects::IGraphicsEffectSource*>(
					detach_abi(m_source.as<IGraphicsEffectSource>()));
				return S_OK;
			}
			catch (...)
			{
				return to_hresult();
			}
		}

		HRESULT __stdcall GetSourceCount(UINT* count) noexcept final
		{
			if (!count) return E_POINTER;
			*count = 1;
			return S_OK;
		}

	private:
		hstring m_name;
		IGraphicsEffectSource m_source{ nullptr };
		float m_scaleX{ 1.0f };
		float m_scaleY{ 1.0f };
		D2D1_BORDER_MODE m_borderMode{ D2D1_BORDER_MODE_SOFT };
	};
}

namespace ScaleEffect
{
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect(
		wchar_t const* effectName,
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source,
		float scaleX,
		float scaleY)
	{
		if (!effectName || !source || !std::isfinite(scaleX) || !std::isfinite(scaleY) || scaleX <= 0.0f || scaleY <= 0.0f)
			throw hresult_invalid_argument();
		return make<Effect>(effectName, source, scaleX, scaleY);
	}
}

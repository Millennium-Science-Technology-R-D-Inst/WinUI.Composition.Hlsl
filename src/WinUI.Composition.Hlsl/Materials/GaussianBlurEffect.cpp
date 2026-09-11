module;
#include <Windows.h>
#include <d2d1effects.h>
#include <windows.graphics.effects.interop.h>

module WinUI.Composition.Hlsl.GaussianBlurEffect;

import winrt.Microsoft.UI.Composition;
import winrt.Windows.Foundation;
import winrt.Windows.Graphics.Effects;

using namespace winrt;
using namespace Microsoft::UI::Composition;
using namespace Windows::Graphics::Effects;
using namespace Windows::Foundation;

namespace
{
	constexpr GUID kGaussianBlurEffectId{
		0x1feb6d69, 0x2fe6, 0x4ac9, { 0x8c, 0x58, 0x1d, 0x7f, 0x93, 0xe7, 0xa6, 0xa5 } };

	struct Effect :
		winrt::implements<
		Effect,
		IGraphicsEffect,
		IGraphicsEffectSource,
		ABI::Windows::Graphics::Effects::IGraphicsEffectD2D1Interop>
	{
		Effect(wchar_t const* sourceName, float blurAmount) :
			m_source(sourceName),
			m_blurAmount(blurAmount)
		{
		}

		hstring Name() const
		{
			return m_name;
		}

		void Name(hstring const& value)
		{
			m_name = value;
		}

		HRESULT __stdcall GetEffectId(GUID* effectId) noexcept final
		{
			if (!effectId)
			{
				return E_POINTER;
			}

			*effectId = kGaussianBlurEffectId;
			return S_OK;
		}

		HRESULT __stdcall GetNamedPropertyMapping(
			LPCWSTR name,
			UINT* index,
			ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING* mapping) noexcept final
		{
			if (!name || !index || !mapping)
			{
				return E_POINTER;
			}

			if (wcscmp(name, L"BlurAmount") == 0)
			{
				*index = D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION;
			}
			else if (wcscmp(name, L"Optimization") == 0)
			{
				*index = D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION;
			}
			else if (wcscmp(name, L"BorderMode") == 0)
			{
				*index = D2D1_GAUSSIANBLUR_PROP_BORDER_MODE;
			}
			else
			{
				return E_INVALIDARG;
			}

			*mapping = ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT;
			return S_OK;
		}

		HRESULT __stdcall GetPropertyCount(UINT* count) noexcept final
		{
			if (!count)
			{
				return E_POINTER;
			}

			*count = 3;
			return S_OK;
		}

		HRESULT __stdcall GetProperty(
			UINT index,
			ABI::Windows::Foundation::IPropertyValue** value) noexcept final
		{
			if (!value)
			{
				return E_POINTER;
			}

			*value = nullptr;
			try
			{
				IPropertyValue propertyValue{ nullptr };
				switch (index)
				{
					case D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION:
						propertyValue = PropertyValue::CreateSingle(m_blurAmount).as<IPropertyValue>();
						break;
					case D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION:
						propertyValue = PropertyValue::CreateUInt32(
							static_cast<uint32_t>(D2D1_GAUSSIANBLUR_OPTIMIZATION_BALANCED)).as<IPropertyValue>();
						break;
					case D2D1_GAUSSIANBLUR_PROP_BORDER_MODE:
						propertyValue = PropertyValue::CreateUInt32(
							static_cast<uint32_t>(D2D1_BORDER_MODE_HARD)).as<IPropertyValue>();
						break;
					default:
						return E_INVALIDARG;
				}

				*value = reinterpret_cast<ABI::Windows::Foundation::IPropertyValue*>(
					detach_abi(propertyValue));
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
			if (!source)
			{
				return E_POINTER;
			}

			*source = nullptr;
			if (index != 0)
			{
				return E_INVALIDARG;
			}

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
			if (!count)
			{
				return E_POINTER;
			}

			*count = 1;
			return S_OK;
		}

	private:
		hstring m_name{ L"GaussianBlurEffect" };
		CompositionEffectSourceParameter m_source{ nullptr };
		float m_blurAmount{};
	};
}

namespace GaussianBlurEffect
{
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect(
		wchar_t const* sourceName,
		float blurAmount)
	{
		return make<Effect>(sourceName, blurAmount);
	}
}


module WinUI.Composition.Hlsl.CustomEffectRuntime;

import winrt_base;
import winrt.Windows.Graphics.Effects;
namespace CustomEffectRuntime
{
	void RegisterEffect(CustomEffectDefinition const&)
	{
		throw winrt::hresult_not_implemented(L"No HLSL Composition native ABI adapter is available for this architecture.");
	}
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect(CustomEffectDefinition const&)
	{
		throw winrt::hresult_not_implemented(L"No HLSL Composition native ABI adapter is available for this architecture.");
	}
}

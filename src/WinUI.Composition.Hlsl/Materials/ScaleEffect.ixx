export module WinUI.Composition.Hlsl.ScaleEffect;

import winrt.Windows.Graphics.Effects;

export namespace ScaleEffect
{
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect(
		wchar_t const* effectName,
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source,
		float scaleX,
		float scaleY);
}

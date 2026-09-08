#pragma once
#include "HlslComposition.g.h"
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct HlslComposition
	{
		static Hlsl::HlslEffectFactory CreateEffectFactory(Microsoft::UI::Composition::Compositor const& compositor, Hlsl::HlslEffect const& effect);
		static Hlsl::HlslEffectBrush CreateBackdropBrush(Microsoft::UI::Composition::Compositor const& compositor, Hlsl::HlslEffect const& effect);
		static Microsoft::UI::Xaml::Media::Brush CreateXamlBrush(Hlsl::HlslEffectBrush const& brush);
	};
}
namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct HlslComposition : HlslCompositionT<HlslComposition, implementation::HlslComposition>
	{
	};
}


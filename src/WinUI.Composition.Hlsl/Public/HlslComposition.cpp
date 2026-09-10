#include "pch.h"
#include "HlslComposition.h"
#include "HlslComposition.g.cpp"
#include "HlslEffect.h"
#include "HlslEffectFactory.h"
#include "EffectEngine.h"
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	Hlsl::HlslEffectFactory HlslComposition::CreateEffectFactory(Microsoft::UI::Composition::Compositor const& compositor, Hlsl::HlslEffect const& effect)
	{
		if (!compositor || !effect) throw hresult_invalid_argument();
		auto definition=get_self<implementation::HlslEffect>(effect)->Definition();
		return make<implementation::HlslEffectFactory>(hlsl::engine::GetFactory(compositor, definition), definition);
	}
	Hlsl::HlslEffectBrush HlslComposition::CreateBackdropBrush(Microsoft::UI::Composition::Compositor const& compositor, Hlsl::HlslEffect const& effect)
	{
		auto brush=CreateEffectFactory(compositor, effect).CreateBrush(); brush.SetSource(get_self<implementation::HlslEffect>(effect)->Definition()->sourceName, compositor.CreateBackdropBrush()); return brush;
	}
	Microsoft::UI::Xaml::Media::Brush HlslComposition::CreateXamlBrush(Hlsl::HlslEffectBrush const& brush)
	{
		if (!brush) throw hresult_invalid_argument();
		return hlsl::engine::AsXamlBrush(brush.Brush());
	}
}

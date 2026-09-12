#include "HlslComposition.h"
#include "HlslComposition.g.cpp"
#include "HlslEffect.h"
#include "HlslEffectFactory.h"
#include "HlslRuntimeCapabilities.h"
#include "EffectEngine.h"
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	Hlsl::HlslRuntimeCapabilities HlslComposition::GetRuntimeCapabilities()
	{
#if defined(_M_X64)
		return make<HlslRuntimeCapabilities>(
			Hlsl::HlslNativeArchitecture::X64,
			Hlsl::HlslRuntimeSupportLevel::Validated,
			true,
			true,
			true);
#elif defined(_M_IX86)
		return make<HlslRuntimeCapabilities>(
			Hlsl::HlslNativeArchitecture::X86,
			Hlsl::HlslRuntimeSupportLevel::Experimental,
			true,
			true,
			false);
#elif defined(_M_ARM64)
		return make<HlslRuntimeCapabilities>(
			Hlsl::HlslNativeArchitecture::Arm64,
			Hlsl::HlslRuntimeSupportLevel::Experimental,
			true,
			true,
			false);
#else
		return make<HlslRuntimeCapabilities>(
			Hlsl::HlslNativeArchitecture::Unknown,
			Hlsl::HlslRuntimeSupportLevel::Unsupported,
			false,
			false,
			false);
#endif
	}

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

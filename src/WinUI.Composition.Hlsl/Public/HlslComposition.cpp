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
			true,
			true,
			false,
			false,
			false,
			true,
			true);
#elif defined(_M_IX86)
		return make<HlslRuntimeCapabilities>(
			Hlsl::HlslNativeArchitecture::X86,
			Hlsl::HlslRuntimeSupportLevel::Experimental,
			true,
			true,
			false,
			true,
			false,
			false,
			false,
			true,
			true);
#elif defined(_M_ARM64)
		return make<HlslRuntimeCapabilities>(
			Hlsl::HlslNativeArchitecture::Arm64,
			Hlsl::HlslRuntimeSupportLevel::Experimental,
			true,
			true,
			false,
			true,
			false,
			false,
			false,
			true,
			true);
#else
		return make<HlslRuntimeCapabilities>(
			Hlsl::HlslNativeArchitecture::Unknown,
			Hlsl::HlslRuntimeSupportLevel::Unsupported,
			false,
			false,
			false,
			false,
			false,
			false,
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
		auto brush = CreateEffectFactory(compositor, effect).CreateBrush();
		auto const definition = get_self<implementation::HlslEffect>(effect)->Definition();
		auto const backdrop = compositor.CreateBackdropBrush();
		if (definition->sourceNames.empty())
		{
			brush.SetSource(definition->sourceName, backdrop);
		}
		else
		{
			for (auto const& sourceName : definition->sourceNames)
			{
				brush.SetSource(hstring{ sourceName }, backdrop);
			}
		}
		return brush;
	}
	Hlsl::HlslEffectBrush HlslComposition::CreateBrushWithSources(
		Microsoft::UI::Composition::Compositor const& compositor,
		Hlsl::HlslEffect const& effect,
		Windows::Foundation::Collections::IVectorView<Microsoft::UI::Composition::CompositionBrush> const& sources)
	{
		if (!compositor || !effect || !sources)
		{
			throw hresult_invalid_argument(L"Compositor, effect, and source collection are required.");
		}

		auto const definition = get_self<implementation::HlslEffect>(effect)->Definition();
		auto const expectedCount = definition->sourceNames.empty()
			? 1u
			: static_cast<uint32_t>(definition->sourceNames.size());
		if (sources.Size() != expectedCount)
		{
			throw hresult_invalid_argument(L"The number of Composition brushes must match the effect source count.");
		}

		auto brush = CreateEffectFactory(compositor, effect).CreateBrush();
		if (definition->sourceNames.empty())
		{
			brush.SetSource(definition->sourceName, sources.GetAt(0));
			return brush;
		}

		for (uint32_t index = 0; index < sources.Size(); ++index)
		{
			brush.SetSource(hstring{ definition->sourceNames[index] }, sources.GetAt(index));
		}
		return brush;
	}
	Microsoft::UI::Xaml::Media::Brush HlslComposition::CreateXamlBrush(Hlsl::HlslEffectBrush const& brush)
	{
		if (!brush) throw hresult_invalid_argument();
		return hlsl::engine::AsXamlBrush(brush.Brush());
	}
	Microsoft::UI::Xaml::Media::Brush HlslComposition::CreateXamlBrushFromCompositionBrush(Microsoft::UI::Composition::CompositionBrush const& brush)
	{
		if (!brush) throw hresult_invalid_argument();
		return hlsl::engine::AsXamlBrush(brush);
	}
}

#pragma once
#ifndef WINRT_IMPORT_MODULE
#define WINRT_IMPORT_MODULE
#endif
#include "HlslCompiler.g.h"

import winrt.Windows.Foundation;
import winrt.Windows.Foundation.Collections;

namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct HlslCompiler
	{
		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> CompileAsync(
			hstring const& shader,
			Hlsl::HlslEffectKind kind,
			Hlsl::HlslShaderProfile profile);

		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> CompileWithPropertiesAsync(
			hstring const& shader,
			Hlsl::HlslEffectKind kind,
			Hlsl::HlslShaderProfile profile,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties);
	};
}

namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct HlslCompiler : HlslCompilerT<HlslCompiler, implementation::HlslCompiler>
	{
	};
}

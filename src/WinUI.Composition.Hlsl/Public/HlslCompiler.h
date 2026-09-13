#pragma once
#ifndef WINRT_IMPORT_MODULE
#define WINRT_IMPORT_MODULE
#endif
#include "HlslCompiler.g.h"

import std;
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

		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> CompileWithDefinesAsync(
			hstring const& shader,
			Hlsl::HlslEffectKind kind,
			Hlsl::HlslShaderProfile profile,
			Windows::Foundation::Collections::IVectorView<hstring> const& defines);

		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> CompileWithPropertiesAsync(
			hstring const& shader,
			Hlsl::HlslEffectKind kind,
			Hlsl::HlslShaderProfile profile,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties);

		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> CompileWithPropertiesAndDefinesAsync(
			hstring const& shader,
			Hlsl::HlslEffectKind kind,
			Hlsl::HlslShaderProfile profile,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties,
			Windows::Foundation::Collections::IVectorView<hstring> const& defines);

		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> CompileAdvancedAsync(
			hstring const& shader,
			Hlsl::HlslEffectKind kind,
			Hlsl::HlslShaderProfile profile,
			std::uint32_t sourceCount);

		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> CompileAdvancedWithDefinesAsync(
			hstring const& shader,
			Hlsl::HlslEffectKind kind,
			Hlsl::HlslShaderProfile profile,
			std::uint32_t sourceCount,
			Windows::Foundation::Collections::IVectorView<hstring> const& defines);

		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> CompileAdvancedWithPropertiesAsync(
			hstring const& shader,
			Hlsl::HlslEffectKind kind,
			Hlsl::HlslShaderProfile profile,
			std::uint32_t sourceCount,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties);

		static Windows::Foundation::IAsyncOperation<Hlsl::HlslShaderLibrary> CompileAdvancedWithPropertiesAndDefinesAsync(
			hstring const& shader,
			Hlsl::HlslEffectKind kind,
			Hlsl::HlslShaderProfile profile,
			std::uint32_t sourceCount,
			Windows::Foundation::Collections::IVectorView<Hlsl::HlslFloatProperty> const& properties,
			Windows::Foundation::Collections::IVectorView<hstring> const& defines);
	};
}

namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct HlslCompiler : HlslCompilerT<HlslCompiler, implementation::HlslCompiler>
	{
	};
}

module;

#include <Windows.h>
#include "KawaseBlurShader.g.h"

export module WinUI.Composition.Hlsl.Shaders.KawaseBlur;

import std;

export namespace WinUI::Composition::Hlsl::Shaders
{
	struct KawaseShaderLibraryView
	{
		void const* data{};
		std::size_t size{};
	};

	KawaseShaderLibraryView KawaseBlurShader() noexcept
	{
		return { g_KawaseBlurShader, sizeof(g_KawaseBlurShader) };
	}
}

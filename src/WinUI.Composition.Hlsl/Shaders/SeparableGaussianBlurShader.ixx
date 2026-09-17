module;

#include <Windows.h>
#include "SeparableGaussianBlurShader.g.h"

export module WinUI.Composition.Hlsl.Shaders.SeparableGaussianBlur;

import std;

export namespace WinUI::Composition::Hlsl::Shaders
{
	struct SeparableGaussianBlurShaderLibraryView
	{
		void const* data{};
		std::size_t size{};
	};

	SeparableGaussianBlurShaderLibraryView SeparableGaussianBlurShader() noexcept
	{
		return { g_SeparableGaussianBlurShader, sizeof(g_SeparableGaussianBlurShader) };
	}
}

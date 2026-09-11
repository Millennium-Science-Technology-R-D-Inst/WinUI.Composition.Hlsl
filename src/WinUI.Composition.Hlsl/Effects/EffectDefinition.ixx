module;
#include "CustomEffectRuntime.h"

export module WinUI.Composition.Hlsl.EffectDef;

import std;
import winrt.Windows.Foundation;
import winrt.Windows.Graphics.Effects;
import winrt.Microsoft.UI.Composition;

export namespace hlsl::engine
{
	struct ScalarProperty
	{
		std::wstring name;
		float initial{};
		float minimum{};
		float maximum{};
	};

	struct EffectDefinition
	{
		winrt::guid id{};
		bool sampler{};
		std::string shader;
		std::wstring sourceName{ L"Backdrop" };
		std::wstring effectName{ L"HlslEffect" };
		std::vector<ScalarProperty> properties;
		// Built-ins provide the same native definition format, never special brush behavior.
		CustomEffectRuntime::CustomEffectDefinition const* nativeTemplate{};
	};

	winrt::guid DeriveId(EffectDefinition const& definition);
	void Validate(EffectDefinition const& definition);
	winrt::Windows::Graphics::Effects::IGraphicsEffect Compile(std::shared_ptr<EffectDefinition const> const& definition);
	winrt::Microsoft::UI::Composition::CompositionEffectFactory GetFactory(winrt::Microsoft::UI::Composition::Compositor const& compositor, std::shared_ptr<EffectDefinition const> const& definition);
}

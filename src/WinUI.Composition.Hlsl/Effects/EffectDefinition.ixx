module;

export module WinUI.Composition.Hlsl.EffectDef;

import std;
import WinUI.Composition.Hlsl.CustomEffectRuntime;
import winrt.Windows.Foundation;
import winrt.Windows.Graphics.Effects;
import winrt.Microsoft.UI.Composition;

export namespace hlsl::engine
{
	enum class PropertyType : std::uint8_t
	{
		Scalar,
		Vector2,
		Vector3,
		Vector4,
		Matrix3x2,
		Matrix4x4,
	};

	struct Property
	{
		std::wstring name;
		PropertyType type{ PropertyType::Scalar };
		std::vector<float> initial;
		float minimum{};
		float maximum{};

		Property() = default;
		Property(std::wstring valueName, float value, float min, float max) :
			name(std::move(valueName)), initial{ value }, minimum(min), maximum(max)
		{
		}
		Property(std::wstring valueName, PropertyType valueType, std::vector<float> values) :
			name(std::move(valueName)), type(valueType), initial(std::move(values)),
			minimum(-std::numeric_limits<float>::max()), maximum(std::numeric_limits<float>::max())
		{
		}
	};

	struct EffectDefinition
	{
		winrt::guid id{};
		bool sampler{};
		bool materializedSampler{};
		std::string shader;
		std::vector<std::uint8_t> shaderBytecode;
		std::uint8_t shaderProfile{ CustomEffectRuntime::kShaderProfileLevel93 };
		std::wstring sourceName{ L"Backdrop" };
		std::vector<std::wstring> sourceNames;
		std::wstring effectName{ L"HlslEffect" };
		std::vector<Property> properties;
		// Built-ins provide the same native definition format, never special brush behavior.
		CustomEffectRuntime::CustomEffectDefinition const* nativeTemplate{};
	};

	using Definition = std::shared_ptr<EffectDefinition const>;

	winrt::guid DeriveId(EffectDefinition const& definition);
	void Validate(EffectDefinition const& definition);
	winrt::Windows::Graphics::Effects::IGraphicsEffect Compile(Definition const& definition);
	winrt::Windows::Graphics::Effects::IGraphicsEffect Compile(
		Definition const& definition,
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource const& source);
	winrt::Windows::Graphics::Effects::IGraphicsEffect Compile(
		Definition const& definition,
		std::span<winrt::Windows::Graphics::Effects::IGraphicsEffectSource const> sources);
	winrt::Microsoft::UI::Composition::CompositionEffectFactory GetFactory(winrt::Microsoft::UI::Composition::Compositor const& compositor, Definition const& definition);
}

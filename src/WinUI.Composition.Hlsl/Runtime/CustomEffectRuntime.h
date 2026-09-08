#pragma once

namespace CustomEffectRuntime
{
	enum class SourceKind
	{
		Backdrop,
	};

	struct SourceDescriptor
	{
		wchar_t const* name;
		SourceKind kind;
		bool requiresSamplerData;
		bool requiresSamplerDataExt;
	};

	struct PropertyDescriptor
	{
		wchar_t const* publicName;
		uint32_t index;
		ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING mapping;
		HRESULT(*getDefaultValue)(ABI::Windows::Foundation::IPropertyValue** value);
	};

	struct NativePropertyMetadata
	{
		char const* shaderName;
		uint32_t propertyOffset;
		uint32_t expressionType;
		uint32_t propertyType;
		uint32_t valueCount;
		void* validator;
	};

	struct ConstantBufferPropertyMapping
	{
		uint32_t propertyIndex;
		uint32_t constantBufferOffset;
	};

	// DWM names this D3DShaderProfileVersion. It is a single byte at
	// ShaderLinkingBody+0x2E (not a bool, not a sampler count).
	//
	// dwmcorei!LinkShader reads the main body's +0x2E, stores it on
	// CShaderLinkingGraphBuilder+0x54, then:
	//   0 -> Link("ps_4_0_level_9_1") + fragment module case 0
	//   1 -> Link("ps_4_0_level_9_3") + fragment module case 1
	//   2 -> Link("ps_4_0")           + fragment module case 2
	//
	// wuceffectsi generated effects always write 1 and compile
	// "lib_4_0_level_9_3_ps_only". CCustomKernelEffect (blur) writes 0/1/2
	// from D3D_FEATURE_LEVEL (<9_3 -> 0, 9_3..10_0 -> 1, >=10_0 -> 2).
	// The library profile and this byte must match; SM5 is unsupported.
	//
	// Profile isolation scope (what "one graph" means):
	//
	//   Unit of consistency is one CRenderingTechnique / one LinkShader call,
	//   not a brush, not a visual tree, not the whole process.
	//
	//   CRenderingTechnique::GetShaders walks that technique's fragment chain,
	//   then CreateLinkedShader(mainBody, span[depBodies...]) -> LinkShader.
	//   Only those bodies share one profile (main body's +0x2E wins; deps must
	//   be compatible). Different techniques link separately.
	//
	//   One CompositionEffectBrush can produce multiple techniques (multi-pass
	//   blur, materialize-then-sample). Two brushes on two visuals always get
	//   separate techniques: each draws to a surface, the next samples pixels
	//   as Texture2D — never 0x0500 body-to-body dependency across brushes.
	//
	//   Inside one ICompiledEffect, subgraph flags matter:
	//     flags==0  -> materialize intermediate RT, next technique samples it
	//                  (profiles can differ across those techniques)
	//     0x0500 dep / same technique chain -> same LinkShader, same profile
	//
	//   So two stacked brushes (e.g. system GaussianBlur + LiquidGlass) do not
	//   force a shared profile. Mixing a profile-2 custom body with a profile-1
	//   wuceffectsi body in the *same* technique link still fails.
	constexpr uint8_t kShaderProfileLevel91 = 0;
	constexpr uint8_t kShaderProfileLevel93 = 1;
	constexpr uint8_t kShaderProfilePs40 = 2;

	struct CustomEffectDefinition
	{
		GUID id;
		wchar_t const* effectName;
		char const* fragmentName;

		char const* shaderSource;
		size_t shaderSourceSize;
		char const* shaderFunctionName;

		SourceDescriptor const* sources;
		uint32_t sourceCount;

		PropertyDescriptor const* properties;
		uint32_t propertyCount;
		void const* nativePropertyMetadata;
		uint32_t nativePropertyMetadataCount;
		uint32_t propertiesStructSize;

		ConstantBufferPropertyMapping const* constantBufferProperties;
		uint32_t constantBufferPropertyCount;

		uint16_t const* shaderArguments;
		uint64_t shaderArgumentCount;
		uint16_t linkingArgType;
		// Was misnamed hasCustomSamplers. DWM consumes this as
		// D3DShaderProfileVersion (kShaderProfile*).
		uint8_t shaderProfileVersion;

		uint32_t constantBufferSize;
		void const* constantBufferInitialValue;

		bool flattenSourceBeforeCustomSampler;
		char const* flattenShaderFunctionName;
	};

	void RegisterEffect(CustomEffectDefinition const& definition);

	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect(
		CustomEffectDefinition const& definition);
}


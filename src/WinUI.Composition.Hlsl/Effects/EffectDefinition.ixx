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

	struct PropertyAbiSpec
	{
		char const* hlslType;
		std::uint32_t expressionType;
		std::uint32_t valueCount;
		std::uint32_t propertyAlignment;
		std::uint32_t rows;
		std::uint32_t columns;
		bool matrix;
	};

	inline PropertyAbiSpec GetPropertyAbiSpec(PropertyType type)
	{
		switch (type)
		{
			case PropertyType::Scalar: return { "float", 18, 1, 4, 1, 1, false };
			case PropertyType::Vector2: return { "float2", 35, 2, 8, 1, 2, false };
			case PropertyType::Vector3: return { "float3", 52, 3, 16, 1, 3, false };
			case PropertyType::Vector4: return { "float4", 69, 4, 16, 1, 4, false };
			case PropertyType::Matrix3x2: return { "float3x2", 104, 6, 16, 3, 2, true };
			case PropertyType::Matrix4x4: return { "float4x4", 265, 16, 16, 4, 4, true };
		}
		throw winrt::hresult_invalid_argument(L"Unknown HLSL property type.");
	}

	inline constexpr std::uint32_t AbiAlignUp(std::uint32_t value, std::uint32_t alignment) noexcept
	{
		return (value + alignment - 1u) & ~(alignment - 1u);
	}

	struct PropertyLayoutCursor
	{
		std::uint32_t propertySize{};
		std::uint32_t constantBufferSize{};
	};

	struct PropertyLayoutEntry
	{
		std::uint32_t propertyOffset{};
		std::uint32_t propertySize{};
		std::uint32_t constantBufferOffset{};
		std::uint32_t constantBufferStorageSize{};
	};

	inline PropertyLayoutEntry AppendPropertyLayout(PropertyLayoutCursor& cursor, PropertyType type)
	{
		auto const spec = GetPropertyAbiSpec(type);
		PropertyLayoutEntry result{};
		result.propertyOffset = AbiAlignUp(cursor.propertySize, spec.propertyAlignment);
		result.propertySize = spec.valueCount * sizeof(float);
		cursor.propertySize = result.propertyOffset + result.propertySize;

		// DirectPropertyUpdater copies valueCount*sizeof(float) bytes verbatim from
		// the native property blob to the mapped constant-buffer offset. Therefore
		// the shader-facing representation must also be contiguous. Matrices are
		// emitted by TypedPropertyAbi as packed float vectors plus a macro that
		// reconstructs the logical HLSL matrix, avoiding HLSL's implicit 16-byte
		// matrix row/column stride.
		result.constantBufferOffset = AbiAlignUp(cursor.constantBufferSize, spec.propertyAlignment);
		result.constantBufferStorageSize = result.propertySize;
		if ((result.constantBufferOffset & 15u) + result.constantBufferStorageSize > 16u && !spec.matrix)
		{
			result.constantBufferOffset = AbiAlignUp(result.constantBufferOffset, 16);
		}
		if (spec.matrix)
		{
			result.constantBufferOffset = AbiAlignUp(result.constantBufferOffset, 16);
		}
		cursor.constantBufferSize = result.constantBufferOffset + result.constantBufferStorageSize;
		return result;
	}

	inline constexpr std::uint32_t FinalPropertyStructSize(PropertyLayoutCursor const& cursor) noexcept
	{
		return AbiAlignUp(cursor.propertySize, 16);
	}

	inline constexpr std::uint32_t FinalConstantBufferSize(PropertyLayoutCursor const& cursor) noexcept
	{
		return AbiAlignUp(cursor.constantBufferSize, 16);
	}

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

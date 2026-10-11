#include <Windows.h>
#include <windows.graphics.effects.interop.h>
#include "CustomLiquidGlassPostProcessingEffect.h"

import winrt.Windows.Foundation;
import WinUI.Composition.Hlsl.CustomEffectRuntime;
import WinUI.Composition.Hlsl.Shaders.LiquidGlassPostProcessing;

namespace
{
    constexpr GUID kId{ 0xa4f284d8, 0x2e5e, 0x4536, { 0xa3, 0x6a, 0x95, 0x04, 0xba, 0x66, 0x39, 0xe1 } };
    constexpr float kInitial[8]{ 1.0f, 0.0f, 0.0f, 0.0f, 1.25f, 1.0f, 0.0f, 0.0f };
    static_assert(sizeof(kInitial) == 32);
    using namespace CustomEffectRuntime;
    constexpr auto kDirect = ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT;
    PropertyDescriptor const kProperties[]{
        { L"BloomAmount", 0, kDirect, nullptr, kInitial[0] },
        { L"Brightness", 1, kDirect, nullptr, kInitial[1] },
        { L"Temperature", 2, kDirect, nullptr, kInitial[2] },
        { L"Vibrance", 3, kDirect, nullptr, kInitial[3] },
        { L"Saturation", 4, kDirect, nullptr, kInitial[4] },
        { L"Contrast", 5, kDirect, nullptr, kInitial[5] },
        { L"Exposure", 6, kDirect, nullptr, kInitial[6] },
    };
    NativePropertyMetadata const kMetadata[]{
        { "BloomAmount", 0, 18, 8, 1, nullptr },
        { "Brightness", 4, 18, 8, 1, nullptr },
        { "Temperature", 8, 18, 8, 1, nullptr },
        { "Vibrance", 12, 18, 8, 1, nullptr },
        { "Saturation", 16, 18, 8, 1, nullptr },
        { "Contrast", 20, 18, 8, 1, nullptr },
        { "Exposure", 24, 18, 8, 1, nullptr },
    };
    ConstantBufferPropertyMapping const kMappings[]{ {0,0}, {1,4}, {2,8}, {3,12}, {4,16}, {5,20}, {6,24} };
    SourceDescriptor const kSources[]{
        { L"Blurred", SourceKind::Backdrop, false, false },
        { L"RawBackdrop", SourceKind::Backdrop, false, false },
    };
    std::uint16_t const kArguments[]{ 0x0200, 0x0201 };
    auto const kShader = WinUI::Composition::Hlsl::Shaders::LiquidGlassPostProcessingShader();
    CustomEffectDefinition const kDefinition{
        kId, CustomLiquidGlassPostProcessingEffect::EffectName, "LiquidGlassPostProcessing",
        nullptr, 0, kShader.data, kShader.size, "PSBody",
        kSources, ARRAYSIZE(kSources), kProperties, ARRAYSIZE(kProperties),
        kMetadata, ARRAYSIZE(kMetadata), sizeof(kInitial),
        kMappings, ARRAYSIZE(kMappings), kArguments, ARRAYSIZE(kArguments), 0,
        kShaderProfilePs40, sizeof(kInitial), kInitial,
        CustomEffectInputMode::LinkedColor, GraphLoweringPolicy::SingleCustom,
    };
}

namespace CustomLiquidGlassPostProcessingEffect
{
    std::shared_ptr<hlsl::engine::EffectDefinition const> Description()
    {
        static auto const definition = [] {
            auto value = std::make_shared<hlsl::engine::EffectDefinition>();
            value->id = kId;
            value->effectName = EffectName;
            value->sourceName = L"Blurred";
            value->sourceNames = { L"Blurred", L"RawBackdrop" };
            value->nativeTemplate = &kDefinition;
            value->properties = {
                { L"BloomAmount", 1.0f, 0.0f, 1.0f },
                { L"Brightness", 0.0f, -1.0f, 1.0f },
                { L"Temperature", 0.0f, -1.0f, 1.0f },
                { L"Vibrance", 0.0f, 0.0f, 1.0f },
                { L"Saturation", 1.25f, 0.0f, 4.0f },
                { L"Contrast", 1.0f, 0.0f, 4.0f },
                { L"Exposure", 0.0f, -4.0f, 4.0f },
            };
            return value;
        }();
        return definition;
    }
    winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect()
    {
        return CustomEffectRuntime::CreateEffect(kDefinition);
    }
}

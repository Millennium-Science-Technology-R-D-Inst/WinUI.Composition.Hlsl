#pragma once
#include <winrt/Windows.Foundation.h>
#include <string>
#include <vector>
#include <memory>
#include "CustomEffectRuntime.h"

namespace hlsl::engine {
struct ScalarProperty {
    std::wstring name;
    float initial{};
    float minimum{};
    float maximum{};
};
struct EffectDefinition {
    winrt::guid id{};
    bool sampler{};
    std::string shader;
    std::wstring sourceName{L"Backdrop"};
    std::wstring effectName{L"HlslEffect"};
    std::vector<ScalarProperty> properties;
    // Built-ins provide the same native definition format, never special brush behavior.
    CustomEffectRuntime::CustomEffectDefinition const* nativeTemplate{};
};
using Definition = std::shared_ptr<EffectDefinition const>;
winrt::guid DeriveId(EffectDefinition const& definition);
void Validate(EffectDefinition const& definition);
winrt::Windows::Graphics::Effects::IGraphicsEffect Compile(Definition const& definition);
winrt::Microsoft::UI::Composition::CompositionEffectFactory GetFactory(
    winrt::Microsoft::UI::Composition::Compositor const& compositor,Definition const& definition);
}

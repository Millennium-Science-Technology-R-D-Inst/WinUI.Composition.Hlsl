#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <cstring>
#include <iostream>
#include <string>

#include "ConsumerSampler.g.h"
#include "ConsumerMaterializedSampler.g.h"
#include "ConsumerMultiSourceColor.g.h"
#include "ConsumerMultiSourceSampler.g.h"

static_assert(sizeof(g_ConsumerSampler_Shader) > 4);
static_assert(sizeof(g_ConsumerMaterializedSampler_Shader) > 4);
static_assert(sizeof(g_ConsumerMultiSourceColor_Shader) > 4);
static_assert(sizeof(g_ConsumerMultiSourceSampler_Shader) > 4);

namespace
{
    template <std::size_t Size, typename Callback>
    bool ReflectLibrary(unsigned char const (&bytecode)[Size], Callback&& callback)
    {
        ID3D11LibraryReflection* reflection{};
        auto const result = D3DReflectLibrary(
            bytecode,
            Size,
            __uuidof(ID3D11LibraryReflection),
            reinterpret_cast<void**>(&reflection));
        if (FAILED(result) || !reflection) return false;
        auto const matched = callback(reflection);
        reflection->Release();
        return matched;
    }

    template <std::size_t Size>
    bool HasLibraryExport(unsigned char const (&bytecode)[Size], char const* expected)
    {
        return ReflectLibrary(bytecode, [&](ID3D11LibraryReflection* reflection)
        {
            D3D11_LIBRARY_DESC library{};
            if (FAILED(reflection->GetDesc(&library))) return false;
            for (UINT index = 0; index < library.FunctionCount; ++index)
            {
                auto* function = reflection->GetFunctionByIndex(index);
                if (!function) continue;
                D3D11_FUNCTION_DESC description{};
                if (SUCCEEDED(function->GetDesc(&description)) && description.Name &&
                    std::strcmp(description.Name, expected) == 0)
                    return true;
            }
            return false;
        });
    }

    template <std::size_t Size>
    bool HasResourceBinding(
        unsigned char const (&bytecode)[Size],
        char const* resourceName,
        D3D_SHADER_INPUT_TYPE type,
        UINT bindPoint,
        D3D_SRV_DIMENSION dimension = D3D_SRV_DIMENSION_UNKNOWN)
    {
        return ReflectLibrary(bytecode, [&](ID3D11LibraryReflection* reflection)
        {
            D3D11_LIBRARY_DESC library{};
            if (FAILED(reflection->GetDesc(&library))) return false;
            for (UINT index = 0; index < library.FunctionCount; ++index)
            {
                auto* function = reflection->GetFunctionByIndex(index);
                if (!function) continue;
                D3D11_SHADER_INPUT_BIND_DESC binding{};
                if (SUCCEEDED(function->GetResourceBindingDescByName(resourceName, &binding)) &&
                    binding.Type == type && binding.BindPoint == bindPoint && binding.BindCount == 1 &&
                    (dimension == D3D_SRV_DIMENSION_UNKNOWN || binding.Dimension == dimension))
                {
                    return true;
                }
            }
            return false;
        });
    }

    template <std::size_t Size>
    bool HasSamplerBindings(unsigned char const (&bytecode)[Size], UINT sourceCount)
    {
        for (UINT source = 0; source < sourceCount; ++source)
        {
            auto const textureName = std::string("texture") + std::to_string(source);
            auto const samplerName = std::string("sampler") + std::to_string(source);
            if (!HasResourceBinding(
                    bytecode, textureName.c_str(), D3D_SIT_TEXTURE, source, D3D_SRV_DIMENSION_TEXTURE2D) ||
                !HasResourceBinding(bytecode, samplerName.c_str(), D3D_SIT_SAMPLER, source))
                return false;
        }
        return true;
    }

    template <std::size_t Size>
    bool IsDxbc(unsigned char const (&bytecode)[Size])
    {
        return Size >= 4 &&
            bytecode[0] == 'D' && bytecode[1] == 'X' &&
            bytecode[2] == 'B' && bytecode[3] == 'C';
    }

    int Fail(int code, char const* message)
    {
        std::cerr << "NativeShaderConsumer validation failed [" << code << "]: " << message << '\n';
        return code;
    }
}

int main()
{
    if (!IsDxbc(g_ConsumerSampler_Shader))
        return Fail(11, "ConsumerSampler is not DXBC.");
    if (!HasLibraryExport(g_ConsumerSampler_Shader, "PSBody") ||
        !HasLibraryExport(g_ConsumerSampler_Shader, "PSBodyCC") ||
        !HasLibraryExport(g_ConsumerSampler_Shader, "__WinUICompositionHlsl_Metadata_K1_P2_S1"))
        return Fail(12, "ConsumerSampler exports or metadata are incomplete.");
    if (!HasSamplerBindings(g_ConsumerSampler_Shader, 1))
        return Fail(13, "ConsumerSampler texture0/sampler0 bindings do not map to t0/s0.");

    if (!IsDxbc(g_ConsumerMaterializedSampler_Shader) ||
        !HasLibraryExport(g_ConsumerMaterializedSampler_Shader, "MaterializeColor") ||
        !HasLibraryExport(g_ConsumerMaterializedSampler_Shader, "PSBody") ||
        !HasLibraryExport(g_ConsumerMaterializedSampler_Shader, "__WinUICompositionHlsl_Metadata_K2_P2_S1"))
        return Fail(21, "ConsumerMaterializedSampler exports or metadata are incomplete.");
    if (!HasSamplerBindings(g_ConsumerMaterializedSampler_Shader, 1))
        return Fail(22, "ConsumerMaterializedSampler texture0/sampler0 bindings do not map to t0/s0.");

    if (!IsDxbc(g_ConsumerMultiSourceColor_Shader) ||
        !HasLibraryExport(g_ConsumerMultiSourceColor_Shader, "PSBody") ||
        !HasLibraryExport(g_ConsumerMultiSourceColor_Shader, "__WinUICompositionHlsl_Metadata_K0_P2_S2"))
        return Fail(31, "ConsumerMultiSourceColor exports or metadata are incomplete.");

    if (!IsDxbc(g_ConsumerMultiSourceSampler_Shader) ||
        !HasLibraryExport(g_ConsumerMultiSourceSampler_Shader, "PSBody") ||
        !HasLibraryExport(g_ConsumerMultiSourceSampler_Shader, "PSBodyCC") ||
        !HasLibraryExport(g_ConsumerMultiSourceSampler_Shader, "__WinUICompositionHlsl_Metadata_K1_P2_S2"))
        return Fail(41, "ConsumerMultiSourceSampler exports or metadata are incomplete.");
    if (!HasSamplerBindings(g_ConsumerMultiSourceSampler_Shader, 2))
        return Fail(42, "ConsumerMultiSourceSampler resource bindings do not map source order to tN/sN.");

    return 0;
}

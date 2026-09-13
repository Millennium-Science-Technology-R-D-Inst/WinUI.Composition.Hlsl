#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <cstring>

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
    template <std::size_t Size>
    bool HasLibraryExport(unsigned char const (&bytecode)[Size], char const* expected)
    {
        ID3D11LibraryReflection* reflection{};
        auto const result = D3DReflectLibrary(
            bytecode,
            Size,
            IID_ID3D11LibraryReflection,
            reinterpret_cast<void**>(&reflection));
        if (FAILED(result) || !reflection)
        {
            return false;
        }

        D3D11_LIBRARY_DESC library{};
        if (FAILED(reflection->GetDesc(&library)))
        {
            reflection->Release();
            return false;
        }

        bool found{};
        for (UINT index = 0; index < library.FunctionCount; ++index)
        {
            auto* function = reflection->GetFunctionByIndex(index);
            if (!function)
            {
                continue;
            }

            D3D11_FUNCTION_DESC description{};
            if (SUCCEEDED(function->GetDesc(&description)) &&
                description.Name &&
                std::strcmp(description.Name, expected) == 0)
            {
                found = true;
                break;
            }
        }

        reflection->Release();
        return found;
    }

    template <std::size_t Size>
    bool IsDxbc(unsigned char const (&bytecode)[Size])
    {
        return Size >= 4 &&
            bytecode[0] == 'D' && bytecode[1] == 'X' &&
            bytecode[2] == 'B' && bytecode[3] == 'C';
    }
}

int main()
{
    if (!IsDxbc(g_ConsumerSampler_Shader) ||
        !HasLibraryExport(g_ConsumerSampler_Shader, "PSBody") ||
        !HasLibraryExport(g_ConsumerSampler_Shader, "PSBodyCC") ||
        !HasLibraryExport(g_ConsumerSampler_Shader, "__WinUICompositionHlsl_Metadata_K1_P2_S1"))
    {
        return 1;
    }

    if (!IsDxbc(g_ConsumerMaterializedSampler_Shader) ||
        !HasLibraryExport(g_ConsumerMaterializedSampler_Shader, "MaterializeColor") ||
        !HasLibraryExport(g_ConsumerMaterializedSampler_Shader, "PSBody") ||
        !HasLibraryExport(g_ConsumerMaterializedSampler_Shader, "__WinUICompositionHlsl_Metadata_K2_P2_S1"))
    {
        return 2;
    }

    if (!IsDxbc(g_ConsumerMultiSourceColor_Shader) ||
        !HasLibraryExport(g_ConsumerMultiSourceColor_Shader, "PSBody") ||
        !HasLibraryExport(g_ConsumerMultiSourceColor_Shader, "__WinUICompositionHlsl_Metadata_K0_P2_S2"))
    {
        return 3;
    }

    if (!IsDxbc(g_ConsumerMultiSourceSampler_Shader) ||
        !HasLibraryExport(g_ConsumerMultiSourceSampler_Shader, "PSBody") ||
        !HasLibraryExport(g_ConsumerMultiSourceSampler_Shader, "PSBodyCC") ||
        !HasLibraryExport(g_ConsumerMultiSourceSampler_Shader, "__WinUICompositionHlsl_Metadata_K1_P2_S2"))
    {
        return 4;
    }

    return 0;
}

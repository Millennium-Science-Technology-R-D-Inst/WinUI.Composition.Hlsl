module;

// This is a Win32/WinRT ABI interop surface, not a C++/WinRT projection.
// Parse the real platform headers in the global module fragment so RPC/COM
// annotations, IUnknown and Windows.Foundation ABI declarations are complete
// before the exported interface is declared.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windows.foundation.h>
#include <windows.graphics.effects.h>
#include <sdkddkver.h>

export module windows.graphic.effects.interop;

#ifndef BUILD_WINDOWS
export namespace ABI
{
#endif
    namespace Windows
    {
        namespace Graphics
        {
            namespace Effects
            {
                typedef interface IGraphicsEffectSource IGraphicsEffectSource;
                typedef interface IGraphicsEffectD2D1Interop IGraphicsEffectD2D1Interop;

                typedef enum GRAPHICS_EFFECT_PROPERTY_MAPPING
                {
                    GRAPHICS_EFFECT_PROPERTY_MAPPING_UNKNOWN,
                    GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT,
                    GRAPHICS_EFFECT_PROPERTY_MAPPING_VECTORX,
                    GRAPHICS_EFFECT_PROPERTY_MAPPING_VECTORY,
                    GRAPHICS_EFFECT_PROPERTY_MAPPING_VECTORZ,
                    GRAPHICS_EFFECT_PROPERTY_MAPPING_VECTORW,
                    GRAPHICS_EFFECT_PROPERTY_MAPPING_RECT_TO_VECTOR4,
                    GRAPHICS_EFFECT_PROPERTY_MAPPING_RADIANS_TO_DEGREES,
                    GRAPHICS_EFFECT_PROPERTY_MAPPING_COLORMATRIX_ALPHA_MODE,
                    GRAPHICS_EFFECT_PROPERTY_MAPPING_COLOR_TO_VECTOR3,
                    GRAPHICS_EFFECT_PROPERTY_MAPPING_COLOR_TO_VECTOR4
                } GRAPHICS_EFFECT_PROPERTY_MAPPING;

#undef INTERFACE
#define INTERFACE IGraphicsEffectD2D1Interop
                DECLARE_INTERFACE_IID_(IGraphicsEffectD2D1Interop, IUnknown, "2FC57384-A068-44D7-A331-30982FCF7177")
                {
                    STDMETHOD(GetEffectId)(
                        _Out_ GUID* id
                        ) PURE;

                    STDMETHOD(GetNamedPropertyMapping)(
                        LPCWSTR name,
                        _Out_ UINT* index,
                        _Out_ GRAPHICS_EFFECT_PROPERTY_MAPPING* mapping
                        ) PURE;

                    STDMETHOD(GetPropertyCount)(
                        _Out_ UINT* count
                        ) PURE;

                    STDMETHOD(GetProperty)(
                        UINT index,
                        _Outptr_ Windows::Foundation::IPropertyValue** value
                        ) PURE;

                    STDMETHOD(GetSource)(
                        UINT index,
                        _Outptr_ IGraphicsEffectSource** source
                        ) PURE;

                    STDMETHOD(GetSourceCount)(
                        _Out_ UINT* count
                        ) PURE;
                };
            }
        }
    }
#ifndef BUILD_WINDOWS
}
#endif

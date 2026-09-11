# HlslShaderLibrary class

Owns an immutable precompiled DXBC shader-linking library.

**Namespace:** `WinUI.Composition.Hlsl`  
**Assembly:** `WinUI.Composition.Hlsl.dll`

## Create

```csharp
public static HlslShaderLibrary Create(IBuffer bytecode, HlslShaderProfile profile);
```

The input is deep-copied. It must be a reflectable HLSL DXBC library no larger than 16 MiB. Construction rejects malformed/non-library DXBC before it can reach the private Composition linker.

When the library is used to create an effect, the public ABI is validated with `D3DReflectLibrary`:

- Color: `float4 PSBody(float4 color)`.
- Sampler: `PSBody` and all clamp/wrap/mirror `PSBody*` variants must use `float4(float2 uv, float4 samplerDataExt)`.
- If scalar properties are declared, the library must provide `cbuffer UserConstants : register(b0)` with the expected named scalar layout and 16-byte cbuffer padding.

The declared `HlslShaderProfile` is still an explicit contract. The current validator does not infer the private Composition profile byte from DXBC and compare it with the enum, so callers/build tooling must keep the enum and FXC target consistent.

## Profile

Returns `Level91`, `Level93`, or `Pixel40` (`lib_4_0`).

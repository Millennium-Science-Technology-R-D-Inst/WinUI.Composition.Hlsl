# HlslShaderLibrary class

Owns an immutable precompiled FXC SM4 DXBC shader-linking library.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v1.0.0  
**Assembly:** `WinUI.Composition.Hlsl.dll`

## Create

```csharp
public static HlslShaderLibrary Create(IBuffer bytecode, HlslShaderProfile profile);
```

The input is deep-copied. It must be a reflectable HLSL DXBC library no larger than 16 MiB. Construction rejects malformed/non-library payloads before they can reach the private Composition linker.

## Profile

Returns `Level91`, `Level93`, or `Pixel40`. The profile remains an explicit build/runtime contract; the current implementation does not infer the private Composition profile byte from arbitrary DXBC.

## Bytecode

```csharp
public IBuffer Bytecode { get; }
```

Returns a copy of the immutable DXBC payload. This is intended for application-managed persistent shader caches:

```text
first run: HLSL -> HlslCompiler.CompileAsync -> Bytecode -> disk/cache
later:     disk/cache -> HlslShaderLibrary.Create -> HlslEffect.CreateCompiled*
```

No private Composition object is required to compile or serialize this bytecode.

## Defensive validation

When arbitrary bytecode is associated with an effect, one-time reflection checks its public ABI:

- `Color`: `float4 PSBody(float4 color)`.
- `Sampler`: every `PSBody*` edge-mode export uses `float4(float2 uv, float4 samplerDataExt)`.
- `MaterializedSampler`: every `PSBody*` export uses `float4(float2 uv, float4 samplerDataExt, float4 samplerData)` and the library exports `float4 MaterializeColor(float4 color)`.
- Declared scalar properties must match `cbuffer UserConstants : register(b0)` in order, offset, and padded size.

These checks happen when accepting untrusted/dynamic DXBC; they are not part of the per-frame rendering or animation path. For package-owned production shaders, prefer `<HlslCompositionShader>` so FXC catches source and entry-point failures during the build.

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

Use `Create` when the application already owns a WinRT `IBuffer`, for example a cache or another Windows API result.

## CreateFromByteArray

```csharp
public static HlslShaderLibrary CreateFromByteArray(
    byte[] bytecode,
    HlslShaderProfile profile);
```

`CreateFromByteArray` performs the same deep-copy and DXBC-library validation as `Create`, but accepts a normal byte array. This is the preferred bridge for native C++ build-generated headers because a MIDL `UInt8[]` input projects to `winrt::array_view<uint8_t const>`.

For a native project:

```xml
<HlslCompositionShader Include="Effects\Glass.hlsl">
  <Kind>MaterializedSampler</Kind>
  <Profile>Pixel40</Profile>
</HlslCompositionShader>
```

FXC generates `Glass.g.h` containing an `unsigned char` byte array. C++ can pass that array directly without constructing an `IBuffer`:

```cpp
#include "Glass.g.h"

import winrt.WinUI.Composition.Hlsl;

using namespace winrt::WinUI::Composition::Hlsl;

auto library = HlslShaderLibrary::CreateFromByteArray(
    g_Effects_Glass_Shader,
    HlslShaderProfile::Pixel40);

auto effect = HlslEffect::CreateCompiledMaterializedSampler({}, library);
```

The exact generated variable name follows the shader relative path unless `HeaderVariableName` is supplied in MSBuild metadata.

## LoadFromFileAsync

```csharp
public static IAsyncOperation<HlslShaderLibrary> LoadFromFileAsync(
    StorageFile file,
    HlslShaderProfile profile);
```

Reads a DXBC library from a `StorageFile` and applies the same immutable-copy/container/reflection checks as `Create`. This avoids duplicating `IBuffer` file-reading plumbing in applications that intentionally use file-backed shaders.

## LoadFromApplicationUriAsync

```csharp
public static IAsyncOperation<HlslShaderLibrary> LoadFromApplicationUriAsync(
    Uri uri,
    HlslShaderProfile profile);
```

Loads a packaged application resource, for example:

```csharp
var library = await HlslShaderLibrary.LoadFromApplicationUriAsync(
    new Uri("ms-appx:///Hlsl/Effects/MyGlass.dxbc"),
    HlslShaderProfile.Pixel40);
```

Managed `<HlslCompositionShader>` consumers publish generated shader libraries under the `Hlsl\...` application-content path by default, so `ms-appx:///Hlsl/...` is the normal managed packaged-resource contract.

Native C++ consumers instead generate self-contained `.g.h` byte arrays by default and do **not** duplicate the same bytecode as loose application content. A native project can opt into packaged/loose DXBC by setting:

```xml
<HlslCompositionPublishAsContent>true</HlslCompositionPublishAsContent>
```

The intermediate DXBC is still produced during a native build because it is the canonical FXC compilation result from which the generated header is emitted; it simply stays under the intermediate output tree unless publishing is requested.

## Profile

Returns `Level91`, `Level93`, or `Pixel40`. The profile remains an explicit build/runtime contract; the current implementation does not infer the private Composition profile byte from arbitrary DXBC.

## Bytecode

```csharp
public IBuffer Bytecode { get; }
```

Returns a copy of the immutable DXBC payload. This is intended for application-managed persistent shader caches:

```text
first run: HLSL -> HlslCompiler.CompileAsync -> Bytecode -> disk/cache
later:     disk/cache -> HlslShaderLibrary.Create/LoadFromFileAsync -> HlslEffect.CreateCompiled*
```

No private Composition object is required to compile, load, or serialize this bytecode.

## Defensive validation

When arbitrary bytecode is associated with an effect, one-time reflection checks its public ABI:

- `Color`: `float4 PSBody(float4 color)`.
- `Sampler`: every `PSBody*` edge-mode export uses `float4(float2 uv, float4 samplerDataExt)`.
- `MaterializedSampler`: every `PSBody*` export uses `float4(float2 uv, float4 samplerDataExt, float4 samplerData)` and the library exports `float4 MaterializeColor(float4 color)`.
- Declared scalar properties must match `cbuffer UserConstants : register(b0)` in order, offset, and padded size.

These checks happen when accepting external/cached bytecode; they are not part of the per-frame rendering or animation path. For package-owned production shaders, prefer `<HlslCompositionShader>` so FXC catches source and entry-point failures during the build.

## Lifetime and registry memory

When a library is ultimately registered as a private Composition effect, the runtime must own the native shader payload for as long as Composition can retain the corresponding synthetic effect type. That registry is process-lifetime by design.

Deterministic descriptor keys use SHA-256 fingerprints rather than embedding a second copy of the complete DXBC payload, so registry identity does not double the resident size of large precompiled shaders. See [Runtime safety and lifetime contract](../design/runtime-safety.md).

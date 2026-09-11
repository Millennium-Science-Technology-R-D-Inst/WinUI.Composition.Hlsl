# HlslShaderLibrary class

Owns an immutable precompiled DXBC shader-linking library.

**Namespace:** `WinUI.Composition.Hlsl`
**Assembly:** `WinUI.Composition.Hlsl.dll`

## Create

```csharp
public static HlslShaderLibrary Create(IBuffer bytecode, HlslShaderProfile profile);
```

The input is copied. It must be a DXBC library no larger than 16 MiB and must export `PSBody`. The selected profile must match the FXC library target.

## Profile

Returns `Level91`, `Level93`, or `Pixel40` (`lib_4_0`).


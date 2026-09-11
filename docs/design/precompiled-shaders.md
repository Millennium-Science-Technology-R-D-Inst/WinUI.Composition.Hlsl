# Precompiled shader libraries

## Goal

Built-in effects should not compile invariant HLSL source on the consumer's machine. Dynamic source-based APIs remain supported for development and runtime-generated effects.

## Build-time path

`Shaders/*.hlsl` is compiled by the Visual C++/Windows SDK `FXC` MSBuild task. For the private Composition linker used by this project, the output must be a shader-linking DXBC library in the SM4 family, not a standalone pixel shader and not DXIL/SM6.

For LiquidGlass the project uses:

```xml
<FXC Source="$(ProjectDir)Shaders\LiquidGlass.hlsl"
     ShaderType="Library"
     ShaderModel="4.0"
     HeaderFileOutput="$(GeneratedFilesDir)Hlsl\LiquidGlassShader.g.h"
     ObjectFileOutput="$(GeneratedFilesDir)Hlsl\LiquidGlassShader.dxbc"
     VariableName="g_LiquidGlassShader"
     TreatWarningAsError="true"
     SuppressStartupBanner="true"
     AdditionalOptions="/Ges /O3" />
```

The generated header embeds the DXBC bytecode in the native DLL. `CustomEffectDefinition` points at that bytecode and the runtime passes it directly to DWM's shader-linking body. No `D3DCompile` call is required for that built-in effect.

## Runtime source path

The existing `HlslEffect.Create*` APIs accept source strings. Those definitions still populate `shaderSource`; the registry compiles the source once with `D3DCompile` and caches the resulting library blob for the process.

Thus the runtime has two explicit payload forms:

- source HLSL: dynamic/development path, compiled once at runtime;
- precompiled DXBC library: production/built-in path, compiled by MSBuild.

The registry requires exactly one representation and deep-copies whichever representation is supplied.

## Why FXC instead of DXC

This project is not creating an ordinary application-owned D3D12 shader. The reverse-engineered DWM/WUCEffectsI path loads an SM4 shader-linking library and links exported functions with Microsoft's own fragment modules. The observed profile byte maps to `lib_4_0_level_9_1_ps_only`, `lib_4_0_level_9_3_ps_only`, or `lib_4_0`. A DXC-produced SM6/DXIL library is therefore not a drop-in replacement for this ABI.

FXC is also already integrated into the Visual C++ MSBuild toolchain, can produce the required library target, a `.dxbc` object, and a generated C/C++ byte-array header, so a separate shader compiler executable or custom packaging format is unnecessary for built-in effects.

## Proposed public API direction

Do not remove the source-string APIs. If consumers later need to ship their own precompiled libraries, add an explicit immutable bytecode object rather than overloading `String shader`:

```idl
enum HlslShaderProfile
{
    Level91,
    Level93,
    Pixel40,
};

runtimeclass HlslShaderLibrary
{
    HlslShaderLibrary(Windows.Storage.Streams.IBuffer bytecode, HlslShaderProfile profile);
    HlslShaderProfile Profile{ get; };
}
```

Then add compiled overloads such as `CreateCompiledSampler` / `CreateCompiledColor`. This keeps source compilation and bytecode consumption type-safe and makes the shader profile explicit.

A future build-transitive NuGet target can compile consumer `.hlsl` files with the same FXC settings and generate descriptors automatically. That is deliberately separate from this change: the first step is to make the runtime payload model and built-in LiquidGlass use precompiled bytecode correctly.

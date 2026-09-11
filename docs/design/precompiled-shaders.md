# Precompiled shader libraries

## Goal

Built-in and production effects should be able to avoid runtime `D3DCompile`. Dynamic source-based APIs remain available for development and runtime-generated effects.

## Built-in build-time path

`Shaders/*.hlsl` is compiled by the standard Visual C++ `FxCompile` MSBuild item. For the private Composition shader linker used by this project, the payload must be an FXC SM4 shader-linking DXBC library, not a standalone pixel shader and not DXIL/SM6.

LiquidGlass uses:

```xml
<FxCompile Include="Shaders\LiquidGlass.hlsl">
  <EntryPointName />
  <ShaderType>Library</ShaderType>
  <ShaderModel>4.0</ShaderModel>
  <HeaderFileOutput>$(IntDir)LiquidGlassShader.g.h</HeaderFileOutput>
  <ObjectFileOutput>$(IntDir)LiquidGlassShader.dxbc</ObjectFileOutput>
  <VariableName>g_LiquidGlassShader</VariableName>
  <TreatWarningAsError>true</TreatWarningAsError>
  <SuppressStartupBanner>true</SuppressStartupBanner>
  <AdditionalOptions>/Ges /O3 %(AdditionalOptions)</AdditionalOptions>
</FxCompile>
```

`ShaderType=Library` plus `ShaderModel=4.0` produces the `lib_4_0` shader-linking library. `EntryPointName` is empty because the library exposes exported functions instead of compiling one fixed `main` entry point.

The generated C/C++ byte-array header is included only in the global module fragment of `LiquidGlassShader.ixx`; it is not exported through the C++ module interface. The embedded bytes are deep-copied by the runtime registry when the effect definition is registered. Built-in LiquidGlass therefore reaches the private linker without calling `D3DCompile` at runtime.

## Runtime source path

Source-string `HlslEffect.Create*` APIs populate HLSL source. The runtime compiles that source once with `D3DCompile` and caches the shader library for the process. Compiler diagnostics are propagated as an `hresult_error` message identifying `UserShader.hlsl`.

The two payload forms are intentionally distinct:

- source HLSL: dynamic/development path;
- precompiled DXBC library: production/build-time path.

A definition must contain exactly one payload form.

## Public precompiled API

Consumers create an immutable bytecode object instead of overloading a `String` with source/path/bytecode semantics:

```idl
enum HlslShaderProfile
{
    Level91,
    Level93,
    Pixel40,
};

runtimeclass HlslShaderLibrary
{
    static HlslShaderLibrary Create(Windows.Storage.Streams.IBuffer bytecode, HlslShaderProfile profile);
    HlslShaderProfile Profile{ get; };
}
```

`HlslEffect.CreateCompiledColor`, `CreateCompiledSampler`, `CreateCompiledColorWithProperties`, and `CreateCompiledSamplerWithProperties` consume this object. The WinRT API exposes `IBuffer`, not a native pointer; internally the bytes are owned by `std::vector<std::uint8_t>`.

Before registration, the library is inspected with `D3DReflectLibrary`. Malformed/non-library DXBC, missing required exports, incompatible public function signatures, and mismatched `UserConstants` scalar layouts fail before reaching the private Composition linker. The selected `HlslShaderProfile` remains an explicit caller/build contract; profile-byte inference from the DXBC library is not implemented yet.

### Public compiled ABI

Color libraries export:

```hlsl
export float4 PSBody(float4 color);
```

Sampler libraries export the same `float4(float2 uv, float4 samplerDataExt)` ABI for `PSBody` and the clamp/wrap/mirror variants used by the Composition linker (`PSBodyCC`, `PSBodyCW`, ..., `PSBodyM`). Dynamic sampler source uses a simpler `Shade` function because the runtime generates those wrappers before `D3DCompile`; precompiled libraries must contain them at build time.

If scalar properties are declared, the library must contain `cbuffer UserConstants : register(b0)` with the declared scalar names in API order. The runtime validates offsets and the required 16-byte cbuffer padding.

## Native NuGet consumer build integration

A C++/WinRT consumer can declare:

```xml
<ItemGroup>
  <HlslCompositionShader Include="Effects\MyGlass.hlsl">
    <Kind>Sampler</Kind>
    <ShaderModel>4.0</ShaderModel>
  </HlslCompositionShader>
</ItemGroup>
```

The NuGet package installs its native target through `buildTransitive/native`. The target converts each item to a standard Visual C++ `FxCompile` library item, emits a generated C/C++ header under `$(IntDir)Hlsl`, emits `$(IntDir)Hlsl\<name>.dxbc`, and copies the `.dxbc` into `$(OutDir)Hlsl` as a language-neutral build output.

`Kind` is currently validation metadata (`Color` or `Sampler`); it does not synthesize HLSL wrappers. The HLSL file must therefore export the compiled ABI described above. Output naming currently uses `%(Filename)`, so two input files with the same leaf name are not supported without further target work.

## CsWinRT / managed consumers

The WinRT bytecode consumption API is language-neutral: a C# or other CsWinRT consumer can load build-produced DXBC into an `IBuffer`, create `HlslShaderLibrary`, and call the compiled effect APIs. It is not tied to C++ Modules.

However, the current managed NuGet target does **not** automatically turn `<HlslCompositionShader>` into Visual C++ `FxCompile`, because SDK-style C# projects do not import the Visual C++ HLSL build task pipeline. Automatic managed-project HLSL compilation therefore remains a build-integration gap. A future shared shader-build target should either invoke the Windows SDK FXC tool explicitly with equivalent `lib_4_0` arguments or use a dedicated MSBuild task, while keeping the emitted `.dxbc` contract identical.

## Why FXC instead of DXC

This runtime is not creating an ordinary application-owned D3D12 shader. The reverse-engineered DWM/WUCEffectsI path consumes SM4 shader-linking libraries and links exported functions with Microsoft's private fragments. The observed private profile byte maps to the `lib_4_0_level_9_1_ps_only`, `lib_4_0_level_9_3_ps_only`, or `lib_4_0` family. DXC-produced SM6/DXIL is therefore not a drop-in payload for this ABI.

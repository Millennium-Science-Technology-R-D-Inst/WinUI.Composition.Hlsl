# HlslRuntimeCapabilities class

Reports the package capability contract for the current native architecture.

Namespace: `WinUI.Composition.Hlsl`

## Definition

```text
[default_interface] runtimeclass HlslRuntimeCapabilities
```

Retrieve the object with `HlslComposition.GetRuntimeCapabilities()`.

```csharp
var caps = HlslComposition.GetRuntimeCapabilities();
```

## Properties

| Property | Type | Description |
| --- | --- | --- |
| `Architecture` | `HlslNativeArchitecture` | Current process architecture (`X86`, `X64`, `Arm64`, or `Unknown`). |
| `SupportLevel` | `HlslRuntimeSupportLevel` | Package support level for the architecture. x86, x64, and ARM64 currently report `Validated`. |
| `NativeAdapterAvailable` | `Boolean` | Indicates that the package contains the private Composition adapter for the architecture. |
| `SupportsCompositionGraphNodes` | `Boolean` | Custom HLSL can participate in the Composition graphics-effect graph path. |
| `SupportsMaterializedGraphs` | `Boolean` | Single-source materialized custom sampling is supported. |
| `SupportsLinkedMultiSource` | `Boolean` | Linked `Color`/`Sampler` effects can use multiple ordered sources. |
| `SupportsMaterializedMultiSource` | `Boolean` | Multiple independently materialized sources are publicly supported. Currently `false`. |
| `SupportsMultipleCustomNodes` | `Boolean` | Multiple custom HLSL nodes in one lowered graph are publicly supported. Currently `false`. |
| `SupportsNativeNodesAfterCustom` | `Boolean` | Arbitrary native graph nodes after a custom materialized pass are publicly supported. Currently `false`. |
| `SupportsVectorProperties` | `Boolean` | `Vector2`, `Vector3`, and `Vector4` typed properties are supported. |
| `SupportsMatrixProperties` | `Boolean` | `Matrix3x2` and `Matrix4x4` typed properties are supported. |
| `SupportsAsyncCompilation` | `Boolean` | Asynchronous `HlslCompiler` APIs are available. |
| `UsesPrivateCompositionAbi` | `Boolean` | Custom shader execution uses a private Composition implementation ABI. |

## Current architecture contract

| Architecture | SupportLevel | Graph nodes | Single materialized graph | Linked multi-source | Vector/matrix properties |
| --- | --- | --- | --- | --- | --- |
| x64 | `Validated` | Yes | Yes | Yes | Yes |
| x86 | `Validated` | Yes | Yes | Yes | Yes |
| ARM64 | `Validated` | Yes | Yes | Yes | Yes |

The NuGet package requires Windows App SDK 1.6 or later.

x86/x64 private-adapter resolution is version-independent rather than selected from a hard-coded Windows App SDK version table. The project therefore does not impose an artificial upper Windows App SDK version in the package dependency. If the underlying private mechanism is removed or fundamentally redesigned by Windows, adapter activation is expected to fail closed rather than operate on an unknown layout.

## Remarks

This type reports **publicly supported behavior**, not every code path that exists internally. A capability remains `false` while an experimental lowering path lacks the topology/bounds/runtime validation required to make it part of the public contract.

`GetRuntimeCapabilities()` intentionally does not perform expensive runtime discovery. It is a side-effect-free description of the shipped architecture implementation. Resolver/ABI work is deferred until a custom effect actually needs the private backend.

Do not poll this object in a render loop. Capability values are stable for the lifetime of a process/package configuration.

## Example

```csharp
var caps = HlslComposition.GetRuntimeCapabilities();

if (!caps.SupportsLinkedMultiSource)
{
    throw new NotSupportedException("This effect requires linked multi-source HLSL.");
}
```

For functionality that is already a hard application requirement, it is also reasonable to declare that requirement in application/package documentation and call the API directly rather than branching on capabilities at every call site.

## See also

- [HlslComposition](hlsl-composition.md)
- [Architecture](../architecture.md)
- [Runtime safety](../design/runtime-safety.md)

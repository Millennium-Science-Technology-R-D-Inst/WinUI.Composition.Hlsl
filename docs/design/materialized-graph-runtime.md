# Materialized graph compilation

The LiquidGlass graph is `Backdrop -> GaussianBlur -> custom sampler`. The native
traverser also inserts composite wrappers. On Windows App Runtime 2.4.0.0 x64,
this produces four subgraphs, not the three in the standalone shader template.
Returning that template as the complete result caused `EffectInstance` to access
a missing constant-buffer entry (`effectinstance.cpp:307`, `0x8000000B`). The
custom property updater also used subgraph slot 1 instead of effect node 2.

`CompileMaterializedGraph` now reads subgraph membership from the traversed graph
and requires an isolated terminal custom sampler followed by a composite wrapper.
It asks the native compiler to compile the whole graph, supplying a temporary
identity code-generation body for the custom node. This copies the node locally;
the original description, properties, and effect registry are not modified.

The returned object retains the native compiled result and borrows its upstream
vectors, shader bodies, and property updaters. Only the custom sampler and final
identity body are replaced with the precompiled library. Their input bindings
come from the native result. The final identity body uses the custom library's
profile so it can link with the custom sampler. Constant-buffer property updaters
use the graph's effect-node index, independently of the shader's subgraph index.
Owned vectors are released separately from the retained native result.

This remains a private x64 ABI. It does not implement multiple custom nodes or
arbitrary downstream native effects. A materialized-input declaration by itself
is not evidence that the standalone three-stage template matches a native graph.

## Regression check

Build the library with `build.ps1 -Offline`, and the C++ demo with
`tests/build.ps1 -Language Cpp`. The demo consumes a NuGet package; for local source
testing, copy `artifacts/x64/Debug/WinUI.Composition.Hlsl.dll` into
`tests/Cpp/Output/x64/Debug` **after** building the demo, or rebuild the package.
Otherwise the package's older DLL may overwrite the local fix.

Run `WUILiquidGlassDemo_Hlsl.exe --smoke` from the demo output directory. Require a
fresh `smoke.log` with the PASS marker and successful process exit. The test waits
between changes so the composition thread processes each batch. It covers glass
creation, scalar updates, resize, blur radii 0 and 64, dispersion, effect switching,
and material recreation. This checks runtime stability, not pixel correctness.

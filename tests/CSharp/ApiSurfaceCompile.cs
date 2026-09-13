using System;
using Microsoft.UI.Composition;
using Windows.Storage;
using WinUI.Composition.Hlsl;

namespace HlslCSharpConsumer;

internal static class ApiSurfaceCompile
{
    private const string MaterializedShader = """
        float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData)
        {
            return texture0.Sample(sampler0, uv + samplerDataExt.zw * 0.0f + samplerData.xy * 0.0f);
        }
        """;

    // Intentionally never called. This keeps the managed projection/API surface in
    // the normal compiler graph so CI catches IDL/projection drift without touching
    // the private Composition runtime during the build.
    private static void Validate(HlslShaderLibrary library, Compositor compositor, StorageFile file)
    {
        _ = HlslComposition.GetRuntimeCapabilities();
        _ = HlslCompiler.CompileAsync(
            MaterializedShader,
            HlslEffectKind.MaterializedSampler,
            HlslShaderProfile.Pixel40);
        _ = HlslCompiler.CompileWithDefinesAsync(
            MaterializedShader,
            HlslEffectKind.MaterializedSampler,
            HlslShaderProfile.Pixel40,
            new[] { "RUNTIME_VARIANT=1" });
        _ = HlslCompiler.CompileWithPropertiesAndDefinesAsync(
            MaterializedShader,
            HlslEffectKind.MaterializedSampler,
            HlslShaderProfile.Pixel40,
            Array.Empty<HlslFloatProperty>(),
            new[] { "RUNTIME_VARIANT=2" });
        _ = HlslShaderLibrary.LoadFromFileAsync(file, HlslShaderProfile.Pixel40);
        _ = HlslShaderLibrary.LoadFromApplicationUriAsync(
            new Uri("ms-appx:///Hlsl/ConsumerMaterializedSampler.dxbc"),
            HlslShaderProfile.Pixel40);
        _ = HlslShaderLibrary.LoadGeneratedFromFileAsync(file);
        _ = HlslShaderLibrary.LoadGeneratedFromApplicationUriAsync(
            new Uri("ms-appx:///Hlsl/ConsumerMaterializedSampler.dxbc"));

        // Compile-only checks for the generated-bytecode convenience surface. The
        // placeholder bytes are never evaluated because this method is never called.
        byte[] generatedBytes = [0x44, 0x58, 0x42, 0x43];
        _ = HlslShaderLibrary.CreateFromGeneratedByteArray(generatedBytes);
        _ = library.EffectKind;

        var effect = HlslEffect.CreateCustomMaterializedSampler(MaterializedShader);
        _ = HlslEffect.CreateCompiled(Guid.Empty, library);
        _ = HlslEffect.CreateCompiledFromGeneratedByteArray(Guid.Empty, generatedBytes);
        _ = HlslEffect.CreateCompiledMaterializedSampler(Guid.Empty, library);
        var graph = effect.CreateGraphicsEffect();
        var paths = effect.GetAnimatablePropertyPaths();
        _ = compositor.CreateEffectFactory(graph, paths);

        var factory = HlslComposition.CreateEffectFactory(compositor, effect);
        var brush = factory.CreateBrush();
        _ = factory.Factory;
        _ = brush.EffectBrush;
        _ = brush.Properties;
        _ = library.Bytecode;
        _ = HlslComposition.CreateXamlBrushFromCompositionBrush(brush.Brush);
    }
}

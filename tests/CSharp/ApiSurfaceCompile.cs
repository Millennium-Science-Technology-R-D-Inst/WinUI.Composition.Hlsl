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

    private const string MultiSourceSamplerShader = """
        float4 Shade(float2 uv0, float4 samplerDataExt0, float2 uv1, float4 samplerDataExt1)
        {
            float4 first = texture0.Sample(sampler0, uv0);
            float4 second = texture1.Sample(sampler1, uv1);
            return lerp(first, second, 0.5f + (samplerDataExt0.x + samplerDataExt1.x) * 0.0f);
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
            HlslEffectKind.Auto,
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
        _ = HlslCompiler.CompileAdvancedAsync(
            MultiSourceSamplerShader,
            HlslEffectKind.Auto,
            HlslShaderProfile.Pixel40,
            2);
        _ = HlslCompiler.CompileAdvancedWithPropertiesAndDefinesAsync(
            MultiSourceSamplerShader,
            HlslEffectKind.Sampler,
            HlslShaderProfile.Pixel40,
            2,
            Array.Empty<HlslFloatProperty>(),
            new[] { "MULTI_SOURCE=1" });
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
        _ = library.SourceCount;

        var effect = HlslEffect.CreateCustomMaterializedSampler(MaterializedShader);
        _ = HlslEffect.CreateCompiled(Guid.Empty, library);
        _ = HlslEffect.CreateCompiledFromGeneratedByteArray(Guid.Empty, generatedBytes);
        _ = HlslEffect.CreateCompiledMaterializedSampler(Guid.Empty, library);

        string[] sourceNames = ["First", "Second"];
        var multiSourceEffect = HlslEffect.CreateAdvanced(
            MultiSourceSamplerShader,
            HlslEffectKind.Sampler,
            sourceNames,
            Array.Empty<HlslProperty>());
        _ = multiSourceEffect.SourceNames;
        _ = HlslComposition.CreateBackdropBrush(compositor, multiSourceEffect);
        _ = HlslEffect.CreateCompiledAdvanced(
            Guid.Empty,
            library,
            HlslEffectKind.Sampler,
            sourceNames,
            Array.Empty<HlslProperty>());

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

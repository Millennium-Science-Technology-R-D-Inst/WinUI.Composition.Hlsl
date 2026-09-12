using System;
using Microsoft.UI.Composition;
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
    private static void Validate(HlslShaderLibrary library, Compositor compositor)
    {
        _ = HlslComposition.GetRuntimeCapabilities();
        _ = HlslCompiler.CompileAsync(
            MaterializedShader,
            HlslEffectKind.MaterializedSampler,
            HlslShaderProfile.Pixel40);

        var effect = HlslEffect.CreateMaterializedSampler(Guid.Empty, MaterializedShader);
        _ = HlslEffect.CreateCompiledMaterializedSampler(Guid.Empty, library);
        _ = effect.CreateGraphicsEffect();

        var factory = HlslComposition.CreateEffectFactory(compositor, effect);
        var brush = factory.CreateBrush();
        _ = factory.Factory;
        _ = brush.EffectBrush;
        _ = brush.Properties;
        _ = library.Bytecode;
        _ = HlslComposition.CreateXamlBrushFromCompositionBrush(brush.Brush);
    }
}

using System;
using System.Numerics;
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

    private const string TypedPropertyShader = """
        float4 Shade(float2 uv, float4 samplerDataExt)
        {
            float2 transformed = mul(float3(uv, 1.0f), Transform);
            float4 projected = mul(float4(transformed + Offset, 0.0f, 1.0f), Projection);
            float4 color = texture0.Sample(sampler0, projected.xy + samplerDataExt.zw * 0.0f);
            return color * float4(Gain, 1.0f) * Tint * Strength;
        }
        """;

    private static HlslProperty[] TypedProperties() =>
    [
        new("Strength", HlslPropertyType.Scalar, new float[] { 1.0f }),
        new("Offset", HlslPropertyType.Vector2, new float[] { 0.0f, 0.0f }),
        new("Gain", HlslPropertyType.Vector3, new float[] { 1.0f, 1.0f, 1.0f }),
        new("Tint", HlslPropertyType.Vector4, new float[] { 1.0f, 1.0f, 1.0f, 1.0f }),
        new("Transform", HlslPropertyType.Matrix3x2, new float[] { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f }),
        new("Projection", HlslPropertyType.Matrix4x4, new float[]
        {
            1.0f,0.0f,0.0f,0.0f,
            0.0f,1.0f,0.0f,0.0f,
            0.0f,0.0f,1.0f,0.0f,
            0.0f,0.0f,0.0f,1.0f,
        }),
    ];

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

        var typedProperties = TypedProperties();
        _ = HlslCompiler.CompileWithTypedPropertiesAsync(
            TypedPropertyShader,
            HlslEffectKind.Sampler,
            HlslShaderProfile.Pixel40,
            typedProperties);
        _ = HlslCompiler.CompileWithTypedPropertiesAndDefinesAsync(
            TypedPropertyShader,
            HlslEffectKind.Sampler,
            HlslShaderProfile.Pixel40,
            typedProperties,
            new[] { "TYPED_PROPERTIES=1" });
        _ = HlslCompiler.CompileAdvancedWithTypedPropertiesAsync(
            TypedPropertyShader,
            HlslEffectKind.Sampler,
            HlslShaderProfile.Pixel40,
            1,
            typedProperties);
        _ = HlslCompiler.CompileAdvancedWithTypedPropertiesAndDefinesAsync(
            TypedPropertyShader,
            HlslEffectKind.Sampler,
            HlslShaderProfile.Pixel40,
            1,
            typedProperties,
            new[] { "TYPED_PROPERTIES=2" });

        _ = HlslShaderLibrary.LoadFromFileAsync(file, HlslShaderProfile.Pixel40);
        _ = HlslShaderLibrary.LoadFromApplicationUriAsync(
            new Uri("ms-appx:///Hlsl/ConsumerMaterializedSampler.dxbc"),
            HlslShaderProfile.Pixel40);
        _ = HlslShaderLibrary.LoadGeneratedFromFileAsync(file);
        _ = HlslShaderLibrary.LoadGeneratedFromApplicationUriAsync(
            new Uri("ms-appx:///Hlsl/ConsumerMaterializedSampler.dxbc"));

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

        CompositionBrush[] compositionSources =
        [
            compositor.CreateBackdropBrush(),
            compositor.CreateBackdropBrush(),
        ];
        _ = HlslComposition.CreateBrushWithSources(
            compositor,
            multiSourceEffect,
            compositionSources);

        _ = HlslEffect.CreateCompiledAdvanced(
            Guid.Empty,
            library,
            HlslEffectKind.Sampler,
            sourceNames,
            Array.Empty<HlslProperty>());

        var typedEffect = HlslEffect.CreateAdvanced(
            TypedPropertyShader,
            HlslEffectKind.Sampler,
            new[] { "Backdrop" },
            typedProperties);
        var typedBrush = HlslComposition.CreateBackdropBrush(compositor, typedEffect);
        typedBrush.SetFloat("Strength", 0.75f);
        typedBrush.SetVector2("Offset", new Vector2(1.0f, 2.0f));
        typedBrush.SetVector3("Gain", new Vector3(1.0f, 0.9f, 0.8f));
        typedBrush.SetVector4("Tint", Vector4.One);
        typedBrush.SetMatrix3x2("Transform", Matrix3x2.CreateTranslation(4.0f, 8.0f));
        typedBrush.SetMatrix4x4("Projection", Matrix4x4.Identity);

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

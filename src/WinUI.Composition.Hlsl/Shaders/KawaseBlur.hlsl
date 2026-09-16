// Dual-Kawase-style blur kernels for the WinUI Composition custom-effect path.
//
// The down/up tap patterns are adapted from the public D3D11 liquid-glass
// implementation in poncippg-spec/liquidDX11. Unlike an app-owned render-target
// chain, Composition owns the intermediates here: native D2D Scale nodes establish
// each pyramid level and these custom samplers provide the Kawase reconstruction
// kernels at the destination resolution.
//
// A finite materialized Composition surface must never be treated as transparent
// black outside its logical content rectangle. MirrorContentUv folds every tap back
// into the valid content rect; this is deliberately independent from LiquidGlass
// coverage so blur/refraction cannot alter the final rounded-rect silhouette.

Texture2D texture0;
SamplerState sampler0;

cbuffer KawaseConstants : register(b0)
{
    // Down/Up: tap spread in source texels. Resolve: raw/blurred mix in [0,1].
    float Amount;
    float3 _padding;
};

float Mirror01(float value)
{
    float folded = abs(value);
    folded = folded - 2.0f * floor(folded * 0.5f);
    return 1.0f - abs(folded - 1.0f);
}

float2 MirrorContentUv(float2 uv, float4 samplerData, float4 samplerDataExt)
{
    float2 lo = min(samplerData.xy, samplerData.zw);
    float2 hi = max(samplerData.xy, samplerData.zw);
    float2 span = hi - lo;
    bool hasContentRect = all(span > float2(1e-6f, 1e-6f));
    if (!hasContentRect)
    {
        lo = float2(0.0f, 0.0f);
        hi = float2(1.0f, 1.0f);
        span = float2(1.0f, 1.0f);
    }

    float2 local = (uv - lo) / max(span, float2(1e-6f, 1e-6f));
    local = float2(Mirror01(local.x), Mirror01(local.y));
    float2 mirrored = lo + local * span;

    // Stay on texel centres at the materialized texture edge. samplerDataExt.zw
    // contains inverse source width/height for DWM custom samplers.
    float2 halfTexel = max(abs(samplerDataExt.zw) * 0.5f, float2(1e-6f, 1e-6f));
    float2 center = (lo + hi) * 0.5f;
    float2 innerLo = min(lo + halfTexel, center);
    float2 innerHi = max(hi - halfTexel, center);
    return clamp(mirrored, innerLo, innerHi);
}

float4 SampleSource(float2 uv, float4 samplerDataExt, float4 samplerData)
{
    return texture0.Sample(sampler0, MirrorContentUv(uv, samplerData, samplerDataExt));
}

float4 KawaseDownCore(float2 uv, float4 samplerDataExt, float4 samplerData)
{
    // Standard Dual Kawase downsample kernel: centre weight 4 plus four diagonal
    // taps. The preceding native Scale node owns the actual 1/2-resolution target.
    float2 h = abs(samplerDataExt.zw) * (0.5f * max(Amount, 0.0f));
    float4 sum = SampleSource(uv, samplerDataExt, samplerData) * 4.0f;
    sum += SampleSource(uv + float2(-h.x, -h.y), samplerDataExt, samplerData);
    sum += SampleSource(uv + float2( h.x, -h.y), samplerDataExt, samplerData);
    sum += SampleSource(uv + float2(-h.x,  h.y), samplerDataExt, samplerData);
    sum += SampleSource(uv + float2( h.x,  h.y), samplerDataExt, samplerData);
    return sum * (1.0f / 8.0f);
}

float4 KawaseUpCore(float2 uv, float4 samplerDataExt, float4 samplerData)
{
    // Standard Dual Kawase reconstruction kernel: four axial taps (weight 1)
    // and four diagonal taps (weight 2). The preceding Scale node owns the 2x
    // destination resolution while this pass supplies the wide reconstruction.
    float2 h = abs(samplerDataExt.zw) * (0.5f * max(Amount, 0.0f));
    float4 sum = SampleSource(uv + float2(-h.x * 2.0f, 0.0f), samplerDataExt, samplerData);
    sum += SampleSource(uv + float2(-h.x,  h.y), samplerDataExt, samplerData) * 2.0f;
    sum += SampleSource(uv + float2(0.0f,  h.y * 2.0f), samplerDataExt, samplerData);
    sum += SampleSource(uv + float2( h.x,  h.y), samplerDataExt, samplerData) * 2.0f;
    sum += SampleSource(uv + float2( h.x * 2.0f, 0.0f), samplerDataExt, samplerData);
    sum += SampleSource(uv + float2( h.x, -h.y), samplerDataExt, samplerData) * 2.0f;
    sum += SampleSource(uv + float2(0.0f, -h.y * 2.0f), samplerDataExt, samplerData);
    sum += SampleSource(uv + float2(-h.x, -h.y), samplerDataExt, samplerData) * 2.0f;
    return sum * (1.0f / 12.0f);
}

// Linked-color resolve keeps BlurRadius=0 bit-exact with the raw backdrop even
// though the fixed Composition graph retains its down/up pyramid. Amount is the
// blur mix for this definition, not the Kawase tap spread.
export float4 KawaseResolve(float4 rawColor, float4 blurredColor)
{
    return lerp(rawColor, blurredColor, saturate(Amount));
}

// MaterializedTexture lowering needs a color passthrough for source/final helper
// subgraphs. The sampler pass itself is still the PSBody-style custom sampler.
export float4 MaterializeColor(float4 color) { return color; }

export float4 KawaseDown(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseDownCore(uv, samplerDataExt, samplerData); }
export float4 KawaseDownCC(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseDownCore(uv, samplerDataExt, samplerData); }
export float4 KawaseDownCW(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseDownCore(uv, samplerDataExt, samplerData); }
export float4 KawaseDownCM(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseDownCore(uv, samplerDataExt, samplerData); }
export float4 KawaseDownWC(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseDownCore(uv, samplerDataExt, samplerData); }
export float4 KawaseDownWW(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseDownCore(uv, samplerDataExt, samplerData); }
export float4 KawaseDownWM(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseDownCore(uv, samplerDataExt, samplerData); }
export float4 KawaseDownMC(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseDownCore(uv, samplerDataExt, samplerData); }
export float4 KawaseDownMW(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseDownCore(uv, samplerDataExt, samplerData); }
export float4 KawaseDownMM(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseDownCore(uv, samplerDataExt, samplerData); }
export float4 KawaseDownC(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseDownCore(uv, samplerDataExt, samplerData); }
export float4 KawaseDownW(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseDownCore(uv, samplerDataExt, samplerData); }
export float4 KawaseDownM(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseDownCore(uv, samplerDataExt, samplerData); }

export float4 KawaseUp(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseUpCore(uv, samplerDataExt, samplerData); }
export float4 KawaseUpCC(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseUpCore(uv, samplerDataExt, samplerData); }
export float4 KawaseUpCW(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseUpCore(uv, samplerDataExt, samplerData); }
export float4 KawaseUpCM(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseUpCore(uv, samplerDataExt, samplerData); }
export float4 KawaseUpWC(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseUpCore(uv, samplerDataExt, samplerData); }
export float4 KawaseUpWW(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseUpCore(uv, samplerDataExt, samplerData); }
export float4 KawaseUpWM(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseUpCore(uv, samplerDataExt, samplerData); }
export float4 KawaseUpMC(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseUpCore(uv, samplerDataExt, samplerData); }
export float4 KawaseUpMW(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseUpCore(uv, samplerDataExt, samplerData); }
export float4 KawaseUpMM(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseUpCore(uv, samplerDataExt, samplerData); }
export float4 KawaseUpC(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseUpCore(uv, samplerDataExt, samplerData); }
export float4 KawaseUpW(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseUpCore(uv, samplerDataExt, samplerData); }
export float4 KawaseUpM(float2 uv, float4 samplerDataExt, float4 samplerData) { return KawaseUpCore(uv, samplerDataExt, samplerData); }

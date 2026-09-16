// Full-resolution separable Gaussian blur used by LiquidGlassMaterial.
//
// The logical kernel radius is 20 and sigma is radius/3. CPU-side setup computes
// normalized Gaussian weights, then merges adjacent integer taps (1,2), (3,4),
// ... (19,20) into ten bilinear samples. A horizontal pass followed by a vertical
// pass therefore reproduces the 41-tap reference kernel with 21 texture reads per
// pass while keeping the sample count constant as BlurRadius changes.
//
// BlurAmount scales sample spacing. LiquidGlassMaterial maps authored BlurRadius
// to BlurAmount = BlurRadius / 20, so the farthest tap remains approximately the
// authored radius and the effective sigma remains approximately BlurRadius/3.

Texture2D texture0;
SamplerState sampler0;

cbuffer BlurConstants : register(b0)
{
    float BlurAmount;
    float3 _padding0;
    float CenterWeight;
    float3 _padding1;
    float4 PairData[5];
};

float Mirror01(float value)
{
    float folded = abs(value);
    folded = folded - 2.0f * floor(folded * 0.5f);
    return folded > 1.0f ? 2.0f - folded : folded;
}

float4 SampleHorizontal(float2 uv)
{
    return texture0.Sample(sampler0, float2(Mirror01(uv.x), uv.y));
}

float4 SampleVertical(float2 uv)
{
    return texture0.Sample(sampler0, float2(uv.x, Mirror01(uv.y)));
}

float4 BlurHorizontalCore(float2 uv, float4 samplerDataExt)
{
    const float step = abs(samplerDataExt.z) * max(BlurAmount, 0.0f);
    float4 sum = texture0.Sample(sampler0, uv) * CenterWeight;

    [unroll]
    for (int block = 0; block < 5; ++block)
    {
        const float4 pair = PairData[block];
        const float offsetA = pair.x * step;
        const float weightA = pair.y;
        sum += SampleHorizontal(uv + float2(offsetA, 0.0f)) * weightA;
        sum += SampleHorizontal(uv - float2(offsetA, 0.0f)) * weightA;

        const float offsetB = pair.z * step;
        const float weightB = pair.w;
        sum += SampleHorizontal(uv + float2(offsetB, 0.0f)) * weightB;
        sum += SampleHorizontal(uv - float2(offsetB, 0.0f)) * weightB;
    }

    return sum;
}

float4 BlurVerticalCore(float2 uv, float4 samplerDataExt)
{
    const float step = abs(samplerDataExt.w) * max(BlurAmount, 0.0f);
    float4 sum = texture0.Sample(sampler0, uv) * CenterWeight;

    [unroll]
    for (int block = 0; block < 5; ++block)
    {
        const float4 pair = PairData[block];
        const float offsetA = pair.x * step;
        const float weightA = pair.y;
        sum += SampleVertical(uv + float2(0.0f, offsetA)) * weightA;
        sum += SampleVertical(uv - float2(0.0f, offsetA)) * weightA;

        const float offsetB = pair.z * step;
        const float weightB = pair.w;
        sum += SampleVertical(uv + float2(0.0f, offsetB)) * weightB;
        sum += SampleVertical(uv - float2(0.0f, offsetB)) * weightB;
    }

    return sum;
}

export float4 FlattenSource(float4 color) { return color; }

export float4 BlurHorizontal(float2 uv, float4 samplerDataExt) { return BlurHorizontalCore(uv, samplerDataExt); }
export float4 BlurHorizontalCC(float2 uv, float4 samplerDataExt) { return BlurHorizontalCore(uv, samplerDataExt); }
export float4 BlurHorizontalCW(float2 uv, float4 samplerDataExt) { return BlurHorizontalCore(uv, samplerDataExt); }
export float4 BlurHorizontalCM(float2 uv, float4 samplerDataExt) { return BlurHorizontalCore(uv, samplerDataExt); }
export float4 BlurHorizontalWC(float2 uv, float4 samplerDataExt) { return BlurHorizontalCore(uv, samplerDataExt); }
export float4 BlurHorizontalWW(float2 uv, float4 samplerDataExt) { return BlurHorizontalCore(uv, samplerDataExt); }
export float4 BlurHorizontalWM(float2 uv, float4 samplerDataExt) { return BlurHorizontalCore(uv, samplerDataExt); }
export float4 BlurHorizontalMC(float2 uv, float4 samplerDataExt) { return BlurHorizontalCore(uv, samplerDataExt); }
export float4 BlurHorizontalMW(float2 uv, float4 samplerDataExt) { return BlurHorizontalCore(uv, samplerDataExt); }
export float4 BlurHorizontalMM(float2 uv, float4 samplerDataExt) { return BlurHorizontalCore(uv, samplerDataExt); }
export float4 BlurHorizontalC(float2 uv, float4 samplerDataExt) { return BlurHorizontalCore(uv, samplerDataExt); }
export float4 BlurHorizontalW(float2 uv, float4 samplerDataExt) { return BlurHorizontalCore(uv, samplerDataExt); }
export float4 BlurHorizontalM(float2 uv, float4 samplerDataExt) { return BlurHorizontalCore(uv, samplerDataExt); }

export float4 BlurVertical(float2 uv, float4 samplerDataExt) { return BlurVerticalCore(uv, samplerDataExt); }
export float4 BlurVerticalCC(float2 uv, float4 samplerDataExt) { return BlurVerticalCore(uv, samplerDataExt); }
export float4 BlurVerticalCW(float2 uv, float4 samplerDataExt) { return BlurVerticalCore(uv, samplerDataExt); }
export float4 BlurVerticalCM(float2 uv, float4 samplerDataExt) { return BlurVerticalCore(uv, samplerDataExt); }
export float4 BlurVerticalWC(float2 uv, float4 samplerDataExt) { return BlurVerticalCore(uv, samplerDataExt); }
export float4 BlurVerticalWW(float2 uv, float4 samplerDataExt) { return BlurVerticalCore(uv, samplerDataExt); }
export float4 BlurVerticalWM(float2 uv, float4 samplerDataExt) { return BlurVerticalCore(uv, samplerDataExt); }
export float4 BlurVerticalMC(float2 uv, float4 samplerDataExt) { return BlurVerticalCore(uv, samplerDataExt); }
export float4 BlurVerticalMW(float2 uv, float4 samplerDataExt) { return BlurVerticalCore(uv, samplerDataExt); }
export float4 BlurVerticalMM(float2 uv, float4 samplerDataExt) { return BlurVerticalCore(uv, samplerDataExt); }
export float4 BlurVerticalC(float2 uv, float4 samplerDataExt) { return BlurVerticalCore(uv, samplerDataExt); }
export float4 BlurVerticalW(float2 uv, float4 samplerDataExt) { return BlurVerticalCore(uv, samplerDataExt); }
export float4 BlurVerticalM(float2 uv, float4 samplerDataExt) { return BlurVerticalCore(uv, samplerDataExt); }

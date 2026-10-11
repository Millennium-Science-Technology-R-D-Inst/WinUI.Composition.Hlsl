// Two linked color inputs: blurred backdrop and raw backdrop. BloomAmount is
// the BLURRED fraction (0 = sharp, 1 = fully blurred), as in the reference's code.
// Color adjustments operate on straight color, then restore premultiplication.
cbuffer PostProcessingConstants : register(b0)
{
    float BloomAmount;
    float Brightness;
    float Temperature;
    float Vibrance;
    float Saturation;
    float Contrast;
    float Exposure; // EV stops, preserving this library's existing API convention
    float _padding;
};

export float4 PSBody(float4 blurred, float4 raw)
{
    float4 mixed = lerp(raw, blurred, saturate(BloomAmount));
    float alpha = saturate(mixed.a);
    float3 color = alpha > 1e-5f ? mixed.rgb / alpha : 0.0f.xxx;
    color *= exp2(Exposure);
    color *= float3(1.0f + Temperature * 0.25f,
                    1.0f + Temperature * 0.10f,
                    1.0f - Temperature * 0.25f);
    color += Brightness;
    color = (color - 0.5f) * Contrast + 0.5f;
    float gray = dot(color, float3(0.299f, 0.587f, 0.114f));
    float high = max(color.r, max(color.g, color.b));
    float low = min(color.r, min(color.g, color.b));
    float chroma = saturate((high - low) / max(abs(high), 1e-5f));
    color = lerp(gray.xxx, color, Saturation * (1.0f + Vibrance * (1.0f - chroma)));
    return float4(saturate(color) * alpha, alpha);
}

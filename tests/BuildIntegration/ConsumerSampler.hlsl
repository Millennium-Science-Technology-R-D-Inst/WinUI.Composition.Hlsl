float4 ConsumerSamplerCore(float2 uv, float4 samplerDataExt)
{
    return float4(uv, samplerDataExt.x, 1.0f);
}

export float4 PSBody(float2 uv, float4 samplerDataExt) { return ConsumerSamplerCore(uv, samplerDataExt); }
export float4 PSBodyCC(float2 uv, float4 samplerDataExt) { return ConsumerSamplerCore(uv, samplerDataExt); }
export float4 PSBodyCW(float2 uv, float4 samplerDataExt) { return ConsumerSamplerCore(uv, samplerDataExt); }
export float4 PSBodyCM(float2 uv, float4 samplerDataExt) { return ConsumerSamplerCore(uv, samplerDataExt); }
export float4 PSBodyWC(float2 uv, float4 samplerDataExt) { return ConsumerSamplerCore(uv, samplerDataExt); }
export float4 PSBodyWW(float2 uv, float4 samplerDataExt) { return ConsumerSamplerCore(uv, samplerDataExt); }
export float4 PSBodyWM(float2 uv, float4 samplerDataExt) { return ConsumerSamplerCore(uv, samplerDataExt); }
export float4 PSBodyMC(float2 uv, float4 samplerDataExt) { return ConsumerSamplerCore(uv, samplerDataExt); }
export float4 PSBodyMW(float2 uv, float4 samplerDataExt) { return ConsumerSamplerCore(uv, samplerDataExt); }
export float4 PSBodyMM(float2 uv, float4 samplerDataExt) { return ConsumerSamplerCore(uv, samplerDataExt); }
export float4 PSBodyC(float2 uv, float4 samplerDataExt) { return ConsumerSamplerCore(uv, samplerDataExt); }
export float4 PSBodyW(float2 uv, float4 samplerDataExt) { return ConsumerSamplerCore(uv, samplerDataExt); }
export float4 PSBodyM(float2 uv, float4 samplerDataExt) { return ConsumerSamplerCore(uv, samplerDataExt); }

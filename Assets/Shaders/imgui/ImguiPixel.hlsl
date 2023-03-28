// Copyright 2020 Google LLC

Texture2D fontTexture : register(t0);
SamplerState fontSampler : register(s0);

struct PushConstants
{
    float2 scale;
    float2 translate;
};

//[[vk::push_constant]]
//PushConstants pushConstants;

//[[vk::push_constant]]
//cbuffer PushConstants
//{
//    float2 scale;
//    float2 translate;
//};

struct VSOutput
{
    [[vk::location(0)]] float2 UV : TEXCOORD0;
    [[vk::location(1)]] float4 Color : COLOR0;
};

float4 main(VSOutput input) : SV_TARGET
{
    return input.Color * fontTexture.Sample(fontSampler, input.UV);
}
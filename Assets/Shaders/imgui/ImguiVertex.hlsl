// Copyright 2020 Google LLC

//Texture2D fontTexture : register(t0);
//SamplerState fontSampler : register(s0);

struct VSInput
{
    [[vk::location(0)]] float2 Pos : POSITION0;
    [[vk::location(1)]] float2 UV : TEXCOORD0;
    [[vk::location(2)]] float4 Color : COLOR0;
};

[[vk::push_constant]]
cbuffer PushConstants
{
    float2 scale;
    float2 translate;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    [[vk::location(0)]] float2 UV : TEXCOORD0;
    [[vk::location(1)]] float4 Color : COLOR0;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    output.UV = input.UV;
    output.Color = input.Color;
    output.Pos = float4(input.Pos * scale + translate, 0.0, 1.0);
    return output;
}
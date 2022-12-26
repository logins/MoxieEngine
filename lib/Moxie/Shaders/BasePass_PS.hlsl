/*
 BasePass_PS.hlsl

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

// To generate BasePass_PS.cso I will be using: fxc /Zi /T ps_5_1 /Fo BasePass_PS.cso BasePass_PS.hlsl

// Note: This shader shows a very basic implementation of multiple features
// with the FeaturesFieldCB variable directing what features to use for the current draw.
// It has to be stated through, that a more realistic implementation would see the
// base pass generating shaders depending on its possibilities and requests
// (e.g. a PSO that uses only the color modifier, another PSO with only the cubemap
// and a third PSO using both of them), and switching pipeline on demand.


// HLSL language syntax at this page:
// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-language-syntax

// Note: important to set static const for literals, otherwise these will be interpreted as buffers!
static const uint DRAW_FEATURES_COLOR_MOD = 1;
static const uint DRAW_FEATURES_COLOR_TEX = 2;
static const uint DRAW_FEATURES_COLOR_CUBE = 4;

// Note: Both COLOR and TEXCOORD are float4 system value semantics, but it's possible to define them smaller
// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics
struct PixelShaderInput
{
    float3 PrimitiveColor : COLOR0;
    float3 TextureCoords : TEXCOORD0;
};

// Structs are required for ConstantBuffer type parameters..
// Meanwhile, if I just try to define a base type with an associated register in space 1,
// it just won't find it and somehow register a dependency to register c0 space 0, which we don't want.
// So for this example, I will keep using ConstantBuffer variable type.
struct UIntBuf
{
    uint value;
};

struct FloatBuf
{
    float value;
};

// 32-bit unsigned integer
// Communicates the chosen features for this draw call
ConstantBuffer<UIntBuf> FeaturesFieldCB : register(b0, space1);
// 32-bit floating point 
ConstantBuffer<FloatBuf> ColorModifierCB : register(b1, space1);

Texture2D ColorTexture : register(t0, space1);
TextureCube ColorCube : register(t1, space1);

SamplerState TexSampler : register(s0); // Note: Since we are using a static sampler, we do not specify a register space


float4 main(PixelShaderInput IN) : SV_Target
{
    // If texture is active, start from color sampled from it
    if (FeaturesFieldCB.value & DRAW_FEATURES_COLOR_TEX)
    {
        IN.PrimitiveColor = ColorTexture.Sample(TexSampler, IN.TextureCoords.xy).xyz;
        // Sample function documentation at this page
        // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-to-sample
    }
    // If cube is active, start from color sampled from it
    else if (FeaturesFieldCB.value & DRAW_FEATURES_COLOR_CUBE)
    {
        IN.PrimitiveColor = ColorCube.Sample(TexSampler, IN.TextureCoords.xyz).xyz;
    }

    if (FeaturesFieldCB.value & DRAW_FEATURES_COLOR_MOD)
    {
        IN.PrimitiveColor *= ColorModifierCB.value;
    }
    
    return float4(IN.PrimitiveColor, 1.f);
}



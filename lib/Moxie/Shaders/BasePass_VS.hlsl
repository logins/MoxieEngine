/*
 BasePass_VS.hlsl

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

// To generate BasePass_VS.cso I will be using: fxc /Zi /T vs_5_1 /Fo BasePass_VS.cso BasePass_VS.hlsl

// HLSL language syntax at this page:
// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-language-syntax
// And System Value Semantics at this page:
// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics

float4x4 MVP_Matrix : register(b0,space0); // Template Constructs require ShaderModel 5.1

struct VertexPosColor
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float3 CubemapCoords : CUBETEXCOORD;
};

struct VertexShaderOutput
{
    float3 PrimitiveColor : COLOR0;
    float3 CubemapCoords : TEXCOORD0;
    float4 Position : SV_POSITION; // Every vertex shader must write out a parameter with this semantic.
};

VertexShaderOutput main(VertexPosColor IN)
{
	VertexShaderOutput OUT;

    OUT.PrimitiveColor = IN.Color;
	
    OUT.CubemapCoords = IN.CubemapCoords;
    
    OUT.Position = mul(MVP_Matrix, float4(IN.Position, 1.0f));
	
	return OUT;
}
// 
cbuffer ShaderData : register(b0)
{
    float totalTime; // 
    float currTime; // 
    float radius; // 
    int shaderMode; // - Тип шейдера (0=Bloom, 1=Blur, 2=Glow...)
    float4 color; // 
    float4 params; // Доп. параметры (x: intensity, y: speed, z: scale, w: unused)
}; // Итого: 48 байт (кратно 16, DirectX доволен)

struct VS_INPUT
{
    float2 pos : POSITION;
    float2 uv : TEXCOORD0; // 
    float4 col : COLOR0;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0; // 
    float4 col : COLOR0;
};
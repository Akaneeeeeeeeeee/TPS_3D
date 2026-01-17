cbuffer WorldBuffer : register(b0)
{
    matrix World;
}
cbuffer ViewBuffer : register(b1)
{
    matrix View;
}
cbuffer ProjectionBuffer : register(b2)
{
    matrix Projection;
}

struct VS_IN
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
    int4 BoneIndex : BONEINDEX;
    float4 BoneWeight : BONEWEIGHT;
};

struct VS_OUT
{
    float4 PosH : SV_Position;
    float3 NrmW : TEXCOORD0;
    float2 UV : TEXCOORD1;
    float4 Col : TEXCOORD2;
};

VS_OUT main(VS_IN v)
{
    VS_OUT o;

    float4 posW = mul(float4(v.Position, 1), World);
    float4 posV = mul(posW, View);
    o.PosH = mul(posV, Projection);

    float3 nW = mul(float4(v.Normal, 0), World).xyz;
    o.NrmW = normalize(nW);

    o.UV = v.TexCoord;
    o.Col = v.Color;
    return o;
}

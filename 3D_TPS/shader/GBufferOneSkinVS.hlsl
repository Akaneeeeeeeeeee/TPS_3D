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

#define MAX_BONE 400
cbuffer BoneMatrixBuffer : register(b5)
{
    matrix BoneMatrix[MAX_BONE];
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

    // skin (row-vector * matrix •ûŽ®)
    float4 p = float4(v.Position, 1);
    float4 n = float4(v.Normal, 0);

    float4 sp = 0;
    float4 sn = 0;

    [unroll]
    for (int i = 0; i < 4; i++)
    {
        int bi = v.BoneIndex[i];
        float bw = v.BoneWeight[i];
        if (bw <= 0)
            continue;

        sp += mul(p, BoneMatrix[bi]) * bw;
        sn += mul(n, BoneMatrix[bi]) * bw;
    }

    float4 posW = mul(sp, World);
    float4 posV = mul(posW, View);
    o.PosH = mul(posV, Projection);

    float3 nW = mul(float4(sn.xyz, 0), World).xyz;
    o.NrmW = normalize(nW);

    o.UV = v.TexCoord;
    o.Col = v.Color;
    return o;
}

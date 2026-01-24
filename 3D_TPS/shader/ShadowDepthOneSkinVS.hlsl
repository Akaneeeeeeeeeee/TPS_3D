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
    int4 BoneIndex : BONEINDEX;
    float4 BoneWeight : BONEWEIGHT;
};

struct VS_OUT
{
    float4 PosH : SV_Position;
};

VS_OUT main(VS_IN v)
{
    VS_OUT o;

    float4 p = float4(v.Position, 1.0);

    float4 sp = 0;

    [unroll]
    for (int i = 0; i < 4; i++)
    {
        int bi = v.BoneIndex[i];
        float bw = v.BoneWeight[i];
        if (bw <= 0)
            continue;

        sp += mul(p, BoneMatrix[bi]) * bw;
    }

    float4 posW = mul(sp, World);
    float4 posV = mul(posW, View);
    o.PosH = mul(posV, Projection);

    return o;
}

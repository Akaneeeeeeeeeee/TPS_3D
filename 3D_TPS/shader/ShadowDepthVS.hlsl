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
};

struct VS_OUT
{
    float4 PosH : SV_Position;
};

VS_OUT main(VS_IN v)
{
    VS_OUT o;

    float4 p = float4(v.Position, 1.0);
    float4 posW = mul(p, World);
    float4 posV = mul(posW, View);
    o.PosH = mul(posV, Projection);

    return o;
}

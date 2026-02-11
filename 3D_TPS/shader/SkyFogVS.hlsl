struct VS_OUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

VS_OUT main(uint vid : SV_VertexID)
{
    // 頂点バッファ無しでフルスクリーン三角形を描く
    float2 p[3] = { float2(-1, -1), float2(-1, 3), float2(3, -1) };
    float2 uv[3] = { float2(0, 1), float2(0, -1), float2(2, 1) };

    VS_OUT o;
    o.pos = float4(p[vid], 0, 1);
    o.uv = uv[vid];
    return o;
}

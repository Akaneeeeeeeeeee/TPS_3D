
//struct PBR_MATERIAL
//{
//    float4 BaseColorFactor; // rgb:色, a:透明
//    float MetallicFactor; // 0..1
//    float RoughnessFactor; // 0..1
//    float NormalScale; // 1.0推奨
//    float3 EmissiveFactor; // 発光
//    int UseBaseColorMap; // 0/1
//    int UseNormalMap; // 0/1
//    int UseMRMap; // 0/1  (MetallicRoughness)
//    int UseAOMap; // 0/1
//    int UseEmissiveMap; // 0/1
//    float2 _pad;
//};

//cbuffer MaterialBuffer : register(b3)
//{
//    PBR_MATERIAL Mat;
//};

#include "common.hlsl"

PS_IN main(in VSONESKIN_IN In)
{
    PS_IN Out;

    // ----------------------------
    // 1) スキニング（位置）
    // ----------------------------
    float4x4 comb = (float4x4) 0;
    for (int i = 0; i < 4; i++)
    {
        comb += BoneMatrix[In.BoneIndex[i]] * In.BoneWeight[i];
    }

    float4 localPos = mul(In.Position, comb);
    //float4 localPos = In.Position; // combを使わない

    // ----------------------------
    // 2) スキニング（法線）: 3x3で回す
    // ----------------------------
    float3 skinnedN = 0.0f;
    for (int i = 0; i < 4; i++)
    {
        float3x3 b3 = (float3x3) BoneMatrix[In.BoneIndex[i]];
        skinnedN += mul(In.Normal.xyz, b3) * In.BoneWeight[i];
    }
    skinnedN = normalize(skinnedN);

    // ----------------------------
    // 3) ワールド位置
    // ----------------------------
    float4 worldPos4 = mul(localPos, World);
    Out.WorldPos = worldPos4.xyz;

    // ----------------------------
    // 4) ワールド法線（拡縮に強い方法）
    //    World の左上3x3を使って逆行列→転置
    // ----------------------------
    float3x3 normalMatrix = Inverse3x3((float3x3) World);
    normalMatrix = transpose(normalMatrix);
    Out.NormalW = normalize(mul(skinnedN, normalMatrix));

    // ----------------------------
    // 5) 画面座標（World→View→Proj）
    // ----------------------------
    float4 viewPos = mul(worldPos4, View);
    Out.Position = mul(viewPos, Projection);

    // ----------------------------
    // 6) 色・UV
    //    ここでは「頂点色 * Material.Diffuse」までにする（光はPS）
    // ----------------------------
    Out.Diffuse = In.Diffuse * Material.Diffuse;
    Out.Diffuse.a = In.Diffuse.a * Material.Diffuse.a;

    Out.TexCoord = In.TexCoord;
    return Out;
}


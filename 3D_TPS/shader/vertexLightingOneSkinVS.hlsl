//#include "common.hlsl"

//PS_IN main(in VSONESKIN_IN In)
//{
//    PS_IN Out;

//	// ワンスキン頂点ブレンドの処理
//    float4x4 comb = (float4x4) 0;
//    for (int i = 0; i < 4; i++)
//    {
//		// 重みを計算しながら行列生成
//        comb += BoneMatrix[In.BoneIndex[i]] * In.BoneWeight[i];
//    }

//    float4 Pos;

//    Pos = mul(In.Position,comb);
//    In.Position = Pos;

//	//
//    matrix wvp;
//    wvp = mul(World, View);
//    wvp = mul(wvp, Projection);
	
//    float4 worldNormal, normal;
//    normal = float4(In.Normal.xyz, 0.0);
//    worldNormal = mul(normal, World);
//    worldNormal = normalize(worldNormal);

//    float light = -(dot(Light.Direction.xyz, worldNormal.xyz))* 0.5 + 0.5;
//    light = saturate(light);  // ランバート反射の計算

//    Out.Diffuse = In.Diffuse * Material.Diffuse * light * Light.Diffuse;
//    Out.Diffuse += In.Diffuse * Material.Ambient * Light.Ambient;
//    Out.Diffuse += Material.Emission;
//    Out.Diffuse.a = In.Diffuse.a * Material.Diffuse.a;

//    Out.Position = mul(In.Position, wvp);
//    Out.TexCoord = In.TexCoord;

//    return Out;    
//}


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


//#include "common.hlsl"

//PS_IN main(in VS_IN In)
//{
//    PS_IN Out;

//	matrix wvp;
//	wvp = mul(World, View);
//	wvp = mul(wvp, Projection);
	
//    // 法線変換行列を計算（拡縮成分を取り除く）
//    float3x3 normalMatrix = Inverse3x3(float3x3(World._11, World._12, World._13,
//                                             World._21, World._22, World._23,
//                                             World._31, World._32, World._33));
//    // 転置
//    normalMatrix = transpose(normalMatrix);

//    // 法線ベクトルの方向をワールド座標系に変換
//	float3 worldNormal, normal;
//	normal = In.Normal.xyz;
    
//	worldNormal = mul(normal, normalMatrix);
//	worldNormal = normalize(worldNormal);

//	float d = -dot(Light.Direction.xyz, worldNormal.xyz);
//	d = saturate(d);

//	Out.Diffuse.xyz = In.Diffuse.xyz * Material.Diffuse.xyz * d * Light.Diffuse.xyz;
//	Out.Diffuse.xyz += In.Diffuse.xyz * Material.Ambient.xyz * Light.Ambient.xyz;
//	Out.Diffuse.xyz += Material.Emission.xyz;
//    Out.Diffuse.a = In.Diffuse.a* Material.Diffuse.a;
	
//	Out.Position = mul( In.Position, wvp );
//	Out.TexCoord = In.TexCoord;
	
//    return Out;
//}


#include "common.hlsl"

PS_IN main(in VS_IN In)
{
    PS_IN Out;

    // ワールド位置
    float4 worldPos4 = mul(In.Position, World);
    Out.WorldPos = worldPos4.xyz;

    // ワールド法線（拡縮に強い方法）
    float3x3 normalMatrix = Inverse3x3((float3x3) World);
    normalMatrix = transpose(normalMatrix);
    Out.NormalW = normalize(mul(In.Normal.xyz, normalMatrix));

    // 画面座標
    float4 viewPos = mul(worldPos4, View);
    Out.Position = mul(viewPos, Projection);

    // 色・UV（光はPSで）
    Out.Diffuse = In.Diffuse * Material.Diffuse;
    Out.Diffuse.a = In.Diffuse.a * Material.Diffuse.a;

    Out.TexCoord = In.TexCoord;
    return Out;
}


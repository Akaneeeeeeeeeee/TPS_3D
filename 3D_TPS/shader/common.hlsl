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

struct MATERIAL
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Emission;
	float Shininess;
	bool TextureEnable;
	float2 Dummy;
};

cbuffer MaterialBuffer : register(b3)
{
	MATERIAL Material;
}

struct LIGHT
{
	bool Enable;					// �g�p���邩�ۂ�
	bool3 Dummy;					// PADDING
	float4 Direction;				// ����
	float4 Diffuse;					// �g�U���˗p�̌��̋���
	float4 Ambient;					// �����p�̌��̋���
};

cbuffer LightBuffer : register(b4)
{
    LIGHT Light;
};

#define MAX_BONE 400
cbuffer BoneMatrixBuffer : register(b5)
{
    matrix BoneMatrix[MAX_BONE];
}

struct VS_IN
{
	float4 Position		: POSITION0;
	float4 Normal		: NORMAL0;
	float4 Diffuse		: COLOR0;
	float2 TexCoord		: TEXCOORD0;
};

struct VSONESKIN_IN
{
    float4 Position		: POSITION0;
    float4 Normal		: NORMAL0;
    float4 Diffuse		: COLOR0;
    float2 TexCoord		: TEXCOORD0;
    int4   BoneIndex	: BONEINDEX;
    float4 BoneWeight	: BONEWEIGHT;
};

struct PS_IN
{
	float4 Position		: SV_POSITION;
	float4 Diffuse		: COLOR0;
	float2 TexCoord		: TEXCOORD0;
};

// 3x3 �t�s����v�Z����֐�
float3x3 Inverse3x3(float3x3 m)
{
    float a = m[0].x, b = m[0].y, c = m[0].z;
    float d = m[1].x, e = m[1].y, f = m[1].z;
    float g = m[2].x, h = m[2].y, i = m[2].z;

    float A = e * i - f * h;
    float B = f * g - d * i;
    float C = d * h - e * g;

    float det = a * A + b * B + c * C;
    if (abs(det) < 1e-6)
    {
        float3x3 zeroMatrix = float3x3(0.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 0.0f);
        return zeroMatrix; // �񐳑��Ȃ�[���s��       
    }

    float invDet = 1.0 / det;

    return float3x3(
         A, c * h - b * i, b * f - c * e,
         B, a * i - c * g, c * d - a * f,
         C, b * g - a * h, a * e - b * d
    ) * invDet;
}
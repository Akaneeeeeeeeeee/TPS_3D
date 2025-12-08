#include "JoltDebugRendererDX11.h"
#include "renderer.h"

#include "commontypes.h"

#ifdef JPH_DEBUG_RENDERER

#include <Jolt/Core/Color.h>

#include <limits>
#include <dx11helper.h>

namespace
{
	Color ToColor(JPH::ColorArg src)
	{
		constexpr float inv255 = 1.0f / 255.0f;
		return Color(src.r * inv255, src.g * inv255, src.b * inv255, src.a * inv255);
	}

	Vector3 ToVector3(JPH::RVec3Arg v)
	{
		return Vector3(static_cast<float>(v.GetX()), static_cast<float>(v.GetY()), static_cast<float>(v.GetZ()));
	}

	std::once_flag g_init_flag;


	static const char* g_DebugLineVS = R"(
cbuffer CBVP : register(b0)
{
    float4x4 VP;
};

struct VSInput
{
    float3 pos   : POSITION;
    float4 color : COLOR0;
};

struct VSOutput
{
    float4 pos   : SV_POSITION;
    float4 color : COLOR0;
};

VSOutput VSMain(VSInput input)
{
    VSOutput o;
    o.pos = mul(float4(input.pos, 1.0f), VP);
    o.color = input.color;
    return o;
}
)";

	static const char* g_DebugLinePS = R"(
struct PSInput
{
    float4 pos   : SV_POSITION;
    float4 color : COLOR0;
};

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
)";


	struct CBVP {
		DirectX::XMFLOAT4X4 VP;
	};
}

// 簡易エラーヘルパ
inline void CheckHR(HRESULT hr)
{
	assert(SUCCEEDED(hr));
}

// 簡易シェーダコンパイル
ComPtr<ID3DBlob> CompileShader(const wchar_t* path, const char* entry, const char* target)
{
	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = D3DCompileFromFile(
		path,
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entry,
		target,
		flags,
		0,
		&blob,
		&error);

	if (FAILED(hr))
	{
		if (error)
		{
			OutputDebugStringA((char*)error->GetBufferPointer());
		}
		CheckHR(hr);
	}

	return blob;
}

void JoltDebugRendererDX11::EnsureResources()
{
	std::call_once(g_init_flag, []() {
		LineInstancedDrawerInit();  // 追加
		SetLineWidth(1.0f);         // 太さの初期値
		});
}


void JoltDebugRendererDX11::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
	EnsureResources();

	Vector3 start = ToVector3(inFrom);
	Vector3 end = ToVector3(inTo);

	if ((end - start).LengthSquared() <= std::numeric_limits<float>::epsilon())
		return;

	LineInstanceParam p{};
	p.start = start;
	p.end = end;
	p.color = ToColor(inColor);

	m_Lines.push_back(p);
}

void JoltDebugRendererDX11::DrawText3D(JPH::RVec3Arg, const std::string_view&, JPH::ColorArg, float)
{
	// Text drawing is not required for the current collider debug rendering use-case.
}

void JoltDebugRendererDX11::Begin(const DirectX::XMMATRIX& vp)
{
	EnsureResources();

	m_Lines.clear();
	mVP = vp;
}

void JoltDebugRendererDX11::End()
{
	if (!m_Lines.empty())
	{
		// まとめて描画
		LineInstancedDrawerDraw(m_Lines);
		m_Lines.clear();
	}
}
#endif // JPH_DEBUG_RENDERER
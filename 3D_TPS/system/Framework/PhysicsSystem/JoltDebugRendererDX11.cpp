#include "JoltDebugRendererDX11.h"
#include "renderer.h"

#include "system/LineDrawer.h"
#include "commontypes.h"

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
		LineDrawerInit();
		SetLineWidth(1.0f);
		});
}


//void JoltDebugRendererDX11::EnsureResources()
//{
//    if (mInitialized)
//        return;
//
//    ID3D11Device* dev = Renderer::GetDevice();
//    assert(dev);
//
//    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
//#ifdef _DEBUG
//    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
//#endif
//
//    ComPtr<ID3DBlob> vsBlob, psBlob, errBlob;
//
//    // ==== VS ====
//    CheckHR(D3DCompile(
//        g_DebugLineVS,
//        strlen(g_DebugLineVS),
//        nullptr, nullptr, nullptr,
//        "VSMain",
//        "vs_5_0",
//        flags, 0,
//        &vsBlob,
//        &errBlob));
//
//    // ==== PS ====
//    CheckHR(D3DCompile(
//        g_DebugLinePS,
//        strlen(g_DebugLinePS),
//        nullptr, nullptr, nullptr,
//        "PSMain",
//        "ps_5_0",
//        flags, 0,
//        &psBlob,
//        &errBlob));
//
//    // ==== 実際にシェーダを作成 ====
//    CheckHR(dev->CreateVertexShader(
//        vsBlob->GetBufferPointer(),
//        vsBlob->GetBufferSize(),
//        nullptr,
//        &mVS));
//
//    CheckHR(dev->CreatePixelShader(
//        psBlob->GetBufferPointer(),
//        psBlob->GetBufferSize(),
//        nullptr,
//        &mPS));
//
//    // ==== InputLayout ====
//    D3D11_INPUT_ELEMENT_DESC layout[] = {
//        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
//        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//    };
//
//    CheckHR(dev->CreateInputLayout(
//        layout,
//        _countof(layout),
//        vsBlob->GetBufferPointer(),
//        vsBlob->GetBufferSize(),
//        &mIL));
//
//    // ==== VP constant buffer ====
//    {
//        D3D11_BUFFER_DESC bd{};
//        bd.Usage = D3D11_USAGE_DYNAMIC;
//        bd.ByteWidth = sizeof(CBVP);
//        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//        CheckHR(dev->CreateBuffer(&bd, nullptr, &mCB));
//    }
//
//    // ==== Vertex Buffer ====
//    {
//        mVBCapacity = 1024;
//        D3D11_BUFFER_DESC bd{};
//        bd.Usage = D3D11_USAGE_DYNAMIC;
//        bd.ByteWidth = sizeof(LineVertex) * mVBCapacity;
//        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//
//        CheckHR(dev->CreateBuffer(&bd, nullptr, &mVB));
//    }
//
//    mInitialized = true;
//}




void JoltDebugRendererDX11::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
	EnsureResources();

	Vector3 start = ToVector3(inFrom);
	Vector3 end = ToVector3(inTo);
	Vector3 direction = end - start;
	float length = direction.Length();
	if (length <= std::numeric_limits<float>::epsilon())
	{
		return;
	}

	direction /= length;
	SetLineWidth(1.0f);
	LineDrawerDraw(length, start, direction, ToColor(inColor));

	/*LineVertex v0{}, v1{};

	auto a = ToVector3(inFrom);
	auto b = ToVector3(inTo);

	v0.pos = { a.x, a.y, a.z };
	v1.pos = { b.x, b.y, b.z };

	Color c = ToColor(inColor);
	DirectX::XMFLOAT4 col = { c.R(), c.G(), c.B(), c.A()};

	v0.color = col;
	v1.color = col;

	m_Lines.push_back(v0);
	m_Lines.push_back(v1);*/
}

void JoltDebugRendererDX11::DrawText3D(JPH::RVec3Arg, const std::string_view&, JPH::ColorArg, float)
{
	// Text drawing is not required for the current collider debug rendering use-case.
}

void JoltDebugRendererDX11::Begin(const DirectX::XMMATRIX& vp)
{
	if (!mInitialized)
	{
		EnsureResources();
		mInitialized = true;
	}

	m_Lines.clear();
	mVP = vp;
}

void JoltDebugRendererDX11::End()
{
	if (!m_Lines.empty())
	{
		FlushLines();
	}
}


void JoltDebugRendererDX11::FlushLines()
{
	ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();
	ID3D11Device* dev = Renderer::GetDevice();

	// === VP constant buffer 更新 ===
	{
		/*D3D11_MAPPED_SUBRESOURCE ms;
		ctx->Map(mCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		DirectX::XMStoreFloat4x4(
			&reinterpret_cast<CBVP*>(ms.pData)->VP,
			DirectX::XMMatrixTranspose(mVP)
		);
		ctx->Unmap(mCB.Get(), 0);*/
		D3D11_MAPPED_SUBRESOURCE ms;
		ctx->Map(mCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);

		auto* cb = reinterpret_cast<CBVP*>(ms.pData);

		// ★ 転置をやめる ★
		DirectX::XMStoreFloat4x4(&cb->VP, mVP);

		ctx->Unmap(mCB.Get(), 0);
	}

	// === VB リサイズ ===
	if (m_Lines.size() > mVBCapacity)
	{
		mVBCapacity = std::max(m_Lines.size(), mVBCapacity * 2);

		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = UINT(sizeof(LineVertex) * mVBCapacity);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		mVB.Reset();
		dev->CreateBuffer(&bd, nullptr, &mVB);
	}

	// === VB へコピー ===
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		ctx->Map(mVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		memcpy(ms.pData, m_Lines.data(), sizeof(LineVertex) * m_Lines.size());
		ctx->Unmap(mVB.Get(), 0);
	}

	// === パイプライン設定 ===
	UINT stride = sizeof(LineVertex), offset = 0;
	ID3D11Buffer* vb = mVB.Get();
	ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	ctx->IASetInputLayout(mIL.Get());
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	ctx->VSSetShader(mVS.Get(), nullptr, 0);
	ID3D11Buffer* cb = mCB.Get();
	ctx->VSSetConstantBuffers(0, 1, &cb);

	ctx->PSSetShader(mPS.Get(), nullptr, 0);

	// === 一括描画 ===
	ctx->Draw(static_cast<UINT>(m_Lines.size()), 0);
}

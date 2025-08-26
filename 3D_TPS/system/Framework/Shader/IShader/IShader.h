#pragma once
#include <d3d11.h>
#include <vector>
#include <wrl/client.h>
#include <filesystem>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")


using namespace Microsoft::WRL;

class Texture_Low; // 前方宣言

//! 各シェーダーの識別用
enum class ShaderStage {
	Vertex,
	Pixel,
	Geometry,
	Compute,
	Hull,
	Domain,

	Stage_MAX
};

class IShader
{
public:
	virtual ~IShader();

	// シェーダーファイルを読み込んだ後、シェーダーの種類別に処理を行う
	HRESULT Analyze_to_Adjust(void* pData, UINT size);

	virtual void Bind() = 0;	// 各シェーダーの割り当て用純粋仮想関数
	virtual void Unbind() = 0;	// 各シェーダーの割り当て解除用純粋仮想関数
	virtual void WriteCBuffer(UINT _Slot, const void* _pData);
	virtual void SetTexture(UINT slot, Texture_Low* tex);


	//virtual void SetSRV(UINT _Slot, ID3D11ShaderResourceView* _pSRV) = 0;	// シェーダーリソースビューをセット

	virtual std::string GetName() const { return m_Name; }	// シェーダーの名前を取得

protected:
	// シェーダーの作成(各シェーダーで実装)
	virtual HRESULT Create(void* pData, UINT size) = 0;

	IShader(ShaderStage _Kind, const std::string& _Name);
	ShaderStage m_Kind;		// シェーダーの種類（頂点、ピクセル、ジオメトリなど）
	std::string m_Name;		// シェーダーの名前（ファイル名など）
	std::vector<ComPtr<ID3D11Buffer>> m_pCBuffers;			// 定数バッファの配列
	std::vector<ComPtr<ID3D11ShaderResourceView>> m_pSRVs;	// シェーダーリソースビューの配列
};


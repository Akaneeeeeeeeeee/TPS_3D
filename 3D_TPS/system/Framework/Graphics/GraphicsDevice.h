#pragma once
#include "system/Framework/Application/Entry/main.h"
#include "system/noncopyable.h"
#include "d3d11.h"

using namespace Microsoft::WRL;

//////////////////////////////////////////////////
//				DirectXフレームワーク				//
//////////////////////////////////////////////////

/// <summary>
/// DiectXのグラフィックスデバイスを管理するクラス
/// →シングルトンにするとどこからでもPresent()を呼べてしまうのでRenderManagerクラスに依存性を持たせることで解決する
/// このクラスは、DirectXのデバイス、コンテキスト、スワップチェインなどを管理し、描画処理の呼び出し機能だけを提供する
/// 基本的に描画の呼び出しはRenderManagerクラスに任せる
/// </summary>
class GraphicsDevice : public NonCopyable
{
public:

	GraphicsDevice() {};	//! コンストラクタ
	~GraphicsDevice() {};	//! デストラクタ

	ID3D11Device* GetDevice(void) const { return m_pDevice.Get(); }
	ID3D11DeviceContext* GetContext(void) const { return m_pDeviceContext.Get(); };
	IDXGISwapChain* GetSwapChain(void) const { return m_pSwapChain.Get(); }
	HRESULT Init();		// 初期化（ゲームクラスではこの初期化が成功した場合→ゲームの初期化処理実行にする）
	void StartRender(void);			// 描画処理
	void FinishRender(void);		// 描画終了処理
	void Uninit(void);				// 終了処理

	//// シェーダー系生成
	//// 頂点シェーダーオブジェクトを生成、同時に頂点レイアウトも生成
	//HRESULT CreateVertexShader(ID3D11Device* device, const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel,
	//	D3D11_INPUT_ELEMENT_DESC* layout, unsigned int numElements, ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout);
	//// ピクセルシェーダーオブジェクトを生成
	//HRESULT CreatePixelShader(ID3D11Device* device, const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D11PixelShader** ppPixelShader);

	//// シェーダーコンパイル
	//HRESULT CompileShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, void** ShaderObject, size_t& ShaderObjectSize, ID3DBlob** ppBlobOut);

private:
	// D3D_FEATURE_LEVELはDirect3Dのバージョン
	D3D_FEATURE_LEVEL				m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
	// デバイス＝DirectXの各種機能を作る(これ一番重要。これがないと↓のGPUリソース(テクスチャ、バッファ、シェーダーなど)が作れない)
	ComPtr<ID3D11Device>			m_pDevice;
	// コンテキスト＝描画関連を司る機能(描画を実行するための機能、っていう感じ)
	ComPtr<ID3D11DeviceContext>		m_pDeviceContext;
	// スワップチェイン＝ダブルバッファ機能(これはそのまま。現在描画中の画面の後ろで新しい画面を作っておいて、それを切り替える。)
	ComPtr<IDXGISwapChain>			m_pSwapChain;
	// レンダーターゲット＝描画先を表す機能(GPUが描画する「最終的な出力先」を決める。例えば「画面全体に描画する」のか、「一部のテクスチャに描画する」のかを決める。)
	ComPtr<ID3D11RenderTargetView>	m_pRenderTargetView;
	// デプスバッファ＝Z値（奥行き情報）を管理する。近いオブジェクトを手前に、遠いオブジェクトを奥に描画。
	ComPtr<ID3D11DepthStencilView>	m_pDepthStencilView;
	// 深度バッファ
	ComPtr<ID3D11DepthStencilState> m_pDepthStateEnable;
	ComPtr<ID3D11DepthStencilState> m_pDepthStateDisable;
	



	
	// 最終的にGPUに送る定数バッファ用変数
	//ComPtr<ID3D11Buffer> m_pConstantBuffer;

	//// ＜ブレンドステート用変数（アルファブレンディング）＞
	//// アルファ値をどの具合にブレンド(混ぜる)するかの「ブレンド方法」 を決める必要がある。
	//// 単純に「このオブジェクトは半透明だよ！」と設定しても、どうやって透明度を計算するかが決まっていなければ、正しく描画できない。
	//ComPtr<ID3D11BlendState> m_pBlendState;

	//// ＜サンプラー用変数＞
	//// 「テクスチャの拡大・縮小・フィルタリング方法」を指定する
	//// テクスチャを正しく表示するために、どんな補間方法を使うか を指定する必要がある。
	//// 設定しないと、意図しない表示になったり、ガタガタの見た目になる可能性がある。
	//ComPtr<ID3D11SamplerState> m_pSampler;
	
};



// 構造体の定義
// 頂点データを表す構造体
struct Vertex
{
	// 頂点の位置座標
	Vector3 position;
	//色
	Color color;
	// テクスチャ座標（UV座標）
	Vector2 uvpos;
};

// 定数バッファ用構造体
struct ConstBuffer
{
	// 頂点カラー行列
	DirectX::XMFLOAT4 color;
	// UV座標移動行列
	DirectX::XMMATRIX matrixTex;
	// プロジェクション変換行列
	DirectX::XMMATRIX matrixProj;
	// ワールド変換行列
	DirectX::XMMATRIX matrixWorld;
};


//// ブレンドステート
//enum EBlendState {
//	BS_NONE = 0,							// 半透明合成無し	
//	BS_ALPHABLEND,							// 半透明合成
//	BS_ADDITIVE,							// 加算合成
//	BS_SUBTRACTION,							// 減算合成
//	MAX_BLENDSTATE
//};
//
//// 平行光源
//struct LIGHT {
//	BOOL Enable;							// 光を使うかのフラグ
//	BOOL Dunny[3];							// アラインメント調整用配列
//	DirectX::SimpleMath::Vector4 Direction;	// 平行光源の方向
//	DirectX::SimpleMath::Color Diffuse;		// 平行光源の強さと色
//	DirectX::SimpleMath::Color Ambient;		// 環境光の強さと色
//};
//
//// サブセット
//struct SUBSET {
//	std::string		MtrlName;			// マテリアル名
//	unsigned int	IndexNum = 0;		// インデックス数
//	unsigned int	VertexNum = 0;		// 頂点数
//	unsigned int	IndexBase = 0;		// 開始インデックス数
//	unsigned int	VertexBase = 0;		// 頂点ベース
//	unsigned int	MaterialIdx = 0;	// マテリアル番号
//};
//
//// マテリアル
//struct MATERIAL {
//	DirectX::SimpleMath::Color Ambient;		// 環境反射
//	DirectX::SimpleMath::Color Diffuse;		// 拡散反射
//	DirectX::SimpleMath::Color Specular;	// 鏡面反射
//	DirectX::SimpleMath::Color Emission;	// 発光
//	float Shiness;			// 光沢の滑らかさ
//	BOOL TextureEnable;		// テクスチャ使用確認フラグ
//	BOOL Dummy[2];			// アラインメント調整
//
//};

// Direct3D解放の簡略化マクロ
#define SAFE_RELEASE(p) { if( NULL != p ) { p->Release(); p = NULL; } }


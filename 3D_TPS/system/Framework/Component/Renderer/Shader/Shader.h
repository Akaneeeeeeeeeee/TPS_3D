#pragma once
#include "../../../Graphics/Graphics.h"

using namespace DirectX::SimpleMath;
using namespace Microsoft::WRL;

/**
 * @brief シェーダーに必要な情報をまとめたクラス(これをRendererとかが管理する)
*/
class Shader
{
public:
	Shader() {};
	~Shader() {};

	// 初期化
	void Init(const std::string& vsFile, const std::string& vsEntryPoint,
		const std::string& psFile, const std::string& psEntryPoint, D3D11_INPUT_ELEMENT_DESC* layout, UINT numElements);
	
	// 更新
	void Update(void);
	
	// 終了
	void Uninit(void);

	ID3D11VertexShader* GetVertexShader(void);
	ID3D11PixelShader* GetPixelShader(void);

private:
	//-----シェーダー系-----//
	// ↓の二つはGPUに登録されたシェーダープログラムへの参照→GPU上のシェーダーのハンドル」だと思えばOK！
	// ＜頂点シェーダーオブジェクト＞
	// 頂点データを加工して画面上の正しい位置に変換。
	// スプライトやオブジェクトの位置、スケール、回転、変形を行う。これにより、描画対象を画面上の適切な位置に変換したり、アニメーションさせたりできる
	ComPtr<ID3D11VertexShader> m_pVertexShader;

	// ＜ピクセルシェーダーオブジェクト＞
	// テクスチャをサンプリングして最終的な色を決定。
	// テクスチャの色やアルファ値（透明度）を計算する。たとえば、スプライトの色を変更したり、テクスチャの一部を取り込んで表示することができる。
	ComPtr<ID3D11PixelShader> m_pPixelShader;

	// ＜インプットレイアウト＞
	// スプライトの頂点データは (座標 + UV座標) を持ってるけど、3Dモデルの頂点データは (座標 + 法線 + UV座標) みたいに構造が違う。
	// DirectXは、「この頂点データのこの部分が座標で、この部分がUV座標ですよ！」っていう情報を知らないと、正しく描画できない。
	// ので、ID3D11InputLayout で「この頂点データはこういう構造になってるよ」と教えてあげる必要がある！
	// スプライトの頂点データを渡すために保持しておく(これは基本頂点シェーダーとセットで使う)
	ComPtr<ID3D11InputLayout> m_pVertexLayout;
};


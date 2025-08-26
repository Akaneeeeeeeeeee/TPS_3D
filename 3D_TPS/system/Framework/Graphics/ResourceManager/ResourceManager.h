#pragma once
#include "Src/Application/Application/Entry/main.h"
#include "Src/Framework/NonCopyable/Singleton_Template.h"
#include "Src/Framework/Shader/Shader.h"

/**
 * @brief 画像管理するためのラベル
*/
enum class TextureID
{
	PLAYER = 0,
	ENEMY,
	BOSS,
	UI_ITEM,
	GAMEBACK,
	TITLEBACK,
	RESULTBACK,

	ID_MAX
};

enum class ShaderID {
	SHADER_NORMAL
};


/**
 * @brief ゲームに使われるリソース(テクスチャ、シェーダー、サウンドなど)を一括管理するクラス
 * 
 * ファイルパスと画像データをtexture2dに持たせて、もしミップマップとか使うようにtexture2dは配列にしておく。
 * 画像はラベル(こちら側で定義)を使って管理する
 * 
 * 初期化時にIDに紐づけたファイルパスからtexture2dに画像情報を代入しておく
 * →SpriteRendererに画像情報を送りたい時にComPtr<SRV>を取得して渡す
 * 
 * シェーダーも同じくmapに登録して管理する
 * 
*/
class ResourceManager : public Singleton<ResourceManager>
{
public:
	ResourceManager() {};
	~ResourceManager() {};

	HRESULT Init(void);
	void Update(void);
	void Uninit(void);

	void LoadShader(ShaderID _ID, const std::string& vsFile, const std::string& vsEntryPoint,
		const std::string& psFile, const std::string& psEntryPoint, D3D11_INPUT_ELEMENT_DESC* layout, UINT numElements);

	//! テクスチャのSRVのゲッター
	ComPtr<ID3D11ShaderResourceView> GetSRV(const TextureID& _ID);

	// シェーダーのゲッター(生ポインタを返す)
	Shader* GetShader(const ShaderID& _ID);


private:
	// ミップマップなどのことを考え(１つのTexture2Dに対して１つのSRVが必要)、Texture2DとSRVそれぞれのmapを作っておく
	// Texture2D管理用map
	//std::unordered_map<TextureID, std::vector<ComPtr<ID3D11Texture2D>>> m_Textures;

	// SRV管理用map
	//std::unordered_map<TextureID, std::vector<ComPtr<ID3D11ShaderResourceView>>> m_SRVs;		// vectorはミップマップのために使うが今回は必要ないので使わない
	std::unordered_map<TextureID, ComPtr<ID3D11ShaderResourceView>> m_SRVs;

	// シェーダー管理用map
	std::unordered_map<ShaderID, std::unique_ptr<Shader>> m_Shaders;

	// サウンド管理map
};


#include "ResourceManager.h"
#include "Src/Framework/DX11Helper/dx11helper.h"
#include <array>


/**
 * @brief 初期化
 * ここで全ての画像を読み込んでTexture2DとSRVを作成、保持しておく
*/
HRESULT ResourceManager::Init(void)
{
	/////////////////////////////////////////////////
	///				テクスチャ読み込み
	/////////////////////////////////////////////////

	//! テクスチャIDに対応したファイルパスを設定
	std::unordered_map<TextureID, std::wstring> Filepath_Texture = {
		{TextureID::PLAYER, L"Game/Asset/Character/Player.png"},
		{TextureID::ENEMY, L"Game/Asset/GameObject/Slime.png"},
		{TextureID::TITLEBACK, L"Game/Asset/UI/TitleLogo.png"},
		{TextureID::GAMEBACK, L"Game/Asset/BackGround/TitleBack.png"}
	};


	//! 設定したファイルパスにある画像を読み込んでTexture2DとSRVを作成、キャッシュする
	//! D3d11の扱いどうしよう？
	//! 依存性注入？そもそもD3D11クラスを存在させておくべきかどうか。切り分けて別クラスにしてしまったほうがいいのか？
	for (auto& tex : Filepath_Texture)
	{
		// map登録用の変数作成
		ComPtr<ID3D11ShaderResourceView> srv;
		// テクスチャのパスを読み込む
		HRESULT hr = CreateWICTextureFromFileEx(
			D3D11::GetInstance().GetDevice(),
			D3D11::GetInstance().GetDeviceContext(),
			tex.second.c_str(),
			0,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE,
			0,
			0,
			DirectX::WIC_LOADER_IGNORE_SRGB,
			nullptr,
			srv.GetAddressOf()
		);

		if (FAILED(hr))
		{
			MessageBoxA(NULL, "テクスチャ読み込み失敗", "エラー", MB_ICONERROR | MB_OK);
			return hr;
		}

		// 作成したSRVをmapに登録
		m_SRVs.emplace(tex.first, std::move(srv));
	}

	//////////////////////////////////////
	///			シェーダー読み込み
	//////////////////////////////////////
	/// シェーダーのmap（シェーダーID,シェーダーファイルパス）を作る→そのmapに登録されている
	std::unordered_map<ShaderID, std::array<std::string, 4>> Filepath_VS_PS = {
		{ShaderID::SHADER_NORMAL, {"Shader/VertexShader.hlsl", "vs_main", "Shader/PixelShader.hlsl", "ps_main"}}
	};

	HRESULT hr;

	// インプットレイアウト作成(シェーダーの種類が増える場合、ここにInputLayoutを個別で記述)
	// POSITION → XMFLOAT3 だから DXGI_FORMAT_R32G32B32_FLOAT
	// TEXCOORD → XMFLOAT2 だから DXGI_FORMAT_R32G32_FLOAT
	// 0, 12 の部分で、それぞれのデータがバッファ内でどこにあるかを指定している
	D3D11_INPUT_ELEMENT_DESC layout[]
	{
		// 位置座標があるということを伝える
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 色情報があるということを伝える
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// UV座標
		{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// シェーダーの作成、読み込み
	this->LoadShader(ShaderID::SHADER_NORMAL,
		Filepath_VS_PS[ShaderID::SHADER_NORMAL][0],
		Filepath_VS_PS[ShaderID::SHADER_NORMAL][1],
		Filepath_VS_PS[ShaderID::SHADER_NORMAL][2],
		Filepath_VS_PS[ShaderID::SHADER_NORMAL][3],
		layout, numElements
	);
}


void ResourceManager::Update(void)
{

}

/**
 * @brief 画像のSRVを取得する関数
 * @param _ID テクスチャ管理ID
 * @return IDで指定したテクスチャのSRV
*/
ComPtr<ID3D11ShaderResourceView> ResourceManager::GetSRV(const TextureID& _ID)
{
	// 指定したキーのSRVを返す
	return m_SRVs[_ID];
}

/**
 * @brief シェーダーのゲッター
 * @param _ID シェーダーID
 * @return 指定されたシェーダーの生ポインタ
*/
Shader* ResourceManager::GetShader(const ShaderID& _ID)
{
	return m_Shaders[_ID].get();
}

/**
 * @brief 終了
 * 各コンテナに保持されている画像情報をクリア
*/
void ResourceManager::Uninit(void)
{
	m_Shaders.clear();
	m_SRVs.clear();
}


/**
 * @brief シェーダー作成
 * @param _ID シェーダーID
 * @param vsFile 頂点シェーダーファイル名
 * @param psFile ピクセルシェーダーファイル名
 * @param device D3Dのデバイス
 * @param layout InputLayout
 * @param numElements InputLayputの配置？みたいな感じ
*/
void ResourceManager::LoadShader(ShaderID _ID, const std::string& vsFile, const std::string& vsEntryPoint,
	const std::string& psFile, const std::string& psEntryPoint, D3D11_INPUT_ELEMENT_DESC* layout, UINT numElements)
{
	// シェーダー用変数作成
	auto shader = std::make_unique<Shader>();
	// コンパイル、生成
	shader->Init(vsFile, vsEntryPoint, psFile, psEntryPoint, layout, numElements);
	// mapに登録(所有権を移動)
	m_Shaders[_ID] = std::move(shader);
}


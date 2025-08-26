#pragma once
#include "system/Framework/Shader/IShader/IShader.h"
#include <unordered_map>
#include <memory>

static constexpr const char* TargetList[]
{
	"vs_5_0",
	"gs_5_0",
	"hs_5_0",
	"ds_5_0",
	"ps_5_0"
};

static constexpr const char* EntryPoint = "main"; //! 各シェーダーのエントリーポイント名

class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

	//! 外側の初期化処理で使いたいシェーダーをすべてLoad()し、登録しておく
	//! m_ShaderManager->Load(ShaderStage::Vertex, Assets/Shader/VS_PBR.cso, "VS_PBR");みたいな感じ
	void Init(void);	//! 初期化処理

	HRESULT Load(ShaderStage _Stage, const std::filesystem::path& _csoPath, const std::string& _name);

	IShader* GetShader(const std::string& _name) const;	//! 名前でシェーダーを取得

private:
	// 名前で登録したい→VS_PBRとかPS_PBRとか
	// 初期化時にcsoを読み込み、名前で登録する
	// →各シェーダーオブジェクトは自身の名前（VS_PBRとか）を持ち、初期化時はファイルパスを引数で渡す
	std::unordered_map<std::string, std::unique_ptr<IShader>> m_Shaders; //! シェーダーのコンテナ
};


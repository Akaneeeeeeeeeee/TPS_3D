#include "ShaderManager.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include <fstream>
#include "Src/Framework/Shader/Shaders/VertexShader.h"
#include "Src/Framework/Shader/Shaders/PixelShader.h"
#include "Src/Framework/Graphics/RenderManager.h"

ShaderManager::ShaderManager()
{
}

ShaderManager::~ShaderManager()
{
}

void ShaderManager::Init()
{
    // シェーダーの初期化処理が必要な場合はここに記述
    // シェーダーを一括でロードしてキャッシュ
	this->Load(ShaderStage::Vertex, "Assets/Shader/unlitTextureVS.cso", "unlitTextureVS");
	this->Load(ShaderStage::Pixel, "Assets/Shader/unlitTexturePS.cso", "unlitTexturePS");
	this->Load(ShaderStage::Vertex, "Assets/Shader/DefaultVS.cso", "DefaultVS");
	this->Load(ShaderStage::Pixel, "Assets/Shader/DefaultPS.cso", "DefaultPS");
}

/// <summary>
/// 指定されたシェーダーステージとCSOファイルパスからシェーダーを読み込みと作成を行う
/// </summary>
/// <param name="_stage">読み込むシェーダーのステージ（例: Vertex, Pixel など）。</param>
/// <param name="csoPath">CSOファイルのパス</param>
/// <returns>シェーダーの読み込みと作成が成功した場合は S_OK、失敗した場合は E_FAIL を返す</returns>
HRESULT ShaderManager::Load(ShaderStage _stage, const std::filesystem::path& csoPath, const std::string& _name)
{
	// ファイルパスに存在しない場合はエラー
    if (!std::filesystem::exists(csoPath)) { return E_FAIL; }

    // バイナリ読み込み
    std::ifstream file(csoPath, std::ios::binary | std::ios::ate);
    
	// ファイルが開けなかった場合はエラー
    if (!file) { return E_FAIL; }

	// ファイルサイズを取得
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

	// ファイルサイズが0以下の場合はエラー
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) { return E_FAIL; }

	std::unique_ptr<IShader> shader;

    switch (_stage)
    {
    case ShaderStage::Vertex:
		shader = std::make_unique<VertexShader>(_name);
        break;
    case ShaderStage::Pixel:
		shader = std::make_unique<PixelShader>(_name);
        break;
    case ShaderStage::Geometry:
        break;
    case ShaderStage::Compute:
        break;
    case ShaderStage::Hull:
        break;
    case ShaderStage::Domain:
        break;
    default:
        break;
    }

	// ここで全体のリフレクション作成→定数バッファとsrvの設定を行い、各シェーダーの作成を行う

    if (FAILED(shader->Analyze_to_Adjust(buffer.data(), static_cast<UINT>(size)))) {
		return E_FAIL;
    }

    m_Shaders[_name] = std::move(shader);
	return S_OK;
}


IShader* ShaderManager::GetShader(const std::string& _name) const
{
    auto it = m_Shaders.find(_name);
    if (it != m_Shaders.end()) {
        return it->second.get();
    }
    return nullptr;
}
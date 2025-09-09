#include "AssetManager.h"

AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{
}

void AssetManager::Init(void)
{
	// ここで必要なアセットをすべて読み込む
	//m_pTexture = std::make_unique<CTexture>();
	//m_pTexture->Load("Assets/texture.png");
	this->LoadAnimationMesh("Akai", "assets/model/akai/akai.fbx", "assets/model/akai/");
	this->LoadAnimationData("Akai_Run", "assets/model/akai/Akai_Run.fbx");
	this->LoadAnimationData("Akai_Idle", "assets/model/akai/Akai_Idle.fbx");
}

void AssetManager::Uninit(void)
{
	// アセットの解放
	m_AnimationDataList.clear();
	m_AnimationMeshList.clear();
}


void AssetManager::LoadAnimationData(const std::string& name, const std::filesystem::path& filepath)
{
	m_AnimationDataList[name] = std::make_unique<CAnimationData>();
	m_AnimationDataList[name]->LoadAnimation(filepath.string(), name);
}

CAnimationData* AssetManager::GetAnimationData(const std::string& name) const
{
	return m_AnimationDataList.at(name).get();
}

void AssetManager::LoadAnimationMesh(
	const std::string& name, 
	const std::filesystem::path& filepath, 
	const std::filesystem::path& texturepath)
{
	m_AnimationMeshList[name] = std::make_unique<CAnimationMesh>();
	m_AnimationMeshList[name]->Load(filepath.string(), texturepath.string());
}

CAnimationMesh* AssetManager::GetAnimationMesh(const std::string& name) const
{
	return m_AnimationMeshList.at(name).get();
}

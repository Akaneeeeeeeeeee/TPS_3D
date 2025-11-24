#include "AssetManager.h"
#include "system/CAnimationData.h"
#include "system/CAnimationMesh.h"
#include "system/CStaticMesh.h"


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
	this->LoadStaticMesh("Rock", "assets/model/Rock-Set/Rock_2/Rock_2.fbx", "assets/model/Rock-Set/Rock_2/Textures");

	this->LoadAnimationData("Walking", "assets/model/akai/Walking.fbx");
	this->LoadAnimationData("Jump", "assets/model/akai/Jump.fbx");
	this->LoadAnimationData("Running", "assets/model/akai/Running.fbx");
	this->LoadAnimationData("Watering", "assets/model/akai/Watering.fbx");
	this->LoadAnimationData("Crouching_Idle", "assets/model/akai/Crouching_Idle.fbx");
	this->LoadAnimationData("Crouched_Walking", "assets/model/akai/Crouched_Walking.fbx");
}

void AssetManager::Uninit(void)
{
	// アセットの解放
	m_AnimationDataList.clear();
	m_AnimationMeshList.clear();
	m_StaticMeshList.clear();
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

void AssetManager::LoadStaticMesh(
	const std::string& name,
	const std::filesystem::path& filepath,
	const std::filesystem::path& texturepath)
{
	m_StaticMeshList[name] = std::make_unique<CStaticMesh>();
	m_StaticMeshList[name]->Load(filepath.string(), texturepath.string());
}

CStaticMesh* AssetManager::GetStaticMesh(const std::string& name) const
{
	return m_StaticMeshList.at(name).get();
}
#pragma once
#include "system/Framework/Application/Entry/main.h"
#include "system/CAnimationData.h"
#include "system/CAnimationMesh.h"


class AssetManager
{
public:
	AssetManager();
	~AssetManager();

	static AssetManager& GetInstance()
	{
		static AssetManager instance;
		return instance;
	}

	void Init(void);
	void Uninit(void);
	
	void LoadAnimationData(const std::string& name, const std::filesystem::path& filepath);
	CAnimationData* GetAnimationData(const std::string& name) const;
	void LoadAnimationMesh(const std::string& name, const std::filesystem::path& filepath, const std::filesystem::path& texturepath);
	CAnimationMesh* GetAnimationMesh(const std::string& name) const;
	void LoadStaticMesh(const std::string& name, const std::filesystem::path& filepath, const std::filesystem::path& texturepath);
	CStaticMesh* GetStaticMesh(const std::string& name) const;

private:
	std::unordered_map<std::string, std::unique_ptr<CAnimationData>> m_AnimationDataList;	// アニメーションデータリスト
	std::unordered_map<std::string, std::unique_ptr<CAnimationMesh>> m_AnimationMeshList;	// アニメーションメッシュリスト
	std::unordered_map<std::string, std::unique_ptr<CStaticMesh>> m_StaticMeshList;			// 静的メッシュリスト
};


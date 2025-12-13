#pragma once
#include "system/Framework/Application/Entry/main.h"
#include <filesystem>

class CAnimationData;
class CAnimationMesh;
class CStaticMesh;
class CMesh;
class CMeshRenderer;
class CShader;

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



public:
	// ========= Mesh 系 =========
	template<class TMesh>
	bool RegisterMesh(const std::string& key, std::unique_ptr<TMesh> mesh)
		requires std::derived_from<TMesh, CMesh>;

	template<class TMesh = CMesh>
	TMesh* GetMesh(const std::string& key) const;

	// ========= MeshRenderer 系 =========
	template<class TMeshRenderer>
	bool RegisterMeshRenderer(const std::string& key, std::unique_ptr<TMeshRenderer> renderer)
		requires std::derived_from<TMeshRenderer, CMeshRenderer>;

	template<class TMeshRenderer = CMeshRenderer>
	TMeshRenderer* GetMeshRenderer(const std::string& key) const;


	// ========= Shader 系 =========
	template<class TShader>
	bool RegisterShader(const std::string& key, std::unique_ptr<TShader> shader)
		requires std::derived_from<TShader, CShader>;

	template<class TShader = CShader>
	TShader* GetShader(const std::string& key) const;

	// ========= AnimationData 系 =========
	template<class TAnimData>
	bool RegisterAnimationData(const std::string& key, std::unique_ptr<TAnimData> data)
		requires std::derived_from<TAnimData, CAnimationData>;

	template<class TAnimData = CAnimationData>
	TAnimData* GetAnimationData(const std::string& key) const;

private:
	std::unordered_map<std::string, std::unique_ptr<CAnimationData>> m_AnimationDataList;	// アニメーションデータリスト
	std::unordered_map<std::string, std::unique_ptr<CAnimationMesh>> m_AnimationMeshList;	// アニメーションメッシュリスト
	std::unordered_map<std::string, std::unique_ptr<CStaticMesh>> m_StaticMeshList;			// 静的メッシュリスト
	
	
	std::unordered_map<std::string, std::unique_ptr<CMesh>> m_MeshList;
	std::unordered_map<std::string, std::unique_ptr<CMeshRenderer>> m_RendererList;
	std::unordered_map<std::string, std::unique_ptr<CShader>> m_ShaderList;
	std::unordered_map<std::string, std::unique_ptr<CAnimationData>> m_AnimData;
};


// ========= Mesh =========
template<class TMesh>
inline bool AssetManager::RegisterMesh(const std::string& key, std::unique_ptr<TMesh> mesh)
	requires std::derived_from<TMesh, CMesh>
{
	if (!mesh) { return false; }

	// すでに同じキーがあれば登録しない
	if (m_MeshList.contains(key)) { return false; }

	m_MeshList.emplace(key, std::move(mesh));
	return true;
}

template<class TMesh>
inline TMesh* AssetManager::GetMesh(const std::string& key) const
{
	auto it = m_MeshList.find(key);
	if (it == m_MeshList.end()) { return nullptr; }

	// TMesh が CMesh あるいはその派生であることが前提
	return static_cast<TMesh*>(it->second.get());
}

// ========= MeshRenderer =========
template<class TMeshRenderer>
inline bool AssetManager::RegisterMeshRenderer(const std::string& key, std::unique_ptr<TMeshRenderer> renderer)
	requires std::derived_from<TMeshRenderer, CMeshRenderer>
{
	if (!renderer) { return false; }
	if (m_RendererList.contains(key)) { return false; }

	m_RendererList.emplace(key, std::move(renderer));
	return true;
}

template<class TMeshRenderer>
inline TMeshRenderer* AssetManager::GetMeshRenderer(const std::string& key) const
{
	auto it = m_RendererList.find(key);
	if (it == m_RendererList.end()) { return nullptr; }
	return static_cast<TMeshRenderer*>(it->second.get());
}


// ========= Shader =========
template<class TShader>
inline bool AssetManager::RegisterShader(const std::string& key, std::unique_ptr<TShader> shader)
	requires std::derived_from<TShader, CShader>
{
	if (!shader) { return false; }
	if (m_ShaderList.contains(key)) { return false; }

	std::unique_ptr<CShader> basePtr(std::move(shader));
	m_ShaderList.emplace(key, std::move(basePtr));
	return true;
}

template<class TShader>
inline TShader* AssetManager::GetShader(const std::string& key) const
{
	auto it = m_ShaderList.find(key);
	if (it == m_ShaderList.end()) { return nullptr; }

	return static_cast<TShader*>(it->second.get());
}


// ========= AnimationData =========
template<class TAnimData>
inline bool AssetManager::RegisterAnimationData(const std::string& key, std::unique_ptr<TAnimData> data)
	requires std::derived_from<TAnimData, CAnimationData>
{
	if (!data) { return false; }
	if (m_AnimData.contains(key)) { return false; }

	m_AnimData.emplace(key, std::move(data));
	return true;
}

template<class TAnimData>
inline TAnimData* AssetManager::GetAnimationData(const std::string& key) const
{
	auto it = m_AnimData.find(key);
	if (it == m_AnimData.end()) { return nullptr; }

	return static_cast<TAnimData*>(it->second.get());
}
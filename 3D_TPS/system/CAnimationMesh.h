#pragma once

#include "CStaticMesh.h"
#include "AssimpPerse.h"
#include "CAnimationData.h"
#include "CTreeNode.h"	
#include "renderer.h"
#include "BoneCombMatrix.h"
#include "CStaticMeshRenderer.h"
#include "Framework/Component/Transform/Transform.h"

class CAnimationMesh : public CStaticMesh
{
public:
	CAnimationMesh() = default;
	~CAnimationMesh() = default;

	void Load(std::string filename, std::string texturedirectory = "");

	// ローカルポーズ生成
	void BuildLocalPoseMap(
		const aiAnimation* animationdata,
		const int& CurrentFrame,
		std::unordered_map<std::string, Transform>& localposemap);

	// ポーズのブレンド
	void BlendLocalPose(
		const std::unordered_map<std::string, Transform>& localposeFrom,
		const std::unordered_map<std::string, Transform>& localposeTo,
		float rate,
		std::unordered_map<std::string, Transform>& blendedLocalPose);

	// 階層構造を考慮したボーンコンビネーション行列を更新
	void UpdateBoneMatrix(CTreeNode<std::string>* ptree, DirectX::SimpleMath::Matrix matrix);

	// アニメーションの更新
	void Update(BoneCombMatrix& bonecombarray, const aiAnimation* animation, const float& timeSec);

	// ブレンドアニメーションの更新
	void UpdateBlended(BoneCombMatrix& bonecombarray,
		const aiAnimation* animFrom,
		int frameFrom,
		const aiAnimation* animTo,
		int frameTo,
		float alpha);

	// 描画
	void Draw();

	CStaticMeshRenderer& GetRenderer() { return m_StaticMeshRenderer; }
	const CStaticMeshRenderer& GetRenderer() const { return m_StaticMeshRenderer; }
	
private:
	// ボーン辞書
	std::unordered_map<std::string, BONE> m_BoneDictionary{};

	// assimp ノード名ツリー（親子関係がわかる）
	CTreeNode<std::string>	m_AssimpNodeNameTree{};

	// レンダラ
	CStaticMeshRenderer m_StaticMeshRenderer{};
};


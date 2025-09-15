#pragma once

#include "CStaticMesh.h"
#include "AssimpPerse.h"
#include "CAnimationData.h"
#include "CTreeNode.h"	
#include "renderer.h"
#include "BoneCombMatrix.h"
#include "CStaticMeshRenderer.h"

class CAnimationMesh : public CStaticMesh
{
	// ボーン辞書
	std::unordered_map<std::string, BONE> m_BoneDictionary{};	// 20240714 DX化

	// 再生中のアニメーションデータ
	aiAnimation* m_CurrentAnimation{};

	// assimp ノード名ツリー（親子関係がわかる）
	CTreeNode<std::string>	m_AssimpNodeNameTree{};

	// レンダラ
	CStaticMeshRenderer m_StaticMeshRenderer{};

public:
	void SetCurentAnimation(aiAnimation* currentanimation);

	void Load(std::string filename, std::string texturedirectory = "");

	// 階層構造を考慮したボーンコンビネーション行列を更新
	void UpdateBoneMatrix(CTreeNode<std::string>* ptree, DirectX::SimpleMath::Matrix matrix);		// 20240714 DX化	

	// アニメーションの更新
	void Update(BoneCombMatrix& bonecombarray, int& CurrentFrame);

	// 描画
	void Draw();
};

//class CAnimationMesh : public CStaticMesh {
//    std::unordered_map<std::string, BONE> m_BoneDictionary{};
//    CTreeNode<std::string> m_AssimpNodeNameTree{};
//    CStaticMeshRenderer m_StaticMeshRenderer{};
//    const aiScene* m_pScene = nullptr;
//
//public:
//    void Load(std::string filename, std::string texturedirectory = "");
//
//    // アニメーションを名前で返すだけ（状態は持たない）
//    aiAnimation* GetAnimation(const std::string& name);
//
//    // ルートノードを取得する getter
//    CTreeNode<std::string>* GetRootNode() { return &m_AssimpNodeNameTree; }
//
//    // ボーン更新（アニメーションの状態は外から渡す）
//    void UpdateBoneMatrix(CTreeNode<std::string>* ptree,
//        const DirectX::SimpleMath::Matrix& parent,
//        BoneCombMatrix& outMatrix,
//        float timeInTicks,
//        aiAnimation* anim);
//
//    void Draw();
//};

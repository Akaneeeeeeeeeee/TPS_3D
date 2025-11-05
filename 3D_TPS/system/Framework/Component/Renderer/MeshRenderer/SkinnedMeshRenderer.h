#pragma once
#include "Component/Renderer/IRenderer/IRenderer.h"
#include "system/MyVertexBuffer.h"
#include "system/MyIndexBuffer.h"
#include "system/CAnimationMesh.h"

// 前方宣言
class SkinnedMeshRenderData;
class MyMaterial;
class IShader;
class VERTEX_SKIN;


struct SkinnedMeshRenderData {
    std::vector<VERTEX_3D> vertices;
    std::vector<uint32_t> indices;
    std::vector<SUBSET> subsets;
    std::vector<MATERIAL> materials;
    std::vector<ID3D11ShaderResourceView*> textures;

    // ボーン辞書やツリーなど CPU 側メッシュ情報
    CAnimationMesh* mesh;
};

/*
* @brief	スキンメッシュ描画コンポーネント
* @detail	スキンメッシュを描画するためのコンポーネント
* @remark	頂点バッファ、インデックスバッファ、シェーダー名を保持する
* @auther	赤根和樹
* @date		2025/11/03
*/
class SkinnedMeshRenderer : public IRenderer
{
public:
	SkinnedMeshRenderer(const SkinnedMeshRenderData& data);
	~SkinnedMeshRenderer() = default;

    void Init(void) override;
    void Uninit(void) override;
	void Update(const uint64_t deltatime) override;
    bool GetRenderInfo(RenderInfo& out) override;
	void SetBoneMatrices(BoneCombMatrix mat) { m_BoneMatrices = mat; }

	void EnumerateRenderInfos(std::vector<RenderInfo>& outInfos) override;

private:
    // GPUリソース
    MyVertexBuffer<VERTEX_3D>   m_VB;
    MyIndexBuffer               m_IB;

    // メッシュ構造
    std::vector<SUBSET> m_Subsets;
    std::vector<ID3D11ShaderResourceView*> m_Textures;
    std::vector<std::unique_ptr<MyMaterial>> m_Materials;

	ComPtr<ID3D11Buffer> m_pWorldBuffer;		// ワールド行列バッファ

    // スキニング用
    BoneCombMatrix m_BoneMatrices;

    // 元データを保存（再生成用 or debug 用）
    SkinnedMeshRenderData m_Data;
};


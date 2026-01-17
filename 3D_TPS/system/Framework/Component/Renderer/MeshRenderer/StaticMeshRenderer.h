#pragma once
#include "system/Framework/Component/Renderer/IRenderer/IRenderer.h"
#include "system/CVertexBuffer.h"
#include "system/CIndexBuffer.h"
#include "system/Framework/GameObject/GameObject.h"
#include "system/Framework/Graphics/RenderInfo.h"
#include "system/CMaterial.h"
#include "system/CStaticMeshRenderer.h"

/*
* @brief	メッシュ描画コンポーネント
* @detail	メッシュを描画するためのコンポーネント
* @remark	頂点バッファ、インデックスバッファ、シェーダー名を保持する
* @remark	メッシュデータをポインタで持ち、そのデータを元に頂点バッファ、インデックスバッファを生成、描画を行う
* @auther	赤根和樹
* @date		2025/10/16
* 
* todo: マテリアル、テクスチャ、シェーダーをRenderInfoに入れてRendereManagerから描画、で行けそう
*/
class StaticMeshRendererComponent final : public IRenderer
{
public:
	DECLARE_COMPONENT_TYPE(StaticMeshRendererComponent, IRenderer)
    StaticMeshRendererComponent() = default;
    ~StaticMeshRendererComponent() override = default;

    void Init() override {}
    void Update(const float) override {}
    void Uninit() override {}

    // AssetManagerに登録した MeshRenderer のキー
    void SetMeshRendererKey(const std::string& key) { m_MeshRendererKey = key; }

    // AssetManagerに登録した Shader のキー（"unlightshader" 等）
    void SetShaderKey(const std::string& key) { m_ShaderKey = key; }

    // 半透明にしたい場合（とりあえずここで切替）
    void SetTransparent(bool isTransparent) { m_IsTransparent = isTransparent; }

    //bool GetRenderInfo(RenderInfo& outInfo) override;
	void CollectRenderPackets(std::vector<RenderPacket>& out) override;

private:
    // サブセット情報から DrawItem を組み立てる
    void BuildDrawItems(const CStaticMeshRenderer& renderer);

private:
    std::string m_MeshRendererKey;
    std::string m_ShaderKey;

    bool m_IsTransparent = false;

    // RenderInfo.items で参照されるのでメンバで保持
    std::vector<DrawItem> m_DrawItems;
};

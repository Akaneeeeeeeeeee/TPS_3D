#include "MeshRenderer.h"


void MeshRenderer::Init(void)
{
    // 頂点バッファ、インデックスバッファ、マテリアル、シェーダーの初期化処理をここに記述
    m_VertexBuffer.Create(m_MeshData.vertices);
    m_IndexBuffer.Create(m_MeshData.indices);

    // ワールド行列用 CB 作成
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(Matrix4x4);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    Renderer::GetDevice()->CreateBuffer(&desc, nullptr, m_pWorldBuffer.GetAddressOf());

    // --- Materials ---
    m_Materials.reserve(m_MeshData.m_materials.size());
    for (const auto& m : m_MeshData.m_materials)
    {
        auto mat = std::make_unique<MyMaterial>("vertexLightingVS", "vertexLightingPS", "PBRMaterial");
        mat->SetMaterial(m);
        m_Materials.push_back(std::move(mat));
    }
}

void MeshRenderer::EnumerateRenderInfos(std::vector<RenderInfo>& outInfos)
{
    for (size_t i = 0; i < m_Subsets.size(); ++i)
    {
        RenderInfo info;
        const auto& subset = m_Subsets[i];
        const auto& mat = m_Materials[subset.MaterialIdx];

        info.vertexBuffer = m_VertexBuffer.GetBuffer();
        info.indexBuffer = m_IndexBuffer.GetBuffer();
        info.stride = m_VertexBuffer.GetStride();
        info.indexCount = subset.IndexNum;

        info.startIndex = subset.IndexBase;
        info.baseVertex = subset.VertexBase;

        mat->FillRenderInfo(info);

        outInfos.push_back(info);
    }
}
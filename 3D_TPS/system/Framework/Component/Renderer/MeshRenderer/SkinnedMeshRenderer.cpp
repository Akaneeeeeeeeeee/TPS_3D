#include "SkinnedMeshRenderer.h"
#include "Material/MyMaterial.h"


SkinnedMeshRenderer::SkinnedMeshRenderer(const SkinnedMeshRenderData& data)
	: IRenderer(), m_Data(data)
{
}



void SkinnedMeshRenderer::Init(void)
{
    // --- Vertex Buffer ---
    m_VB.Create(m_Data.vertices);

    // --- Index Buffer ---
    m_IB.Create(m_Data.indices);

    // --- Subsets ---
    m_Subsets = m_Data.subsets;

    // ワールド行列用 CB 作成
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(Matrix4x4);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    Renderer::GetDevice()->CreateBuffer(&desc, nullptr, m_pWorldBuffer.GetAddressOf());

    // --- Materials ---
    m_Materials.reserve(m_Data.materials.size());
    for (const auto& m : m_Data.materials)
    {
        auto mat = std::make_unique<MyMaterial>("vertexLightingOneSkinVS", "vertexLightingPS", "PBRMaterial");
        mat->SetMaterial(m);
        m_Materials.push_back(std::move(mat));
    }

    // Textures
    m_Textures = m_Data.textures;

    // Bone CB
    m_BoneMatrices.Create();
}

void SkinnedMeshRenderer::Update(const uint64_t deltatime)
{
    // Bone CB 更新
    m_BoneMatrices.Update();
}

void SkinnedMeshRenderer::Uninit(void)
{
    //IRenderer::Uninit();
}

bool SkinnedMeshRenderer::GetRenderInfo(RenderInfo& out)
{
    //out.vertexBuffer = m_VB.GetBuffer();
    //out.indexBuffer = m_IB.GetBuffer();
    //out.stride = m_VB.GetStride();
    //out.indexCount = m_IB.GetIndexCount();

    //out.vs = m_pS;
    //out.ps = m_PS;

    //// Clear and fill
    //out.srvs.clear();
    //out.cBuffers.clear();

    //for (auto& mat : m_Materials)
    //{
    //    mat->FillRenderInfo(out);
    //}

    //// スキニング用 CB
    //out.boneMatrixCB = m_BoneMatrices.ConstantBuffer.Get();
    //out.boneCount = MAX_BONE;

    return true;
}
    
void SkinnedMeshRenderer::EnumerateRenderInfos(std::vector<RenderInfo>& outInfos)
{
    for (size_t i = 0; i < m_Subsets.size(); ++i)
    {
        RenderInfo info;
        const auto& subset = m_Subsets[i];
        const auto& mat = m_Materials[subset.MaterialIdx];

        info.vertexBuffer = m_VB.GetBuffer();
        info.indexBuffer = m_IB.GetBuffer();
        info.stride = m_VB.GetStride();
        info.indexCount = subset.IndexNum;

        info.startIndex = subset.IndexBase;
        info.baseVertex = subset.VertexBase;

        mat->FillRenderInfo(info);

        info.boneMatrixCB = m_BoneMatrices.ConstantBuffer.Get();
        info.boneCount = MAX_BONE;

        outInfos.push_back(info);
    }
}
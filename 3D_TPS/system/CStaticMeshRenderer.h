#pragma once

#include	"CStaticMesh.h"
#include	"CMeshRenderer.h"
#include	"CTexture.h"
#include    "CMaterial.h"

class CStaticMeshRenderer : public CMeshRenderer 
{
	std::vector<SUBSET> m_Subsets;
	std::vector<std::unique_ptr<CTexture>> m_DiffuseTextures;
	std::vector<std::unique_ptr<CMaterial>> m_Materiales;

public:	
	void Init(CStaticMesh& mesh);
	void Draw();

    const std::vector<SUBSET>& GetSubsets() const { return m_Subsets; }

    CMaterial* GetMaterial(size_t idx) const { return m_Materiales[idx].get(); }
    CTexture* GetDiffuseTextureOrNull(size_t idx) const
    {
        if (!m_Materiales[idx]->isDiffuseTextureEnable()) return nullptr;
        return m_DiffuseTextures[idx].get();
    }
};

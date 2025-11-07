#include	"obstacle.h"	
#include    "system/CDirectInput.h"
#include	"system/meshmanager.h"

#include	"scene/TestScene.h"

void obstacle::Init() 
{
	m_mesh = MeshManager::getMesh<CStaticMesh>("obstaclebox");
	m_shader = MeshManager::getShader<CShader>("unlightshader");
	m_meshrenderer = MeshManager::getRenderer<CStaticMeshRenderer>("obstaclebox");
}

void obstacle::Update(const uint64_t dt) {


}

void obstacle::Draw(void) const {


	Matrix4x4 mtx = m_Transform.GetWorldMatrix();

	Renderer::SetWorldMatrix(&mtx);

	m_shader->SetGPU();
	m_meshrenderer->Draw();

}

void obstacle::Uninit(void) {

}

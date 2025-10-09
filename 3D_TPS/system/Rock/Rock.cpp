#include "Rock.h"
#include "system/Framework/Component/Renderer/MeshRenderer/MeshRenderer.h"

Rock::Rock(EngineContext& context, uint64_t id, const std::string& name, const Tag& tag)
	: GameObject(context, id, name, tag)
{
}

Rock::~Rock()
{
}

void Rock::Init(void)
{
	AddComponent<MeshRenderer<VERTEX_3D>>()
}

void Rock::Update(uint64_t deltatime)
{

}

void Rock::Draw(uint64_t deltatime)
{

}

void Rock::Uninit(void)
{

}
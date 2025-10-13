#include "Rock.h"
#include "system/Framework/Component/Renderer/MeshRenderer/MeshRenderer.h"

Rock::Rock(EngineContext& context, const uint64_t id, 
	const std::string& name, const Tag& tag,
	const Transform& transform)
	: GameObject(context, id, name, tag, transform)
{
}

Rock::~Rock()
{
}

void Rock::Init(void)
{
	//AddComponent<MeshRenderer>("");
}

void Rock::Update(const uint64_t deltatime)
{

}

void Rock::Draw(const uint64_t deltatime) const
{

}

void Rock::Uninit(void)
{

}
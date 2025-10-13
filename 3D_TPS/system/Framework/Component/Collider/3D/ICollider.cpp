#include "ICollider.h"
#include "system/Framework/EngineContext/EngineContext.h"

ICollider::ICollider() : IComponent()
{
}

ICollider::~ICollider()
{
}

void ICollider::Attach(EngineContext& context)
{
	context.colliderManager.Register(this);
}

void ICollider::Detach(EngineContext& context)
{
	context.colliderManager.UnRegister(this);
}
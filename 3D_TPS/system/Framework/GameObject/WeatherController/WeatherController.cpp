#include "WeatherController.h"
#include "Framework/Component/Particle/ParticleComponent.h"

WeatherController::WeatherController(ComponentFactory* factory, const uint64_t id,
	const std::string& name, const Tag& tag,
	const Transform& transform)
	: GameObject(factory, id, name, tag, transform)
{
}

WeatherController::~WeatherController()
{
}

void WeatherController::SetSpawnAreaXZ(float halfWidth, float halfDepth)
{
	if (m_ParticleComp)
	{
		m_ParticleComp->SetSpawnAreaXZ(halfWidth, halfDepth);
	}
}

void WeatherController::Awake(void)
{
	m_ParticleComp = AddComponent<ParticleComponent>("WeatherParticle");
}

void WeatherController::Update(const float dt)
{
}

void WeatherController::Uninit(void)
{
}
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
	//m_ParticleComp->SetSpawnAreaXZ(200000.0f, 200000.0f); // L‚¢”ÍˆÍ‚É¶¬
}

void WeatherController::Update(const float dt)
{
    GameObject::Update(dt);

    // ƒJƒƒ‰‚É’Ç]‚³‚¹‚½‚¢‚È‚ç‚±‚±‚ÅˆÊ’u‚ğ‡‚í‚¹‚é
}

void WeatherController::Uninit(void)
{
	GameObject::Uninit();
}
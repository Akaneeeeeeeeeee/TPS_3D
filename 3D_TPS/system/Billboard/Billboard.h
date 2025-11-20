#pragma once
#include "system/CSprite.h"
#include "system/Framework/GameObject/GameObject.h"
#include "system/camera.h"


class Billboard : public GameObject
{
public:
	Billboard(ComponentFactory* factory, const uint64_t id,
		const std::string& name = "", const Tag& tag = Tag::None, 
		const Transform& transform = Transform::One())
		: GameObject(factory, id, name, tag, transform),
		m_Sprite(), m_pCamera()
	{
	}
	/*Billboard(uint64_t id, const std::string& name = "", const Tag& tag = Tag::None)
		: GameObject(id, name, tag),
		m_Sprite(width, height, texfilename), m_pCamera(cam)
	{
	}*/

	virtual ~Billboard() { m_Sprite.Dispose(); }

	void Init(void) override;
	void Init(int width, int height, const std::string& texfilename, FreeCamera* cam);

	void Update(const float deltatime) override;

	void Draw(void) const override;
	

private:
	CSprite m_Sprite;
	Camera* m_pCamera = nullptr;

	// Quaternion Ç Euler äpÇ…ïœä∑Åiä»à’î≈Åj
	Vector3 GetRotationEuler() const;
};
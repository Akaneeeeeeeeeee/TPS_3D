#pragma once
#include "system/CSprite.h"
#include "system/Framework/GameObject/GameObject.h"
#include "system/camera.h"


class Billboard : public GameObject
{
public:
	Billboard(int width, int height, const std::string& texfilename, FreeCamera* cam)
		: m_Sprite(width, height, texfilename), m_pCamera(cam)
	{
	}

	virtual ~Billboard() { m_Sprite.Dispose(); }

	void Init(void) override;

	void Update(uint64_t deltatime) override;

	void Draw(uint64_t deltatime) override;
	

private:
	CSprite m_Sprite;
	Camera* m_pCamera = nullptr;

	// Quaternion Ç Euler äpÇ…ïœä∑Åiä»à’î≈Åj
	Vector3 GetRotationEuler() const;
};
#pragma once
#include "Src/Framework/Component/IComponent/IComponent.h"


enum class ForceMode {
	Force,			// �͂�������
	Impulse,		// �Ռ���^����
	Acceleration,	// �����x��^����
};

class Rigidbody final : public IComponent
{
public:
	Rigidbody();
	~Rigidbody();

	void Init(void) override;
	void Update(void) override;
	void Uninit(void) override;

	void AddForce(const Vector3& _force, ForceMode _mode = ForceMode::Force);

private:

};

Rigidbody::Rigidbody()
{
}

Rigidbody::~Rigidbody()
{
}
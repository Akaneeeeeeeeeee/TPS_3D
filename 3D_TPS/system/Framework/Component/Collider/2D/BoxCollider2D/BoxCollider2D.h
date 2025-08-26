#pragma once
#include "../../2D/Collider2D/Collider2D.h"

/**
 * @brief 箱の当たり判定
*/
class BoxCollider2D :public Collider2D
{
public:
	BoxCollider2D(GameObject* _Owner);
	~BoxCollider2D();

private:

};




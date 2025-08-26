#pragma once
#include "../../../IComponent/IComponent.h"

/**
 * @brief 当たり判定
*/
class Collider2D :public IComponent
{
public:
	Collider2D(GameObject* _Owner);
	~Collider2D();

private:

};



#pragma once
#include "system/Framework/Component/IComponent/IComponent.h"

/*
* @brief	IColliderクラス
* @detail	当たり判定用の基底クラス
* @remark	このクラスを継承した各コライダーをColliderManagerで管理する
*/
class ICollider : public IComponent
{
public:
	
	virtual ~ICollider();

	virtual void Init(void) override = 0;
	virtual void Update(const uint64_t deltatime) override = 0;
	virtual void Uninit(void) override = 0;

protected:
	ICollider();
	virtual void Attach(EngineContext& context) override;
	virtual void Detach(EngineContext& context) override;
};



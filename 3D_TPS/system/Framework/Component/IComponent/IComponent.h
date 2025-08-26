#pragma once
#include "system/Framework/Application/Entry/main.h"

class GameObject;	// 前方宣言

/// <summary>
/// 全てのコンポーネントの元となるクラス
/// コンポーネントはそれをアタッチ（取りつけ）するオブジェクトがあるのでメンバ変数としてGameObjcetクラスのポインタを持つ
/// →それによってコンポーネントからアタッチしているオブジェクトにアクセスすることができる
/// </summary>

class IComponent
{
public:
	virtual ~IComponent() = default;	// デストラクタ

	virtual void Init(void) = 0;		// 初期化
	virtual void Update(void) = 0;		// 更新
	virtual void Uninit(void) = 0;		// 終了

	virtual void SetIsActive(const bool flg) { IsActive = flg; }
	virtual bool GetIsActive(void) { return IsActive; }

	void SetOwner(GameObject* _obj);		// オブジェクトのアタッチ(参照渡し)
	GameObject* GetOwner(void);				// アタッチ先のオブジェクトの取得
	//void RemoveOwner(void);			// アタッチされているオブジェクトからの取り外し

protected:
	// インターフェースクラスなのでprotected
	IComponent() {};
	// sharedだとややこしくなるので生ポインタにする
	GameObject* m_pOwner = nullptr;
	// 有効化フラグ
	bool IsActive = true;
};


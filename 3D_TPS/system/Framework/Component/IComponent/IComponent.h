#pragma once

class GameObject;	// 前方宣言
struct EngineContext;	// 前方宣言(管理システム)

/// <summary>
/// 全てのコンポーネントの元となるクラス
/// コンポーネントはそれをアタッチ（取りつけ）するオブジェクトがあるのでメンバ変数としてGameObjcetクラスのポインタを持つ
/// →それによってコンポーネントからアタッチしているオブジェクトにアクセスすることができる
/// </summary>

class IComponent
{
public:
	virtual ~IComponent();	// デストラクタ

	virtual void Init(void) = 0;		// 初期化
	virtual void Update(const float deltatime) = 0;		// 更新
	virtual void Uninit(void) = 0;		// 終了

	virtual void SetIsValid(bool flg) { IsValid = flg; }
	virtual bool GetIsValid(void) { return IsValid; }

	virtual void SetOwner(GameObject* _obj);		// オブジェクトのアタッチ(参照渡し)
	virtual GameObject* GetOwner(void);				// アタッチ先のオブジェクトの取得
	//void RemoveOwner(void);			// アタッチされているオブジェクトからの取り外し

	virtual void Attach(EngineContext& context) = 0;	// アタッチされたときの処理(各派生コンポーネントでどの管理システムに登録するかを実装)
	virtual void Detach(EngineContext& context) = 0;	// デタッチされたときの処理(各派生コンポーネントでどの管理システムから解除するかを実装)

protected:
	// インターフェースクラスなのでprotected
	explicit IComponent();
	// コンポーネントの所有者(sharedだとややこしくなるので生ポインタにする)
	GameObject* m_pOwner = nullptr;
	// 有効化フラグ
	bool IsValid = true;
};


#pragma once
#include "system/Framework/Application/Entry/main.h"

// 前方宣言
class GameObject;
struct EngineServices;

/// <summary>
/// 全てのコンポーネントの元となるクラス
/// コンポーネントはそれをアタッチ（取りつけ）するオブジェクトがあるのでメンバ変数としてGameObjcetクラスのポインタを持つ
/// →それによってコンポーネントからアタッチしているオブジェクトにアクセスすることができる
/// </summary>

class IComponent
{
public:
	using TypeId = uint32_t;

	virtual ~IComponent();	// デストラクタ

	// RTTI不要の型ID
	template<class T>
	static TypeId TypeIdOf()
	{
		static const TypeId id = NewTypeId();
		return id;
	}

	// 実型ID
	virtual TypeId GetTypeId() const { return TypeIdOf<IComponent>(); }

	// 継承チェーン判定（BaseでDerivedを取るため）
	virtual bool IsA(TypeId id) const { return id == TypeIdOf<IComponent>(); }

	template<class T>
	bool IsA() const { return IsA(TypeIdOf<T>()); }

	virtual void Init(void) = 0;		// 初期化
	virtual void Update(const float deltatime) = 0;		// 更新
	virtual void LateUpdate(const float deltatime) {}	// 後更新(必要なら派生先でオーバーライド)
	virtual void Uninit(void) = 0;		// 終了
	virtual void Draw(void) const {}	// 描画(必要なら派生先でオーバーライド)

	void SetIsValid(bool flg) { IsValid = flg; }
	bool GetIsValid(void) const { return IsValid; }

	void Destroy(void) { IsDestroy = true; }
	bool IsDestroyRequested(void) const { return IsDestroy; }

	virtual void SetOwner(GameObject* _obj);		// オブジェクトのアタッチ(参照渡し)
	virtual GameObject* GetOwner(void) const;		// アタッチ先のオブジェクトの取得
	//void RemoveOwner(void);			// アタッチされているオブジェクトからの取り外し

	virtual void Attach(EngineServices& context) = 0;	// アタッチされたときの処理(各派生コンポーネントでどの管理システムに登録するかを実装)
	virtual void Detach(void) = 0;	// デタッチされたときの処理(各派生コンポーネントでどの管理システムから解除するかを実装)

protected:
	// 型ID発行
	static TypeId NewTypeId()
	{
		static std::atomic<TypeId> s{ 1 };
		return s.fetch_add(1, std::memory_order_relaxed);
	}

	// インターフェースクラスなのでprotected
	explicit IComponent();
	// コンポーネントの所有者(sharedだとややこしくなるので生ポインタにする)
	GameObject* m_pOwner = nullptr;
	// 有効化フラグ
	bool IsValid = true;
	// 破棄フラグ
	bool IsDestroy = false;
};

// 1行で宣言するためのマクロ
#define DECLARE_COMPONENT_TYPE(Derived, Base) \
public: \
    TypeId GetTypeId() const override { return IComponent::TypeIdOf<Derived>(); } \
    bool IsA(TypeId id) const override \
    { \
        return (id == IComponent::TypeIdOf<Derived>()) || Base::IsA(id); \
    }
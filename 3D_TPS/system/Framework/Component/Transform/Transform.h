#pragma once
#include "system/Framework/Application/Entry/main.h"
#include "system/commontypes.h"

/// <summary>
/// Transformクラス：全てのオブジェクトが持っている情報（ここでは数値のみを扱う）
/// ・座標
/// ・回転
/// ・大きさ
/// の情報を持つ
/// 
/// 回転情報はQuaternionで管理しておき、必要に応じて回転角からQuaternionに変換する
/// </summary>
class Transform
{
public:
	Transform(
		const Vector3& _pos = Vector3::Zero, 
		const Quaternion& _rot = Quaternion::Identity,
		const Vector3& _scale = Vector3::Zero) 
		: Position(_pos), Rotation(_rot), Scale(_scale) {};
	~Transform() = default;

	inline static const Transform One(void) { return Transform(Vector3::Zero, Quaternion::Identity, Vector3::One); }
	inline static const Transform Zero(void) { return Transform(Vector3::Zero, Quaternion::Identity, Vector3::Zero); }

	// 行列変換
	Matrix4x4 GetLocalMatrix(void) const;
	Matrix4x4 GetWorldMatrix(void) const;

	//-------------------------------------------------------------
	//				ゲッターセッター(ImGui用に参照渡しを用意)
	//-------------------------------------------------------------
	Vector3 GetPosition(void) const { return Position; }
	Vector3& PositionRef(void) { SetDirty(); return Position; }
	void SetPosition(const Vector3& pos) { SetDirty(); Position = pos; }

	Quaternion GetRotation(void) const { return Rotation; }
	Quaternion& RotationRef(void) { return Rotation; }
	void SetRotation(const Quaternion& rot) { SetDirty(); Rotation = rot; }

	Vector3 GetScale(void) const { return Scale; }
	Vector3& ScaleRef(void) { return Scale; }
	void SetScale(const Vector3& scale) { SetDirty(); Scale = scale; }

	void SetParent(Transform* _parent);

	void SetChild(Transform* _child);

	void SetDirty(void);

	// 前方ベクトルを取得する
	Vector3 GetForward(void) const
	{
		// デフォルトの前方ベクトル (Zプラス方向)
		static const Vector3 defaultForward = Vector3::Forward;
		// 回転を適用して前方ベクトルを返す
		return Vector3::Transform(defaultForward, Rotation);
	}

private:
	Vector3		Position;	// ローカル座標
	Quaternion	Rotation;	// ローカル回転
	Vector3		Scale;		// ローカル大きさ
	
	Transform* m_pParent = nullptr;			// 親オブジェクトのポインタ
	std::vector<Transform*> m_pChildren;	// 子オブジェクトのコンテナ

	mutable bool m_IsDirty = true;			// 行列が更新されたかどうかのフラグ
	mutable Matrix4x4 m_WorldMatrix;		// 行列キャッシュ
};

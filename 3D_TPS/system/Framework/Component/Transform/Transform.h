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
	Transform() : Position(0, 0, 0), Rotation(0, 0, 0, 1), Scale(1, 1, 1) {};
	Transform(const Vector3& _pos, const Quaternion& _rot, const Vector3& _scale) : Position(_pos), Rotation(_rot), Scale(_scale) {};
	~Transform() {};

	// 行列変換
	Matrix4x4 GetLocalMatrix(void) const;

	Matrix4x4 GetWorldMatrix(void) const;

	//-----------------------------------------
	//				ゲッターセッター
	//-----------------------------------------
	Vector3 GetPosition(void) const { return Position; }
	const Vector3& GetPositionRef(void) const { return Position; }		// 参照渡し
	void SetPosition(const Vector3& pos) { Position = pos; }

	Quaternion GetRotation(void) const { return Rotation; }
	const Quaternion& GetRotationRef(void) const { return Rotation; }	// 参照渡し
	void SetRotation(const Quaternion& rot) { Rotation = rot; }

	Vector3 GetScale(void) const { return Scale; }
	const Vector3& GetScaleRef(void) const { return Scale; }			// 参照渡し
	void SetScale(const Vector3& scale) { Scale = scale; }

	void SetParent(Transform* _parent);

	void SetChild(Transform* _child);

private:
	Vector3		Position;	// ローカル座標
	Quaternion	Rotation;	// ローカル回転
	Vector3		Scale;		// ローカル大きさ
	
	Transform* m_pParent = nullptr;		// 親オブジェクトのポインタ
	std::vector<Transform*> m_pChildren;	// 子オブジェクトのコンテナ
};

#pragma once
#include "system/Framework/Application/Entry/main.h"

class GameObject;

/// <summary>
/// Transformクラス：全てのオブジェクトが持っている情報（ここでは数値のみを扱う）
/// ・座標
/// ・回転
/// ・大きさ
/// の情報を持つ
/// 
/// 各数値の行列変換は毎フレーム行う→更新は不要
/// ワールド行列への変換はTransform内で行う
/// </summary>
class Transform
{
public:
	Transform() :Position(0.0f, 0.0f, 0.0f), Rotation(0.0f, 0.0f, 0.0f, 0.0f), Scale(75.0f, 75.0f, 0.0f) {};
	Transform(const Vector3& _pos, const Quaternion& _rot, const Vector3& _scale) : Position(_pos), Rotation(_rot), Scale(_scale) {};
	~Transform() {};

	// 行列変換
	DirectX::SimpleMath::Matrix GetLocalMatrix(void) const;

	DirectX::SimpleMath::Matrix GetWorldMatrix(const DirectX::SimpleMath::Matrix& _parentmatrix);

	//-----------------------------------------
	//				ゲッターセッター
	//-----------------------------------------
	const Vector3& GetPosition() const { return Position; }
	void SetPosition(const Vector3& pos) { Position = pos; }

	const Quaternion& GetRotation() const { return Rotation; }
	void SetRotation(const Quaternion& rot) { Rotation = rot; }

	const Vector3& GetScale() const { return Scale; }
	void SetScale(const Vector3& scale) { Scale = scale; }

	void SetParent(GameObject* _parent) { m_pParent = _parent; }

	void SetChild(GameObject* _child) { m_pChildren.push_back(_child); }

private:
	Vector3		Position;	// 座標
	Quaternion	Rotation;	// 回転
	Vector3		Scale;		// 大きさ	
	
	GameObject* m_pParent = nullptr;		// 親オブジェクトのポインタ
	std::vector<GameObject*> m_pChildren;	// 子オブジェクトのコンテナ
};



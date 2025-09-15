#pragma once
#include "system/CSprite.h"
#include "system/Framework/GameObject/GameObject.h"
#include "system/camera.h"


class Billboard : public GameObject
{
public:
	Billboard(int width, int height, const std::string& texfilename, FreeCamera* cam)
		: m_Sprite(width, height, texfilename), m_pCamera(cam)
	{
	}

	virtual ~Billboard() { m_Sprite.Dispose(); }

	void Init() override
	{
		// 特になし。必要なら初期化処理
	}

	void Update(uint64_t deltatime) override
	{
		// カメラ方向に向ける
		if (!m_pCamera) return;

		Matrix4x4 view = m_pCamera->GetViewMatrix();
		//view = view.Invert();
		//view._41 = 0.0f;
		//view._42 = 0.0f;
		//view._43 = 0.0f;
		//SetRotation(Quaternion::CreateFromRotationMatrix(view));

		
	}

	void Draw(uint64_t deltatime) override
	{
		// カメラの座標からビルボードの座標へのベクトルを計算
		// ビルボードの位置
		Vector3 pos = m_Transform.GetPosition();

		// カメラ → ビルボード方向ベクトル
		Vector3 look = pos - m_pCamera->GetPosition();
		look.Normalize();

		// 上ベクトル（ワールドのYを基準）
		Vector3 up(0, 1, 0);
		Matrix4x4 scale = Matrix4x4::CreateScale(m_Transform.GetScale());

		Matrix4x4 world = Matrix4x4::CreateBillboard(pos, m_pCamera->GetPosition(), up, &look);

		world = scale * world;
		Renderer::DisableCulling(false);
		m_Sprite.Draw(world, m_pCamera->GetViewMatrix(), m_pCamera->GetProjMatrix());
		Renderer::DisableCulling(true);
	}

private:
	CSprite m_Sprite;
	Camera* m_pCamera = nullptr;

	// Quaternion を Euler 角に変換（簡易版）
	Vector3 GetRotationEuler() const
	{
		const Quaternion& q = GetRotation();
		Vector3 euler;
		euler.y = atan2f(2.0f * (q.w * q.y + q.z * q.x), 1.0f - 2.0f * (q.y * q.y + q.z * q.z));
		euler.x = asinf(2.0f * (q.w * q.x - q.y * q.z));
		euler.z = atan2f(2.0f * (q.w * q.z + q.x * q.y), 1.0f - 2.0f * (q.x * q.x + q.z * q.z));
		return euler;
	}
};
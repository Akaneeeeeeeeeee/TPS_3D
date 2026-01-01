#include <memory>
#include <utility>

#include "Framework/Component/Throw/IThrowAction.h"
#include "Framework/ObjectManager/ObjectManager.h"
#include "Framework/GameObject/GameObject.h"
#include "GameObject/Rock.h"
// #include "GameObject/Can.h"
// #include "GameObject/Grenade.h"

// ここに Rock/Can/Grenade の具象Actionを全部まとめる。
// 外に公開するのは MakeRockThrowAction などの生成関数だけ。


namespace
{
	// ------------------------------------------------------------
	// 「速度の渡し方」がプロジェクトごとに違っても壊れにくいように
	// 存在する関数だけを呼ぶ（無ければ何もしない）
	// ------------------------------------------------------------
	template<class T>
	static void TryApplyVelocity(T& obj, const Vector3& v)
	{
		if constexpr (requires(T & o, const Vector3 & vv) { o.SetVelocity(vv); })
		{
			obj.SetVelocity(v);
		}
		else if constexpr (requires(T & o, const Vector3 & vv) { o.SetLinearVelocity(vv); })
		{
			obj.SetLinearVelocity(v);
		}
		else if constexpr (requires(T & o, const Vector3 & vv) { o.SetInitialVelocity(vv); })
		{
			obj.SetInitialVelocity(v);
		}
		else
		{
			// 速度を渡す口が無いなら何もしない。
			// 必要なら Rock 側に SetInitialVelocity 等を用意する。
		}
	}

	template<class T>
	static void TryApplyAngularVelocity(T& obj, const Vector3& w)
	{
		if constexpr (requires(T & o, const Vector3 & ww) { o.SetAngularVelocity(ww); })
		{
			obj.SetAngularVelocity(w);
		}
		else if constexpr (requires(T & o, const Vector3 & ww) { o.SetInitialAngularVelocity(ww); })
		{
			obj.SetInitialAngularVelocity(w);
		}
		else
		{
			// 角速度を渡す口が無いなら何もしない
		}
	}

	static Vector3 MakeSpinAngularVelocity(const Vector3& vel, float spinRadPerSec)
	{
		// vel がほぼ0なら回転なし
		if (vel.LengthSquared() < 1e-6f) return Vector3(0, 0, 0);

		Vector3 dir = vel;
		dir.Normalize();

		const Vector3 up(0, 1, 0);

		// 回転軸：up × dir（進行方向に対して“横回転”）
		Vector3 axis = up.Cross(dir);
		if (axis.LengthSquared() < 1e-6f)
		{
			// 真上/真下に投げた等で軸が作れない場合の保険
			axis = Vector3(1, 0, 0);
		}
		else
		{
			axis.Normalize();
		}

		return axis * spinRadPerSec; // 角速度（ラジアン/秒）
	}


	// ------------------------------------------------------------
	// Rock
	// ------------------------------------------------------------
	class RockThrowAction final : public IThrowAction
	{
	public:
		ThrowItemId Id() const override { return ThrowItemId::Rock; }

		const ThrowTuning& Tuning() const override
		{
			// Rock専用の調整値（必要ならここを変える）
			static ThrowTuning t{};
			t.holdNorm = 0.20f;
			t.releaseNorm = 0.55f;
			t.speed = 900.0f;
			t.lob = 120.0f;
			return t;
		}

		void Spawn(const ThrowSpawnArgs& a) override
		{
			// 生成
			auto* rock = a.om.Instantiate<Rock>("Rock" + std::to_string(m_Index), Tag::Object);
			if (!rock) { return; }

			// 初期位置
			rock->SetPosition(a.pos);

			// 初速（Rock側が対応していれば反映）
			TryApplyVelocity(*rock, a.vel);

			// 角速度（投げた瞬間に回す）
			constexpr float SPIN = 25.0f; // まず 15〜30 で調整（ラジアン/秒）
			const Vector3 angVel = MakeSpinAngularVelocity(a.vel, SPIN);
			TryApplyAngularVelocity(*rock, angVel);

			++m_Index;
		}
	};

	// ------------------------------------------------------------
	// Can / Grenade も同じ要領で追加していく
	// （まだ不要ならコメントアウトのままでOK）
	// ------------------------------------------------------------
#if 0
	class CanThrowAction final : public IThrowAction
	{
	public:
		ThrowItemId Id() const override { return ThrowItemId::Can; }
		const ThrowTuning& Tuning() const override
		{
			static ThrowTuning t{};
			// Can用に調整
			return t;
		}
		void Spawn(const ThrowSpawnArgs& a) override
		{
			auto* can = a.om.Instantiate<Can>("Can", Tag::Object);
			if (!can) return;
			can->SetPosition(a.pos);
			TryApplyVelocity(*can, a.vel);
		}
	};

	class GrenadeThrowAction final : public IThrowAction
	{
	public:
		ThrowItemId Id() const override { return ThrowItemId::Grenade; }
		const ThrowTuning& Tuning() const override
		{
			static ThrowTuning t{};
			// Grenade用に調整（lob強め、cooldown長め等）
			return t;
		}
		void Spawn(const ThrowSpawnArgs& a) override
		{
			auto* g = a.om.Instantiate<Grenade>("Grenade", Tag::Object);
			if (!g) return;
			g->SetPosition(a.pos);
			TryApplyVelocity(*g, a.vel);

			// 例：タイマー開始など
			// g->Arm(a.owner);
		}
	};
#endif
} // namespace

// ------------------------------------------------------------
// 外に公開するのは「生成関数」だけ
// ThrowComponent.cpp が forward 宣言して呼べる
// ------------------------------------------------------------
std::unique_ptr<IThrowAction> MakeRockThrowAction()
{
	return std::make_unique<RockThrowAction>();
}

// std::unique_ptr<IThrowAction> MakeCanThrowAction()
// {
//     return std::make_unique<CanThrowAction>();
// }
//
// std::unique_ptr<IThrowAction> MakeGrenadeThrowAction()
// {
//     return std::make_unique<GrenadeThrowAction>();
// }

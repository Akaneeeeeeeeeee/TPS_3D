#include "RigitBody2D.h"


/**
 * @brief 初期化
 * @param
*/
void RigidBody2D::Init(void)
{

}

/**
 * @brief 更新
 *
 * 物理計算の順番は、
 * １：力
 * ２：加速度
 * ３：速度
 * ４：位置
 * なのでその順番で計算する
 *
 * TODO:2025/0206 物理演算の仕組みを理解する
 *
*/
void RigidBody2D::Update(void)
{
	// 物理シミュレーションしない場合はスキップ
	if (IsKinematic)
	{ 
		// 明示的に速度をリセットしておく
		m_Velocity = Vector3(0.0f, 0.0f, 0.0f);
		return;
	}

	//---------------------------------------
	//			物体にかかる力の計算
	//---------------------------------------
	// force に 重力による力（F = m * g） を加算する
	// force は 次のフレームの加速度計算に使う

	// ここdeltatime使わないので毎フレーム色々減少させる必要あり→forceを毎フレームリセットする(速度はvelocityに保持されてるので問題なし)

	// 地球では重力は下向きにかかるのでy成分に加算する
	m_Force.y += m_Mass * m_GravityScale;

	// 加速度を求める
	// ニュートンの運動方程式 F = m * a を使って加速度 a を計算
	// a = F(力) / m(質量) なので、質量 m が大きいほど加速しにくい

	// 三項演算子					条件式			真の場合				偽の場合
	Vector3 acceleration = (m_Mass > 0) ? (m_Force / m_Mass) : Vector3(0.0f, 0.0f, 0.0f);;

	// 速度の更新
	// 速度 v に従ってオブジェクトを移動
	// p = p + v * dt（積分）→deltatimeは使わない(FPSが固定されているため)
	// 位置 p が更新されることで、オブジェクトが動く
	m_Velocity += acceleration;

	// 力のリセット(ここまでに呼んだaddforceのforceを一番最後のもので上書きしないためにここで一回だけリセットする)
	m_Force = Vector3(0.0f, 0.0f, 0.0f);;

}

/**
 * @brief 終了
 * @param
*/
void RigidBody2D::Uninit(void)
{
	// アタッチされているオブジェクトのポインタをnullptrに
	m_pOwner = nullptr;
}

/**
 * @brief 力を加える関数
 * @param _force 加える力
 * @param _mode 力の加え方(デフォルトはForce)
*/
void RigidBody2D::AddForce(Vector3 _force, ForceMode2D _mode = ForceMode2D::Force)
{
	// 力の加え方によって処理を変える
	switch (_mode)
	{
	// Forceの場合、じわじわ加速する動きになるため、毎フレーム呼び続ける必要があるから注意!
	case ForceMode2D::Force:
		m_Force += _force;		// 力を加算
		break;
	//case ForceMode2D::Acceleration:
	//	Vector3 acceleration = _force;  // 質量を考慮しない
	//	m_Velocity += acceleration;
		break;
	// 一フレームで爆発的に加速
	case ForceMode2D::Impulse:
		m_Velocity += _force / m_Mass;	// 速度に直接影響
		break;
	// 速度を直で変更
	case ForceMode2D::VelocityChange:
		m_Velocity = _force;
		break;
	default:
		break;
	}
}


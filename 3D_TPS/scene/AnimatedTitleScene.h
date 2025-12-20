#pragma once

#include "system/camera.h"
#include "system/CSprite.h"
#include "system/C3DShape.h"
#include "system/CShader.h"
#include "system/SceneClassFactory.h"
#include "system/Framework/Scene/IScene.h"

class TitlePlayerActor;
class Terrain;
class Enemy;
class StaticMeshCollider;
class obstacle;
class WeatherController;

namespace {
	static constexpr uint32_t ENEMYMAX = 10;
	static constexpr uint32_t OBSTACLEMAX = 10;
}



// AnimatedTitleScene.h か .cpp 内に置いてOK（まずは）
class TitleScript
{
public:
	void Setup(TitlePlayerActor* player, Enemy* enemy, obstacle* cover);
	void Tick(float dt);

	bool IsFinished() const { return m_Finished; }

private:
	enum class State { 
		EnterMove,
		HideIdle,
		LookAround,
		ThrowRock,
		ExitMove,
		ReEnter,
	};

	TitlePlayerActor* m_Player = nullptr;
	Enemy* m_Enemy = nullptr;
	obstacle* m_Cover = nullptr;

	State	m_State = State::EnterMove;
	float	m_Timer = 0.0f;
	bool	m_Finished = false;
	bool	m_Thrown = false;
	bool	m_Scanned = false;

	Vector3 m_EnterPos{};
	Vector3 m_HidePos{};
	Vector3 m_ExitPos{};
	Vector3 m_RockLandPos{};
	Vector3 m_ReEnterSpawnPos = Vector3(1000.0f, 100.0f, -3350.0f);


	void Enter(State next);

	static Vector3 DirXZ(const Vector3& from, const Vector3& to);

	bool ArrivedXZ(const Vector3& target, float r) const;

	void MoveTo(const Vector3& target, bool run, bool sideways);

	void StopIdle();

	void FaceTo(const Vector3& targetPos);

	void LookLeftRight(float dt);

	bool IsEnemyNear(float dist) const;

	void EmitRockSound(const Vector3& landPos);

	void StopAndCheckOverWall(void);
};






/**
 * @brief スケルタルメッシュを表示する
 */
class AnimatedTitleScene : public IScene {
public:
	/// @brief コピーコンストラクタは使用不可
	AnimatedTitleScene(const AnimatedTitleScene&) = delete;

	/// @brief 代入演算子も使用不可
	AnimatedTitleScene& operator=(const AnimatedTitleScene&) = delete;

	/**
	 * @brief コンストラクタ
	 *
	 * カメラや画像スプライト、遷移演出の初期化を行う。
	 */
	explicit AnimatedTitleScene();
	//explicit AnimatedTitleScene(ObjectManager& _Mgr);

	/**
	 * @brief 毎フレームの更新処理
	 * @param deltatime 前フレームからの経過時間（マイクロ秒）
	 *
	 * 入力処理、アニメーション、遷移タイミングなどの制御を行う。
	 */
	void Update(const float deltatime) override;

	/**
	 * @brief 毎フレームの描画処理
	 * @param deltatime 前フレームからの経過時間（マイクロ秒）
	 *
	 * タイトルロゴや背景などのスプライト描画を行う。
	 */
	void Draw(void) override;

	/**
	 * @brief シーンの初期化処理
	 *
	 * スプライトの生成、カメラ設定、音声再生など、表示に必要な準備を行う。
	 */
	void Init(ObjectManager* _Mgr) override;
	//void Init(void) override;

	/**
	 * @brief シーンの終了処理
	 *
	 * リソースの解放など、他のシーンへの遷移前に必要な処理を行う。
	 */
	void Uninit() override;

	/**
	 * @brief ワールド変換行列を調整
	 *
	 * ワールド変換行列を調整
	 */
	void debugSRT();


	/**
	 * @brief shape select
	 *
	 * 3D Model Select
	 */
	void debug3DModelSelect();

	/**
	 * @brief Directional Light
	*
		* Directional Light
	 */
	void debugDirectionalLight();

	/**
	 * @brief Free Camera
	 *
	 * Free Camera
	 */
	void debugFreeCamera();

	void ResetTitleLoop(void);
	void DebugTitleDirector(void);

private:
	// 台本
	enum class TitlePhase {
		Intro,			// シーン開始
		WeatherShift,	// 天候切り替え
		Lure,			// 石を投げて敵を誘導
		ExitReturn,		// プレイヤーが退場して戻ってくる
	};

	struct TitleConfig
	{
		float loopSec = 30.0f;     // デフォルト30, 変更可

		// ループ内の区切り（秒）
		float introEnd = 6.0f;
		float weatherEnd = 12.0f;
		float lureEnd = 18.0f;
		float end = 30.0f;         // loopSec と同じにしておく
	};

	// 台本状態
	TitleConfig m_cfg{};
	float m_Time = 0.0f;					// ループ内時刻（秒）
	TitlePhase m_Phase = TitlePhase::Intro;
	bool m_StoneThrown = false;				// 石を投げたか？


	// 入力フェード
	bool m_fadeOut = false;
	float m_fadeT = 0.0f;             // 0..1
	float m_fadeSec = 0.6f;           // フェード時間（秒）
	int m_nextSceneId = 0;            // 次シーン識別（あなたの仕組みに合わせる）

	/**
	 * @brief このシーンで使用するカメラ
	 */
	FreeCamera m_camera;

	/**
	 * @brief 描画対象の3Dオブジェクト
	 */
	std::unique_ptr<C3DShape> m_shape;

	// タイトル画像スプライト
	std::unique_ptr<CSprite> m_TitleImage;

	WeatherController* m_Weather = nullptr;

	// プレイヤー
	TitlePlayerActor* m_Player = nullptr;

	std::array<std::unique_ptr<Segment>, 3> m_segments;			// ワールド軸表示用線分

	// 地形
	Terrain* m_Terrain = nullptr;
	StaticMeshCollider* m_TerrainCol = nullptr;

	// ワールド変換行列
	Matrix4x4 m_mtxWorld{};
	
	/**
	* @brief 敵群
	*/
	Enemy* m_Enemy = nullptr;

	/**
	* @brief 障害物群
	*/
	std::array<obstacle*, OBSTACLEMAX>	m_Obstacles;	// 障害物

	bool m_WeatherChanged = false; // そのループで天候切替を実行したか
	TitleScript m_TitleScript;          // 台本制御
};

REGISTER_SCENE(AnimatedTitleScene)

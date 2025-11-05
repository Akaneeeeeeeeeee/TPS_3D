#pragma once
#include "scene/IScene.h"
#include "SceneClassFactory.h"
#include "camera.h"

class TestScene : public IScene {
public:
	/// @brief コピーコンストラクタは使用不可
	TestScene(const TestScene&) = delete;

	/// @brief 代入演算子も使用不可
	TestScene& operator=(const TestScene&) = delete;

	/**
	 * @brief コンストラクタ
	 *
	 * カメラや画像スプライト、遷移演出の初期化を行う。
	 */
	explicit TestScene();
	//explicit TestScene(ObjectManager& _Mgr);

	/**
	 * @brief 毎フレームの更新処理
	 * @param deltatime 前フレームからの経過時間（マイクロ秒）
	 *
	 * 入力処理、アニメーション、遷移タイミングなどの制御を行う。
	 */
	void Update(const uint64_t deltatime) override;

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

private:
	/**
	 * @brief このシーンで使用するカメラ
	 */
	FreeCamera m_camera;

};

REGISTER_CLASS(TestScene);
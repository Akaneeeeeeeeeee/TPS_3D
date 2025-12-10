#pragma once

#include <array>
#include <memory>

#include "system/camera.h"
#include "system/Framework/Scene/IScene.h"

#include "system/C3DShape.h"

#include "Framework/Component/Transform/Transform.h"
#include "system/SceneClassFactory.h"
#include "GameObject/Field.h"
#include "Framework/GameObject/Player/Player.h"
#include "Framework/GameObject/Enemy/Enemy.h"
#include "Framework/GameObject/Terrain/Terrain.h"
#include "GameObject/obstacle.h"

/**
 * @brief メッシュフィールドを表示する
 */
class CollisionTestScene : public IScene {
public:
	static constexpr uint32_t ENEMYMAX = 10;
	static constexpr uint32_t OBSTACLEMAX = 10;

	/// @brief コピーコンストラクタは使用不可
	CollisionTestScene(const CollisionTestScene&) = delete;

	/// @brief 代入演算子も使用不可
	CollisionTestScene& operator=(const CollisionTestScene&) = delete;

	/**
	 * @brief コンストラクタ
	 *
	 *
	 */
	explicit CollisionTestScene();

	/**
	 * @brief 毎フレームの更新処理
	 * @param deltatime 前フレームからの経過時間（マイクロ秒）
	 */
	void Update(const float deltatime) override;

	/**
	 * @brief 毎フレームの描画処理
	 * @param deltatime 前フレームからの経過時間（マイクロ秒）
	 *
	 */
	void Draw(void) override;

	/**
	 * @brief シーンの初期化処理
	 *
	 */
	void Init(ObjectManager* mgr) override;

	/**
	 * @brief シーンの終了処理
	 *
	 */
	void Uninit(void) override;

	/**
	 * @brief Free Camera
	 *
	 * Free Camera;
	 */
	void debugFreeCamera();

	/**
	 * @brief field remake
	 *
	 * field remake;
	 */
	void debugFieldRemake();

	/**
	 * @brief field undulation
	 *
	 * field remake;
	 */
	void debugFieldUnduration();

	/**
	 * @brief field height
	 *
	 * field height disp
	 */
	void debugFieldHeight();

	/**
	 * @brief get player
	 *
	 * get player object address
	 */
	Player* getplayer() {
		return m_player;
	}

private:

	/**
	* @brief このシーンで使用するカメラ
	*/
	FreeCamera m_camera;

	std::array<std::unique_ptr<Segment>, 3> m_segments;			// ワールド軸表示用線分
	std::array<std::unique_ptr<Segment>, 1> m_playersegment;	// ワールド軸表示用線分

	/**
	* @brief フィールド
	*/
	Field* m_field;

	/**
	* @brief 読み込んだ地形
	*/
	Terrain* m_terrain;

	/**
	* @brief プレイヤ
	*/
	Player* m_player;		// プレイヤ

	/**
	* @brief 敵群
	*/
	std::array<Enemy*, ENEMYMAX>		m_enemies;	// 敵

	/**
	* @brief 障害物群
	*/
	std::array<obstacle*, OBSTACLEMAX>	m_obstacles;	// 障害物

};

REGISTER_SCENE(CollisionTestScene)
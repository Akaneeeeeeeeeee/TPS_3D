#pragma once

#include "../system/camera.h"
#include "../system/IScene.h"
#include "../system/CSprite.h"
#include "../system/C3DShape.h"
#include "../system/CShader.h"
#include "../system/SceneClassFactory.h"

#include "../system/CAnimationMesh.h"
#include "../system/CAnimationData.h"
#include "../system/CAnimationObject.h"

/**
 * @brief スケルタルメッシュを表示する
 */
class SkeltalmeshScene : public IScene {
public:
	/// @brief コピーコンストラクタは使用不可
	SkeltalmeshScene(const SkeltalmeshScene&) = delete;

	/// @brief 代入演算子も使用不可
	SkeltalmeshScene& operator=(const SkeltalmeshScene&) = delete;

	/**
	 * @brief コンストラクタ
	 *
	 * カメラや画像スプライト、遷移演出の初期化を行う。
	 */
	explicit SkeltalmeshScene();

	/**
	 * @brief 毎フレームの更新処理
	 * @param deltatime 前フレームからの経過時間（マイクロ秒）
	 *
	 * 入力処理、アニメーション、遷移タイミングなどの制御を行う。
	 */
	void update(uint64_t deltatime) override;

	/**
	 * @brief 毎フレームの描画処理
	 * @param deltatime 前フレームからの経過時間（マイクロ秒）
	 *
	 * タイトルロゴや背景などのスプライト描画を行う。
	 */
	void draw(uint64_t deltatime) override;

	/**
	 * @brief シーンの初期化処理
	 *
	 * スプライトの生成、カメラ設定、音声再生など、表示に必要な準備を行う。
	 */
	void init() override;

	/**
	 * @brief シーンの終了処理
	 *
	 * リソースの解放など、他のシーンへの遷移前に必要な処理を行う。
	 */
	void dispose() override;

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

	/**
	 * @brief 描画対象の3Dオブジェクト
	 */
	std::unique_ptr<C3DShape> m_shape;

	/**
	 * @brief ワールド軸表示用
	 */
	std::array<std::unique_ptr<Segment>, 3> m_segments;			// ローカル軸表示用線分

	/**
	 * @brief SRT
	 */
	SRT m_srt{};

	/**
	 * @brief ワールド変換行列
	 */
	Matrix4x4 m_mtxWorld{};

	/**
	 * @brief 描画のための情報(ユニークポインタ用)
	 */
	std::unique_ptr<CAnimationMesh>			m_pmesh;		// メッシュデータ
	std::unique_ptr<CAnimationObject>		m_panimobject;	// アニメーションオブジェクト
	std::unique_ptr<CAnimationData>			m_panimdata;	// アニメーションデータ

	/**
	 * @brief 光の方向を表すための矢印メッシュ
	 */
	std::unique_ptr<CStaticMeshRenderer>	m_arrowmeshrenderer;
	std::unique_ptr<CStaticMesh>			m_arrowmesh;	// メッシュデータ

	// 描画の為の情報（見た目に関わる部分）
	CShader			m_shader;							// シェーダ
	CShader			m_arrowshader;						// シェーダ
};

REGISTER_CLASS(SkeltalmeshScene)
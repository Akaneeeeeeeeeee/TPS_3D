#pragma once

#include "system/camera.h"
#include "system/CSprite.h"
#include "system/C3DShape.h"
#include "system/CShader.h"
#include "system/SceneClassFactory.h"
#include "system/Framework/Scene/IScene.h"

/**
 * @brief スケルタルメッシュを表示する
 */
class ResultScene : public IScene {
public:
	/// @brief コピーコンストラクタは使用不可
	ResultScene(const ResultScene&) = delete;

	/// @brief 代入演算子も使用不可
	ResultScene& operator=(const ResultScene&) = delete;

	/**
	 * @brief コンストラクタ
	 *
	 * カメラや画像スプライト、遷移演出の初期化を行う。
	 */
	explicit ResultScene();
	//explicit ResultScene(ObjectManager& _Mgr);

	/**
	 * @brief 毎フレームの更新処理
	 * @param deltatime 前フレームからの経過時間（マイクロ秒）
	 *
	 * 入力処理、アニメーション、遷移タイミングなどの制御を行う。
	 */
	void Update(uint64_t deltatime) override;

	/**
	 * @brief 毎フレームの描画処理
	 * @param deltatime 前フレームからの経過時間（マイクロ秒）
	 *
	 * タイトルロゴや背景などのスプライト描画を行う。
	 */
	void Draw(uint64_t deltatime) override;

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

	// 結果に応じて画像を変更するために実装
	void SetTexture(std::unique_ptr<CSprite> sprite);


private:
	/**
	 * @brief このシーンで使用するカメラ
	 */
	FreeCamera m_camera;

	/**
	 * @brief 描画対象の3Dオブジェクト
	 */
	std::unique_ptr<C3DShape> m_shape;

	// タイトル画像スプライト
	std::unique_ptr<CSprite> m_ResultImage;

	/**
	 * @brief SRT
	 */
	SRT m_srt{};

	/**
	 * @brief ワールド変換行列
	 */
	Matrix4x4 m_mtxWorld{};

	// 描画の為の情報（見た目に関わる部分）
	CShader			m_shader;							// シェーダ
};

REGISTER_CLASS(ResultScene)
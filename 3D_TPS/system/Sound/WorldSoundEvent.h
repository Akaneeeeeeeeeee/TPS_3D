#pragma once
#include "system/commontypes.h"


// サウンドファイル
typedef enum SOUND_LABEL
{
	BGM_STAGESELECT,	// ステージ選択BGM
	BGM_GAMECLEAR,		// ゲームクリアシーンBGM
	BGM_INGAME,			// インゲームBGM
	BGM_GAMEOVER,		// ゲームオーバーBGM
	BGM_TITLE,			// タイトルシーンBGM
	SE_STONE,			// 石着地SE
	SE_CAN,				// 缶着地SE
	SE_HEAVYRAIN,		// 豪雨SE
	SE_LIGHTRAIN,		// 小雨SE
	SE_SANDSTORM,		// 砂嵐SE
	SE_THUNDER,			// 雷SE
	SE_WALKING_NORMAL,	// 歩行SE
	SE_RUNNING,			// 走行SE
	SE_WALKING_RAIN,	// 雨中歩行SE
	SE_LANDING,			// 着地SE
	SE_HEARTBEAT,		// 心音SE
	SE_GUNSHOT,			// 銃声SE
	SE_THROW,			// 投擲SE
	SE_COUNTDOWN,		// カウントダウンSE
	SE_CONFIRM,			// 決定SE
	SE_SWITCHCURSOR,	// カーソル移動SE
	SE_STARTSLOWMOTION,	// スローモーション開始SE
	SE_ENDSLOWMOTION,	// スローモーション終了SE

	SOUND_LABEL_MAX,
};

enum class SoundEmitterKind : uint8_t { Player, Enemy, PlayerItem, Other };

// 音の種類
enum SoundType
{
	None = -1,
    Footstep,       // 足音
    StoneImpact,    // 石が落ちた音
    Gunshot,        // 銃声
    Weather,        // 天候
    Custom,
};

/*
* @brief	ワールドサウンドイベント構造体
* @detail	ワールド空間上で発生する音のイベント情報を格納する構造体
*/
struct WorldSoundEvent
{
	Vector3 Position = Vector3::Zero;   // 音源位置
	float   Radius = 0.0f;				// 効果範囲
	float	Volume = 1.0f;				// 音量
	float	Loudness = 1.0f;			// 音の大きさ（敵の聴覚判定用）
	SoundType Type = SoundType::None;   // 音の種類

	SOUND_LABEL PlayLabel = SOUND_LABEL_MAX; // どのSE/BGMを鳴らすか
	SoundEmitterKind Emitter = SoundEmitterKind::Other;
};
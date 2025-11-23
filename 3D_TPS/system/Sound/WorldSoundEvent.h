#pragma once
#include "system/commontypes.h"

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
};
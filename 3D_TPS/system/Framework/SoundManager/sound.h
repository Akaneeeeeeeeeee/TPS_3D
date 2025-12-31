#pragma once

#include <xaudio2.h>

// サウンドファイル
typedef enum
{
	BGM_STAGESELECT,	// ステージ選択BGM
	BGM_GAMECLEAR,		// ゲームクリアシーンBGM
	BGM_INGAME,			// インゲームBGM
	BGM_GAMEOVER,		// ゲームオーバーBGM
	BGM_TITLE,			// タイトルシーンBGM
	SE_BIRIBIRI,		// ビリビリSE
	SE_CLICK,			// クリックSE
	SE_DOON,			// ドォンSE
	SE_GETMAGAZINE,		// マガジン取得SE
	SE_PATAPATA,		// パタパタSE
	SE_SHOT,			// 擬音発射SE
	SE_POYON,			// ポヨンSE
	SE_SUCTION,			// 吸い込みSE



	SOUND_LABEL_MAX,
} SOUND_LABEL;

// パラメータ構造体
typedef struct
{
	LPCSTR filename;	// 音声ファイルまでのパスを設定
	bool bLoop;			// trueでループ。通常BGMはture、SEはfalse。
	//bool SandW;			//! 音の強弱の切り替えが必要なサウンドを判定
	//float volume;		//! 音の大きさ（1.0fがノーマル)
} PARAM;		//PARAM型構造体


class Sound {

private:

	PARAM m_param[SOUND_LABEL_MAX] =
	{
		{"Game/Asset/Sound/BGM/StageSelectSceneBGM.wav", true},	// ステージ選択BGM（ループさせるのでtrue設定）
		{"Game/Asset/Sound/BGM/ClearSceneBGM.wav", true},		// ゲームクリアシーンBGM
		{"Game/Asset/Sound/BGM/GameBGM.wav", true},  			// インゲームBGM
		{"Game/Asset/Sound/BGM/GameOverSceneBGM.wav", true},	// ゲームオーバーシーンBGM
		{"Game/Asset/Sound/BGM/TitleSceneBGM.wav", true},		// タイトルシーンBGM
		{"Game/Asset/Sound/SE/BiriBiri.wav", false},			// 「ビリビリ」SE（ループしないのでfalse設定）
		{"Game/Asset/Sound/SE/Click.wav", false},				// クリックSE（ループしないのでfalse設定）
		{"Game/Asset/Sound/SE/Doon.wav", false},				// 「ビリビリ」SE（ループしないのでfalse設定）
		{"Game/Asset/Sound/SE/Magazine.wav", false},			// マガジン取得SE（ループしないのでfalse設定）
		{"Game/Asset/Sound/SE/PataPata.wav", false},			// 「パタパタ」SE（ループしないのでfalse設定）
		{"Game/Asset/Sound/SE/Shot.wav", false},				// 擬音発射SE（ループしないのでfalse設定）		
		{"Game/Asset/Sound/SE/Slime_Poyon.wav", false},			// 「ポヨン」SE（ループしないのでfalse設定）
		{"Game/Asset/Sound/SE/Suction.wav", false},				// 吸い込みSE（ループしないのでfalse設定）


	};

	IXAudio2* m_pXAudio2 = NULL;
	IXAudio2MasteringVoice* m_pMasteringVoice = NULL;
	IXAudio2SourceVoice* m_pSourceVoice[SOUND_LABEL_MAX];
	WAVEFORMATEXTENSIBLE m_wfx[SOUND_LABEL_MAX]; // WAVフォーマット
	XAUDIO2_BUFFER m_buffer[SOUND_LABEL_MAX];
	BYTE* m_DataBuffer[SOUND_LABEL_MAX];

	HRESULT FindChunk(HANDLE, DWORD, DWORD&, DWORD&);
	HRESULT ReadChunkData(HANDLE, void*, DWORD, DWORD);

	int Distance = 0;	//プレイヤーとオブジェクトの距離をもらう変数



public:
	// ゲームループ開始前に呼び出すサウンドの初期化処理
	HRESULT Init(void);

	// ゲームループ終了後に呼び出すサウンドの解放処理
	void Uninit(void);

	// 引数で指定したサウンドを再生する
	void Play(SOUND_LABEL label);		//enumで決めたデータで引数として渡している

	// 引数で指定したサウンドを停止する
	void Stop(SOUND_LABEL label);

	//! 引数で指定したサウンドの音量を変更する	//後から追加（出口）
	void ChangeVolume(SOUND_LABEL label,int );

	// 引数で指定したサウンドの再生を再開する
	void Resume(SOUND_LABEL label);

	//volume変数のセッター・ゲッター関数（音量調整のため）
	int SetDistance(int);
	int GetDistance(void);


	static Sound& GetInstance(void);
};

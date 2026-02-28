#include "SoundSystem.h"

#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif
#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

//=============================================================================
//// 初期化
////=============================================================================
//HRESULT SoundSystem::Init()
//{
//	HRESULT hr;
//
//	HANDLE hFile;
//	DWORD  dwChunkSize;
//	DWORD  dwChunkPosition;
//	DWORD  filetype;
//
//	// COMの初期化
//	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
//	if (FAILED(hr)) {
//		CoUninitialize();
//		return -1;
//	}
//
//	/**** Create XAudio2 ****/
//	hr = XAudio2Create(&m_pXAudio2, 0);		// 第二引数は､動作フラグ デバッグモードの指定(現在は未使用なので0にする)
//	//hr=XAudio2Create(&g_pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);		// 第三引数は、windowsでは無視
//	if (FAILED(hr)) {
//		CoUninitialize();
//		return -1;
//	}
//
//	/**** Create Mastering Voice ****/
//	hr = m_pXAudio2->CreateMasteringVoice(&m_pMasteringVoice);			// 今回はＰＣのデフォルト設定に任せている
//	/*, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL*/		// 本当６個の引数を持っている
//	if (FAILED(hr)) {
//		if (m_pXAudio2)	m_pXAudio2->Release();
//		CoUninitialize();
//		return -1;
//	}
//
//	/**** Initalize SoundSystem ****/
//	for (int i = 0; i < SOUND_LABEL_MAX; i++)
//	{
//		memset(&m_wfx[i], 0, sizeof(WAVEFORMATEXTENSIBLE));
//		memset(&m_buffer[i], 0, sizeof(XAUDIO2_BUFFER));
//
//		hFile = CreateFileA(m_param[i].filename, GENERIC_READ, FILE_SHARE_READ, NULL,
//			OPEN_EXISTING, 0, NULL);
//		if (hFile == INVALID_HANDLE_VALUE) {
//			return HRESULT_FROM_WIN32(GetLastError());
//		}
//		if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
//			return HRESULT_FROM_WIN32(GetLastError());
//		}
//
//		//check the file type, should be fourccWAVE or 'XWMA'
//		FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
//		ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
//		if (filetype != fourccWAVE)		return S_FALSE;
//
//		FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
//		ReadChunkData(hFile, &m_wfx[i], dwChunkSize, dwChunkPosition);
//
//		//fill out the audio data buffer with the contents of the fourccDATA chunk
//		FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
//		m_DataBuffer[i] = new BYTE[dwChunkSize];
//		ReadChunkData(hFile, m_DataBuffer[i], dwChunkSize, dwChunkPosition);
//
//		CloseHandle(hFile);
//
//		// 	サブミットボイスで利用するサブミットバッファの設定
//		m_buffer[i].AudioBytes = dwChunkSize;
//		m_buffer[i].pAudioData = m_DataBuffer[i];
//		m_buffer[i].Flags = XAUDIO2_END_OF_STREAM;
//		if (m_param[i].bLoop)
//			m_buffer[i].LoopCount = XAUDIO2_LOOP_INFINITE;
//		else
//			m_buffer[i].LoopCount = 0;
//
//		m_pXAudio2->CreateSourceVoice(&m_pSourceVoice[i], &(m_wfx[i].Format));
//	}
//
//	return hr;
//}
//
////=============================================================================
//// 開放処理
////=============================================================================
//void SoundSystem::Uninit(void)
//{
//	for (int i = 0; i < SOUND_LABEL_MAX; i++)
//	{
//		if (m_pSourceVoice[i])
//		{
//			m_pSourceVoice[i]->Stop(0);
//			m_pSourceVoice[i]->FlushSourceBuffers();
//			m_pSourceVoice[i]->DestroyVoice();			// オーディオグラフからソースボイスを削除
//			delete[]  m_DataBuffer[i];
//		}
//	}
//
//	m_pMasteringVoice->DestroyVoice();
//
//	if (m_pXAudio2) m_pXAudio2->Release();
//
//	// COMの破棄
//	//CoUninitialize();
//}
//
////=============================================================================
//// 再生
////=============================================================================
//void SoundSystem::Play(SOUND_LABEL label)
//{
//	IXAudio2SourceVoice*& pSV = m_pSourceVoice[(int)label];
//
//	if (pSV != nullptr)
//	{
//		pSV->DestroyVoice();
//		pSV = nullptr;
//	}
//
//	// ソースボイス作成
//	m_pXAudio2->CreateSourceVoice(&pSV, &(m_wfx[(int)label].Format));
//	pSV->SubmitSourceBuffer(&(m_buffer[(int)label]));	// ボイスキューに新しいオーディオバッファーを追加
//
//	// 再生
//	pSV->Start(0);
//
//}
//
////=============================================================================
//// 停止
////=============================================================================
//void SoundSystem::Stop(SOUND_LABEL label)
//{
//	if (m_pSourceVoice[(int)label] == NULL) return;
//
//	XAUDIO2_VOICE_STATE xa2state;
//	m_pSourceVoice[(int)label]->GetState(&xa2state);
//	if (xa2state.BuffersQueued)
//	{
//		m_pSourceVoice[(int)label]->Stop(0);
//	}
//}
//
////=============================================================================
//// 一時停止
////=============================================================================
//void SoundSystem::Resume(SOUND_LABEL label)
//{
//	IXAudio2SourceVoice*& pSV = m_pSourceVoice[(int)label];
//	pSV->Start();
//}
//
//
//
////=============================================================================
//// ユーティリティ関数群
////=============================================================================
//HRESULT SoundSystem::FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
//{
//	HRESULT hr = S_OK;
//	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
//		return HRESULT_FROM_WIN32(GetLastError());
//	DWORD dwChunkType;
//	DWORD dwChunkDataSize;
//	DWORD dwRIFFDataSize = 0;
//	DWORD dwFileType;
//	DWORD bytesRead = 0;
//	DWORD dwOffset = 0;
//	while (hr == S_OK)
//	{
//		DWORD dwRead;
//		if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
//			hr = HRESULT_FROM_WIN32(GetLastError());
//		if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
//			hr = HRESULT_FROM_WIN32(GetLastError());
//		switch (dwChunkType)
//		{
//		case fourccRIFF:
//			dwRIFFDataSize = dwChunkDataSize;
//			dwChunkDataSize = 4;
//			if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
//				hr = HRESULT_FROM_WIN32(GetLastError());
//			break;
//		default:
//			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
//				return HRESULT_FROM_WIN32(GetLastError());
//		}
//		dwOffset += sizeof(DWORD) * 2;
//		if (dwChunkType == fourcc)
//		{
//			dwChunkSize = dwChunkDataSize;
//			dwChunkDataPosition = dwOffset;
//			return S_OK;
//		}
//		dwOffset += dwChunkDataSize;
//		if (bytesRead >= dwRIFFDataSize) return S_FALSE;
//	}
//	return S_OK;
//}
//
//HRESULT SoundSystem::ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
//{
//	HRESULT hr = S_OK;
//	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
//		return HRESULT_FROM_WIN32(GetLastError());
//	DWORD dwRead;
//	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
//		hr = HRESULT_FROM_WIN32(GetLastError());
//	return hr;
//}
//
//SoundSystem& SoundSystem::GetInstance(void)
//{
//	static SoundSystem Instance;
//	return Instance;
//}

HRESULT SoundSystem::Init()
{
    if (m_XA) return S_OK;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr)) m_ComInited = true;
    else if (hr == RPC_E_CHANGED_MODE) m_ComInited = false;
    else return hr;

    hr = XAudio2Create(&m_XA, 0);
    if (FAILED(hr)) return hr;

    hr = m_XA->CreateMasteringVoice(&m_Master);
    if (FAILED(hr)) return hr;

    // 実ファイル名
    //m_Param[BGM_STAGESELECT] = { "assets/Sound/BGM/StageSelectSceneBGM.wav", true };
    //m_Param[BGM_GAMECLEAR] = { "assets/Sound/BGM/ClearSceneBGM.wav", true };
    //m_Param[BGM_INGAME] = { "assets/Sound/BGM/GameBGM.wav", true };
    //m_Param[BGM_GAMEOVER] = { "assets/Sound/BGM/GameOverSceneBGM.wav", true };
    //m_Param[BGM_TITLE] = { "assets/Sound/BGM/TitleSceneBGM.wav", true };

    m_Param[SE_THROW] = { "assets/Sound/SE_Throw.wav", false };
    m_Param[SE_STONE] = { "assets/Sound/SE_Stone.wav", false };
    //m_Param[SE_CAN] = { "assets/Sound/SE/Can.wav", false };

	// 環境音
    m_Param[SE_LIGHTRAIN] = { "assets/Sound/LightRain.wav", true };
    m_Param[SE_HEAVYRAIN] = { "assets/Sound/HeavyRain.wav", true };
    m_Param[SE_SANDSTORM] = { "assets/Sound/SandStorm.wav", true };
    //m_Param[SE_THUNDER] = { "assets/Sound/SE/Thunder.wav", false };

    m_Param[SE_WALKING_NORMAL] = { "assets/Sound/SE_Walk.wav", false };
    m_Param[SE_RUNNING] = { "assets/Sound/SE_Run.wav", false };
    //m_Param[SE_WALKING_RAIN] = { "assets/Sound/SE/FootstepRain.wav", false };

    m_Param[SE_COUNTDOWN] = { "assets/Sound/SE_CountDown.wav", true };
    m_Param[SE_CONFIRM] = { "assets/Sound/SE_Confirm.wav", false };
    m_Param[SE_HEARTBEAT] = { "assets/Sound/SE_HeartBeat.wav", false };
    m_Param[SE_GUNSHOT] = { "assets/Sound/SE_GunShot.wav", false };
    m_Param[SE_SWITCHCURSOR] = { "assets/Sound/SE_SwitchCursor.wav", false };
    m_Param[SE_STARTSLOWMOTION] = { "assets/Sound/SE_StartSlowMotion.wav", false };
    m_Param[SE_ENDSLOWMOTION] = { "assets/Sound/SE_EndSlowMotion.wav", false };
	m_Param[SE_GATESOUND] = { "assets/Sound/SE_GateSound.wav", true };  // ゲートの動作音はループさせる

    for (int i = 0; i < (int)SOUND_LABEL_MAX; ++i)
    {
        std::memset(&m_Wfx[i], 0, sizeof(WAVEFORMATEXTENSIBLE));
        std::memset(&m_BaseBuf[i], 0, sizeof(XAUDIO2_BUFFER));
        m_LoopVoice[i] = nullptr;

        const auto& p = m_Param[i];
        if (!p.filename) continue;

        HANDLE hFile = CreateFileA(p.filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return HRESULT_FROM_WIN32(GetLastError());

        DWORD chunkSize = 0, chunkPos = 0, filetype = 0;

        FindChunk(hFile, fourccRIFF, chunkSize, chunkPos);
        ReadChunkData(hFile, &filetype, sizeof(DWORD), chunkPos);
        if (filetype != fourccWAVE) { CloseHandle(hFile); return E_FAIL; }

        FindChunk(hFile, fourccFMT, chunkSize, chunkPos);
        ReadChunkData(hFile, &m_Wfx[i], chunkSize, chunkPos);

        FindChunk(hFile, fourccDATA, chunkSize, chunkPos);
        m_Data[i] = std::make_unique<BYTE[]>(chunkSize);
        ReadChunkData(hFile, m_Data[i].get(), chunkSize, chunkPos);

        CloseHandle(hFile);

        XAUDIO2_BUFFER buf{};
        buf.AudioBytes = chunkSize;
        buf.pAudioData = m_Data[i].get();
        buf.Flags = XAUDIO2_END_OF_STREAM;
        buf.LoopCount = p.loop ? XAUDIO2_LOOP_INFINITE : 0;
        m_BaseBuf[i] = buf;
    }

    return S_OK;
}

void SoundSystem::Uninit()
{
    for (auto& a : m_OneShots)
    {
        if (a.voice)
        {
            a.voice->Stop(0);
            a.voice->FlushSourceBuffers();
            a.voice->DestroyVoice();
            a.voice = nullptr;
        }
    }
    m_OneShots.clear();

    for (auto& v : m_LoopVoice)
    {
        if (v)
        {
            v->Stop(0);
            v->FlushSourceBuffers();
            v->DestroyVoice();
            v = nullptr;
        }
    }

    if (m_Master) { m_Master->DestroyVoice(); m_Master = nullptr; }
    if (m_XA) { m_XA->Release(); m_XA = nullptr; }

    if (m_ComInited) { CoUninitialize(); m_ComInited = false; }
}

void SoundSystem::Update()
{
    for (auto it = m_OneShots.begin(); it != m_OneShots.end(); )
    {
        if (it->cb && it->cb->finished)
        {
            if (it->voice)
            {
                it->voice->Stop(0);
                it->voice->FlushSourceBuffers();
                it->voice->DestroyVoice();
                it->voice = nullptr;
            }
            it = m_OneShots.erase(it);
        }
        else ++it;
    }
}

void SoundSystem::PlayLoop(SOUND_LABEL label, float volume01)
{
    if (!m_XA || !IsValid(label)) return;
    const int idx = (int)label;
    if (!m_Param[idx].loop) return;

    if (!m_LoopVoice[idx])
    {
        if (FAILED(m_XA->CreateSourceVoice(&m_LoopVoice[idx], &(m_Wfx[idx].Format))))
        {
            m_LoopVoice[idx] = nullptr;
            return;
        }
    }

    auto* v = m_LoopVoice[idx];
    v->Stop(0);
    v->FlushSourceBuffers();

    XAUDIO2_BUFFER buf = m_BaseBuf[idx];
    buf.LoopCount = XAUDIO2_LOOP_INFINITE;

    v->SubmitSourceBuffer(&buf);
    v->SetVolume(std::max(0.0f, volume01));
    v->Start(0);
}

void SoundSystem::StopLoop(SOUND_LABEL label)
{
    if (!IsValid(label)) return;
    auto* v = m_LoopVoice[(int)label];
    if (!v) return;
    v->Stop(0);
    v->FlushSourceBuffers();
}

void SoundSystem::SetLoopVolume(SOUND_LABEL label, float volume01)
{
    if (!IsValid(label)) return;
    auto* v = m_LoopVoice[(int)label];
    if (!v) return;
    v->SetVolume(std::max(0.0f, volume01));
}

void SoundSystem::PlayOneShot(SOUND_LABEL label, float volume01)
{
    if (!m_XA || !IsValid(label)) return;
    const int idx = (int)label;
    if (m_Param[idx].loop) return; // ループ音はOneShotでは鳴らさない

    auto cb = std::make_unique<OneShotCallback>();

    IXAudio2SourceVoice* voice = nullptr;
    if (FAILED(m_XA->CreateSourceVoice(&voice, &(m_Wfx[idx].Format), 0, XAUDIO2_DEFAULT_FREQ_RATIO, cb.get())))
        return;

    XAUDIO2_BUFFER buf = m_BaseBuf[idx];
    buf.LoopCount = 0;

    voice->SubmitSourceBuffer(&buf);
    voice->SetVolume(std::max(0.0f, volume01));
    voice->Start(0);

    m_OneShots.push_back({ voice, std::move(cb) });
}

HRESULT SoundSystem::FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwChunkType = 0;
    DWORD dwChunkDataSize = 0;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType = 0;
    DWORD dwOffset = 0;

    while (true)
    {
        DWORD dwRead = 0;
        if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
            return HRESULT_FROM_WIN32(GetLastError());
        if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
            return HRESULT_FROM_WIN32(GetLastError());

        if (dwChunkType == fourccRIFF)
        {
            dwRIFFDataSize = dwChunkDataSize;
            dwChunkDataSize = 4;
            if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
                return HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
                return HRESULT_FROM_WIN32(GetLastError());
        }

        dwOffset += sizeof(DWORD) * 2;

        if (dwChunkType == fourcc)
        {
            dwChunkSize = dwChunkDataSize;
            dwChunkDataPosition = dwOffset;
            return S_OK;
        }

        dwOffset += dwChunkDataSize;
        if (dwOffset >= dwRIFFDataSize) return S_FALSE;
    }
}

HRESULT SoundSystem::ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
{
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwRead = 0;
    if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
}
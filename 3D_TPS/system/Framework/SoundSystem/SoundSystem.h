#pragma once

#include <windows.h>
#include <xaudio2.h>
#include <mmreg.h>

#include <array>
#include <vector>
#include <memory>
#include <algorithm>

#include "system/Sound/WorldSoundEvent.h"

// WAV読み込み用
struct SoundParam
{
    LPCSTR filename = nullptr;
    bool   loop = false;
};

class SoundSystem
{
public:
    HRESULT Init();
    void Uninit();

    // OneShot回収（OneShotを使うなら毎フレーム呼ぶ）
    void Update();

    // ループ（天候音など）
    void PlayLoop(SOUND_LABEL label, float volume01);
    void StopLoop(SOUND_LABEL label);
    void SetLoopVolume(SOUND_LABEL label, float volume01);

    // OneShot（足音/着地音など）
    void PlayOneShot(SOUND_LABEL label, float volume01);

private:
    HRESULT FindChunk(HANDLE, DWORD, DWORD&, DWORD&);
    HRESULT ReadChunkData(HANDLE, void*, DWORD, DWORD);

    bool IsValid(SOUND_LABEL l) const { return 0 <= (int)l && (int)l < (int)SOUND_LABEL_MAX; }

private:
    IXAudio2* m_XA = nullptr;
    IXAudio2MasteringVoice* m_Master = nullptr;

    std::array<SoundParam, SOUND_LABEL_MAX> m_Param{};
    std::array<WAVEFORMATEXTENSIBLE, SOUND_LABEL_MAX> m_Wfx{};
    std::array<XAUDIO2_BUFFER, SOUND_LABEL_MAX> m_BaseBuf{};
    std::array<std::unique_ptr<BYTE[]>, SOUND_LABEL_MAX> m_Data{};

    // ループはラベルごとに1本
    std::array<IXAudio2SourceVoice*, SOUND_LABEL_MAX> m_LoopVoice{};

    struct OneShotCallback final : public IXAudio2VoiceCallback
    {
        bool finished = false;
        void STDMETHODCALLTYPE OnBufferEnd(void*) override { finished = true; }
        void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32) override {}
        void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override {}
        void STDMETHODCALLTYPE OnStreamEnd() override {}
        void STDMETHODCALLTYPE OnBufferStart(void*) override {}
        void STDMETHODCALLTYPE OnLoopEnd(void*) override {}
        void STDMETHODCALLTYPE OnVoiceError(void*, HRESULT) override {}
    };

    struct ActiveOneShot
    {
        IXAudio2SourceVoice* voice = nullptr;
        std::unique_ptr<OneShotCallback> cb;
    };

    std::vector<ActiveOneShot> m_OneShots;
    bool m_ComInited = false;
};
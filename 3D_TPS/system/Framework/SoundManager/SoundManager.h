#pragma once
#include "system/Framework/NonCopyable/Singleton_Template.h"
#include "system/Sound/WorldSoundEvent.h"
#include <vector>

class SoundManager : public Singleton<SoundManager>
{
public:
	friend class Singleton<SoundManager>;

	static SoundManager& Get()
	{
		static SoundManager instance;
		return instance;
	}

	// フレーム頭で呼ぶ。今フレの音一覧をクリア
	void BeginFrame()
	{
		m_Events.clear();
	}

	// 音イベントの追加
	void EmitSound(const WorldSoundEvent& ev)
	{
		m_Events.push_back(ev);

		// ここで実際のサウンド再生に繋いでもよい（必要なら）
		// AudioSystem::Get().Play3DSound(ev.type, ev.position, ev.loudness);
	}

	// 今フレーム発生した音の一覧
	const std::vector<WorldSoundEvent>& GetEvents() const
	{
		return m_Events;
	}

private:
	std::vector<WorldSoundEvent> m_Events;
};
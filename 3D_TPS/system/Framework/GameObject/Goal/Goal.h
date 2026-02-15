#pragma once
#include "Framework/GameObject/GameObject.h"
#include "Framework/Component/DayNightObserver/DayNightObserver.h"

class CStaticMesh;
class CStaticMeshRenderer;
class CShader;
class StaticMeshRendererComponent;

/*
* @brief	ゴールクラス
* @detail	ゴールオブジェクトを表すクラス
* @auther	赤根　和樹
* @date		2025/12/10
*/
class Goal : public GameObject, public IDayNightListener
{
public:
	Goal(ComponentFactory* factory,
		const uint64_t id,
		const std::string& name = "",
		const Tag tag = Tag::Goal,
		const Transform& transform = Transform::One())
		: GameObject(factory, id, name, tag, transform),
		m_meshrenderer(nullptr) {
	}
	~Goal() = default;

	void Awake(void) override;
	void Update(const float delta) override;
	void Draw(void) const override;
	void Uninit(void) override;
	bool IsReached(void) const { return m_Reached; }

	void OnCollisionCharacterEnter(GameObject& other) override;

	void OnDayNightChanged(bool isNight) override;

	void DebugImGui(void);

private:
	CStaticMesh* m_mesh{};
	CStaticMeshRenderer* m_meshrenderer{};
	CShader* m_shader{};

	StaticMeshRendererComponent* m_RenderComp = nullptr;

	GameObject* m_BeamA = nullptr;
	GameObject* m_BeamB = nullptr;
	SpotLightComponent* m_SpotA = nullptr;
	SpotLightComponent* m_SpotB = nullptr;
	DayNightObserverComponent* m_DayNight = nullptr;

	float m_Yaw = 0.0f;
	float m_YawSpeed = 1.2f; // rad/s（約69度/秒）
	float m_BeamHeight = 750.0f; // 灯台の上の高さ

	bool m_Reached = false;

	bool m_IsNight = false;
	bool m_NightOnly = true;     // 夜だけ点灯にするなら true 固定でもOK
	bool m_LightEnabled = true;  // 予備（デバッグで切りたい時用）
	bool m_RuntimeLit = false;   // 前回状態（無駄なSetを減らす）

	void RefreshLighting(void);
};


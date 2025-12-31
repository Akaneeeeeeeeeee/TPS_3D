#pragma once
#include "system/Framework/ObjectManager/ObjectManager.h"
#include "system/Framework/SceneManager/SceneManager.h"
#include "system/Framework/EngineSystem/EngineSystem.h"
#include "system/Framework/Graphics/GraphicsDevice.h"
#include "system/Framework/EngineSystem/GameFeatureSystems.h"

/**
 * @brief ゲームクラス
 * アプリケーションクラスが保持するゲームクラス
 * →このクラスを切り替えれば他のゲームでもこの外側のクラスは使いまわせる
*/
class Game
{
public:
    Game() = default;
    ~Game() = default;

    void Init();
    void Update(float dt);
    void Draw();
    void Uninit();

private:
    EngineSystems m_Engine;
	GameFeatureSystems m_GameFeatures;

    ComponentFactory  m_ComponentFactory;
    GameObjectFactory m_ObjectFactory;
    ObjectManager     m_ObjectManager;
    SceneManager      m_SceneManager;
};
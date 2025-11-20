#pragma once
#include "Framework/GameObject/GameObject.h"
#include "Framework/Factory/ComponentFactory.h"

/*
* @brief    ゲームオブジェクトの生成を担当するファクトリ
* @detail   テンプレートを用いて任意のゲームオブジェクトを生成できるようにする
* @auther   赤根 和樹
* @date     2025/11/20
*/
class GameObjectFactory
{
public:
    explicit GameObjectFactory()
        : m_pComponentFactory(nullptr)
    {};

	~GameObjectFactory() = default;

    void Init(ComponentFactory* factory)
    {
        m_pComponentFactory = factory;
	}

    template<typename T, typename... Args>
        requires std::derived_from<T, GameObject>
    std::unique_ptr<T> Create(const uint64_t id, const std::string& name, Tag tag, Args&&... args)
    {
        // ここでだけ new する
        return std::make_unique<T>(m_pComponentFactory, id, name, tag, std::forward<Args>(args)...);
    }

private:
    ComponentFactory* m_pComponentFactory = nullptr;
};

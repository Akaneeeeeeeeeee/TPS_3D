#pragma once
#include "Framework/EngineSystem/EngineSystem.h"
#include "Framework/Component/IComponent/IComponent.h"

/*
* @brief     コンポーネントファクトリークラス
* @detail    コンポーネントの生成を担当するファクトリークラス
* @remark    テンプレートを用いて任意のコンポーネントを生成できるようにする
* @auther    赤根 和樹
* @date      2025/11/20
*/
class ComponentFactory
{
public:
    explicit ComponentFactory()
        : m_pServices(nullptr) {};

    ~ComponentFactory() = default;

    void Init(EngineServices* service)
    {
        m_pServices = service;
    }

    template<typename T, typename... Args>
        requires std::derived_from<T, IComponent>
    std::unique_ptr<T> Create(GameObject& owner, Args&&... args)
    {
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);

        comp->SetOwner(&owner);
        comp->Attach(*m_pServices);   // ここでコンテキストを渡す

        return comp;
    }

private:
    EngineServices* m_pServices = nullptr;
};

#pragma once
#include "Framework/EngineContext/EngineContext.h"
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
        : m_pContext(nullptr) {};

    ~ComponentFactory() = default;

    void Init(EngineContext* context) 
    {
        m_pContext = context; 
    }

    template<typename T, typename... Args>
        requires std::derived_from<T, IComponent>
    std::unique_ptr<T> Create(GameObject& owner, Args&&... args)
    {
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);

        comp->SetOwner(&owner);
        comp->Attach(*m_pContext);   // ★ ここでコンテキストを渡す

        return comp;
    }

private:
    EngineContext* m_pContext = nullptr;
};

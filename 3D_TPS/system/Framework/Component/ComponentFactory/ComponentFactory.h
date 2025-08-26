#pragma once
#include "system/noncopyable.h"
#include "system/Framework/Graphics/RenderManager.h"
#include "system/Framework/ShaderManager/ShaderManager.h"
#include "system/Framework/Component/IComponent/IComponent.h"

class ComponentFactory : public NonCopyable
{
public:
	ComponentFactory();
	~ComponentFactory();

	void Init(RenderManager* renderManager, ShaderManager* shaderManager);
	void Init(ShaderManager* shaderManager);

    template<typename T>
    std::unique_ptr<T> CreateComponent(void)
    {
        // 継承チェック
        static_assert(std::is_base_of<IComponent, T>::value, "T must inherit from IComponent");

        // コンポーネントが IRenderer を継承している場合、RenderManager に登録
        if constexpr (std::is_base_of<IRenderer, T>::value)
		{
			// コンポーネントのインスタンスを生成し、ポインタを取得
			std::unique_ptr<T> component = std::make_unique<T>(m_pRenderManager);
            //m_pRenderManager->RegisterRenderComponent(component.get());
            RenderManager::GetInstance().RegisterRenderComponent(component.get());
			return std::move(component);
        }

		// IRenderer を継承していない場合は通常のコンポーネントとして生成
		std::unique_ptr<T> component = std::make_unique<T>();
		return std::move(component);
	}

private:
	ComponentFactory(const ComponentFactory&) = delete;
	ComponentFactory& operator=(const ComponentFactory&) = delete;

	RenderManager* m_pRenderManager;
	ShaderManager* m_pShaderManager;
};

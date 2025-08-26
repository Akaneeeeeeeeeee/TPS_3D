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
        // �p���`�F�b�N
        static_assert(std::is_base_of<IComponent, T>::value, "T must inherit from IComponent");

        // �R���|�[�l���g�� IRenderer ���p�����Ă���ꍇ�ARenderManager �ɓo�^
        if constexpr (std::is_base_of<IRenderer, T>::value)
		{
			// �R���|�[�l���g�̃C���X�^���X�𐶐����A�|�C���^���擾
			std::unique_ptr<T> component = std::make_unique<T>(m_pRenderManager);
            //m_pRenderManager->RegisterRenderComponent(component.get());
            RenderManager::GetInstance().RegisterRenderComponent(component.get());
			return std::move(component);
        }

		// IRenderer ���p�����Ă��Ȃ��ꍇ�͒ʏ�̃R���|�[�l���g�Ƃ��Đ���
		std::unique_ptr<T> component = std::make_unique<T>();
		return std::move(component);
	}

private:
	ComponentFactory(const ComponentFactory&) = delete;
	ComponentFactory& operator=(const ComponentFactory&) = delete;

	RenderManager* m_pRenderManager;
	ShaderManager* m_pShaderManager;
};

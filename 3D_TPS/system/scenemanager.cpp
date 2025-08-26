#include	"scenemanager.h"
#include	"SceneClassFactory.h"

// �o�^����Ă���V�[����S�Ĕj������
void SceneManager::Dispose() 
{
	// �o�^����Ă��邷�ׂăV�[���̏I������
	for (auto& s : m_scenes) 
	{
		s.second->dispose();
	}

	m_scenes.clear();
	m_currentSceneName.clear();
}

void SceneManager::SetCurrentScene(std::string currentscenename) 
{
	m_currentSceneName = currentscenename;
	auto obj = SceneClassFactory::getInstance().create(currentscenename);
	obj->init();
	m_scenes[m_currentSceneName] = std::move(obj);
}

void SceneManager::Init()
{
}

void SceneManager::Draw(uint64_t deltatime)
{

	// ���݂̃V�[����`��
	m_scenes[m_currentSceneName]->draw(deltatime);
}

void SceneManager::Update(uint64_t deltatime)
{
	// ���݂̃V�[�����X�V
	m_scenes[m_currentSceneName]->update(deltatime);
}
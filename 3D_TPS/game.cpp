#include	"system/renderer.h"
#include    "system/DebugUI.h"
#include    "system/CDirectInput.h"
#include	"system/scenemanager.h"
#include	"fpscontrol.h"

void gameinit() 
{
	// �����_���̏�����    
	Renderer::Init();

	// DirectInput�̏�����
	CDirectInput::GetInstance().Init(Application::GetHInstance(), 
		Application::GetWindow(),
		Application::GetWidth(),
		Application::GetHeight());

	// �f�o�b�OUI�̏�����
	DebugUI::Init(Renderer::GetDevice(), Renderer::GetDeviceContext());

	// �V�[���}�l�[�W���̏�����
	SceneManager::Init();

	//�@�V�[���I��
	SceneManager::SetCurrentScene("SkeltalmeshScene");

}

void gameupdate(uint64_t deltatime)
{
	CDirectInput::GetInstance().GetKeyBuffer();		// �L�[�{�[�h�̏�Ԃ��擾
	CDirectInput::GetInstance().GetMouseState();	// �}�E�X�̏�Ԃ��擾

	// �V�[���}�l�[�W���̍X�V
	SceneManager::Update(deltatime);

}

void gamedraw(uint64_t deltatime) 
{
	// �����_�����O�O����
	Renderer::Begin();

	// �V�[���}�l�[�W���̕`��
	SceneManager::Draw(deltatime);

	// �f�o�b�OUI�̕`��
	DebugUI::Render();

	// �����_�����O�㏈��
	Renderer::End();
}

void gamedispose() 
{
	// �f�o�b�OUI�̏I������
	DebugUI::DisposeUI();

	// �V�[���}�l�[�W���̏I������
	SceneManager::Dispose();

	// �����_���̏I������
	Renderer::Uninit();

}

void gameloop()
{
	uint64_t delta_time = 0;

	// �t���[���̑҂����Ԃ��v�Z����
	static FPS fpsrate(65);

	// �O����s����Ă���̌o�ߎ��Ԃ��v�Z����
	delta_time = fpsrate.CalcDelta();

	//std::cout << delta_time << std::endl;

	// �X�V�����A�`�揈�����Ăяo��
	gameupdate(delta_time);
	gamedraw(delta_time);

	// �K�莞�Ԃ܂�WAIT
	fpsrate.Tick();

}
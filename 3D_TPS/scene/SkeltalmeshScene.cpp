#include <string>
#include <array>

#include "../main.h"
#include "../system/CDirectInput.h"
#include "../system/scenemanager.h"
#include "../system/DebugUI.h"
#include "../system/utility.h"
#include "../system/AimOrientation.h"

#include "SkeltalmeshScene.h"

struct Load3DInfo{
	std::string filename;
	std::string texdirectoryname;
	Load3DInfo(std::string p1, std::string p2) {
		filename = p1;
		texdirectoryname = p2;
	}
};

std::array<Load3DInfo,1> g_loadmodel = 
{
		Load3DInfo(
			"assets/model/akai/akai.fbx",			// ���f����
			"assets/model/akai/")					// �e�N�X�`���̃p�X
};

// ���s�����̕����Z�b�g
void SkeltalmeshScene::debugDirectionalLight()
{
	static Vector4 direction = Vector4(0.0f, 0.0f, 1.0f, 0.0f); // Z��+�����Ɍ��𓖂Ă�	

	ImGui::Begin("debug Directional Light");

	ImGui::SliderFloat3("direction ", &direction.x, -1, 1);
	direction.Normalize();										// ���K��

	LIGHT light{};
	light.Enable = true;
	light.Direction = direction;

	light.Direction.Normalize();
	light.Ambient = Color(0.2f, 0.2f, 0.2f, 1.0f);
	light.Diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);

	Vector4 Direction = Vector4(direction.x, direction.y, direction.z, 0.0f);
	Renderer::SetLight(light);

	ImGui::End();
}

// �f�o�b�O�t���[�J����
void SkeltalmeshScene::debugFreeCamera()
{
	ImGui::Begin("debug Free camera");

	static float radius = 100.0f;
	static Vector3 pos = Vector3(0, 0, radius);
	static Vector3 lookat = Vector3(0, 0, 0);
	static float elevation = -90.0f * PI / 180.0f;
	static float azimuth = PI / 2.0f;

	static Vector3 spherecenter = Vector3(0, 0, 0);

	ImGui::SliderFloat("Radius", &radius, 1, 800);
	ImGui::SliderFloat("Elevation", &elevation, -PI, PI);
	ImGui::SliderFloat("Azimuth", &azimuth, -PI, PI);

	ImGui::SliderFloat3("lookat ", &lookat.x, -100, 100);

	// �J�����̈ʒu���ɍ��W����f�J���g���W�ɕϊ�
	m_camera.SetRadius(radius);
	m_camera.SetElevation(elevation);
	m_camera.SetAzimuth(azimuth);
	m_camera.SetLookat(lookat);

	// �J�����̈ʒu���ɍ��W���狁�߂�
	m_camera.CalcCameraPosition();

	ImGui::End();
}

// �f�o�b�OModel select
void SkeltalmeshScene::debug3DModelSelect()
{
	ImGui::Begin("debug Shape Select");

	// �I�𒆂̃C���f�b�N�X
	static int current_item = 0;
	static int old_item = -1;

	// �A�C�e���̃��X�g
	const char* items[] = {
		g_loadmodel[0].filename.c_str(),
//		g_loadmodel[1].filename.c_str(),
//		g_loadmodel[2].filename.c_str(),
//		g_loadmodel[3].filename.c_str(),
//		g_loadmodel[4].filename.c_str(),
//		g_loadmodel[5].filename.c_str(),
//		g_loadmodel[6].filename.c_str(),
//		g_loadmodel[7].filename.c_str(),
//		g_loadmodel[8].filename.c_str(),
	};

	ImGui::Text("\n\n");
	ImGui::Separator();

	ImGui::Text("%s",g_loadmodel[current_item].filename.c_str());
	ImGui::Text("\n\n");

	ImGui::Separator();
	// ���_���@�O�p�`���@�T�u�Z�b�g���@�}�e���A����
	ImGui::Text("vertex num : %d", m_pmesh->GetVertices().size());
	ImGui::Text("triangle num : %d", m_pmesh->GetIndices().size()/3);
	ImGui::Text("subset num : %d", m_pmesh->GetSubsets().size());
	ImGui::Text("material num : %d", m_pmesh->GetMaterials().size());

	ImGui::End();
}

// �f�o�b�OSRT
void SkeltalmeshScene::debugSRT()
{
	ImGui::Begin("debug SRT");

	static Vector3 scale = Vector3(1, 1, 1);
	static Vector3 rotate = Vector3(0, 0, 0);
	static Vector3 trans = Vector3(0, 0, 0);

	ImGui::SliderFloat3("scale", &scale.x, 0.1f, 20.0f);
	ImGui::SliderFloat3("rotate", &rotate.x, -PI, PI);
	ImGui::SliderFloat3("trans", &trans.x, -100, 100);

	Matrix4x4 mtxscale = Matrix4x4::CreateScale(scale);

	Matrix4x4 mtxrotx = Matrix4x4::CreateRotationX(rotate.x);
	Matrix4x4 mtxroty = Matrix4x4::CreateRotationY(rotate.y);
	Matrix4x4 mtxrotz = Matrix4x4::CreateRotationZ(rotate.z);

	Matrix4x4 mtxtrans = Matrix4x4::CreateTranslation(trans);

	// �`�掞�Ɏg�p����s��ɂ܂Ƃ߂�
	m_mtxWorld = mtxscale * mtxrotx * mtxroty * mtxrotz * mtxtrans;

	static int selected = 0;		// 0;SOLID 1:WIREFRAME

	ImGui::RadioButton("Solid", &selected, 0);
	ImGui::RadioButton("WireFrame", &selected, 1);

	if (selected == 0) {
		Renderer::SetFillMode(D3D11_FILL_SOLID);
	}
	else {
		Renderer::SetFillMode(D3D11_FILL_WIREFRAME);
	}

	ImGui::End();
}

/**
 * @brief �R���X�g���N�^
 */
SkeltalmeshScene::SkeltalmeshScene()
{
}

/**
 * @brief �N���A�V�[���̍X�V����
 *
 * @param deltatime �O�t���[������̌o�ߎ��ԁi�~���b�j
 */
void SkeltalmeshScene::update(uint64_t deltatime)
{
	m_panimobject->Update(1.0f);
}

/**
 * @brief �`�揈��
 *
 * @param deltatime �O�t���[������̌o�ߎ��ԁi�~���b�j
 */
void SkeltalmeshScene::draw(uint64_t deltatime)
{
	m_camera.Draw();

	// 3���J���[
	Color axiscol[3] = {
		Color(1, 0, 0, 1),
		Color(0, 1, 0, 1),
		Color(0, 1, 1, 1)
	};

	// ���[���h����`��
	for (int axisno = 0; axisno < 3; axisno++)
	{
		Matrix4x4 rotmtx = Matrix4x4::Identity;
		m_segments[axisno]->Draw(rotmtx, axiscol[axisno]);
	}

	// ���[�J������`��
	for (int axisno = 0; axisno < 3; axisno++)
	{
		m_segments[axisno]->Draw(m_mtxWorld, axiscol[axisno]);
	}

	// ���s�����̕�������������`�� 
	LIGHT l = Renderer::GetLight();
	Vector3 dir = Vector3(-l.Direction.x, -l.Direction.y, -l.Direction.z);

	// �}�`�̕`��iANIMMESH�`��j
	Renderer::SetWorldMatrix(&m_mtxWorld);
	m_shader.SetGPU();
	m_panimobject->Draw();

	// �ڕW�����̎p�������
	AimOrientation aimorien(dir);
	aimorien.VisualizeDirection(
		Vector3(0, 0, 0),20,1,Color(1,1,0,1),2,Color(1,0,0,1)
	);
}

/**
 * @brief �V�[���̏���������
 */
void SkeltalmeshScene::init()
{
	// �J����(3D)�̏�����
	m_camera.Init();

	// ���[�J�����\���p�����̏�����
	m_segments[0] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(100, 0, 0));
	m_segments[1] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 100, 0));
	m_segments[2] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 0, 100));

	// ���b�V���𐶐�
	m_pmesh = std::make_unique<CAnimationMesh>();
	m_pmesh->Load(g_loadmodel[0].filename, g_loadmodel[0].texdirectoryname);

	// �A�j���[�V�����I�u�W�F�N�g�𐶐�
	m_panimobject = std::make_unique<CAnimationObject>();
	m_panimobject->Init();	

	// �A�j���[�V�������b�V�����Z�b�g
	m_panimobject->SetAnimationMesh(m_pmesh.get());

	// �A�j���[�V�����f�[�^�ǂݍ���
	m_panimdata = std::make_unique<CAnimationData>();
	m_panimdata->LoadAnimation("assets/model/akai/Akai_Run.fbx","Run");

	// ���݂̃A�j���[�V�������Z�b�g
	aiAnimation* animation = m_panimdata->GetAnimation("Run", 0);
	m_pmesh->SetCurentAnimation(animation);

	// �V�F�[�_�[�̏�����
	m_shader.Create("shader/vertexLightingOneSkinVS.hlsl", "shader/vertexLightingPS.hlsl");

	// ���b�V���𐶐�
	m_arrowmesh = std::make_unique<CStaticMesh>();
	m_arrowmesh->Load(g_loadmodel[0].filename, g_loadmodel[0].texdirectoryname);
	m_arrowmeshrenderer = std::make_unique<CStaticMeshRenderer>();
	m_arrowmeshrenderer->Init(*m_arrowmesh);

	// �V�F�[�_�[�̏�����
	m_arrowshader.Create(
		"shader/unlitTextureVS.hlsl",			// ���_�V�F�[�_�[
		"shader/unlitTexturePS.hlsl");			// �s�N�Z���V�F�[�_�[

	// �f�o�b�OSRT
	DebugUI::RedistDebugFunction([this]() {
		debugSRT();
		});


	// �f�o�b�O Directional light
	DebugUI::RedistDebugFunction([this]() {
		debugDirectionalLight();
		});

	// �f�o�b�O Free Camera
	DebugUI::RedistDebugFunction([this]() {
		debugFreeCamera();
		});

}

/**
 * @brief �V�[���̏I������
 */
void SkeltalmeshScene::dispose()
{
}

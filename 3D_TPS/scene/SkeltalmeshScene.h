#pragma once

#include "../system/camera.h"
#include "../system/IScene.h"
#include "../system/CSprite.h"
#include "../system/C3DShape.h"
#include "../system/CShader.h"
#include "../system/SceneClassFactory.h"

#include "../system/CAnimationMesh.h"
#include "../system/CAnimationData.h"
#include "../system/CAnimationObject.h"

/**
 * @brief �X�P���^�����b�V����\������
 */
class SkeltalmeshScene : public IScene {
public:
	/// @brief �R�s�[�R���X�g���N�^�͎g�p�s��
	SkeltalmeshScene(const SkeltalmeshScene&) = delete;

	/// @brief ������Z�q���g�p�s��
	SkeltalmeshScene& operator=(const SkeltalmeshScene&) = delete;

	/**
	 * @brief �R���X�g���N�^
	 *
	 * �J������摜�X�v���C�g�A�J�ډ��o�̏��������s���B
	 */
	explicit SkeltalmeshScene();

	/**
	 * @brief ���t���[���̍X�V����
	 * @param deltatime �O�t���[������̌o�ߎ��ԁi�}�C�N���b�j
	 *
	 * ���͏����A�A�j���[�V�����A�J�ڃ^�C�~���O�Ȃǂ̐�����s���B
	 */
	void update(uint64_t deltatime) override;

	/**
	 * @brief ���t���[���̕`�揈��
	 * @param deltatime �O�t���[������̌o�ߎ��ԁi�}�C�N���b�j
	 *
	 * �^�C�g�����S��w�i�Ȃǂ̃X�v���C�g�`����s���B
	 */
	void draw(uint64_t deltatime) override;

	/**
	 * @brief �V�[���̏���������
	 *
	 * �X�v���C�g�̐����A�J�����ݒ�A�����Đ��ȂǁA�\���ɕK�v�ȏ������s���B
	 */
	void init() override;

	/**
	 * @brief �V�[���̏I������
	 *
	 * ���\�[�X�̉���ȂǁA���̃V�[���ւ̑J�ڑO�ɕK�v�ȏ������s���B
	 */
	void dispose() override;

	/**
	 * @brief ���[���h�ϊ��s��𒲐�
	 *
	 * ���[���h�ϊ��s��𒲐�
	 */
	void debugSRT();


	/**
	 * @brief shape select
	 *
	 * 3D Model Select
	 */
	void debug3DModelSelect();

	/**
	 * @brief Directional Light
	*
		* Directional Light
	 */
	void debugDirectionalLight();

	/**
	 * @brief Free Camera
	 *
	 * Free Camera
	 */
	void debugFreeCamera();

private:
	/**
	 * @brief ���̃V�[���Ŏg�p����J����
	 */
	FreeCamera m_camera;

	/**
	 * @brief �`��Ώۂ�3D�I�u�W�F�N�g
	 */
	std::unique_ptr<C3DShape> m_shape;

	/**
	 * @brief ���[���h���\���p
	 */
	std::array<std::unique_ptr<Segment>, 3> m_segments;			// ���[�J�����\���p����

	/**
	 * @brief SRT
	 */
	SRT m_srt{};

	/**
	 * @brief ���[���h�ϊ��s��
	 */
	Matrix4x4 m_mtxWorld{};

	/**
	 * @brief �`��̂��߂̏��(���j�[�N�|�C���^�p)
	 */
	std::unique_ptr<CAnimationMesh>			m_pmesh;		// ���b�V���f�[�^
	std::unique_ptr<CAnimationObject>		m_panimobject;	// �A�j���[�V�����I�u�W�F�N�g
	std::unique_ptr<CAnimationData>			m_panimdata;	// �A�j���[�V�����f�[�^

	/**
	 * @brief ���̕�����\�����߂̖�󃁃b�V��
	 */
	std::unique_ptr<CStaticMeshRenderer>	m_arrowmeshrenderer;
	std::unique_ptr<CStaticMesh>			m_arrowmesh;	// ���b�V���f�[�^

	// �`��ׂ̈̏��i�����ڂɊւ�镔���j
	CShader			m_shader;							// �V�F�[�_
	CShader			m_arrowshader;						// �V�F�[�_
};

REGISTER_CLASS(SkeltalmeshScene)
#pragma once
#include    "CommonTypes.h"
#include	<string>
#include	<assimp/scene.h>

namespace utility
{
	std::string wide_to_multi_winapi(std::wstring const& src);
	std::wstring utf8_to_wide_winapi(std::string const& src);
	std::string utf8_to_multi_winapi(std::string const& src);

	Matrix4x4 aiMtxToDxMtx(aiMatrix4x4 asimpmtx);

	Matrix4x4 CaliculateBillBoardMtx(DirectX::SimpleMath::Matrix mtxView);

	// �^�[�Q�b�g�̕����������N�I�[�^�j�I�����쐬����֐�(��Œ�Ȃ�)
	Quaternion CreateTargetQuaternion(
		const Vector3 start,
		const Vector3 end);

	// �^�[�Q�b�g�̕����������N�I�[�^�j�I�����쐬����֐�(��Œ肠��)
	Quaternion CreateLookatQuaternion(
		Vector3 start,
		Vector3 end,
		Vector3 up,
		Matrix4x4& mtx);
}
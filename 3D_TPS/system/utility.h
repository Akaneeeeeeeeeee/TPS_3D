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

	// ターゲットの方向を向くクオータニオンを作成する関数(上固定なし)
	Quaternion CreateTargetQuaternion(
		const Vector3 start,
		const Vector3 end);

	// ターゲットの方向を向くクオータニオンを作成する関数(上固定あり)
	Quaternion CreateLookatQuaternion(
		Vector3 start,
		Vector3 end,
		Vector3 up,
		Matrix4x4& mtx);
}
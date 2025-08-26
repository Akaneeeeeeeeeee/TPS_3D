#include    "CommonTypes.h"
#include	<filesystem>
#include	<string>
#include	<Windows.h>
#include	<assimp/scene.h>

namespace utility {
    // std::string �p�̃f�B���N�g���擾�֐�
    std::filesystem::path get_directory(const std::string& path) {
        return std::filesystem::path(path).parent_path();
    }

    // std::u8string �p�̃f�B���N�g���擾�֐�
    std::filesystem::path get_directory(const std::u8string& path) {
        return std::filesystem::path(path).parent_path();
    }

    // std::wstring �p�̃f�B���N�g���擾�֐�
    std::filesystem::path get_directory(const std::wstring& path) {
        return std::filesystem::path(path).parent_path();
    }

	// ���C�h����(utf16)�����|��������
	std::string wide_to_multi_winapi(std::wstring const& src)
	{
		auto const dest_size = ::WideCharToMultiByte(
			CP_ACP,
			0U,
			src.data(),
			-1,
			nullptr,
			0,
			nullptr,
			nullptr);
		std::vector<char> dest(dest_size, '\0');
		if (::WideCharToMultiByte(
			CP_ACP,
			0U,
			src.data(),
			-1,
			dest.data(),
			static_cast<int>(dest.size()),
			nullptr,
			nullptr) == 0) {
			throw std::system_error{ static_cast<int>(::GetLastError()), std::system_category() };
		}
		dest.resize(std::char_traits<char>::length(dest.data()));
		dest.shrink_to_fit();
		return std::string(dest.begin(), dest.end());
	}

	// utf-8�����C�h����(utf-16)��
	std::wstring utf8_to_wide_winapi(std::string const& src)
	{
		auto const dest_size = ::MultiByteToWideChar(
			CP_UTF8,			 // �\�[�X����UTF-8
			0U,
			src.data(),
			-1,
			nullptr,
			0U);
		std::vector<wchar_t> dest(dest_size, L'\0');
		if (::MultiByteToWideChar(CP_UTF8, 0U, src.data(), -1, dest.data(), static_cast<int>(dest.size())) == 0) {
			throw std::system_error{ static_cast<int>(::GetLastError()), std::system_category() };
		}
		dest.resize(std::char_traits<wchar_t>::length(dest.data()));
		dest.shrink_to_fit();
		return std::wstring(dest.begin(), dest.end());
	}

	// utf8��S-JIS��
	std::string utf8_to_multi_winapi(std::string const& src)
	{
		auto const wide = utf8_to_wide_winapi(src);
		return wide_to_multi_winapi(wide);
	}


	DirectX::SimpleMath::Matrix aiMtxToDxMtx(aiMatrix4x4 asimpmtx) 
	{
		DirectX::SimpleMath::Matrix dxmtx;

		dxmtx._11 = asimpmtx.a1; dxmtx._12 = asimpmtx.b1; dxmtx._13 = asimpmtx.c1; dxmtx._14 = asimpmtx.d1;
		dxmtx._21 = asimpmtx.a2; dxmtx._22 = asimpmtx.b2; dxmtx._23 = asimpmtx.c2; dxmtx._24 = asimpmtx.d2;
		dxmtx._31 = asimpmtx.a3; dxmtx._32 = asimpmtx.b3; dxmtx._33 = asimpmtx.c3; dxmtx._34 = asimpmtx.d3;
		dxmtx._41 = asimpmtx.a4; dxmtx._42 = asimpmtx.b4; dxmtx._43 = asimpmtx.c4; dxmtx._44 = asimpmtx.d4;
		return dxmtx;
	}

	DirectX::SimpleMath::Matrix CaliculateBillBoardMtx(DirectX::SimpleMath::Matrix mtxView)
	{
		// ���[���h�}�g���b�N�X�̏�����
		DirectX::SimpleMath::Matrix mtxWorld = DirectX::SimpleMath::Matrix::Identity;

		// �r���{�[�h�p�s��쐬
		mtxWorld._11 = mtxView._11;
		mtxWorld._12 = mtxView._21;
		mtxWorld._13 = mtxView._31;
		mtxWorld._21 = mtxView._12;
		mtxWorld._22 = mtxView._22;
		mtxWorld._23 = mtxView._32;
		mtxWorld._31 = mtxView._13;
		mtxWorld._32 = mtxView._23;
		mtxWorld._33 = mtxView._33;

		return mtxWorld;
	}

	// �^�[�Q�b�g�̕����������N�I�[�^�j�I�����쐬����֐�(��Œ�Ȃ�)
	Quaternion CreateTargetQuaternion(
		const Vector3 start,
		const Vector3 end)
	{

		Vector3 normalizedStart = start;
		Vector3 normalizedEnd = end;

		normalizedStart.Normalize();
		normalizedEnd.Normalize();

		Vector3 cross = normalizedStart.Cross(normalizedEnd);
		float dotFloat = normalizedStart.Dot(normalizedEnd);

		if (dotFloat < -0.999999f) {
			// �x�N�g�����قڔ��Ε����̏ꍇ
			// �C�ӂ̒����x�N�g�������Ƃ���180�x��]
			Vector3 orthogonal = normalizedStart.Cross(DirectX::SimpleMath::Vector3(1.0f, 0.0f, 0.0f));

			if (orthogonal.LengthSquared() < 0.1f) {
				orthogonal = Vector3(0.0f, 1.0f, 0.0f);
			}
			orthogonal.Normalize();
			return Quaternion::CreateFromAxisAngle(orthogonal, DirectX::XM_PI);
		}

		if (dotFloat > 0.999999f) {
			// �x�N�g�����قړ��������̏ꍇ
			// �C�ӂ̒����x�N�g�������Ƃ���0�x��]
			Vector3 orthogonal = normalizedStart.Cross(DirectX::SimpleMath::Vector3(1.0f, 0.0f, 0.0f));

			if (orthogonal.LengthSquared() < 0.1f) {
				orthogonal = DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
			}
			orthogonal.Normalize();
			return Quaternion::CreateFromAxisAngle(orthogonal, 0.0f);
		}

		float angle = acosf(dotFloat);
		cross.Normalize();
		return Quaternion::CreateFromAxisAngle(cross, angle);
	}

	// �^�[�Q�b�g�̕����������N�I�[�^�j�I�����쐬����֐�(��Œ肠��)
	Quaternion CreateLookatQuaternion(
		const Vector3 start,
		const Vector3 end,
		const Vector3 up,
		Matrix4x4& mtx)
	{

		Vector3 axisx{};
		Vector3 axisy{};
		Vector3 axisz=(end-start);

		// �O�ς�X�������߂�
		axisx = up.Cross(axisz);

		// �O�ς�Y�������߂�
		axisy = axisz.Cross(axisx);

		// ���K��
		axisx.Normalize();
		axisy.Normalize();
		axisz.Normalize();

		mtx = Matrix4x4::Identity;

		mtx._11 = axisx.x;
		mtx._12 = axisx.y;
		mtx._13 = axisx.z;

		mtx._21 = axisy.x;
		mtx._22 = axisy.y;
		mtx._23 = axisy.z;

		mtx._31 = axisz.x;
		mtx._32 = axisz.y;
		mtx._33 = axisz.z;

		return Quaternion::CreateFromRotationMatrix(mtx);
	}
}


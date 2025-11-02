#include "MyMaterial.h"
#include "system/Framework/ShaderManager/ShaderManager.h"


MyMaterial::MyMaterial(const std::string& name, const std::array<IShader*, 2> shaders)
	: m_Name(name), m_pShaders(shaders), m_BaseColor(1.0f, 1.0f, 1.0f, 1.0f)
{
	MATERIAL mtrl;
	// マテリアル生成
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = m_BaseColor;
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;
	mtrl.TextureEnable = FALSE;
}
MyMaterial::MyMaterial(const std::string& name)
	: m_Name(name), m_pShaders(nullptr), m_BaseColor(1.0f, 1.0f, 1.0f, 1.0f)
{
	MATERIAL mtrl;
	// マテリアル生成
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = m_BaseColor;
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;
	mtrl.TextureEnable = FALSE;
	m_pShaders[0] = ShaderManager::GetInstance().GetShader("unlitTextureVS");
	m_pShaders[1] = ShaderManager::GetInstance().GetShader("unlitTexturePS");
}

MyMaterial::~MyMaterial()
{
}

void MyMaterial::SetTexture(const std::string& name, CTexture* texture)
{
	m_Textures[name] = texture;
	// シェーダーにテクスチャをセット
	for (auto shader : m_pShaders)
	{
		if (shader)
		{
			shader->SetTexture(0, texture); // スロット0にセット（必要に応じてスロットを変更）
			//// 例えば、"albedo"という名前のテクスチャをセットする場合
			//if (name.find("albedo"))
			//{
			//	shader->SetTexture(0, texture); // スロット0にセット
			//}
			//else if (name == "normal")
			//{
			//	shader->SetTexture(1, texture); // スロット1にセット
			//}
			// 他のテクスチャも同様に追加可能
		}
	}
}

void MyMaterial::WriteCBuffer(const UINT slot, const void* pData) const
{
	// 各シェーダーの定数バッファにデータを書き込む
	for (auto shader : m_pShaders)
	{
		if (shader)
		{
			shader->WriteCBuffer(slot, pData);
		}
	}
}

void MyMaterial::Bind(void) const
{
	// シェーダーのバインド
	for (auto shader : m_pShaders)
	{
		if (shader)
		{
			shader->Bind();
		}
	}
}

void MyMaterial::Unbind(void) const
{
	// シェーダーのアンバインド
	for (auto shader : m_pShaders)
	{
		if (shader)
		{
			shader->Unbind();
		}
	}
}
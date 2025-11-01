#pragma once
#include "system/Framework/Application/Entry/main.h"
#include "system/Framework/Shader/IShader/IShader.h"
#include <array>
#include "system/commontypes.h"
#include "system/CTexture.h"
#include "system/renderer.h"

/*
* @brief	Materialクラス
* @detail	マテリアルを扱うクラス
* @remark	シェーダー、テクスチャ、色などを保持する
* @auther	赤根和樹
* @date		2025/10/16
*/
class MyMaterial
{
public:
	MyMaterial(const std::string& name, const std::array<IShader*, 2> shaders);
	MyMaterial(const std::string& name);
	~MyMaterial();

	void WriteCBuffer(const UINT slot, const void* pData) const;

	void SetTexture(const std::string& name, CTexture* texture);

	void SetColor(const Color& color) { m_BaseColor = color; }

	void Bind(void) const;
	void Unbind(void) const;
private:
	std::string m_Name;					// マテリアルの名前
	std::array<IShader*, 2> m_pShaders; // 頂点シェーダー、ピクセルシェーダー、ジオメトリシェーダーなど
	std::unordered_map<std::string, CTexture*> m_Textures; // テクスチャのマップ
	Color m_BaseColor;					// ベースカラー
};

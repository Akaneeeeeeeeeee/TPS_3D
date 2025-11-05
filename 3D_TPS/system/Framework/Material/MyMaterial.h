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
	MyMaterial(const std::string& vsName, const std::string& psName, const std::string& name);
	MyMaterial(const std::string& name);
	~MyMaterial();

	//void SetShader(IShader* shader);

	void SetColor(const Color& color) { m_BaseColor = color; }

	void SetMaterial(const MATERIAL& material) { m_MaterialData = material; }

	// 描画時に RenderInfo に書き込む
	void FillRenderInfo(RenderInfo& info) const;

	// テクスチャも保持
	void SetTexture(std::string name, CTexture* tex) { m_Textures[name] = tex->GetResource(); }
	//void SetTexture(UINT slot, CTexture* tex) { m_Textures[slot] = tex; }
private:

	struct CBufferEntry {
		UINT slot;
		UINT size;
		ComPtr<ID3D11Buffer> buffer;
		std::vector<uint8_t> cpuData;
	};

	std::string m_Name;					// マテリアルの名前
	std::unordered_map<std::string, ID3D11ShaderResourceView*> m_Textures; // テクスチャのマップ
	//std::unordered_map<std::string, CTexture*> m_Textures; // テクスチャのマップ
	std::array<IShader*, 2> m_pShaders;	// シェーダー
	Color m_BaseColor;					// ベースカラー

	std::unordered_map<std::string, CBufferEntry> m_ConstantBuffers;

	MATERIAL m_MaterialData;			// マテリアルデータ

	void CreateCBuffers(const ShaderReflection& vsRef, const ShaderReflection& psRef);

	//std::unordered_map<UINT, ID3D11ShaderResourceView*> m_Srvs; // slot->SRV
};

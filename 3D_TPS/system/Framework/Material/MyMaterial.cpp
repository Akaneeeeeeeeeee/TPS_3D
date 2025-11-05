#include "MyMaterial.h"
#include "system/Framework/ShaderManager/ShaderManager.h"

namespace {
	constexpr UINT MATERIAL_SLOT = 3; // マテリアル用定数バッファのスロット(固定)
}

MyMaterial::MyMaterial(const std::string& vsName, const std::string& psName, const std::string& name)
	: m_Name(name)
{
	m_pShaders[ShaderStage::Vertex] = ShaderManager::GetInstance().GetShader(vsName);
	m_pShaders[ShaderStage::Pixel] = ShaderManager::GetInstance().GetShader(psName);

	const auto vsReflection = m_pShaders[ShaderStage::Vertex]->GetShaderReflection();
	const auto psReflection = m_pShaders[ShaderStage::Pixel]->GetShaderReflection();
	CreateCBuffers(vsReflection, psReflection);

	// なければ生成
	if (!m_ConstantBuffers.contains("MATERIAL"))
	{
		CBufferEntry entry;
		entry.slot = MATERIAL_SLOT;
		entry.size = sizeof(MATERIAL);
		entry.cpuData.resize(entry.size);

		// 初期値コピー
		memcpy(entry.cpuData.data(), &m_MaterialData, sizeof(MATERIAL));

		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = entry.size;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = 0;
		Renderer::GetDevice()->CreateBuffer(&desc, nullptr, &entry.buffer);

		m_ConstantBuffers["MATERIAL"] = entry;
	}
}

MyMaterial::~MyMaterial()
{
}




void MyMaterial::FillRenderInfo(RenderInfo& info) const
{
	// VS / PS セット
	info.vs = m_pShaders[ShaderStage::Vertex];
	info.ps = m_pShaders[ShaderStage::Pixel];

	// CBuffer（名前 → GPU バッファ）
	for (auto& [name, cb] : m_ConstantBuffers)
	{
		info.cBuffers.emplace_back(
			CBufferBinding{ cb.slot, name, cb.buffer.Get(), cb.cpuData.data(), cb.size }
		);
	}

	// SRV
	for (auto& [name, srv] : m_Textures)
	{
		for (auto& ref : m_pShaders[ShaderStage::Pixel]->GetShaderReflection().srvs)
		{
			if (ref.name == name)
			{
				info.srvs.emplace_back(
					SRVBinding{ ref.slot, name, srv }
				);
			}
		}
	}
}


void MyMaterial::CreateCBuffers(const ShaderReflection& vsRef, const ShaderReflection& psRef)
{
	// 合体して扱う
	auto collectCB = [&](const ShaderReflection& ref)
		{
			for (const auto& cb : ref.cbuffers)
			{
				if (m_ConstantBuffers.contains(cb.name))
					continue;

				// CBuffer 生成
				CBufferEntry entry;
				entry.slot = cb.slot;
				entry.size = cb.size;

				entry.cpuData.resize(cb.size);

				// DirectX のバッファ作成
				D3D11_BUFFER_DESC desc = {};
				desc.Usage = D3D11_USAGE_DYNAMIC;
				desc.ByteWidth = cb.size;
				desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

				Renderer::GetDevice()->CreateBuffer(&desc, nullptr, &entry.buffer);

				m_ConstantBuffers[cb.name] = entry;
			}
		};

	collectCB(vsRef);
	collectCB(psRef);
}
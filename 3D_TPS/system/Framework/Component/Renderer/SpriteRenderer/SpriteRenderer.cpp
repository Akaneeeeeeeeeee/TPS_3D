#include "SpriteRenderer.h"
#include "Src/Framework/Graphics/ResourceManager/ResourceManager.h"
#include "Src/Game/Object_3D/BaseModel/GameObject.h"
#include "Src/Framework/stb_image/stb_image.h"
#include <iostream>

SpriteRenderer::SpriteRenderer()
	: IRenderer(nullptr) // RenderManagerのポインタはnullptrで初期化
{
	// シェーダー名を設定
	/*m_ShaderNames[static_cast<size_t>(ShaderStage::Vertex)] = "SpriteVS";
	m_ShaderNames[static_cast<size_t>(ShaderStage::Pixel)] = "SpritePS";*/
}

SpriteRenderer::SpriteRenderer(RenderManager* renderMgr)
	: IRenderer(renderMgr)
{
	// シェーダー名を設定
	//m_ShaderNames[static_cast<size_t>(ShaderStage::Vertex)] = "SpriteVS";
	//m_ShaderNames[static_cast<size_t>(ShaderStage::Pixel)] = "SpritePS";
	// レンダーマネージャーのポインタを設定
	m_pRenderManager = renderMgr;
}

void SpriteRenderer::Init(void)
{
	// デバイス取得
	//auto device = m_pRenderManager->GetGraphicsDevice()->GetDevice();
	auto device = RenderManager::GetInstance().GetGraphicsDevice()->GetDevice();
	// 頂点情報初期化
	// 頂点データ
	std::vector<VERTEX_2D> vertices;

	vertices.resize(4);

	vertices[0].Position = Vector3(-0.5f, 0.5f, 0);
	vertices[1].Position = Vector3(0.5f, 0.5f, 0);
	vertices[2].Position = Vector3(-0.5f, -0.5f, 0);
	vertices[3].Position = Vector3(0.5f, -0.5f, 0);

	vertices[0].UV = Vector2(0.0f, 0.0f);
	vertices[1].UV = Vector2(1.0f, 0.0f);
	vertices[2].UV = Vector2(0.0f, 1.0f);
	vertices[3].UV = Vector2(1.0f, 1.0f);

	// 頂点バッファ生成
	m_VertexBuffer.Create(vertices);

	// インデックスバッファ生成
	std::vector<unsigned int> indices;
	indices.resize(4);

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 3;

	// インデックスバッファ生成
	m_IndexBuffer.Create(indices);

	// マテリアル情報取得
	m_Material = std::make_unique<Material>();
	MATERIAL	mtrl;
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1, 1, 1, 1);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;
	mtrl.TextureEnable = TRUE; // テクスチャを使うか否かのフラグ
	m_Material->Create(mtrl);

	// シェーダー名設定
	m_ShaderNames[static_cast<size_t>(ShaderStage::Vertex)] = "DefaultVS";
	m_ShaderNames[static_cast<size_t>(ShaderStage::Pixel)] = "DefaultPS";
}

/**
 * @brief 更新処理
 * ここで描画まで行ってしまう
*/
void SpriteRenderer::Update(void)
{
	// 定数バッファ用行列作成(すべて単位行列)
	DirectX::XMFLOAT4X4 matrix[3];
	DirectX::XMStoreFloat4x4(&matrix[0], DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&matrix[1], DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&matrix[2], DirectX::XMMatrixIdentity());

	// PixelShader を取得して定数バッファに書き込む
	auto vs = RenderManager::GetInstance().GetShaderManager()->GetShader(
		m_ShaderNames[static_cast<size_t>(ShaderStage::Vertex)]
	);
	auto ps = RenderManager::GetInstance().GetShaderManager()->GetShader(
		m_ShaderNames[static_cast<size_t>(ShaderStage::Pixel)]
	);
	
	vs->WriteCBuffer(0, matrix); // slot=0 に書き込み

	MATERIAL	mtrl;
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1, 1, 1, 1);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;
	mtrl.TextureEnable = TRUE; // テクスチャを使うか否かのフラグ
}


void SpriteRenderer::Render(void)
{
	auto devicecontext = RenderManager::GetInstance().GetGraphicsDevice()->GetContext();
	//auto devicecontext = m_pRenderManager->GetGraphicsDevice()->GetContext();

	// トポロジーをセット（プリミティブタイプ）
	devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// シェーダーの取得とバインド
	auto vs = RenderManager::GetInstance().GetShaderManager()->GetShader(
		m_ShaderNames[static_cast<size_t>(ShaderStage::Vertex)]
	);
	auto ps = RenderManager::GetInstance().GetShaderManager()->GetShader(
		m_ShaderNames[static_cast<size_t>(ShaderStage::Pixel)]
	);
	//auto vs = m_pRenderManager->GetShaderManager()->GetShader(
	//    m_ShaderNames[static_cast<size_t>(ShaderStage::Vertex)]
	//);
	//auto ps = m_pRenderManager->GetShaderManager()->GetShader(
	//    m_ShaderNames[static_cast<size_t>(ShaderStage::Pixel)]
	//);
	vs->Bind();
	ps->Bind();
	// 頂点・インデックスバッファを GPU にセット
	m_VertexBuffer.SetGPU();
	m_IndexBuffer.SetGPU();


	// マテリアル情報（テクスチャや色）をセット
	//m_Material->SetGPU();

	// 描画
	devicecontext->DrawIndexed(
		4, // 四角形なのでインデックス4つ
		0, // 開始インデックス
		0  // 基準頂点
	);
}


void SpriteRenderer::Uninit(void)
{
	//m_pCBuffer.Reset();
}



bool SpriteRenderer::LoadTexture(const std::filesystem::path& path, DirectX::XMFLOAT2 split)
{
	// 分割数設定
	m_Split = split;
	m_Number = { 0, 0 }; // 初期値は左上の画像を表示

	m_pTexture = std::make_unique<Texture_Low>();

	// 引数のパスからテクスチャを取得
	if (m_pTexture->Create(path.string().c_str()) != S_OK) { return false; }

	// シェーダーのスロット0にセットして GPU にバインド
	auto ps = RenderManager::GetInstance().GetShaderManager()->GetShader(
		m_ShaderNames[static_cast<size_t>(ShaderStage::Pixel)]
	);
	ps->SetTexture(0, m_pTexture.get());

	return true;
}

/**
 * @brief シェーダー設定
 * @param _shader リソースマネージャから持ってきたシェーダーのshared_ptr
*/
//void SpriteRenderer::SetShader(std::shared_ptr<Shader> _shader)
//{
//	this->m_pShader = _shader;
//}


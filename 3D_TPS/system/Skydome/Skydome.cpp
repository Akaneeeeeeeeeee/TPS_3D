#include "Skydome.h"


Skydome::Skydome()
{
}

Skydome::~Skydome()
{
}

void Skydome::Init(void)
{
    // 半径大きめの球体を作成 (例: 半径1000.0f)
    m_SphereMesh.Init(5000.0f, Color(1.0f, 1.0f, 1.0f, 1.0f), 32, 16);
	m_Transform.scale = Vector3(1, 1, 1);

    // マテリアル設定
	// マテリアル情報取得
	MATERIAL mtrl;
	mtrl.Diffuse = Color(1, 1, 1, 1);
	mtrl.Shiness = 1;
	mtrl.TextureEnable = true;		// テクスチャ使用確認フラグ
	m_Material.Create(mtrl);

    // シェーダ読み込み
    m_Shader.Create("shader/litTextureVS.hlsl", "shader/litTexturePS.hlsl");

	// メッシュレンダラー初期化
	m_MeshRenderer.Init(this->m_SphereMesh);
}

void Skydome::Draw(void)
{
	// SRT情報作成
	Matrix4x4 r = Matrix4x4::CreateFromYawPitchRoll(m_Transform.rot.y, m_Transform.rot.x, m_Transform.rot.z);
	Matrix4x4 t = Matrix4x4::CreateTranslation(m_Transform.pos.x, m_Transform.pos.y, m_Transform.pos.z);
	Matrix4x4 s = Matrix4x4::CreateScale(m_Transform.scale.x, m_Transform.scale.y, m_Transform.scale.z);

	Matrix4x4 worldmtx;
	worldmtx = s * r * t;
	Renderer::SetWorldMatrix(&worldmtx); // GPUにセット


	Renderer::DisableCulling(false); // カリング無効化(法線・インデックスバッファを逆転しても描画できなかったため一時的な対策)

	m_Shader.SetGPU();

	// インデックスバッファ・頂点バッファをセット
	m_MeshRenderer.BeforeDraw();

	// マテリアルセット
	m_Material.SetGPU();

	// テクスチャセット
	m_Texture.SetGPU();

	// 描画
	m_MeshRenderer.Draw();
	//m_MeshRenderer.DrawSubset(
	//	static_cast<UINT>(m_SphereMesh.GetIndices().size()), // indexnum : インデックス総数
	//	0,                                               // baseindex : 先頭から
	//	0                                                // basevertexindex : 頂点バッファ先頭から
	//);
	Renderer::DisableCulling(true); // カリング有効化
}

void Skydome::SetTexture(const std::filesystem::path& filepath)
{
	// テクスチャ読み込み
	m_Texture.Load(filepath.string());

	// 法線逆転して頂点バッファ書き換え
	this->InvertNormal();
}

void Skydome::InvertNormal(void)
{
	std::vector<VERTEX_3D> vertices = m_SphereMesh.GetVertices();
	// 法線反転
	for (auto& vertex : vertices) {
		vertex.Normal = -vertex.Normal;
	}

	std::vector<uint32_t> indices = m_SphereMesh.GetIndices();
	// 各三角形ごとに 2番目と3番目を入れ替え
	/*for (size_t i = 0; i < indices.size(); i += 3) {
		std::swap(indices[i + 1], indices[i + 2]);
	}*/
	// インデックスバッファを反転
	std::reverse(indices.begin(), indices.end());

	// 頂点バッファ書き換え
	m_MeshRenderer.Modify(vertices, indices);
}
#include "Terrain.h"
#include "stb_image.h"
#include "commontypes.h"


Terrain::Terrain()
{
}

Terrain::~Terrain()
{
}

void Terrain::Init(int divx, int divy,
	float width, float height)
{
	// サイズセット（幅と高さ）（XZ平面）
	this->m_plane.Init(divx, divy,
		width, height,
		DirectX::SimpleMath::Color(1, 1, 1, 1),
		DirectX::SimpleMath::Vector3(0, 1, 0),
		true, true);
	
	// 頂点バッファ生成
	m_MeshRenderer.Init(this->m_plane);

	// シェーダオブジェクト生成
	//m_Shader.Create("shader/unlitTextureVS.hlsl", "shader/unlitTexturePS.hlsl");
	m_Shader.Create("shader/litTextureVS.hlsl", "shader/litTexturePS.hlsl");

	// テクスチャをロード
	bool sts = m_Texture.Load("assets/texture/field.jpg");
	assert(sts == true);

	// マテリアル情報取得
	m_Material = std::make_unique<CMaterial>();
	MATERIAL mtrl;
	mtrl.Diffuse = Color(1, 1, 1, 1);
	mtrl.Shiness = 1;
	mtrl.TextureEnable = true;		// テクスチャ使用確認フラグ
	m_Material->Create(mtrl);
}

void Terrain::Draw(void)
{
	// SRT情報作成
	Matrix4x4 r = Matrix4x4::CreateFromYawPitchRoll(
		m_transform.rot.y,
		m_transform.rot.x,
		m_transform.rot.z);

	Matrix4x4 t = Matrix4x4::CreateTranslation(
		m_transform.pos.x,
		m_transform.pos.y,
		m_transform.pos.z);

	Matrix4x4 s = Matrix4x4::CreateScale(
		m_transform.scale.x,
		m_transform.scale.y,
		m_transform.scale.z);

	Matrix4x4 worldmtx;		// WorldMatrix：世界の中の自分の情報を持った行列
	worldmtx = s * r * t;
	Renderer::SetWorldMatrix(&worldmtx); // GPUにセット

	// 描画の処理
	m_MeshRenderer.BeforeDraw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_Shader.SetGPU();

	m_Texture.SetGPU();

	m_Material->SetGPU();

	//! カメラ設定を指定
	//m_Camera->SetCamera(0);
	m_MeshRenderer.DrawSubset(
		4 * this->m_plane.GetWidth() * this->m_plane.GetHeight(),		// 描画するインデックス数（四角形なので４）
		0,
		0);
}

void Terrain::SetImage(const std::filesystem::path& _filepath)
{
	// 読み込む画像のファイルパス
	const char* filename = _filepath.string().c_str();

	// 画像データを格納するポインタ
	unsigned char* imageData = nullptr;
	int width, height, channels;

	std::vector<VERTEX_3D> m_Vertices = m_plane.GetVertices();

	// グレースケール（１チャネル）で画像を読み込む
	imageData = stbi_load(filename, &width, &height, &channels, 1);
	if (imageData) {
		for (int z = 0; z < m_plane.GetWidth(); z++) {
			for (int x = 0; x < m_plane.GetHeight(); x++) {

				int picX = x * (float)width / m_plane.GetHeight();
				int picY = z * (float)height / m_plane.GetWidth();
				unsigned char pixelValue = imageData[picY * width + picX];
				float h = (float)pixelValue / 15.0f;	// Y座標
				int n = z * m_plane.GetWidth() * 6 + x * 6;
				m_Vertices[n + 0].Position.y = h;			// 0番目の頂点のy座標をいじる
				// 左隣の四角の頂点
				if (x != 0)m_Vertices[n - 2].Position.y = h;
				if (x != 0)m_Vertices[n - 5].Position.y = h;

				if (z != 0)m_Vertices[n - m_plane.GetHeight() * 6 + 2].Position.y = h;
				if (z != 0)m_Vertices[n - m_plane.GetWidth() * 6 + 3].Position.y = h;

				if (x != 0 && z != 0)m_Vertices[n - m_plane.GetHeight() * 6 - 1].Position.y = h;
				// if分の{}の省略は、初めて出てくる";"までが処理としてカウントされる
			}
		}

		// 頂点ごとの法線を保持する配列
		std::vector<Vector3> vertexNormals(m_Vertices.size(), Vector3(0.0f, 0.0f, 0.0f));

		// 四角形ごとの法線ベクトルを更新
		for (int z = 0; z < m_plane.GetWidth(); z++) {
			for (int x = 0; x < m_plane.GetHeight(); x++) {
				// 四角形を構成する頂点データは一次元配列で連続して管理されている
				// 配列内のどの頂点（インデックス）から計算するかを求める
				int n = z * m_plane.GetWidth() * 6 + x * 6;

				// 0番目の頂点から他の2つ頂点へのベクトルを計算(2辺分で十分→三角形の面の定義は頂点１つとその頂点を通る２つのベクトルであるため)
				Vector3 v1 = m_Vertices[n + 1].Position - m_Vertices[n + 0].Position;
				Vector3 v2 = m_Vertices[n + 2].Position - m_Vertices[n + 0].Position;
				Vector3 normal = v1.Cross(v2);		// 外積で法線を計算
				normal.Normalize();					// 正規化
				m_Vertices[n + 0].Normal = normal;
				m_Vertices[n + 1].Normal = normal;
				m_Vertices[n + 2].Normal = normal;

				// 各頂点に法線を加算
				vertexNormals[n + 0] += normal;
				vertexNormals[n + 1] += normal;
				vertexNormals[n + 2] += normal;

				// 2つのベクトルを計算
				v1 = m_Vertices[n + 4].Position - m_Vertices[n + 3].Position;
				v2 = m_Vertices[n + 5].Position - m_Vertices[n + 3].Position;
				normal = v1.Cross(v2);				// 外積で法線を計算
				normal.Normalize();					// 正規化
				m_Vertices[n + 3].Normal = normal;
				m_Vertices[n + 4].Normal = normal;
				m_Vertices[n + 5].Normal = normal;

				// 各頂点に法線を加算
				vertexNormals[n + 3] += normal;
				vertexNormals[n + 4] += normal;
				vertexNormals[n + 5] += normal;
			}
		}

		// バラバラの向きを向いている法線の向きの平均を取って各頂点に代入
		// →地面の１つの頂点の法線の向きが統一され、シェーディングが綺麗になる
		// 頂点法線を正規化して設定
		for (size_t i = 0; i < m_Vertices.size(); i++) {
			vertexNormals[i].Normalize();		// 法線を正規化
			m_Vertices[i].Normal = vertexNormals[i];

		}

		// メモリを解放
		stbi_image_free(imageData);
	}

	// 頂点バッファ書き換え
	m_MeshRenderer.Modify(m_Vertices);


	// Groundの位置や大きさを調整
	m_transform.pos.y = -20.0f;
	m_transform.scale.x = 20.0f;
	m_transform.scale.z = 20.0f;

	// 頂点情報を変換
	Matrix4x4 r = Matrix4x4::CreateFromYawPitchRoll(
		m_transform.rot.y,
		m_transform.rot.x,
		m_transform.rot.z);

	Matrix4x4 t = Matrix4x4::CreateTranslation(
		m_transform.pos.x,
		m_transform.pos.y,
		m_transform.pos.z);

	Matrix4x4 s = Matrix4x4::CreateScale(
		m_transform.scale.x,
		m_transform.scale.y,
		m_transform.scale.z);

	Matrix4x4 worldmtx;
	worldmtx = s * r * t;

	// 頂点数分ループ
	for (int i = 0; i < m_Vertices.size(); i++) {
		
		/*m_Vertices[i].Position = Vector3::Transform(m_Vertices[i].Position, worldmtx);
		m_Vertices[i].Normal = Vector3::Transform(m_Vertices[i].Normal, worldmtx);*/
	}
}
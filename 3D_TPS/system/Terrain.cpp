#include "Terrain.h"
#include "stb_image.h"
#include "commontypes.h"


Terrain::Terrain(uint64_t id, const std::string& name, const Tag& tag)
    : GameObject(id, name, tag)
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

void Terrain::Draw(uint64_t deltatime)
{
	// SRT情報作成
	Matrix4x4 r = Matrix4x4::CreateFromQuaternion(m_Transform.GetRotation());

	const Vector3& pos = m_Transform.GetPosition();
	Matrix4x4 t = Matrix4x4::CreateTranslation(pos.x, pos.y, pos.z);

	const Vector3& scale = m_Transform.GetScale();
	Matrix4x4 s = Matrix4x4::CreateScale(scale.x, scale.y, scale.z);

	Matrix4x4 worldmtx;		// WorldMatrix：世界の中の自分の情報を持った行列
	worldmtx = s * r * t;
	Renderer::SetWorldMatrix(&worldmtx); // GPUにセット

	// 描画の処理D3D11_PRIMITIVE_TOPOLOGY_TRIANGLEST RIPでよさそう
	m_MeshRenderer.BeforeDraw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

//void Terrain::SetImage(const std::filesystem::path& _filepath)
//{
//	// 読み込む画像のファイルパス
//	std::string filename = _filepath.string();
//
//	// 画像データを格納するポインタ
//	unsigned char* imageData = nullptr;
//	int width, height, channels;
//
//	std::vector<VERTEX_3D> m_Vertices = m_plane.GetVertices();
//
//	// グレースケール（１チャネル）で画像を読み込む
//	imageData = stbi_load(filename.c_str(), &width, &height, &channels, 1);
//	if (imageData) {
//		for (int z = 0; z < m_plane.GetDivY(); z++) {
//			for (int x = 0; x < m_plane.GetDivX(); x++) {
//
//				int picX = x * (float)width / m_plane.GetDivX();
//				int picY = z * (float)height / m_plane.GetDivY();
//				unsigned char pixelValue = imageData[picY * width + picX];
//				float h = static_cast<float>(pixelValue) / 15.0f; // Y座標
//				int n = z * m_plane.GetDivX() * 4 + x * 4;
//				m_Vertices[n + 0].Position.y = h;			// 0番目の頂点のy座標をいじる
//				// 左隣の四角の頂点
//				if (x != 0)m_Vertices[n ].Position.y = h;
//				if (x != 0)m_Vertices[n - 3].Position.y = h;
//
//				if (z != 0)m_Vertices[n - m_plane.GetDivX() * 4 ].Position.y = h;
//				if (z != 0)m_Vertices[n - m_plane.GetDivY() * 4 +1].Position.y = h;
//
//				if (x != 0 && z != 0)m_Vertices[n - m_plane.GetDivX() * 4 - 1].Position.y = h;
//				// if分の{}の省略は、初めて出てくる";"までが処理としてカウントされる
//			}
//		}
//
//		// 頂点ごとの法線を保持する配列
//		std::vector<Vector3> vertexNormals(m_Vertices.size(), Vector3(0.0f, 0.0f, 0.0f));
//
//		// 四角形ごとの法線ベクトルを更新
//		for (int z = 0; z < m_plane.GetDivY(); z++) {
//			for (int x = 0; x < m_plane.GetDivX(); x++) {
//				// 四角形を構成する頂点データは一次元配列で連続して管理されている
//				// 配列内のどの頂点（インデックス）から計算するかを求める
//				int n = z * m_plane.GetDivY() * 6 + x * 6;
//
//				// 0番目の頂点から他の2つ頂点へのベクトルを計算(2辺分で十分→三角形の面の定義は頂点１つとその頂点を通る２つのベクトルであるため)
//				Vector3 v1 = m_Vertices[n + 1].Position - m_Vertices[n + 0].Position;
//				Vector3 v2 = m_Vertices[n + 2].Position - m_Vertices[n + 0].Position;
//				Vector3 normal = v1.Cross(v2);		// 外積で法線を計算
//				normal.Normalize();					// 正規化
//				m_Vertices[n + 0].Normal = normal;
//				m_Vertices[n + 1].Normal = normal;
//				m_Vertices[n + 2].Normal = normal;
//
//				// 各頂点に法線を加算
//				vertexNormals[n + 0] += normal;
//				vertexNormals[n + 1] += normal;
//				vertexNormals[n + 2] += normal;
//
//				// 2つのベクトルを計算
//				v1 = m_Vertices[n + 4].Position - m_Vertices[n + 3].Position;
//				v2 = m_Vertices[n + 5].Position - m_Vertices[n + 3].Position;
//				normal = v1.Cross(v2);				// 外積で法線を計算
//				normal.Normalize();					// 正規化
//				m_Vertices[n + 3].Normal = normal;
//				m_Vertices[n + 4].Normal = normal;
//				m_Vertices[n + 5].Normal = normal;
//
//				// 各頂点に法線を加算
//				vertexNormals[n + 3] += normal;
//				vertexNormals[n + 4] += normal;
//				vertexNormals[n + 5] += normal;
//			}
//		}
//
//		// バラバラの向きを向いている法線の向きの平均を取って各頂点に代入
//		// →地面の１つの頂点の法線の向きが統一され、シェーディングが綺麗になる
//		// 頂点法線を正規化して設定
//		for (size_t i = 0; i < m_Vertices.size(); i++) {
//			vertexNormals[i].Normalize();		// 法線を正規化
//			m_Vertices[i].Normal = vertexNormals[i];
//
//		}
//
//		// メモリを解放
//		stbi_image_free(imageData);
//	}
//
//	// 頂点バッファ書き換え
//	m_MeshRenderer.Modify(m_Vertices);
//
//
//	// Groundの位置や大きさを調整
//	m_transform.pos.y = -20.0f;
//	m_transform.scale.x = 20.0f;
//	m_transform.scale.z = 20.0f;
//
//	// 頂点情報を変換
//	Matrix4x4 r = Matrix4x4::CreateFromYawPitchRoll(
//		m_transform.rot.y,
//		m_transform.rot.x,
//		m_transform.rot.z);
//
//	Matrix4x4 t = Matrix4x4::CreateTranslation(
//		m_transform.pos.x,
//		m_transform.pos.y,
//		m_transform.pos.z);
//
//	Matrix4x4 s = Matrix4x4::CreateScale(
//		m_transform.scale.x,
//		m_transform.scale.y,
//		m_transform.scale.z);
//
//	Matrix4x4 worldmtx;
//	worldmtx = s * r * t;
//
//	// 頂点数分ループ
//	for (int i = 0; i < m_Vertices.size(); i++) {
//		
//		/*m_Vertices[i].Position = Vector3::Transform(m_Vertices[i].Position, worldmtx);
//		m_Vertices[i].Normal = Vector3::Transform(m_Vertices[i].Normal, worldmtx);*/
//	}
//}
void Terrain::SetImage(const std::filesystem::path& _filepath)
{
    std::string filename = _filepath.string();
    unsigned char* imageData = nullptr;
    int width, height, channels;

    std::vector<VERTEX_3D> m_Vertices = m_plane.GetVertices();

    imageData = stbi_load(filename.c_str(), &width, &height, &channels, 1);
    if (imageData) {
        int numVertsX = m_plane.GetDivX() + 1; // X方向の頂点数
        int numVertsZ = m_plane.GetDivY() + 1; // Z方向の頂点数

        for (int z = 0; z < m_plane.GetDivY(); z++) {
            for (int x = 0; x < m_plane.GetDivX(); x++) {
                // 四角形の4頂点を取得
                int v0 = z * numVertsX + x;
                int v1 = v0 + 1;
                int v2 = v0 + numVertsX;
                int v3 = v2 + 1;

                // 画像から高さ取得
                int picX = x * (width - 1) / m_plane.GetDivX();
                int picY = z * (height - 1) / m_plane.GetDivY();
                unsigned char pixelValue = imageData[picY * width + picX];
                float h = static_cast<float>(pixelValue) / 15.0f;

                // 高さを頂点にセット
                m_Vertices[v0].Position.y = h;
                m_Vertices[v1].Position.y = h;
                m_Vertices[v2].Position.y = h;
                m_Vertices[v3].Position.y = h;
            }
        }

        std::vector<Vector3> vertexNormals(m_Vertices.size(), Vector3(0, 0, 0));

        // 法線計算 (4頂点の四角形を2三角形に分けて)
        for (int z = 0; z < m_plane.GetDivY(); z++) {
            for (int x = 0; x < m_plane.GetDivX(); x++) {
                int v0 = z * numVertsX + x;
                int v1 = v0 + 1;
                int v2 = v0 + numVertsX;
                int v3 = v2 + 1;

                Vector3& p0 = m_Vertices[v0].Position;
                Vector3& p1 = m_Vertices[v1].Position;
                Vector3& p2 = m_Vertices[v2].Position;
                Vector3& p3 = m_Vertices[v3].Position;

                // 三角形1 (v0, v1, v2)
                Vector3 normal1 = (p1 - p0).Cross(p2 - p0);
                normal1.Normalize();

                // 三角形2 (v2, v1, v3)
                Vector3 normal2 = (p3 - p2).Cross(p1 - p2);
                normal2.Normalize();

                // 平均を各頂点に加算
                vertexNormals[v0] += normal1;
                vertexNormals[v1] += (normal1 + normal2) * 0.5f;
                vertexNormals[v2] += (normal1 + normal2) * 0.5f;
                vertexNormals[v3] += normal2;
            }
        }

        for (size_t i = 0; i < m_Vertices.size(); i++) {
            vertexNormals[i].Normalize();
            m_Vertices[i].Normal = vertexNormals[i];
        }

        stbi_image_free(imageData);
    }

    m_MeshRenderer.Modify(m_Vertices);

    // ワールド変換
	Vector3 pos = m_Transform.GetPosition();
	Vector3 scale = m_Transform.GetScale();
    pos.y = -20.0f;
    scale.x = 20.0f;
    scale.z = 20.0f;
	const Quaternion& rot = m_Transform.GetRotation();

    Matrix4x4 r = Matrix4x4::CreateFromQuaternion(rot);

    Matrix4x4 t = Matrix4x4::CreateTranslation(pos);

    Matrix4x4 s = Matrix4x4::CreateScale(scale);
    // ワールド行列
    Matrix4x4 worldmtx = s * r * t;

    for (auto& v : m_Vertices) {
        v.Position = Vector3::Transform(v.Position, worldmtx);
        v.Normal = Vector3::Transform(v.Normal, r);
    }

    m_MeshRenderer.Modify(m_Vertices);
}
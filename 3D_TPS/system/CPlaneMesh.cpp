#include	"CPlaneMesh.h"
#include	<iostream>

void CPlaneMesh::Init(
	int divx, int divy,
	float width, float height,
	Color color,
	Vector3 normal,
	bool xzflag,
	bool cwflag) 
{
	// サイズセット（幅と高さ）（XY平面）
	m_width = width;
	m_height = height;

	// 分割数
	m_divY = divy;
	m_divX = divx;

	// 法線ベクトル
	m_normal = normal;

	// 頂点カラー
	m_color = color;

	if (xzflag) {
		// 頂点データ生成
		CreateVertexXZ();
	}
	else {
		// 頂点データ生成
		CreateVertex();
	}

	// インデックスデータ生成
	if (cwflag) {
		CreateIndexCW();
	}
	else {
		CreateIndexCCW();
	}
}

void CPlaneMesh::CreateVertex() {
	// 頂点データクリア
	m_vertices.clear();

	for (unsigned int y = 0; y <= m_divY; y++) {
		VERTEX_SKINNED_GPU	vtx{};
		//VERTEX_3D	vtx{};

		for (unsigned int x = 0; x <= m_divX; x++) {

			// 頂点座標セット
			vtx.Position.x = -m_width / 2.0f + x * m_width / m_divX;
			vtx.Position.y = -m_height / 2.0f + y * m_height / m_divY;
			vtx.Position.z = 0.0f;

			// 法線ベクトルセット
			vtx.Normal = m_normal;				// 法線をセット

			vtx.Diffuse = m_color;					// カラー値セット

			// 繰り返しに対応（テクスチャの）
			float texu = 1.0f * m_divX;
			float texv = 1.0f * m_divY;
			vtx.TexCoord.x = (texu * x / m_divX);
//			vtx.TexCoord.y = (texv * y / m_divY);			// テクスチャ上下さかさまバグを修正
			vtx.TexCoord.y = texv-(texv * y / m_divY);

			m_vertices.emplace_back(vtx);		// 頂点データセット
		}
	}
}

void CPlaneMesh::CreateVertexXZ() {
	// 頂点データクリア
	m_vertices.clear();

	for (unsigned int y = 0; y <= m_divY; y++) {
		VERTEX_SKINNED_GPU	vtx{};
		//VERTEX_3D	vtx{};

		for (unsigned int x = 0; x <= m_divX; x++) {

			// 頂点座標セット
			vtx.Position.x = -m_width / 2.0f + x * m_width / m_divX;
			vtx.Position.y = 0.0f;
			vtx.Position.z = -m_height / 2.0f + y * m_height / m_divY;

			// 法線ベクトルセット
			vtx.Normal = m_normal;				// 法線をセット

			vtx.Diffuse = m_color;					// カラー値セット

			// 繰り返しに対応（テクスチャの）
			float texu = 1.0f * m_divX;
			float texv = 1.0f * m_divY;
			vtx.TexCoord.x = (texu * x / m_divX);
			vtx.TexCoord.y = (texv * y / m_divY);

			m_vertices.emplace_back(vtx);		// 頂点データセット
		}
	}
}

void CPlaneMesh::CreateIndexCW() {
	// インデックスデータクリア
	m_indices.clear();

	// インデックス生成
	for (unsigned int y = 0; y < m_divY; y++) {
		for (unsigned int x = 0; x < m_divX; x++) {
			int count = (m_divX + 1) * y + x;		// 左下座標のインデックス

			// 下半分
			{
				FACE face{};

				face.idx[0] = count;						// 左下
				face.idx[1] = count + 1 + (m_divX + 1);		// 右上
				face.idx[2] = count + 1;					// 右

				m_indices.emplace_back(face.idx[0]);
				m_indices.emplace_back(face.idx[1]);
				m_indices.emplace_back(face.idx[2]);

			}

			// 上半分
			{
				FACE face{};
				face.idx[0] = count;						// 左下
				face.idx[1] = count + (m_divX + 1);			// 上
				face.idx[2] = count + (m_divX + 1) + 1;		// 右上

				m_indices.emplace_back(face.idx[0]);
				m_indices.emplace_back(face.idx[1]);
				m_indices.emplace_back(face.idx[2]);

			}
		}
	}
}

void CPlaneMesh::CreateIndexCCW() {
	// インデックスデータクリア
	m_indices.clear();

	// インデックス生成
	for (unsigned int y = 0; y < m_divY; y++) {
		for (unsigned int x = 0; x < m_divX; x++) {
			int count = (m_divX + 1) * y + x;		// 左下座標のインデックス

			// 下半分
			{
				FACE face{};

				face.idx[0] = count;						// 左下
				face.idx[1] = count + 1 + (m_divX + 1);		// 右上
				face.idx[2] = count + 1;					// 右

				m_indices.emplace_back(face.idx[0]);
				m_indices.emplace_back(face.idx[2]);
				m_indices.emplace_back(face.idx[1]);

			}

			// 上半分
			{
				FACE face{};
				face.idx[0] = count;						// 左下
				face.idx[1] = count + (m_divX + 1);			// 上
				face.idx[2] = count + (m_divX + 1) + 1;		// 右上

				m_indices.emplace_back(face.idx[0]);
				m_indices.emplace_back(face.idx[2]);
				m_indices.emplace_back(face.idx[1]);

			}
		}
	}
}

// 幅
float CPlaneMesh::GetWidth() const {
	return m_width;
}
void CPlaneMesh::SetWidth(float width) {
	m_width = width;
}

// 高さ
float CPlaneMesh::GetHeight() const{
	return m_height;
}
void CPlaneMesh::SetHeight(float height) {
	m_height = height;
}

int CPlaneMesh::GetDivX() {
	return m_divX;
}

int CPlaneMesh::GetDivY() {
	return m_divY;
}


#pragma once
#include	<vector>
#include	"renderer.h"

/*
* @brief	メッシュデータを扱うクラス
* @detail	頂点データ、インデックスデータをの実データ(CPU側)を保持する
* @auther	赤根和樹
* @date		2025/10/16
*/
class CMesh {
public:
	CMesh() = default;
	virtual ~CMesh() = default;

	// 頂点データ取得
	const std::vector<VERTEX_SKINNED_GPU>& GetVertices() const {
		return m_vertices;
	}
	/*const std::vector<VERTEX_3D>& GetVertices() const {
		return m_vertices;
	}*/

	// インデックスデータ取得
	const std::vector<unsigned int>& GetIndices() const {
		return m_indices;
	}
protected:
	std::vector<VERTEX_SKINNED_GPU>	m_vertices;		// 頂点座標群
	//std::vector<VERTEX_3D>	m_vertices;		// 頂点座標群
	std::vector<uint32_t>	m_indices;		// インデックスデータ群
};


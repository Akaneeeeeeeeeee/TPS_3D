#include "Terrain.h"

void Terrain::Init(
	int divx,
	int divz,
	float width,
	float height,
	Color color)
{
	// サイズセット（幅と高さ）（XZ平面）
	this->m_plane.SetWidth(width);
	this->m_plane.SetWidth(height);
	// 分割数
	m_division_x = divx;
	m_division_z = divz;
	// 頂点カラー
	m_color = color;
	// 頂点デタ生成
	CreateVertex();
	// インデックスデータ生成
	CreateIndex();
}
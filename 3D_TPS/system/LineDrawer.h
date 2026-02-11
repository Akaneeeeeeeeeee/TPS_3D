#pragma once

#include	"CommonTypes.h"

void LineDrawerInit();
void LineDrawerUninit();
void LineDrawerDraw(
	float length,
	Vector3 start,
	Vector3 direction,
	Color col);

void SetLineWidth(float);

// ---- インスタンシング版 ----
// 1 本分のパラメータ
struct LineInstanceParam
{
	Vector3 start;   // 始点（ワールド座標）
	Vector3 end;     // 終点（ワールド座標）
	Color   color;   // 色
};

void LineInstancedDrawerInit();
void LineInstancedDrawerUninit();

// 複数本まとめて描く
void LineInstancedDrawerDraw(
	const std::vector<LineInstanceParam>& lines);
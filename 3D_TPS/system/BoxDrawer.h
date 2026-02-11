#pragma once
#include "CommonTypes.h"
#include "system/transform.h"

void BoxDrawerInit();
void BoxDrawerUninit();
void BoxDrawerDraw(
	float width, float height, float depth,
	Color col, float posx, float posy, float posz);
void BoxDrawerDraw(SRT rts, Color col);
void BoxDrawerDraw(Matrix4x4 mtx, Color col);

// ---- インスタンシング版 ----

// 1 インスタンス分の情報
struct BoxInstance
{
    Matrix4x4 world; // スケール・回転・平行移動込み
};

// view/proj と、world 行列の配列を渡してまとめ描き
void BoxInstancedDrawerInit();
void BoxInstancedDrawerUninit();
void BoxInstancedDrawerDraw(
    const Matrix4x4& view,
    const Matrix4x4& proj,
    const std::vector<BoxInstance>& instances,
    Color col);
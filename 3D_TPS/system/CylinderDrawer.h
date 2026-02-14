#pragma once
//#include "Framework/Component/Transform/Transform.h"
#include	"CommonTypes.h"

void CylinderDrawerInit();
void CylinderDrawerDraw(float radius, float hieght, Color col, float posx, float posy, float posz);
void CylinderDrawerDraw(SRT rts, Color col);
void CylinderDrawerDraw(Matrix4x4 mtx, Color col);

// 雨1本分の情報
struct RainInstance
{
    Vector3 pos;    // 中心位置
    float   length; // 縦の長さ
};

void RainInstancedDrawerInit();
void RainInstancedDrawerDraw(
    const Matrix4x4& view,
    const Matrix4x4& proj,
    const std::vector<RainInstance>& instances,
    float radius,   // 横幅（太さ）
    Color col       // 色
);
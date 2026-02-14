#pragma once

#include	"CommonTypes.h"
#include	"transform.h"

void SphereDrawerInit(void);
void SphereDrawerUninit(void);
void SphereInstancedDrawerInit(void);
void SphereInstancedDrawerUninit(void);
void SphereDrawerDraw(float radius, Color col, float ex, float ey, float ez);
void SphereDrawerDraw(SRT rts, Color col);
void SphereDrawerDraw(Matrix4x4 mtx, Color col);
void SphereInstancedDrawerDraw(
	const Matrix4x4& view,
	const Matrix4x4& proj,
	const std::vector<Vector3>& centers,
	float radius,
	Color col);
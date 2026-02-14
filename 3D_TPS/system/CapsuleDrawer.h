#pragma once
#include	"CommonTypes.h"
#include    "renderer.h"

void CapsuleDrawerInit(void);

void CapsuleDrawerUninit(void);

void CapsuleDrawerDraw(float radius, float height, Color col, float posx, float posy, float posz);
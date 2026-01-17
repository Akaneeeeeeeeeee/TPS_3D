// ISpriteSource.h
#pragma once
#include "commontypes.h"

class CSprite;

class ISpriteSource
{
public:
    virtual ~ISpriteSource() = default;

    virtual bool IsVisible() const = 0;
    virtual const CSprite* GetSprite() const = 0;

    // UI(2D)はスクリーンサイズが必要、ビルボードは無視してOK
    virtual Matrix4x4 GetWorld(int screenW, int screenH) const = 0;
};

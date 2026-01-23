#pragma once
#include "system/Sound/WorldSoundEvent.h"

class IWorldSoundListener
{
public:
    virtual ~IWorldSoundListener() = default;
    virtual void OnWorldSound(const WorldSoundEvent& ev) = 0;
};
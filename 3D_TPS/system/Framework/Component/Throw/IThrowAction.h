#pragma once
#include "commontypes.h"

class ObjectManager;
class GameObject;

enum class ThrowItemId
{
    Rock,
    Can,
    Grenade,
};

struct ThrowTuning
{
    float holdNorm = 0.20f;
    float releaseNorm = 0.55f;

    float cooldownSec = 0.20f;

    float spawnForward = 60.0f;
    float spawnUp = 120.0f;

    float speed = 900.0f;
    float lob = 120.0f;
};

struct ThrowSpawnArgs
{
    GameObject& owner;
    ObjectManager& om;

    Vector3 pos;
    Vector3 vel;
};

class IThrowAction
{
public:
    virtual ~IThrowAction() = default;

    virtual ThrowItemId Id() const = 0;
    virtual const ThrowTuning& Tuning() const = 0;
    virtual void Spawn(const ThrowSpawnArgs& a) = 0;
protected:
    int m_Index = 0;
};

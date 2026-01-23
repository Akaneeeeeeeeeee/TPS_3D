#pragma once
#include "Framework/Component/Throw/IThrowAction.h"

struct IThrowEventListener
{
    virtual ~IThrowEventListener() = default;
    virtual void OnThrowReleased(ThrowItemId id) = 0; // Î‚ªè‚ğ—£‚ê‚½uŠÔ
};
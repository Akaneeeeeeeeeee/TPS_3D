#pragma once
#include <vector>
#include <algorithm>
#include "commontypes.h"

class UIImageComponent; // ‘O•ûéŒ¾

class UIManager
{
public:
    void Init() {}
    void Uninit() { m_Images.clear(); }

    void Register(UIImageComponent* img);
    void Unregister(UIImageComponent* img);

    void Update(float /*dt*/) {}

    // Game::Draw() ‚©‚çŒÄ‚Ô
    void Draw(int screenW, int screenH);

private:
    std::vector<UIImageComponent*> m_Images;
};

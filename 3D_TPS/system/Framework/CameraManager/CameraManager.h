#pragma once
#include <vector>

class CameraComponent; // ‘O•ûéŒ¾

class CameraManager
{
public:
    void Register(CameraComponent* cam)
    {
        m_Cameras.push_back(cam);
        // Å‰‚É“o˜^‚³‚ê‚½‚à‚Ì‚ğƒƒCƒ“‚É‚µ‚Ä‚à‚æ‚¢
        if (!m_MainCamera) m_MainCamera = cam;
    }

    void UnRegister(CameraComponent* cam)
    {
        auto it = std::find(m_Cameras.begin(), m_Cameras.end(), cam);
        if (it != m_Cameras.end())
            m_Cameras.erase(it);

        if (m_MainCamera == cam)
            m_MainCamera = m_Cameras.empty() ? nullptr : m_Cameras.front();
    }

    void SetMain(CameraComponent* cam) { m_MainCamera = cam; }
    CameraComponent* GetMain(void) const { return m_MainCamera; }

    const std::vector<CameraComponent*>& GetAll(void) const { return m_Cameras; }

private:
    std::vector<CameraComponent*> m_Cameras;
    CameraComponent* m_MainCamera = nullptr;
};

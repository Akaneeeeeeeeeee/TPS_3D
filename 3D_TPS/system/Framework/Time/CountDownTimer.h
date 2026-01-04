#pragma once
#include <algorithm>



struct CountdownTimer
{
    float remain = 0.0f;
    bool  running = false;

    void Start(float seconds)
    {
        remain = std::max(0.0f, seconds);
        running = true;
    }

    void Stop() { running = false; }

    void Update(float dtSeconds)
    {
        if (!running) return;
        if (dtSeconds <= 0.0f) return; // ポーズ/リセット対策
        remain = std::max(0.0f, remain - dtSeconds);
    }

    bool IsTimeout() const { return running && remain <= 0.0f; }
};

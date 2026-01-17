#pragma once
#include <string>

enum class ResultType : uint8_t
{
    None,
    Clear,
    Found,
    TimeUp
};

struct GameResult
{
    ResultType type = ResultType::None;

    // ¡Œã•K—v‚È‚ç’Ç‰Á
    //float remainSec = 0.0f;
    //std::string foundByEnemyName;
};

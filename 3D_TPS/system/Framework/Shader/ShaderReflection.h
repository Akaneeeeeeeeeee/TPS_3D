#pragma once

#include <vector>
#include <string>
#include <Windows.h>


/*
* @brief    シェーダーリフレクション情報
* @remark   リフレクションを使用して、定数バッファやシェーダーリソースビューの情報を格納する構造体
* @author   赤根　和樹
* @date     2025/11/03
*/

struct CBufferInfo {
    std::string name;
    UINT slot;   // register(b#)
    UINT size;   // バイト
};

struct SRVInfo {
    std::string name;
    UINT slot;   // t# スロット
};

struct ShaderReflection {
    std::vector<CBufferInfo> cbuffers;
    std::vector<SRVInfo> srvs;
};
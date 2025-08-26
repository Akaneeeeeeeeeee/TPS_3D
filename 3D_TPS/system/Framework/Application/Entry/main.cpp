#include    "main.h"
#include    "../Application.h"
#include    "../../../Framework/DetectMemoryLeak/mydebugnew.h"

//=======================================
//エントリーポイント
// ↓デバッグ用コード
// _CrtSetBreakAlloc(400);
//=======================================

#ifdef _DEBUG
//
#define new MYDEBUG_NEW
#endif

int main(void)
{
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif//defined(DEBUG) || defined(_DEBUG)
    //_CrtSetBreakAlloc(385);
    // アプリケーション実行
    Application app(SCREEN_WIDTH, SCREEN_HEIGHT);
    app.Run();

    return 0;
}

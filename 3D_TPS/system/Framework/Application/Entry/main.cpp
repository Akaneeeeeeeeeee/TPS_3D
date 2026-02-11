#include    "main.h"
#include    "system/Framework/Application/Application.h"
#include    "system/Framework/DetectMemoryLeak/mydebugnew.h"
#include <ShellScalingAPI.h>
#pragma comment(lib, "Shcore.lib")
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
    // プロセスを DPI 対応に設定
    //SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    //_CrtSetBreakAlloc(1728078);
    // アプリケーション実行
    Application app(SCREEN_WIDTH, SCREEN_HEIGHT);
    app.Run();

    return 0;
}

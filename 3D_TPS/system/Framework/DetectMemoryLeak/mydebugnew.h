#pragma once

/* MyDbgNew.h
   グローバルな new 演算子をオーバーロードして
   クライアントブロックからメモリを確保する設定を定義
*/

#ifdef _DEBUG
	// デバッグビルド時：
	// new 演算子を、ファイル名と行番号情報付きでメモリ確保する形に置き換える
	#define MYDEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__)
	// ※ _NORMAL_BLOCK を _CLIENT_BLOCK に変更すると、
	//    メモリ確保種別を「クライアントブロック」にできます
#else
	// リリースビルド時：
	// 何も置き換えず、通常の new を使う
	#define MYDEBUG_NEW
#endif // _DEBUG



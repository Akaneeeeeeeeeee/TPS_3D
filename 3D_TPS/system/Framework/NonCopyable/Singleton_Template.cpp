#include "Singleton_Template.h"

template <class T>
T& Singleton<T>::GetInstance(void)
{
	// 静的変数としてシングルトンのインスタンスを保持する
	static T Instance;

	// インスタンスの参照を返す
	return Instance;
}



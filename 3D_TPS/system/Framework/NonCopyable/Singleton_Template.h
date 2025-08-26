#pragma once

/**
 * @brief テンプレート化したシングルトンクラス
 * @tparam T シングルトンにしたいクラスの型
 *
 * このクラスを継承することで派生クラスが全てシングルトンのつくりになる
*/
template <class T>
class Singleton
{
public:
	// インスタンス取得関数
	static T& GetInstance(void)
	{
		// 静的変数としてシングルトンのインスタンスを保持する
		static T Instance;

		// インスタンスの参照を返す
		return Instance;
	}

protected:
	// 外部からのインスタンス生成と削除をさせない
	Singleton() {};
	~Singleton() {};

private:
	Singleton(const Singleton&) = delete;				// コピーコンストラクタを削除
	Singleton& operator=(const Singleton&) = delete;	// 代入演算子を削除
};

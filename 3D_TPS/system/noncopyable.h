#pragma once

/*
* @brief	NonCopyableクラス
* @detail	コピーコンストラクタ、コピー代入演算子を削除したクラス
* @remark	継承することでコピー禁止にできる
* @auther	赤根和樹
*/
class NonCopyable {
public:
    /**
     * @brief デフォルトコンストラクタ。
     */
    NonCopyable() = default;

    /**
     * @brief コピーコンストラクタを削除。
     * @details このクラスのオブジェクトのコピーは許可されていないため、コピーコンストラクタは削除されています。
     */
    NonCopyable(const NonCopyable&) = delete;

    /**
     * @brief コピー代入演算子を削除。
     * @details このクラスのオブジェクトの代入は許可されていないため、コピー代入演算子は削除されています。
     */
    NonCopyable& operator=(const NonCopyable&) = delete;
};

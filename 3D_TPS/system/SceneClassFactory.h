#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include "Framework/Scene/IScene.h"

/**
 * @brief シーンのクラスを名前で登録・生成するためのファクトリクラス
 *
 * @details このクラスは現状はシングルトンとして動作し、文字列名に対応する IScene 派生クラスの
 * インスタンスを生成するための関数を登録・管理します。create() 関数を通じて文字列から
 * シーンインスタンスを動的に生成できます。
 * 必要に応じてインスタンス化し、シーンマネージャの生成時に参照渡しに変更する
 */
class SceneClassFactory {
public:
    /**
     * @brief IScene 派生クラスのインスタンスを生成する関数型
     */
    using SceneCreatorFunc = std::function<std::unique_ptr<IScene>()>;

    /**
     * @brief シングルトンインスタンスを取得
     *
     * @return SceneClassFactory の唯一のインスタンス
     */
    static SceneClassFactory& GetInstance() {
        static SceneClassFactory instance;
        return instance;
    }

    /**
     * @brief クラス名と生成関数を登録する
     *
     * @param name クラス名（create() で指定するキー）
     * @param func クラスインスタンスを生成する関数（例：std::make_unique）
     */
    void RegisterScene(const std::string& name, SceneCreatorFunc func)
    {
        // 既に登録済みか確認（重複防止）
        if (m_Registry.find(name) == m_Registry.end())
        {
            m_Names.push_back(name);
        }
        m_Registry[name] = std::move(func);
    }

    /**
     * @brief 登録されたクラス名からシーンインスタンスを生成する
     *
     * @param name 生成したいクラスの名前（registerClass で登録されたキー）
     * @return std::unique_ptr<IScene> 該当クラスのユニークポインタ（見つからなければ nullptr）
     */
    std::unique_ptr<IScene> Create(const std::string& name) const
    {
		// 登録テーブルから生成関数を探す
        auto it = m_Registry.find(name);

        if (it == m_Registry.end()) { return nullptr; }
		// 見つかったら生成関数を呼び出してインスタンスを返す
        return it->second();
    }

    /*
	* @brief 登録されているシーン名の一覧を取得する
    */
    const std::vector<std::string>& GetRegisteredSceneNames() const
    {
        return m_Names;
    }

private:
	SceneClassFactory() = default;
    /**
     * @brief クラス名と生成関数のマッピングテーブル
     */
    std::unordered_map<std::string, SceneCreatorFunc> m_Registry;

    /**
     * @brief 登録されたシーン名の一覧
	 */
    std::vector<std::string> m_Names;
};

/**
 * @brief クラスを SceneClassFactory に自動登録するマクロ
 *
 * @details IScene 派生クラスのソースファイルにこのマクロを記述することで、静的に
 * SceneClassFactory に登録され、文字列による動的生成が可能になります。
 *
 * 使用例：
 * @code
 * class TitleScene : public IScene { ... };
 * REGISTER_CLASS(TitleScene);
 * @endcode
 *
 * @param CLASSNAME 登録対象のクラス名
 */
#define REGISTER_SCENE(CLASSNAME) \
    namespace { \
        struct CLASSNAME##Registrar { \
            CLASSNAME##Registrar() { \
                SceneClassFactory::GetInstance().RegisterScene( \
                    #CLASSNAME, \
                    []() { return std::make_unique<CLASSNAME>(); } \
                ); \
            } \
        }; \
        static CLASSNAME##Registrar s_##CLASSNAME##_registrar; \
    }
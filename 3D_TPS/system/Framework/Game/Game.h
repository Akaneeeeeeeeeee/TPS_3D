#pragma once
#include "../../Framework/SceneManager/SceneManager.h"
#include "system/Framework/Component/ComponentFactory/ComponentFactory.h"
#include "../../Framework/Graphics/GraphicsDevice.h"

/**
 * @brief ゲームクラス
 * アプリケーションクラスが保持するゲームクラス
 * →このクラスを切り替えれば他のゲームでもこの外側のクラスは使いまわせる
*/
class Game
{
public:
	Game() : m_ObjectManager(), m_SceneManager() {};
	~Game() {};

	void Init(void);
	void Update(void);
	void Draw(void);
	void Uninit(void);

private:
	GraphicsDevice m_GraphicsDevice;	// グラフィックスデバイス
	ObjectManager m_ObjectManager;			// オブジェクト管理クラス
	SceneManager m_SceneManager;			// シーン管理クラス
	ComponentFactory m_ComponentFactory;	// コンポーネントファクトリー
	ShaderManager m_ShaderManager;			// シェーダーマネージャー
	//RenderManager m_RenderManager;			// レンダーマネージャー
};

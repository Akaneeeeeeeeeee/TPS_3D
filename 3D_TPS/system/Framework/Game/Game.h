#pragma once
#include "system/Framework/ObjectManager/ObjectManager.h"
#include "system/Framework/SceneManager/SceneManager.h"
#include "system/Framework/EngineContext/EngineContext.h"
#include "system/Framework/Graphics/GraphicsDevice.h"


/**
 * @brief ゲームクラス
 * アプリケーションクラスが保持するゲームクラス
 * →このクラスを切り替えれば他のゲームでもこの外側のクラスは使いまわせる
*/
class Game
{
public:
	Game();
	~Game();

	void Init(void);
	void Update(uint64_t deltatime);
	void Draw(uint64_t deltatime);
	void Uninit(void);

private:
	GraphicsDevice m_GraphicsDevice;		// グラフィックスデバイス
	ShaderManager m_ShaderManager;			// シェーダーマネージャー
	AssetManager m_AssetManager;			// アセットマネージャー
	//RenderManager m_RenderManager;			// レンダーマネージャー
	std::unique_ptr<EngineContext> m_pContext; // エンジンコンテキスト(Init後に生成したいためunique_ptr)
	
	ObjectManager m_ObjectManager;			// オブジェクト管理クラス
	SceneManager m_SceneManager;			// シーン管理クラス
	//SceneClassFactory m_SceneFactory;	// シーンファクトリー
	//ComponentFactory m_ComponentFactory;	// コンポーネントファクトリー
};

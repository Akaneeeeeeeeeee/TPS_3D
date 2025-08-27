#include "ObjectManager.h"
#include "../../Framework/Component/Collider/2D/BoxCollider2D/BoxCollider2D.h"
#include "../../Framework/Component/Renderer/SpriteRenderer/SpriteRenderer.h"
#include "Src//Framework/Graphics/RenderManager.h"

/**
 * @brief オブジェクト削除関数
 * @param object
*/
void ObjectManager::DeleteObject(Tag _ObjName) {
	//! オブジェクト配列が空でなければ
	if (!this->m_pObjects.empty()) {
		//! 指定した要素を削除
		//Objects.(_ObjName);
	}
}

/// <summary>
/// タグ変更関数
/// <param name="_id">オブジェクトID</param>
/// <param name="_newTag">変えたいタグ</param>
/// </summary>
bool ObjectManager::ChangeTag(uint64_t _id, const Tag& _newTag)
{
	auto it = m_ObjectsByID.find(_id);
	if (it == m_ObjectsByID.end()) { return false; }

	GameObject* obj = it->second;
	Tag oldTag = obj->GetTag();
	if (oldTag == _newTag) { return false; } // 同じなら何もしない

	// 古いタグリストから削除
	auto& oldList = m_ObjectsByTag[oldTag];
	oldList.erase(std::remove(oldList.begin(), oldList.end(), obj), oldList.end());

	// タグを変更
	obj->SetTag(_newTag);

	// 新しいタグリストに追加
	m_ObjectsByTag[_newTag].push_back(obj);

	return true; // 成功
}

/**
 * @brief オブジェクト初期化
 * コンストラクタでオブジェクトを一括追加→Init内で初期化が良さげ？→それぞれ初期化値が違うのでここでオブジェクトのInitを回せない
 *
 * ここではコンテナの初期化だけを行う
*/
void ObjectManager::Init(ComponentFactory* _factory)
{
	// コンポーネントファクトリーへのポインタをセット
	m_pComponentFactory = _factory;
	// オブジェクト管理用コンテナの初期化
	m_pObjects.clear();
	m_ObjectsByID.clear();
	m_ObjectsByName.clear();
	m_ObjectsByTag.clear();
}

void ObjectManager::Init(void)
{
}

void ObjectManager::Update(void)
{
	// 範囲for文
	for (auto& obj : m_pObjects)
	{
		obj->Update();
	}
}

/**
 * @brief 描画
 *
 * カメラがある場合は、そのオブジェクトの大きさ以内にいるものだけを描画する
 * カメラがない場合はそのまま描画
*/
void ObjectManager::Draw(void)
{
	RenderManager::GetInstance().StartRender();

	RenderManager::GetInstance().RenderAll();
	
	RenderManager::GetInstance().EndRender();
	//m_pRenderManager->StartRender();

	//m_pRenderManager->RenderAll();

	//m_pRenderManager->EndRender();
}

void ObjectManager::Uninit(void) {
	// 範囲for文
	for (auto& obj : m_pObjects)
	{
		// オブジェクトの中身を解放
		obj->Uninit();
	}

	// コンテナ全体を解放
	m_pObjects.clear();
	m_ObjectsByID.clear();
	m_ObjectsByName.clear();
	m_ObjectsByTag.clear();
	m_pRenderManager = nullptr;	// レンダリングマネージャーへのポインタをクリア
}

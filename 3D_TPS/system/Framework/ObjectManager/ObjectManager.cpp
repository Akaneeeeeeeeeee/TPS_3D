#include "ObjectManager.h"
//#include "../../Framework/Component/Collider/2D/BoxCollider2D/BoxCollider2D.h"
//#include "../../Framework/Component/Renderer/SpriteRenderer/SpriteRenderer.h"

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

void ObjectManager::DeleteObject(const uint64_t _id) {
	auto it = m_ObjectsByID.find(_id);
	if (it == m_ObjectsByID.end()) { return; }
	GameObject* obj = it->second;
	Tag objTag = obj->GetTag();
	// タグリストから削除
	auto& tagList = m_ObjectsByTag[objTag];
	tagList.erase(std::remove(tagList.begin(), tagList.end(), obj), tagList.end());
	// 名前リストから削除
	auto nameIt = m_ObjectsByName.find(obj->GetName());
	if (nameIt != m_ObjectsByName.end()) {
		m_ObjectsByName.erase(nameIt);
	}
	// IDリストから削除
	m_ObjectsByID.erase(it);
	// オブジェクトコンテナから削除
	m_pObjects.erase(std::remove_if(m_pObjects.begin(), m_pObjects.end(),
		[_id](const std::unique_ptr<GameObject>& o) { return o->GetID() == _id; }),
		m_pObjects.end());
}

void ObjectManager::DeleteObject(const std::string& _name) {
	auto it = m_ObjectsByName.find(_name);
	if (it == m_ObjectsByName.end()) { return; }
	GameObject* obj = it->second;
	Tag objTag = obj->GetTag();
	// タグリストから削除
	auto& tagList = m_ObjectsByTag[objTag];
	tagList.erase(std::remove(tagList.begin(), tagList.end(), obj), tagList.end());
	// IDリストから削除
	auto idIt = m_ObjectsByID.find(obj->GetID());
	if (idIt != m_ObjectsByID.end()) {
		m_ObjectsByID.erase(idIt);
	}
	// 名前リストから削除
	m_ObjectsByName.erase(it);
	// オブジェクトコンテナから削除
	m_pObjects.erase(std::remove_if(m_pObjects.begin(), m_pObjects.end(),
		[_name](const std::unique_ptr<GameObject>& o) { return o->GetName() == _name; }),
		m_pObjects.end());
}

/// <summary>
/// タグ変更関数
/// <param name="_id">オブジェクトID</param>
/// <param name="_newTag">変えたいタグ</param>
/// </summary>
bool ObjectManager::ChangeTag(const uint64_t _id, const Tag _newTag)
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
//void ObjectManager::Init(ComponentFactory* _factory)
//{
//	// コンポーネントファクトリーへのポインタをセット
//	m_pComponentFactory = _factory;
//	// オブジェクト管理用コンテナの初期化
//	m_pObjects.clear();
//	m_ObjectsByID.clear();
//	m_ObjectsByName.clear();
//	m_ObjectsByTag.clear();
//}

void ObjectManager::Init(EngineContext* context)
{
	// エンジンコンテキストのポインタをセット
	m_Context = context;
	// オブジェクト管理用コンテナの初期化
	m_pObjects.clear();
	m_ObjectsByID.clear();
	m_ObjectsByName.clear();
	m_ObjectsByTag.clear();
}

void ObjectManager::Update(const float deltatime)
{
	// 範囲for文
	for (auto& obj : m_pObjects)
	{
		obj->Update(deltatime);
	}
}

/**
 * @brief 描画
 *
 * カメラがある場合は、そのオブジェクトの大きさ以内にいるものだけを描画する
 * カメラがない場合はそのまま描画
*/
void ObjectManager::Draw(void) const
{
	for(auto& obj : m_pObjects)
	{
		obj->Draw();
	}
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
	//m_pRenderManager = nullptr;	// レンダリングマネージャーへのポインタをクリア
}


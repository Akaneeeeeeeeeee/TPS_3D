#include "ObjectManager.h"

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
void ObjectManager::Init(GameObjectFactory* factory)
{
	// ファクトリをセット
	m_ObjectFactory = factory;
	//m_Context = context;
	// オブジェクト管理用コンテナの初期化
	m_pObjects.clear();
	m_ObjectsByID.clear();
	m_ObjectsByName.clear();
	m_ObjectsByTag.clear();
	m_PendingAwake.clear();
	m_PendingStart.clear();
}

void ObjectManager::Update(const float deltatime)
{
	// 1) Awakeを全消化（Awake中に増えた分も処理）
	this->FlushAwakeQueue();

	// 2) Startを全消化（Start中に増えた分は次フレームでもOK）
	this->FlushStartQueue();

	for (auto& obj : m_pObjects)
	{
		// アクティブかつ破棄されていないオブジェクトのみ更新
		if (obj->IsActive() && !obj->IsDestroy())
		{
			obj->Update(deltatime);
		}
	}

	// フレーム最後に一括破棄
	this->FlushDestroyQueue();
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
		// アクティブかつ破棄されていないオブジェクトのみ描画
		if (obj->IsActive() && !obj->IsDestroy())
		{
			obj->Draw();
		}
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
	m_PendingAwake.clear();
	m_PendingStart.clear();
	//m_pRenderManager = nullptr;	// レンダリングマネージャーへのポインタをクリア
}

// FlushAwakeQueue: Awakeキューを消化する関数
void ObjectManager::FlushAwakeQueue(void)
{
	for (;;)
	{
		std::vector<GameObject*> batch;
		batch.swap(m_PendingAwake);
		if (batch.empty()) { break; }

		for (auto* obj : batch)
		{
			if (!obj || obj->IsDestroy()) { continue; }
			obj->AwakeOnce();
			// Awake完了したらStart対象へ
			m_PendingStart.push_back(obj);
		}
	}
}

// FlushStartQueue: Startキューを消化する関数(Awakeキュー消化後に呼び出し)
void ObjectManager::FlushStartQueue(void)
{
	std::vector<GameObject*> batch;
	batch.swap(m_PendingStart);

	// 1) Start前に、全員ぶん Init を先に終わらせる（Awake中に追加された分）
	for (auto* obj : batch)
	{
		if (!obj || obj->IsDestroy()) { continue; }
		obj->FlushInitializeQueue();
	}

	// 2) Start
	for (auto* obj : batch)
	{
		if (!obj || obj->IsDestroy()) { continue; }
		obj->StartOnce();
	}

	// 3) Start中に AddComponent した分も初期化する
	for (auto* obj : batch)
	{
		if (!obj || obj->IsDestroy()) { continue; }
		obj->FlushInitializeQueue();
	}
}

void ObjectManager::FlushDestroyQueue(void)
{
	// 範囲for文
	for (auto it = m_pObjects.begin(); it != m_pObjects.end(); )
	{
		GameObject* obj = it->get();
		if (obj->IsDestroy())
		{
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
			auto idIt = m_ObjectsByID.find(obj->GetID());
			if (idIt != m_ObjectsByID.end()) {
				m_ObjectsByID.erase(idIt);
			}

			// 終了処理
			obj->Uninit();
			// オブジェクトコンテナから削除
			it = m_pObjects.erase(it);
		}
		else
		{
			++it;
		}
	}
}

/**
 * @brief シーン所有オブジェクト削除関数
 * @param sceneName シーン名
 * @remark シーン切り替え時にのみ呼ばれ、シーン所有オブジェクトを削除するための関数
*/
void ObjectManager::DestroySceneObjects(const std::string& sceneName)
{
	for (auto& up : m_pObjects)
	{
		GameObject* obj = up.get();
		if (!obj) { continue; }

		if (obj->GetLifetime() == GameObject::Lifetime::Scene &&
			obj->GetOwnerScene() == sceneName)
		{
			RequestDestroy(obj);
		}
	}
}

// 破棄予約 + 登録解除（実体 erase はしない）
void ObjectManager::RequestDestroy(GameObject* obj)
{
	if (!obj) { return; }
	if (obj->IsDestroy()) { return; }

	obj->Destroy(); // 破棄予約

	// Tag 解除（存在する時だけ）
	auto tagIt = m_ObjectsByTag.find(obj->GetTag());
	if (tagIt != m_ObjectsByTag.end())
	{
		auto& tagList = tagIt->second;
		tagList.erase(std::remove(tagList.begin(), tagList.end(), obj), tagList.end());
	}

	// Name/ID 解除（同名の即再生成ができるように）
	m_ObjectsByName.erase(obj->GetName());
	m_ObjectsByID.erase(obj->GetID());
}

/**
 * @brief オブジェクト削除関数
 * @param object
*/
void ObjectManager::DeleteObject(Tag _tag)
{
	auto it = m_ObjectsByTag.find(_tag);
	if (it == m_ObjectsByTag.end()) { return; }
	// タグに紐づくオブジェクトをすべて破棄リクエスト
	for (auto* obj : it->second)
	{
		RequestDestroy(obj);
	}
}

void ObjectManager::DeleteObject(const uint64_t _id)
{
	auto it = m_ObjectsByID.find(_id);
	if (it == m_ObjectsByID.end()) { return; }
	RequestDestroy(it->second);
}

void ObjectManager::DeleteObject(const std::string& _name)
{
	auto it = m_ObjectsByName.find(_name);
	if (it == m_ObjectsByName.end()) { return; }
	RequestDestroy(it->second);
}

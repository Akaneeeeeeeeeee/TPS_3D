#include "Transform.h"
#include "system//Framework/GameObject/GameObject.h"

void Transform::SetParent(Transform* _parent)
{
	// 既に親がいる場合
    if (m_pParent) {
        // 古い親から自分を外す
        auto& siblings = m_pParent->m_pChildren;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
    }
	
    // 新しい親をセット
    m_pParent = _parent;
    
	// 新しい親の子リストに自分を追加
    if (_parent) {
        auto& ch = _parent->m_pChildren;
        if (std::find(ch.begin(), ch.end(), this) == ch.end())
        {
            ch.push_back(this);
        }
    }
	// 行列更新
	SetDirty();
}

void Transform::SetChild(Transform* _child)
{
    if (_child) {
        // 双方向リンクを保証
        _child->SetParent(this);
    }
}

/*
* 関数の後ろにconstがつく場合→その関数内でメンバ変数を書き換える（編集する）権限を無くすことができる
* →保守性が上がり、その部分でメンバ変数が書き換えられていないことの証明ができる
*/
// ローカル行列取得
Matrix4x4 Transform::GetLocalMatrix(void) const
{
    Matrix4x4 S = Matrix4x4::CreateScale(Scale);
    Matrix4x4 R = Matrix4x4::CreateFromQuaternion(Rotation);
    //Matrix4x4 R = Matrix4x4::CreateFromYawPitchRoll(Rotation.y, Rotation.x, Rotation.z);
    Matrix4x4 T = Matrix4x4::CreateTranslation(Position);
    return S * R * T;
}

// ワールド行列取得
Matrix4x4 Transform::GetWorldMatrix(void) const
{
    // 情報更新があれば再計算
    if (m_IsDirty)
    {
        // 親がいる場合は親のワールド行列を掛ける
        if (m_pParent)
        {
            m_WorldMatrix = GetLocalMatrix() * m_pParent->GetWorldMatrix();
        }
        else
        {
            m_WorldMatrix = GetLocalMatrix();
        }

        m_IsDirty = false;
    }
    return m_WorldMatrix;
}

void Transform::SetDirty(void)
{
    m_IsDirty = true;
    // 子供も更新
    for (auto child : m_pChildren)
    {
        child->SetDirty();
    }
}

void Transform::DetachAllChildren(void)
{
    // ここでは「子->SetParent(nullptr)」を呼ばない
    // （親の m_pChildren をいじりながら走査すると危ないため）
    for (auto* child : m_pChildren)
    {
        if (!child) continue;
        child->m_pParent = nullptr; // Transform のメンバなので直接触れる
        child->SetDirty();
    }
    m_pChildren.clear();
}

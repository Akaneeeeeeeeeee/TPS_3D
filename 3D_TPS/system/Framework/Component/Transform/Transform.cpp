#include "Transform.h"
#include "../../../Game/Object_3D/BaseModel/GameObject.h"

// エイリアス
using Matrix = DirectX::SimpleMath::Matrix;

/*
* 関数の後ろにconstがつく場合→その関数内でメンバ変数を書き換える（編集する）権限を無くすことができる
* →保守性が上がり、その部分でメンバ変数が書き換えられていないことの証明ができる
*/
// ローカル行列取得
Matrix Transform::GetLocalMatrix(void) const {
    Matrix S = Matrix::CreateScale(Scale);
    Matrix R = Matrix::CreateFromQuaternion(Rotation);
    Matrix T = Matrix::CreateTranslation(Position);
    return S * R * T;
}

// ワールド行列取得
Matrix Transform::GetWorldMatrix(const Matrix& _parentmatrix) {
    // 自分のローカル行列を作成
    Matrix S = Matrix::CreateScale(Scale);
    Matrix R = Matrix::CreateFromQuaternion(Rotation);
    Matrix T = Matrix::CreateTranslation(Position);
    Matrix localMatrix = S * R * T;

    // ワールド行列 = ローカル行列 × 親のワールド行列
    Matrix worldMatrix = localMatrix * _parentmatrix;

    // 子も再帰的に更新
    for (size_t idx = 0; idx < m_pChildren.size(); idx++) {
        this->m_pChildren[idx]->GetWorldMatrix(worldMatrix);
    }
    return worldMatrix;
}

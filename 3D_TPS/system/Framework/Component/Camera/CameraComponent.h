#pragma once
#include "system/Framework/Component/IComponent/IComponent.h"
#include "commontypes.h"

// 前方宣言
class CameraManager;

/*
* @brief    カメラコンポーネント
* @detail   
*/
class CameraComponent : public IComponent
{
public:
    CameraComponent();
    ~CameraComponent() override = default;

    // IComponent
    void Attach(EngineContext& context) override;
    void Detach(void) override;

    void Init(void) override;
    void Update(const float deltaTime) override;
    void Uninit(void) override;

    // メインカメラかどうか
    void SetMainCamera(bool flag) { m_IsMainCamera = flag; }
    bool IsMainCamera() const { return m_IsMainCamera; }

    // 位置・注視点
    Vector3 GetPosition() const { return m_Position; }
    void    SetPosition(const Vector3& pos) { m_Position = pos; }

    Vector3 GetLookAt() const { return m_LookAt; }
    void    SetLookAt(const Vector3& look) { m_LookAt = look; }

    // 極座標パラメータ（TPS カメラ制御用）
    float GetElevation() const { return m_Elevation; }  // 仰角[rad]
    float GetAzimuth()   const { return m_Azimuth; }    // 方位角[rad]
    float GetRadius()    const { return m_Radius; }     // 半径

    void SetElevation(float elevation) { m_Elevation = elevation; }
    void SetAzimuth(float azimuth) { m_Azimuth = azimuth; }
    void SetRadius(float radius) { m_Radius = radius; }

    // 行列取得
    const Matrix4x4& GetViewMatrix() const { return m_View; }
    const Matrix4x4& GetProjMatrix() const { return m_Proj; }

    // 投影パラメータ
    void SetPerspective(float fovYRad, float nearPlane, float farPlane);

    // カメラ行列計算＋Renderer にセット
    void ApplyCamera(void);

private:
    CameraManager* m_CameraManager = nullptr;
    
    // メインカメラか？
    bool m_IsMainCamera = true;

    // 軌道カメラ用パラメータ
    float m_Elevation = -90.0f * PI / 180.0f;   // 仰角
    float m_Azimuth = PI / 2.0f;                // 方位角
    float m_Radius = 100.0f;                    // 半径

    // 投影パラメータ
    float m_FovY = DirectX::XMConvertToRadians(45.0f);
    float m_Near = 1.0f;
    float m_Far = 10000.0f;
    // カメラの基本情報
    Vector3 m_Position = Vector3::Zero; // カメラ位置
    Vector3 m_LookAt = Vector3::Zero;   // 注視点
    Vector3 m_Up = Vector3::Up;         // 上方向
    Matrix4x4 m_View{};                 // ビュー行列
    Matrix4x4 m_Proj{};                 // プロジェクション行列
};

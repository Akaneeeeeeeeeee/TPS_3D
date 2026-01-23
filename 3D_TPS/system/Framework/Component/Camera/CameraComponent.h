#pragma once
#include "system/Framework/Component/IComponent/IComponent.h"
#include "commontypes.h"
#include "Framework/PhysicsSystem/Physics.h"

// 前方宣言
class CameraManager;
class PhysicsManager;

/*
* @brief    カメラコンポーネント
* @detail   
*/
class CameraComponent : public IComponent
{
public:
    DECLARE_COMPONENT_TYPE(CameraComponent, IComponent)
    enum class Mode
    {
        Orbit,   // LookAt + Radius/Elevation/Azimuth
        Direct   // Position/LookAt/Up をそのまま使う
    };

    CameraComponent();
    ~CameraComponent() override = default;

    // IComponent
    void Attach(EngineServices& context) override;
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

    void SetMode(Mode m) { m_Mode = m; }
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

    // 肩越し用：右方向へずらす量（+で右肩、-で左肩）
    float GetShoulderOffset() const { return m_ShoulderOffset; }
    void  SetShoulderOffset(float v) { m_ShoulderOffset = v; }

    // 近クリップだけ変えたい（ズーム時に足/体を切りやすくする）
    float GetNearPlane() const { return m_Near; }
    void  SetNearPlane(float nearPlane) { m_Near = nearPlane; }

    void SetCollisionPivot(const Vector3& p) { m_CollisionPivot = p; }
	Vector3 GetCollisionPivot() const { return m_CollisionPivot; }
    void SetIgnoreBody(const JPH::BodyID& id) { m_IgnoreBody = id; }
private:
    CameraManager* m_CameraManager = nullptr;
    JPH::PhysicsSystem* m_Physics = nullptr;
    Mode m_Mode = Mode::Orbit;

    Vector3 m_CollisionPivot = Vector3::Zero;
    JPH::BodyID m_IgnoreBody; // 自分(プレイヤー)除外用

    // メインカメラか？
    bool m_IsMainCamera = true;

    // 軌道カメラ用パラメータ
    float m_Elevation = -90.0f * PI / 180.0f;   // 仰角
    float m_Azimuth = PI / 2.0f;              // 方位角
    float m_Radius = 100.0f;                 // 半径

    // 投影パラメータ
    float m_FovY = DirectX::XMConvertToRadians(45.0f);
    float m_Near = 1.0f;
    float m_Far = 100.0f;

    // 右肩/左肩のオフセット（カメラの“右方向”へずらすだけ）
    float m_ShoulderOffset = 0.0f;

    // カメラの基本情報
    Vector3  m_Position = Vector3::Zero; // カメラ位置
    Vector3  m_LookAt = Vector3::Zero; // 注視点
    Vector3  m_Up = Vector3::Up;   // 上方向
    Matrix4x4 m_View{};                 // ビュー行列
    Matrix4x4 m_Proj{};                 // プロジェクション行列
};

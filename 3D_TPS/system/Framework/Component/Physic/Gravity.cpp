#include"Gravity.h"

const float gravity = 19.6f; // 重力加速度（単位: m/s^2）
float velocityY = 0.5f;     // 垂直方向の初期速度
const float deltaTime = 0.016f; // 1フレームの経過時間（60FPS想定）

bool isJump = false;//ジャンプのフラグ

//重力加速度関数
void Gravity(GameObject* player,Vector3 p_pos)
{
	//位置の更新
	//Vector3 p_pos = player->GetPosition();
	p_pos.y -= 15;
	//p_pos.y -= 5;
	player->SetPosition(p_pos);
	
	

	//デバック
	std::cout << "playerのY座標：" << p_pos.y << ",垂直速度：" << velocityY << std::endl;
}

//ジャンプ関数
void Jump(GameObject* player, Vector3 p_pos)
{
	// 重力の適用
	velocityY += 150;

	// 位置の更新
	p_pos.y += velocityY * deltaTime;
	player->SetPosition(p_pos);

}


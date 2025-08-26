#include "Collider.h"


/////////////////////////////////////////////////////////////////////////////////////////////////
/// ・基本四角で当たり判定を取る→当たったものが何かを特定さえできればオブジェクトマネージャで側からいじれる
/// ・オブジェクトが地面に当たったかの関数も必要→今は個別で回してるのでオブジェクトマネージャで回したい
/// 
/// 
/// メモ：BoxColliderにした場合→当たり判定を取り終わったオブジェクトは判定チェックから外すといいかも
///////////////////////////////////////////////////////////////////////////////////////////////////


//PlayerとGroundの当たり判定(配列)
//template <class T, class U>
bool ColliderPlayer_Ground(Player* _player, std::vector<Object*> _objects)
{
	// 地面の配列の要素数分ループ
	for (auto& ground : _objects) {
		if (BoxCollider(_player, ground))
		{
			// 当たったオブジェクトの速度、方向ベクトルをリセットする
			_player->GetComponent<RigidBody2D>()->SetDirection(Vector3({ 0.0f }));
			_player->GetComponent<RigidBody2D>()->AddForce(Vector3({ 0.0f }), ForceMode2D::VelocityChange);
			std::cout << "地面と触れてます" << std::endl;

			_player->SetOnGround(true);
			return true;
		}
		else
		{
			_player->SetOnGround(false);
			return false;
		}
	}	
}


/**
 * @brief プレイヤーとマガジン
 * 
 * 今はこっち使わない
*/
bool Collider_Player_to_Magazine(Player* obj1, Magazine* obj2)
{
	// ---------------------ここ使いまわせるな？？？？？？？？？？？？？？？？
	float Player_Right_Collider, Player_Left_Collider, Player_Top_Collider, Player_Bottom_Collider;//playerの当たり判定変数
	float Ground_Right_Collider, Ground_Left_Collider, Ground_Top_Collider, Ground_Bottom_Collider;//groundの当たり判定変数

	TransformComponent* player_tf = obj1->GetComponent<TransformComponent>();
	TransformComponent* ground_tf = obj2->GetComponent<TransformComponent>();

	Player_Right_Collider = player_tf->GetPosition().x + player_tf->GetScale().x / 2; //プレイヤーの右当たり判定変数
	Player_Left_Collider = player_tf->GetPosition().x - player_tf->GetScale().x / 2;  //プレイヤーの左当たり判定変数
	Player_Top_Collider = player_tf->GetPosition().y + player_tf->GetScale().y / 2;    //プレイヤーの上当たり判定変数
	Player_Bottom_Collider = player_tf->GetPosition().y - player_tf->GetScale().y / 2;//プレイヤーの下当たり判定変数

	Ground_Right_Collider = ground_tf->GetPosition().x + ground_tf->GetScale().x / 2; //グラウンドの右の当たり判定変数
	Ground_Left_Collider = ground_tf->GetPosition().x - ground_tf->GetScale().x / 2;  //グラウンドの左の当たり判定変数
	Ground_Top_Collider = ground_tf->GetPosition().y + ground_tf->GetScale().y / 2;    //グラウンドの上の当たり判定変数
	Ground_Bottom_Collider = ground_tf->GetPosition().y - ground_tf->GetScale().y / 2;//グラウンドの下の当たり判定変数

	//プレイヤーとマガジンの当たり判定
	if (Player_Left_Collider < Ground_Right_Collider &&
		Ground_Left_Collider < Player_Right_Collider &&
		Player_Bottom_Collider < Ground_Top_Collider &&
		Player_Top_Collider > Ground_Bottom_Collider)
	{
		return true;
	}
	else {
		return false;
	}
}



/**
 * @brief プレイヤーとオブジェクトの当たり判定
 * @return 結果
*/
bool Collider_to_Object(Player* _player, Object* _object) {
	float Player_Right_Collider, Player_Left_Collider, Player_Top_Collider, Player_Bottom_Collider;//playerの当たり判定変数
	float Ground_Right_Collider, Ground_Left_Collider, Ground_Top_Collider, Ground_Bottom_Collider;//groundの当たり判定変数

	TransformComponent* player_tf = _player->GetComponent<TransformComponent>();
	TransformComponent* ground_tf = _object->GetComponent<TransformComponent>();

	Player_Right_Collider = player_tf->GetPosition().x + player_tf->GetScale().x / 2; //プレイヤーの右当たり判定変数
	Player_Left_Collider = player_tf->GetPosition().x - player_tf->GetScale().x / 2;  //プレイヤーの左当たり判定変数
	Player_Top_Collider = player_tf->GetPosition().y + player_tf->GetScale().y / 2;    //プレイヤーの上当たり判定変数
	Player_Bottom_Collider = player_tf->GetPosition().y - player_tf->GetScale().y / 2;//プレイヤーの下当たり判定変数

	Ground_Right_Collider = ground_tf->GetPosition().x + ground_tf->GetScale().x / 2; //グラウンドの右の当たり判定変数
	Ground_Left_Collider = ground_tf->GetPosition().x - ground_tf->GetScale().x / 2;  //グラウンドの左の当たり判定変数
	Ground_Top_Collider = ground_tf->GetPosition().y + ground_tf->GetScale().y / 2;    //グラウンドの上の当たり判定変数
	Ground_Bottom_Collider = ground_tf->GetPosition().y - ground_tf->GetScale().y / 2;//グラウンドの下の当たり判定変数


	
	//// **オブジェクトの上側（Top）で当たった判定**
	//if (Player_Bottom_Collider <= Ground_Top_Collider &&  // プレイヤーの下端がオブジェクトの上端以下
	//	Player_Top_Collider > Ground_Top_Collider &&      // プレイヤーの上端がオブジェクトの上端より上
	//	Player_Right_Collider > Ground_Left_Collider &&   // 横方向でも重なっている
	//	Player_Left_Collider < Ground_Right_Collider &&
	//	_player.()->GetVelocity().y <= 0) {                   // 下方向に移動中のみ適用
	//	_player.()->SetOnGround(true);  // 地面の上にいる
	//	//_player.()->SetVelocityY(0);    // 落下を止める
	//	std::cout << "オブジェクトの上側で当たっています" << std::endl;
	//	return  true;
	//}



	//Playerとオブジェクトが衝突したとき左に進めない
	if (Player_Right_Collider > Ground_Left_Collider &&
		Player_Bottom_Collider <= Ground_Top_Collider &&
		Player_Left_Collider < Ground_Left_Collider)
	{
		_player->GetComponent<RigidBody2D>()->SetMoveRight(false);
		_player()->SetOnGround(true);  // 地面の上にいる

		std::cout << "右側当たっています" << std::endl;
		return true;
	}
	else if (Player_Left_Collider < Ground_Right_Collider &&
		Player_Bottom_Collider <= Ground_Top_Collider &&
		Player_Right_Collider > Ground_Right_Collider)
	{
		_player()->SetMoveLeft(false);
		_player()->SetOnGround(true);  // 地面の上にいる

		std::cout << "左側当たっています" << std::endl;
		return true;
	}
	
	

}


/**
 * @brief プレイヤーとオブジェクト
*/
//void Collider_Player_to_Object(std::weak_ptr<Player> _player, std::vector<std::weak_ptr<GameObject>> _objects) {
//	float Player_Right_Collider, Player_Left_Collider, Player_Top_Collider, Player_Bottom_Collider;//playerの当たり判定変数
//	float Ground_Right_Collider, Ground_Left_Collider, Ground_Top_Collider, Ground_Bottom_Collider;//groundの当たり判定変数
//
//	Player_Right_Collider = _player.()->GetPosition().x + _player.()->GetScale().x / 2; //プレイヤーの右当たり判定変数
//	Player_Left_Collider = _player.()->GetPosition().x - _player.()->GetScale().x / 2;  //プレイヤーの左当たり判定変数
//	Player_Top_Collider = _player.()->GetPosition().y + _player.()->GetScale().y / 2;    //プレイヤーの上当たり判定変数
//	Player_Bottom_Collider = _player.()->GetPosition().y - _player.()->GetScale().y / 2;//プレイヤーの下当たり判定変数
//
//	// vectorのサイズ分ループ
//	for (auto& obj : _objects) {
//		Ground_Right_Collider = obj.()->GetPosition().x + obj.()->GetScale().x / 2; //グラウンドの右の当たり判定変数
//		Ground_Left_Collider = obj.()->GetPosition().x - obj.()->GetScale().x / 2;  //グラウンドの左の当たり判定変数
//		Ground_Top_Collider = obj.()->GetPosition().y + obj.()->GetScale().y / 2;    //グラウンドの上の当たり判定変数
//		Ground_Bottom_Collider = obj.()->GetPosition().y - obj.()->GetScale().y / 2;//グラウンドの下の当たり判定変数
//
//		//プレイヤーとオブジェクトの当たり判定
//		if (Player_Left_Collider < Ground_Right_Collider &&
//			Ground_Left_Collider < Player_Right_Collider &&
//			Player_Bottom_Collider < Ground_Top_Collider &&
//			Player_Top_Collider > Ground_Bottom_Collider)
//		{
//			// 当たったオブジェクトが地面であればの速度、方向ベクトルをリセットする
//			_player.()->SetDirection(Vector3({ 0.0f }));
//			_player.()->AddForce(Vector3({ 0.0f }));
//			_player.()->SetOnGround(true);
//		}
//		else {
//			_player.()->SetOnGround(false);
//		}
//	}
//	
//}



//PlayerとGionの当たり判定
bool ColliderPlayer_Gion(Player* _player, Object* _gion)
{
	TransformComponent* player_tf = _player->GetComponent<TransformComponent>();
	TransformComponent* gion_tf = _gion->GetComponent<TransformComponent>();

	float Player_Right_Collider, Player_Left_Collider, Player_Up_Collider, Player_Bottom_Collider;//playerの当たり判定変数
	float Gion_Right_Collider, Gion_Left_Collider, Gion_Up_Collider, Gion_Bottom_Collider;        //gionの当たり判定変数

	Player_Right_Collider = player_tf->GetPosition().x + player_tf->GetScale().x / 2; //プレイヤーの右当たり判定変数
	Player_Left_Collider = player_tf->GetPosition().x - player_tf->GetScale().x / 2;  //プレイヤーの左当たり判定変数
	Player_Up_Collider = player_tf->GetPosition().y + player_tf->GetScale().y / 2;    //プレイヤーの上当たり判定変数
	Player_Bottom_Collider = player_tf->GetPosition().y - player_tf->GetScale().y / 2;//プレイヤーの下当たり判定変数

	Gion_Right_Collider = gion_tf->GetPosition().x + gion_tf->GetScale().x / 2;   //擬音の右当たり判定
	Gion_Left_Collider = gion_tf->GetPosition().x - gion_tf->GetScale().x / 2;    //擬音の左当たり判定
	Gion_Up_Collider = gion_tf->GetPosition().y + gion_tf->GetScale().y / 2;      //擬音の上当たり判定
	Gion_Bottom_Collider = gion_tf->GetPosition().y - gion_tf->GetScale().y / 2;	//擬音の下当たり判定

	//Playerと擬音の当たり判定
	if (Player_Right_Collider>Gion_Left_Collider&&
		Player_Left_Collider<Gion_Right_Collider&&
		Player_Up_Collider>Gion_Bottom_Collider&&
		Player_Bottom_Collider<Gion_Up_Collider)
	{
		std::cout << "Playerと擬音が衝突しました" << std::endl;
		player->SetOnGround(true);
		return true;
	}
	else {
		player->SetOnGround(false);

		return false;
	}
}


//扇型と擬音の当たり判定

/**
 * @brief プレイヤーと擬音の当たり判定
 * @param fan プレイヤー(そこから扇型に範囲を設定)
 * @param gion 擬音(vectorで全部渡す)
 * @return 当たった擬音
*/
std::pair<std::pair<Tag, std::string>, std::shared_ptr<IOnomatopoeia>> ColliderFan_Gion(std::weak_ptr<Player> fan, std::vector<std::pair<std::pair<Tag, std::string>, std::shared_ptr<IOnomatopoeia>>> _onomatopoeias)
{
	float PI = 3.14159265;
	float fanAngle = PI / 6;
	//扇型の情報取得
	float fanCenterX = fan->GetPosition().x + 200.0f;	//扇型の中心X座標
	//float fanCenterX = fan.()->GetPosition().x + 200.0f;	//扇型の中心X座標

	float fanCenterY = fan.()->GetPosition().y;   //扇型の中心Y座標
	float fanRadius = fan.()->GetScale().x / 2;   //扇型の半径（スケールのX方向を使用）

	//扇型の方向ベクトルを右方向に固定
	float fanDirX = 1.0f;//右方向の成分
	float fanDirY = 0.0f;//上方向の成分
	
	//擬音の情報取得
	float Gion_Right_Collider;		// 擬音の右端
	float Gion_Left_Collider;		// 擬音の左端
	float Gion_Up_Collider;			// 擬音の上端
	float Gion_Bottom_Collider;		// 擬音の下端
	
	// 擬音との当たり判定を取得
	for (auto& onomat : _onomatopoeias) {
		// 擬音の当たり判定範囲を取得
		Gion_Right_Collider = onomat.second->GetPosition().x + onomat.second->GetScale().x / 2; //擬音の右端
		Gion_Left_Collider = onomat.second->GetPosition().x - onomat.second->GetScale().x / 2;  //擬音の左端
		Gion_Up_Collider = onomat.second->GetPosition().y + onomat.second->GetScale().y / 2;    //擬音の上端
		Gion_Bottom_Collider = onomat.second->GetPosition().y - onomat.second->GetScale().y / 2;//擬音の

		//------------扇形と擬音の四つの頂点------------
		float vertices[4][2] = {
			{Gion_Left_Collider,Gion_Up_Collider},//左上
			{Gion_Right_Collider,Gion_Up_Collider},//右上
			{Gion_Left_Collider,Gion_Bottom_Collider},//左下
			{Gion_Right_Collider,Gion_Bottom_Collider},//右下
		};

		//四角形の各頂点が扇形内に含まれるか確認
		for (int i = 0; i < 4; ++i) {
			float dx = vertices[i][0] - fanCenterX;   //扇型中心から頂点へのベクトルX
			float dy = vertices[i][1] - fanCenterY;   //扇型中心から頂点へのベクトルY
			float distance = sqrtf(dx * dx + dy * dy);//距離を計算

			//距離が半径内か確認
			if (distance <= fanRadius) {
				//ベクトルの角度の計算（内積）
				float dot = (dx * fanDirX + dy * fanDirY) / (distance);//cosθ=内積/(|v1|*|v2|)

				//角度が範囲内か確認
				float cosLimit = cosf(fanAngle);//扇型の角度範囲のcos値
				if (dot >= cosLimit) {
					std::cout << "頂点が扇型内です" << std::endl;
					// 擬音の頂点が扇型内なのでその擬音を返す
					return onomat;
				}
			}
		}

		//---------------扇形と四角形の辺が扇型の円弧と交差しているか確認------------------
		float edges[4][4] = {
			{Gion_Left_Collider,Gion_Up_Collider,Gion_Right_Collider,Gion_Up_Collider},        //上辺
			{Gion_Right_Collider,Gion_Up_Collider,Gion_Right_Collider,Gion_Bottom_Collider},   //右辺
			{Gion_Left_Collider,Gion_Bottom_Collider,Gion_Right_Collider,Gion_Bottom_Collider},//下辺
			{Gion_Left_Collider,Gion_Up_Collider,Gion_Left_Collider,Gion_Bottom_Collider}      //左辺
		};
		for (int i = 0; i < 4; ++i) {
			if (LineIntersectsCircle(edges[i][0], edges[i][1], edges[i][2], edges[i][3], fanCenterX, fanCenterY, fanRadius)) {
				std::cout << "四角形の辺が扇型の円弧と交差しています" << std::endl;
				// 四角形の辺が扇型の円弧と交差
				return onomat;
			}
		}
		// 扇型の中心が四角形内にあるか確認
		if (fanCenterX >= Gion_Left_Collider && fanCenterX <= Gion_Right_Collider &&
			fanCenterY >= Gion_Bottom_Collider && fanCenterY <= Gion_Up_Collider) {
			// 扇型の中心が四角形内
			return onomat;
		}
	}
	//どれにも該当しない場合、当たっていないので何も返さない
	return {};
}

// 線分と円が交差しているか判定する関数
bool LineIntersectsCircle(float x1, float y1, float x2, float y2, float cx, float cy, float radius)
{
	float dx = x2 - x1;
	float dy = y2 - y1;

	float fx = x1 - cx;
	float fy = y1 - cy;

	float a = dx * dx + dy * dy;
	float b = 2 * (fx * dx + fy * dy);
	float c = (fx * fx + fy * fy) - radius * radius;

	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0) {
		return false;//交差していない
	}
	else {
		discriminant = sqrtf(discriminant);
		float t1 = (-b - discriminant) / (2 * a);
		float t2 = (-b + discriminant) / (2 * a);

		return (t1 >= 0 && t1 <= 1) || (t2 >= 0 && t2 <= 1);
	}
}


// 当たり判定着色用
//void DrawFan(GameObject* fan, bool isColliding)
//{
//	//扇型の情報取得
//	float centerX = fan->GetPosition().x;
//	float centerY = fan->GetPosition().y;
//	float radius = fan->GetScale().x / 2;
//	float M_PI = 3.14159265;
//	float fanAngle = M_PI / 6;//開き角度（30度）
//	float dirX = cosf(fan->GetRotation().z);
//	float dirY = sinf(fan->GetRotation().z);

//	//色判定
//	float r, g, b;
//	if (isColliding) {
//		r = 1.0f; g = 0.0f; b = 0.0f;//赤
//	}
//	else {
//		r = 0.0f, g = 1.0f, b = 0.0f;//緑
//	}
//}




#include	<iostream>
#include	"CAnimationMesh.h"
#include	"utility.h"

using namespace DirectX::SimpleMath;


// ノードツリー表示(debug用)
static void DispNodeTree(CTreeNode<std::string>* ptree) 
{
	std::cout << ptree->m_nodedata << std::endl;

	for (unsigned int n = 0; n < ptree->m_children.size(); n++)
	{
		DispNodeTree(ptree->m_children[n].get());
	}
}

void CAnimationMesh::Draw()
{
	// メッシュ描画
	m_StaticMeshRenderer.Draw();
}


void CAnimationMesh::Load(std::string filename, std::string texturedirectory) 
{
	// メッシュ読み込み
	CStaticMesh::Load(filename, texturedirectory);

	// アニメーションデータ(ASSIMP用）
	std::unordered_map<std::string, GM31::GE::myAssimp::BONE> assimp_BoneDictionary{};	// 20240714 DX化

	// ボーン辞書取得（ボーン名をキーにしてボーン情報が取れる）
	assimp_BoneDictionary = GM31::GE::myAssimp::GetBoneDictionary();					// 20240714 DX化

	for (auto& asimpbone : assimp_BoneDictionary) {										// 20240714 DX化
		BONE dxbone;																	// 20240714 DX化	

		dxbone.meshname = asimpbone.second.meshname;									// 20240714 DX化
		dxbone.armaturename = asimpbone.second.armaturename;							// 20240714 DX化
		dxbone.bonename = asimpbone.second.bonename;									// 20240714 DX化
		dxbone.idx = asimpbone.second.idx;												// 20240714 DX化

		dxbone.OffsetMatrix = utility::aiMtxToDxMtx(asimpbone.second.OffsetMatrix);
		dxbone.AnimationMatrix = Matrix::Identity;										// 20240714 DX化
		dxbone.Matrix = Matrix::Identity;												// 20240714 DX化

		dxbone.weights.clear();															// 20240714 DX化
		for (auto& asimpweight : asimpbone.second.weights)								// 20240714 DX化	
		{
			WEIGHT dxweight;															// 20240714 DX化			
			dxweight.bonename = asimpweight.bonename;									// 20240714 DX化
			dxweight.meshname = asimpweight.meshname;									// 20240714 DX化
			dxweight.vertexindex = asimpweight.vertexindex;								// 20240714 DX化
			dxweight.weight = asimpweight.weight;										// 20240714 DX化
			dxbone.weights.push_back(dxweight);											// 20240714 DX化		
		}																				// 20240714 DX化

		m_BoneDictionary[asimpbone.first] = dxbone;										// 20240714 DX化
	}																	

	// ボーン名ツリー取得
	m_AssimpNodeNameTree = GM31::GE::myAssimp::GetBoneNameTree();

	// レンダラ初期化
	m_StaticMeshRenderer.Init(*this);

}

// 階層構造を考慮したボーンコンビネーション行列を更新
//void CAnimationMesh::UpdateBoneMatrix(CTreeNode<std::string>* ptree,
//	const DirectX::SimpleMath::Matrix& parent,
//	BoneCombMatrix& outMatrix,
//	float timeInTicks,
//	aiAnimation* anim)															// 20240714 DX化	
//{
//	// ノード名からボーン辞書を使ってボーン情報を取得
//	BONE* bone = &m_BoneDictionary[ptree->m_nodedata];
//
//	// チャンネル取得
//	aiNodeAnim* nodeAnim = nullptr;
//	for (unsigned int c = 0; c < anim->mNumChannels; c++)
//	{
//		if (anim->mChannels[c]->mNodeName.C_Str() == ptree->m_nodedata)
//		{
//			nodeAnim = anim->mChannels[c];
//			break;
//		}
//	}
//
//	Matrix animMatrix = Matrix::Identity;
//
//	if (nodeAnim)
//	{
//		// Rotation
//		aiQuaternion rot;
//		int rotIndex = static_cast<int>(timeInTicks) % nodeAnim->mNumRotationKeys;
//		rot = nodeAnim->mRotationKeys[rotIndex].mValue;
//
//		// Position
//		aiVector3D pos;
//		int posIndex = static_cast<int>(timeInTicks) % nodeAnim->mNumPositionKeys;
//		pos = nodeAnim->mPositionKeys[posIndex].mValue;
//
//		// Scale
//		Vector3 s = { 1.0f, 1.0f, 1.0f };
//		Vector3 t = { pos.x, pos.y, pos.z };
//		Quaternion r = { rot.x, rot.y, rot.z, rot.w };
//
//		Matrix scaleMtx = Matrix::CreateScale(s.x, s.y, s.z);
//		Matrix rotMtx = Matrix::CreateFromQuaternion(r);
//		Matrix transMtx = Matrix::CreateTranslation(t.x, t.y, t.z);
//
//		animMatrix = scaleMtx * rotMtx * transMtx;
//	}
//
//	// ワールド行列に親行列を掛ける
//	Matrix finalMtx = animMatrix * parent;
//
//	// ボーンコンビネーション行列に反映
//	bone->Matrix = bone->OffsetMatrix * finalMtx;
//	outMatrix.ConstantBufferMemory.BoneCombMtx[bone->idx] = bone->Matrix.Transpose();
//
//	// 子ノードを再帰処理
//	for (unsigned int n = 0; n < ptree->m_children.size(); n++)
//	{
//		UpdateBoneMatrix(ptree->m_children[n].get(), finalMtx, outMatrix, timeInTicks, anim);
//	}																// 20240714 DX化
//}

//// 階層構造を考慮したボーンコンビネーション行列を更新
void CAnimationMesh::UpdateBoneMatrix(
	CTreeNode<std::string>* ptree, 
	Matrix matrix)															// 20240714 DX化	
{
	// ノード名からボーン辞書を使ってボーン情報を取得
	BONE* bone = &m_BoneDictionary[ptree->m_nodedata];						// 20240714 DX化		

	Matrix bonecombination;													// 20240714 DX化；

	// ボーンオフセット行列×ボーンアニメメーション行列×逆ボーンオフセット行列
	bonecombination = bone->OffsetMatrix * bone->AnimationMatrix * matrix;	// 20240714 DX化
	bone->Matrix = bonecombination;											// 20240714 DX化

	// 自分の姿勢を表す行列を作成
	Matrix mybonemtx;														// 20240714 DX化
	mybonemtx = bone->AnimationMatrix * matrix;								// 20240714 DX化
	// 子ノードに対して再帰的に処理											// 20240714 DX化
	for (unsigned int n = 0; n < ptree->m_children.size(); n++)				// 20240714 DX化
	{																		// 20240714 DX化
		UpdateBoneMatrix(ptree->m_children[n].get(), mybonemtx);			// 20240714 DX化
	}																		// 20240714 DX化
}

void CAnimationMesh::BlendLocalPose(
	const std::unordered_map<std::string, Transform>& localposeFrom,
	const std::unordered_map<std::string, Transform>& localposeTo,
	float rate,
	std::unordered_map<std::string, Transform>& blendedLocalPose)
{
	blendedLocalPose.clear();
	blendedLocalPose.reserve(std::max(localposeFrom.size(), localposeTo.size()));

	// to 側にあるボーンを基準にブレンド
	for (const auto& [bone, to] : localposeTo)
	{
		auto itFrom = localposeFrom.find(bone);
		const Transform& from = (itFrom != localposeFrom.end()) ? itFrom->second : Transform();

		Transform blended{};
		blended.SetPosition(Vector3::Lerp(from.GetPosition(), to.GetPosition(), rate));
		blended.SetScale(Vector3::Lerp(from.GetScale(), to.GetScale(), rate));
		blended.SetRotation(Quaternion::Slerp(from.GetRotation(), to.GetRotation(), rate));

		blendedLocalPose.emplace(bone, blended);
	}

	// from にしかないボーンも拾う
	for (const auto& [bone, from] : localposeFrom)
	{
		if (blendedLocalPose.find(bone) != blendedLocalPose.end())
			continue;

		const Transform& to = Transform();

		Transform blended{};
		blended.SetPosition(Vector3::Lerp(from.GetPosition(), to.GetPosition(), rate));
		blended.SetScale(Vector3::Lerp(from.GetScale(), to.GetScale(), rate));
		blended.SetRotation(Quaternion::Slerp(from.GetRotation(), to.GetRotation(), rate));

		blendedLocalPose.emplace(bone, blended);
	}
}

// アニメーションの更新
void CAnimationMesh::Update(BoneCombMatrix& bonecombarray, const aiAnimation* animation, int& CurrentFrame)
{
	if (!animation) { return; }

	// ボーン数分ループしてボーン行列を作成
	for (unsigned int c = 0; c < animation->mNumChannels; c++)
	{
		aiNodeAnim* nodeAnim = animation->mChannels[c];

		// ノード名からボーン辞書を使ってassimpのボーン情報を取得
		BONE* bone = &(m_BoneDictionary[nodeAnim->mNodeName.C_Str()]);	// 20240714 DX化

		int f;

		f = CurrentFrame % nodeAnim->mNumRotationKeys;				//簡易実装
		aiQuaternion rot = nodeAnim->mRotationKeys[f].mValue;

		f = CurrentFrame % nodeAnim->mNumPositionKeys;				//簡易実装
		aiVector3D pos = nodeAnim->mPositionKeys[f].mValue;

		// SRTから行列を生成
		Vector3 s = { 1.0f,1.0f,1.0f };		// 20240714 DX化
		Vector3 t = { pos.x,pos.y,pos.z };	// 20240714 DX化
		Quaternion r{};						// 20240714 DX化

		r.x = rot.x;						// 20240714 DX化
		r.y = rot.y;						// 20240714 DX化
		r.z = rot.z;						// 20240714 DX化
		r.w = rot.w;						// 20240714 DX化

		Matrix scalemtx = Matrix::CreateScale(s.x, s.y, s.z);		// 20240714 DX化
		Matrix rotmtx = Matrix::CreateFromQuaternion(r);			// 20240714 DX化
		Matrix transmtx = Matrix::CreateTranslation(t.x, t.y, t.z);	// 20240714 DX化

		bone->AnimationMatrix = scalemtx * rotmtx * transmtx;		// 20240714 DX化
	}

	UpdateBoneMatrix(&m_AssimpNodeNameTree, Matrix::Identity);		// 20240714 DX化	

	// ボーンコンビネーション行列の配列をセット
	for (const auto& bone : m_BoneDictionary)
	{
		bonecombarray.ConstantBufferMemory.BoneCombMtx[bone.second.idx] = bone.second.Matrix.Transpose();	// 20240714 DX化
	}
}

void CAnimationMesh::Update(BoneCombMatrix& bonecombarray, const aiAnimation* animation, float& timeSec)
{
	if (!animation) return;

	// 1) 秒 → アニメ内時間（ticks）に変換
	double ticksPerSecond =
		(animation->mTicksPerSecond != 0.0)
		? animation->mTicksPerSecond
		: 30.0; // mTicksPerSecond が 0 の場合のデフォルト

	double timeInTicks = timeSec * ticksPerSecond;

	// アニメーション長でループ
	double duration = animation->mDuration; // 単位は ticks
	if (duration > 0.0)
	{
		timeInTicks = fmod(timeInTicks, duration);
		if (timeInTicks < 0.0) timeInTicks += duration;
	}

	// 2) 各チャンネルごとに適切なキーを選んで補間
	for (unsigned int c = 0; c < animation->mNumChannels; ++c)
	{
		aiNodeAnim* nodeAnim = animation->mChannels[c];
		BONE* bone = &m_BoneDictionary[nodeAnim->mNodeName.C_Str()];

		// ---- 回転キー選択（線形補間版・簡易）----
		Quaternion rotQ = Quaternion::Identity;
		if (nodeAnim->mNumRotationKeys > 0)
		{
			// ここでは単純に「timeInTicks以下で最大のキー」を探す
			unsigned int idx = 0;
			while (idx + 1 < nodeAnim->mNumRotationKeys &&
				nodeAnim->mRotationKeys[idx + 1].mTime < timeInTicks)
			{
				++idx;
			}
			aiQuaternion rotA = nodeAnim->mRotationKeys[idx].mValue;
			rotQ = Quaternion(rotA.x, rotA.y, rotA.z, rotA.w);
		}

		// ---- 位置キー選択（同様に簡易）----
		Vector3 posV = Vector3::Zero;
		if (nodeAnim->mNumPositionKeys > 0)
		{
			unsigned int idx = 0;
			while (idx + 1 < nodeAnim->mNumPositionKeys &&
				nodeAnim->mPositionKeys[idx + 1].mTime < timeInTicks)
			{
				++idx;
			}
			aiVector3D posA = nodeAnim->mPositionKeys[idx].mValue;
			posV = Vector3(posA.x, posA.y, posA.z);
		}

		Vector3 scaleV = Vector3::One;

		Matrix4x4 S = Matrix4x4::CreateScale(scaleV);
		Matrix4x4 R = Matrix4x4::CreateFromQuaternion(rotQ);
		Matrix4x4 T = Matrix4x4::CreateTranslation(posV);

		bone->AnimationMatrix = S * R * T;
	}

	// 3) 階層を考慮してボーン最終行列を作る
	UpdateBoneMatrix(&m_AssimpNodeNameTree, Matrix4x4::Identity);

	// 4) BoneCombMatrix に転送
	for (const auto& bone : m_BoneDictionary)
	{
		bonecombarray.ConstantBufferMemory.BoneCombMtx[bone.second.idx] =
			bone.second.Matrix.Transpose();
	}
}


void CAnimationMesh::UpdateBlended(BoneCombMatrix& bonecombarray,
	const aiAnimation* animFrom,
	int frameFrom,
	const aiAnimation* animTo,
	int frameTo,
	float alpha)
{
	if (!animFrom && !animTo) return;

	// 1) 2つのローカルポーズを構築
	std::unordered_map<std::string, Transform> localposeFrom;
	std::unordered_map<std::string, Transform> localposeTo;

	if (animFrom)
	{
		BuildLocalPoseMap(animFrom, frameFrom, localposeFrom);
	}
	if (animTo)
	{
		BuildLocalPoseMap(animTo, frameTo, localposeTo);
	}

	// 2) ブレンド結果のローカルポーズを計算
	std::unordered_map<std::string, Transform> blendedLocalPose;
	BlendLocalPose(localposeFrom, localposeTo, alpha, blendedLocalPose);

	// 3) 各ボーンに AnimationMatrix をセット
	for (auto& [bonename, transform] : blendedLocalPose)
	{
		BONE* bone = &m_BoneDictionary[bonename];

		Matrix4x4 scalemtx = Matrix4x4::CreateScale(transform.GetScale());
		Matrix4x4 rotmtx = Matrix4x4::CreateFromQuaternion(transform.GetRotation());
		Matrix4x4 transmtx = Matrix4x4::CreateTranslation(transform.GetPosition());

		bone->AnimationMatrix = scalemtx * rotmtx * transmtx;
	}

	// 4) 階層を考慮して最終ボーン行列を計算
	UpdateBoneMatrix(&m_AssimpNodeNameTree, Matrix4x4::Identity);

	// 5) BoneCombMatrix に書き出し
	for (const auto& bone : m_BoneDictionary)
	{
		bonecombarray.ConstantBufferMemory.BoneCombMtx[bone.second.idx] =
			bone.second.Matrix.Transpose();
	}
}


void CAnimationMesh::BuildLocalPoseMap(
	const aiAnimation* animationdata,
	int& frame,
	std::unordered_map<std::string, Transform>& localposemap)
{
	if (!animationdata) return;

	const aiAnimation* animation = animationdata;

	localposemap.clear();

	for (unsigned int c = 0; c < animation->mNumChannels; c++)
	{
		aiNodeAnim* nodeAnim = animation->mChannels[c];

		int fRot = frame % nodeAnim->mNumRotationKeys;
		int fPos = frame % nodeAnim->mNumPositionKeys;

		aiQuaternion rot = nodeAnim->mRotationKeys[fRot].mValue;
		aiVector3D   pos = nodeAnim->mPositionKeys[fPos].mValue;

		Vector3 scale = Vector3::One;
		Vector3 translation(pos.x, pos.y, pos.z);
		Quaternion rotation(rot.x, rot.y, rot.z, rot.w);

		Transform transform(translation, rotation, scale);

		localposemap[nodeAnim->mNodeName.C_Str()] = transform;
	}
}

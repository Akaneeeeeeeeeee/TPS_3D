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
		//dxbone.AnimationMatrix = Matrix::Identity;										// 20240714 DX化
		dxbone.Matrix = Matrix::Identity;												// 20240714 DX化
		// identity じゃなく bindローカルにする
		dxbone.BindLocalMatrix = utility::aiMtxToDxMtx(asimpbone.second.AnimationMatrix);
		dxbone.AnimationMatrix = dxbone.BindLocalMatrix;

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
void CAnimationMesh::UpdateBoneMatrix(
	CTreeNode<std::string>* ptree,
	Matrix4x4 parentMtx)
{
	// デフォルトでは「親の行列をそのまま渡す」
	Matrix4x4 myBoneMtx = parentMtx;

	// このノード名に対応するボーンが存在する場合だけ処理
	auto it = m_BoneDictionary.find(ptree->m_nodedata);
	if (it != m_BoneDictionary.end())
	{
		BONE& bone = it->second;

		// ローカル行列
		Matrix4x4 localMtx = bone.AnimationMatrix;

		// 親まで含めた合成行列
		myBoneMtx = localMtx * parentMtx;

		// スキン用最終行列 = オフセット × 合成
		Matrix4x4 bonecombination = bone.OffsetMatrix * myBoneMtx;
		bone.Matrix = bonecombination;
	}

	// 子ノードへ伝播
	for (auto& child : ptree->m_children)
	{
		UpdateBoneMatrix(child.get(), myBoneMtx);
	}
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
void CAnimationMesh::Update(BoneCombMatrix& bonecombarray, const aiAnimation* animation, const float& timeSec)
{
	if (!animation) return;
	// ★追加：毎フレ bind姿勢に戻す
	for (auto& [name, bone] : m_BoneDictionary)
		bone.AnimationMatrix = bone.BindLocalMatrix;
		
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


	int hit = 0;
	int miss = 0;

	// 2) 各チャンネルごとに適切なキーを選んで補間
	for (unsigned int c = 0; c < animation->mNumChannels; ++c)
	{
		aiNodeAnim* nodeAnim = animation->mChannels[c];
		//auto itB = m_BoneDictionary.find(nodeAnim->mNodeName.C_Str());
		//if (itB == m_BoneDictionary.end())
		//	continue;
		auto itB = m_BoneDictionary.find(nodeAnim->mNodeName.C_Str());

		const std::string name = nodeAnim->mNodeName.C_Str();
		if (m_BoneDictionary.find(name) != m_BoneDictionary.end())
			++hit;
		else { ++miss; continue; }

		BONE& bone = itB->second;

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

		bone.AnimationMatrix = S * R * T;
	}
	std::cout << "[AnimMap] name=" << animation->mName.C_Str()
		<< " channels=" << animation->mNumChannels
		<< " hit=" << hit << " miss=" << miss << "\n";

	// 3) 階層を考慮してボーン最終行列を作る
	UpdateBoneMatrix(&m_AssimpNodeNameTree, Matrix4x4::Identity);

	// 4) BoneCombMatrix に転送
	for (const auto& bone : m_BoneDictionary)
	{
		bonecombarray.ConstantBufferMemory.BoneCombMtx[bone.second.idx] =
			bone.second.Matrix.Transpose();
	}
}

void CAnimationMesh::UpdateBlended(
	BoneCombMatrix& bonecombarray,
	const aiAnimation* animFrom,
	int frameFrom,
	const aiAnimation* animTo,
	int frameTo,
	float alpha)
{
	if (!animFrom && !animTo) return;

	// 毎フレームリセット(これだとAkaiはきれいに動くが他が動かない)
	//for (auto& [name, bone] : m_BoneDictionary)
	//{
	//	bone.AnimationMatrix = Matrix4x4::Identity;
	//}
	for (auto& [name, bone] : m_BoneDictionary)
	{
		bone.AnimationMatrix = bone.BindLocalMatrix;
	}


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

	std::unordered_map<std::string, Transform> blendedLocalPose;
	BlendLocalPose(localposeFrom, localposeTo, alpha, blendedLocalPose);

	for (auto& [bonename, transform] : blendedLocalPose)
	{
		auto it = m_BoneDictionary.find(bonename);
		if (it == m_BoneDictionary.end())
			continue;

		BONE& bone = it->second;

		// ★ ここもスケールは Transform 内のものを使うか、1固定にするか選べる
		Vector3  s = transform.GetScale();      // 必要なら Vector3::One にしてもよい
		auto     r = transform.GetRotation();
		Vector3  t = transform.GetPosition();

		Matrix4x4 scalemtx = Matrix4x4::CreateScale(s);
		Matrix4x4 rotmtx = Matrix4x4::CreateFromQuaternion(r);
		Matrix4x4 transmtx = Matrix4x4::CreateTranslation(t);

		bone.AnimationMatrix = scalemtx * rotmtx * transmtx;
	}

	UpdateBoneMatrix(&m_AssimpNodeNameTree, Matrix4x4::Identity);

	for (const auto& boneKV : m_BoneDictionary)
	{
		const BONE& b = boneKV.second;
		bonecombarray.ConstantBufferMemory.BoneCombMtx[b.idx] =
			b.Matrix.Transpose();
	}
}


void CAnimationMesh::BuildLocalPoseMap(
	const aiAnimation* animationdata,
	const int& frame,
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

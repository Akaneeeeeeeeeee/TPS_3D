#include "AssetManager.h"
#include "system/CAnimationData.h"
#include "system/CAnimationMesh.h"
#include "system/CStaticMesh.h"
#include "system/CShader.h"

AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{
}


void AssetManager::Init()
{
    // ==== Shader ====
    {
        auto unlit = std::make_unique<CShader>();
        unlit->Create("shader/vertexLightingVS.hlsl",
            "shader/vertexLightingPS.hlsl");
        RegisterShader("unlightshader", std::move(unlit));

        auto anim = std::make_unique<CShader>();
        anim->Create("shader/vertexLightingOneSkinVS.hlsl",
            "shader/vertexLightingPS.hlsl");
        RegisterShader("animshader", std::move(anim));
    }

    // ==== Static Mesh ＋ Renderer ====
    {
        // 障害物用 BOX
        {
            auto boxMesh = std::make_unique<CStaticMesh>();
            boxMesh->Load("assets/model/obj/box.obj",
                "assets/model/obj/");
            RegisterMesh("obstaclebox", std::move(boxMesh));

            auto boxRenderer = std::make_unique<CStaticMeshRenderer>();
            // Mesh はさっき登録したものを取得
            auto* meshPtr = GetMesh<CStaticMesh>("obstaclebox");
            boxRenderer->Init(*meshPtr);
            RegisterMeshRenderer("obstaclebox", std::move(boxRenderer));
        }

        // Terrain 用
        {
            auto terrainMesh = std::make_unique<CStaticMesh>();
            terrainMesh->Load("assets/model/factory/factoryterrainmesh.fbx",
                "assets/model/factory");
            RegisterMesh("terrainmesh", std::move(terrainMesh));

            auto terrainRenderer = std::make_unique<CStaticMeshRenderer>();
            auto* meshPtr = GetMesh<CStaticMesh>("terrainmesh");
            terrainRenderer->Init(*meshPtr);
            RegisterMeshRenderer("terrainmesh", std::move(terrainRenderer));
        }

        // Goal 用
        {
            auto goalMesh = std::make_unique<CStaticMesh>();
            goalMesh->Load("assets/model/obj/cylinder.obj",
                "assets/model/obj");
            RegisterMesh("goalmesh", std::move(goalMesh));

            auto goalRenderer = std::make_unique<CStaticMeshRenderer>();
            auto* meshPtr = GetMesh<CStaticMesh>("goalmesh");
            goalRenderer->Init(*meshPtr);
            RegisterMeshRenderer("goalmesh", std::move(goalRenderer));
        }

        // Rock 用
        {
            auto rockMesh = std::make_unique<CStaticMesh>();
            rockMesh->Load("assets/model/Rock-Set/Rock_2/Rock_2.fbx",
                "assets/model/Rock-Set/Rock_2/Textures");
            RegisterMesh("Rock", std::move(rockMesh));

            auto rockRenderer = std::make_unique<CStaticMeshRenderer>();
            auto* meshPtr = GetMesh<CStaticMesh>("Rock");
            rockRenderer->Init(*meshPtr);
            RegisterMeshRenderer("Rock", std::move(rockRenderer));
        }
    }

    // ==== AnimationMesh / AnimationData（例：Akai）====
    {
        // スキンメッシュ
        auto akaiMesh = std::make_unique<CAnimationMesh>();
        akaiMesh->Load("assets/model/akai/akai.fbx",
            "assets/model/akai/");
        RegisterMesh("Akai", std::move(akaiMesh));

        /*auto test = std::make_unique<CAnimationMesh>();
        test->Load("assets/model/akai/uploads_files_2398597_FBX/SK_ScifiSoldierUE4.fbx",
            "assets/model/akai/uploads_files_2398597_Textures/");
        RegisterMesh("test", std::move(test));*/

        // 各アニメーション
        auto akaiIdle = std::make_unique<CAnimationData>();
        akaiIdle->LoadAnimation("assets/model/akai/Akai_Idle.fbx",
            "Akai_Idle");
        RegisterAnimationData("Akai_Idle", std::move(akaiIdle));

        auto akaiRun = std::make_unique<CAnimationData>();
        akaiRun->LoadAnimation("assets/model/akai/Akai_Run.fbx",
            "Akai_Run");
        RegisterAnimationData("Akai_Run", std::move(akaiRun));

        auto walk = std::make_unique<CAnimationData>();
        walk->LoadAnimation("assets/model/akai/Walking.fbx",
            "Walking");
        RegisterAnimationData("Walking", std::move(walk));

        // Crouching_Idle, Crouched_Walking, など
		auto crouchIdle = std::make_unique<CAnimationData>();
		crouchIdle->LoadAnimation("assets/model/akai/Crouching_Idle.fbx",
			"Crouching_Idle");
		RegisterAnimationData("Crouching_Idle", std::move(crouchIdle));

		auto crouchWalk = std::make_unique<CAnimationData>();
		crouchWalk->LoadAnimation("assets/model/akai/Crouched_Walking.fbx",
			"Crouched_Walking");
		RegisterAnimationData("Crouched_Walking", std::move(crouchWalk));

		auto rightTurn = std::make_unique<CAnimationData>();
		rightTurn->LoadAnimation("assets/model/akai/Right_Turn.fbx",
			"Right_Turn");
		RegisterAnimationData("Right_Turn", std::move(rightTurn));

		auto leftTurn = std::make_unique<CAnimationData>();
		leftTurn->LoadAnimation("assets/model/akai/Left_Turn.fbx",
			"Left_Turn");
		RegisterAnimationData("Left_Turn", std::move(leftTurn));

		auto titleSneak = std::make_unique<CAnimationData>();
		titleSneak->LoadAnimation("assets/model/akai/Title_Sneaking.fbx",
			"Title_Sneaking");
		RegisterAnimationData("Title_Sneaking", std::move(titleSneak));
		
        auto checkOverWall = std::make_unique<CAnimationData>();
        checkOverWall->LoadAnimation("assets/model/akai/Crouch_Check_Over_Wall.fbx",
			"checkOverWall");
		RegisterAnimationData("checkOverWall", std::move(checkOverWall));
        
        auto stoneThrow = std::make_unique<CAnimationData>();
        stoneThrow->LoadAnimation("assets/model/akai/StoneThrow.fbx",
			"StoneThrow");
		RegisterAnimationData("StoneThrow", std::move(stoneThrow));

        auto coverIdle = std::make_unique<CAnimationData>();
        coverIdle->LoadAnimation("assets/model/akai/Cover_Idle.fbx",
			"Cover_Idle");
		RegisterAnimationData("Cover_Idle", std::move(coverIdle));

        auto lookaround = std::make_unique<CAnimationData>();
        lookaround->LoadAnimation("assets/model/akai/Looking.fbx",
			"LookAround");
		RegisterAnimationData("LookAround", std::move(lookaround));
    }
}

void AssetManager::Uninit()
{
    m_MeshList.clear();
    m_RendererList.clear();
    m_ShaderList.clear();
    m_AnimData.clear();
}


void AssetManager::LoadAnimationData(const std::string& name, const std::filesystem::path& filepath)
{
	m_AnimationDataList[name] = std::make_unique<CAnimationData>();
	m_AnimationDataList[name]->LoadAnimation(filepath.string(), name);
}

CAnimationData* AssetManager::GetAnimationData(const std::string& name) const
{
	return m_AnimationDataList.at(name).get();
}

void AssetManager::LoadAnimationMesh(
	const std::string& name, 
	const std::filesystem::path& filepath, 
	const std::filesystem::path& texturepath)
{
	m_AnimationMeshList[name] = std::make_unique<CAnimationMesh>();
	m_AnimationMeshList[name]->Load(filepath.string(), texturepath.string());
}

CAnimationMesh* AssetManager::GetAnimationMesh(const std::string& name) const
{
	return m_AnimationMeshList.at(name).get();
}

void AssetManager::LoadStaticMesh(
	const std::string& name,
	const std::filesystem::path& filepath,
	const std::filesystem::path& texturepath)
{
	m_StaticMeshList[name] = std::make_unique<CStaticMesh>();
	m_StaticMeshList[name]->Load(filepath.string(), texturepath.string());
}

CStaticMesh* AssetManager::GetStaticMesh(const std::string& name) const
{
	return m_StaticMeshList.at(name).get();
}
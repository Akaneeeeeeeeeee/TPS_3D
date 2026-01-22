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
		// 光源なしシェーダー
        auto unlit = std::make_unique<CShader>();
        unlit->Create("shader/vertexLightingVS.hlsl",
            "shader/vertexLightingPS.hlsl");
        RegisterShader("unlightshader", std::move(unlit));

		// アニメーション用シェーダー
        auto anim = std::make_unique<CShader>();
        anim->Create("shader/vertexLightingOneSkinVS.hlsl",
            "shader/vertexLightingPS.hlsl");
        RegisterShader("animshader", std::move(anim));

        // GBuffer（静的）
        auto gbufStatic = std::make_unique<CShader>();
        gbufStatic->Create("shader/GBufferVS.hlsl", "shader/GBufferPS.hlsl");
        RegisterShader("gbuffer_static", std::move(gbufStatic));

        // GBuffer（スキン）
        auto gbufSkin = std::make_unique<CShader>();
        gbufSkin->Create("shader/GBufferOneSkinVS.hlsl", "shader/GBufferPS.hlsl");
        RegisterShader("gbuffer_skin", std::move(gbufSkin));
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
            /*auto terrainMesh = std::make_unique<CStaticMesh>();
            terrainMesh->Load("assets/model/factory/factoryterrainmesh.fbx",
                "assets/model/factory");
            RegisterMesh("terrainmesh", std::move(terrainMesh));*/

            auto terrainRenderer = std::make_unique<CStaticMeshRenderer>();
            auto* meshPtr = GetMesh<CStaticMesh>("terrainmesh");
            terrainRenderer->Init(*meshPtr);
            RegisterMeshRenderer("terrainmesh", std::move(terrainRenderer));
        }

        // Goal 用
        {
            auto goalMesh = std::make_unique<CStaticMesh>();
            goalMesh->Load("assets/model/tower/Only Tower.obj",
                "assets/model/tower");
            //goalMesh->Load("assets/model/obj/cylinder.obj",
            //    "assets/model/obj");
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
        auto woman = std::make_unique<CAnimationMesh>();
        woman->Load("assets/model/Woman/Model/SK_Fio.fbx",
            "assets/model/Woman/Textures/");
        RegisterMesh("Akai", std::move(woman));
        //auto woman = std::make_unique<CAnimationMesh>();
        //woman->Load("assets/model/Woman/Model/SK_Fio.fbx",
        //    "assets/model/Woman/Textures/");
        //RegisterMesh("Akai", std::move(woman));
        auto solider = std::make_unique<CAnimationMesh>();
        solider->Load("assets/model/SciFi_Solider/Model/SK_ScifiSoldierUE4.fbx",
            "assets/model/SciFi_Solider/Textures/");
        RegisterMesh("Solider", std::move(solider));

        // 各アニメーション
        // アイドル
        auto soliderIdle = std::make_unique<CAnimationData>();
        soliderIdle->LoadAnimation("assets/model/SciFi_Solider/Animation/UnarmedIdle01.fbx",
            "Solider_Idle");
        RegisterAnimationData("Solider_Idle", std::move(soliderIdle));
        auto akaiIdle = std::make_unique<CAnimationData>();
        akaiIdle->LoadAnimation("assets/model/Woman/Animation/Idle.fbx",
            "Akai_Idle");
        RegisterAnimationData("Akai_Idle", std::move(akaiIdle));
        //auto akaiIdle = std::make_unique<CAnimationData>();
        //akaiIdle->LoadAnimation("assets/model/Woman/Animation/Idle.fbx",
        //    "Akai_Idle");
        //RegisterAnimationData("Akai_Idle", std::move(akaiIdle));

		// 走る
        auto akaiRun = std::make_unique<CAnimationData>();
        akaiRun->LoadAnimation("assets/model/Woman/Animation/Running.fbx",
            "Akai_Run");
        RegisterAnimationData("Akai_Run", std::move(akaiRun));
        auto soliderRun = std::make_unique<CAnimationData>();
        soliderRun->LoadAnimation("assets/model/SciFi_Solider/Animation/StandardRun.fbx",
            "Solider_Run");
        RegisterAnimationData("Solider_Run", std::move(soliderRun));

		// 歩く
        auto soliderWalk = std::make_unique<CAnimationData>();
        soliderWalk->LoadAnimation("assets/model/SciFi_Solider/Animation/Walking.fbx",
            "Solider_Walking");
        RegisterAnimationData("Solider_Walking", std::move(soliderWalk));
        auto walk = std::make_unique<CAnimationData>();
        walk->LoadAnimation("assets/model/Woman/Animation/Walking.fbx",
            "Walking");
        RegisterAnimationData("Walking", std::move(walk));

        // Crouching_Idle, Crouched_Walking, など
		//auto crouchIdle = std::make_unique<CAnimationData>();
		//crouchIdle->LoadAnimation("assets/model/SciFi_Solider/Animation/CrouchingIdle.fbx",
		//	"Crouching_Idle");
		//RegisterAnimationData("Crouching_Idle", std::move(crouchIdle));
		auto crouchIdle = std::make_unique<CAnimationData>();
		crouchIdle->LoadAnimation("assets/model/Woman/Animation/CrouchingIdle.fbx",
			"Crouching_Idle");
		RegisterAnimationData("Crouching_Idle", std::move(crouchIdle));

		auto crouchWalk = std::make_unique<CAnimationData>();
		crouchWalk->LoadAnimation("assets/model/Woman/Animation/CrouchedWalking.fbx",
			"Crouched_Walking");
		RegisterAnimationData("Crouched_Walking", std::move(crouchWalk));
		//auto crouchWalk = std::make_unique<CAnimationData>();
		//crouchWalk->LoadAnimation("assets/model/SciFi_Solider/Animation/CrouchedWalking.fbx",
		//	"Crouched_Walking");
		//RegisterAnimationData("Crouched_Walking", std::move(crouchWalk));

		auto rightTurn = std::make_unique<CAnimationData>();
		rightTurn->LoadAnimation("assets/model/SciFi_Solider/Animation/Right_Turn.fbx",
			"Right_Turn");
		RegisterAnimationData("Right_Turn", std::move(rightTurn));

		auto leftTurn = std::make_unique<CAnimationData>();
		leftTurn->LoadAnimation("assets/model/SciFi_Solider/Animation/Left_Turn.fbx",
			"Left_Turn");
		RegisterAnimationData("Left_Turn", std::move(leftTurn));

		auto titleSneak = std::make_unique<CAnimationData>();
		titleSneak->LoadAnimation("assets/model/Woman/Animation/Title_Sneaking.fbx",
			"Title_Sneaking");
		RegisterAnimationData("Title_Sneaking", std::move(titleSneak));
		
        auto checkOverWall = std::make_unique<CAnimationData>();
        checkOverWall->LoadAnimation("assets/model/Woman/Animation/Crouch_Check_Over_Wall.fbx",
			"checkOverWall");
		RegisterAnimationData("checkOverWall", std::move(checkOverWall));
        
        auto stoneThrow = std::make_unique<CAnimationData>();
        stoneThrow->LoadAnimation("assets/model/Woman/Animation/StoneThrow.fbx",
			"StoneThrow");
		RegisterAnimationData("StoneThrow", std::move(stoneThrow));

        auto coverIdle = std::make_unique<CAnimationData>();
        coverIdle->LoadAnimation("assets/model/Woman/Animation/Cover_Idle.fbx",
			"Cover_Idle");
		RegisterAnimationData("Cover_Idle", std::move(coverIdle));

        auto lookaround = std::make_unique<CAnimationData>();
        lookaround->LoadAnimation("assets/model/SciFi_Solider/Animation/Looking.fbx",
			"LookAround");
		RegisterAnimationData("LookAround", std::move(lookaround));
        
        auto gunShot = std::make_unique<CAnimationData>();
        gunShot->LoadAnimation("assets/model/SciFi_Solider/Animation/GunShot.fbx",
			"GunShot");
		RegisterAnimationData("GunShot", std::move(gunShot));
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
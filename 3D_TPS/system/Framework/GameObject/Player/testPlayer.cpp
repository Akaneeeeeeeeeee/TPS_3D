#include "testPlayer.h"
#include "Component/Renderer/MeshRenderer/SkinnedMeshRenderer.h"

void TestPlayer::Init(void)
{
	auto animationmesh = AssetManager::GetInstance().GetAnimationMesh("Akai")->GenerateRenderData();

	auto rendercomp = AddComponent<SkinnedMeshRenderer>("AnimationMesh", animationmesh);
	rendercomp->Init();
}

void TestPlayer::Update(const uint64_t deltatime)
{
}

void TestPlayer::Draw(void) const
{
}

void TestPlayer::Uninit(void)
{
}
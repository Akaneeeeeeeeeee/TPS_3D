#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <assert.h>
#include <functional>
#include <locale.h>
#include <string>
#include <memory>
#include <time.h>
#include <chrono>
#include <SimpleMath.h>
#include <DirectXMath.h> //後から余裕があればこちらに変更(こっちの方が高速・複雑)
#include <wrl/client.h>


// エイリアス
using Vector3 = DirectX::SimpleMath::Vector3;
using Quaternion = DirectX::SimpleMath::Quaternion;
using Vector2 = DirectX::SimpleMath::Vector2;
using Color = DirectX::SimpleMath::Color;

#pragma warning(push)
#pragma warning(disable:4005)

#pragma warning(pop)

#pragma comment (lib,"winmm.lib")

const auto ClassName = TEXT("就職作品");     //!< ウィンドウクラス名.
const auto WindowName = TEXT("就職作品");    //!< ウィンドウ名.

constexpr uint32_t SCREEN_WIDTH = 1920;
constexpr uint32_t SCREEN_HEIGHT = 1080;

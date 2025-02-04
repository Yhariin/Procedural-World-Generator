// Precompiled Header
#pragma once

#ifndef NOMINMAX
	#define NOMINMAX
	#define _CRT_DECLARE_NONSTDC_NAMES 0
#endif

#include <Windows.h>
#include <DirectXMath.h>
#include <wrl.h>
namespace DX = DirectX;
using Microsoft::WRL::ComPtr;

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <bitset>
#include <utility>
#include <functional>
#include <random>
#include <numbers>
#include <format>
#include <cmath>
#include <algorithm>
#include <cctype>

#include "Core/Assert.h"
#include "Core/Log.h"
#include "Core/Timer.h"

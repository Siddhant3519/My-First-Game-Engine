#pragma once
// #include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"

#include <string>

#define UNUSED(x) (void)(x);
#define STATIC 
#pragma warning(disable : 26812)	// disable prefer enum class over enum warning
#pragma warning(disable : 4127)		// disable conditional expression is a constant warning

extern NamedStrings g_gameConfigBlackboard;

constexpr float PI = 3.141592653f;
constexpr float INVERSE_PI = 1.f / PI;
constexpr float ONE_EIGHTY = 180.f;
constexpr float INVERSE_ONE_EIGHTY = 1 / ONE_EIGHTY;

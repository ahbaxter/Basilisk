/**
\file   common.h
\author Andrew Baxter
\date   March 11, 2015

Includes universally-required headers and defines some rudimentary functions

*/

#ifndef BASILISK_COMMON_H
#define BASILISK_COMMON_H

#include <stdint.h>
#include <array>
#include <queue>
#include <vector>
#include <algorithm>
#include <string>
#include <memory>
#include <functional>
#include <glm/glm/glm.hpp>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX // Don't let Windows define min() or max()
#endif

#include <vulkan.h> //Auto-includes Windows headers with VK_USE_PLATFORM_WIN32_KHR defined

namespace Basilisk
{
	extern std::queue<std::string> errors;
	extern std::queue<std::string> warnings;
}

//Check Vulkan error codes

inline bool Succeeded(VkResult val) {
	return (val >= 0);
}
inline bool Failed(VkResult val) {
	return (val < 0);
}

template<typename T>
inline T Clamp(const T &val, const T &min, const T &max)
{
	return std::max(
		std::min(val, max),
		min);
}

template<typename T, typename std::enable_if<std::is_integral<T>::value>::type>
inline bool PowerOfTwo(const T &val)
{
	return (!(val <= 0) && !(val &(val - 1)));
}


template<typename T>
void SafeRelease(T *&obj)
{
	if (nullptr != obj)
	{
		obj->Release();
		delete obj;
		obj = nullptr;
	}
}

#endif
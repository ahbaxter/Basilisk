/**
\file   common.h
\author Andrew Baxter
\date   February 28, 2015

Includes universally-required headers and defines some rudimentary functions

*/

#ifndef BASILISK_COMMON_H
#define BASILISK_COMMON_H

#include <stdint.h>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX // Don't let Windows define min() or max()
#endif

#include <vulkan.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#ifdef _DEBUG
#include <D3D12SDKLayers.h>
#endif

#include "result.h" //#includes <string>

/**
Houses all components of the game engine
*/
namespace Basilisk
{
	//Check Vulkan error codes

	inline bool Succeeded(VkResult val) {
		return (val >= 0);
	}
	inline bool Failed(VkResult val) {
		return (val < 0);
	}

	//Check Direct3D error codes

	inline bool Succeeded(HRESULT val) {
		return SUCCEEDED(val);
	}
	inline bool Failed(HRESULT val) {
		return FAILED(val);
	}

	//Alternative representation for VkExtent structs

	template<typename T>
	struct Bounds2D
	{
		T width, height;
	};
	template<typename T>
	struct Bounds3D
	{
		T width, height, depth;
	};

	template<typename T>
	inline T Clamp(const T &val, const T &min, const T &max)
	{
		return std::max(
			std::min(val, max),
			min);
	}

	template<typename T>
	inline T PowerOfTwo(const T &val)
	{
		return (!(val <= 0) && !(val &(val - 1))
	}


	//For use with generic objects
	template<typename T>
	void SafeDelete(T *&obj)
	{
		if (nullptr != obj)
		{
			delete obj;
			obj = nullptr;
		}
	}
	
	//For use with Basilisk objects
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

	//For use with Direct3D 12 objects
	//All the memory allocation is under the hood
	template<typename T>
	void SafeReleaseCom(T *&obj)
	{
		if (nullptr != obj)
		{
			obj->Release();
			obj = nullptr;
		}
	}
}

#endif
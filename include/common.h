/**
\file   common.h
\author Andrew Baxter
\date   February 17, 2015

Includes universally-required headers

*/

#ifndef BASILISK_COMMON_H
#define BASILISK_COMMON_H

#include <stdint.h>

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
}

#endif
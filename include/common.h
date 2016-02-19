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
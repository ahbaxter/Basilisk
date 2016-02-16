/**
\file   common.h
\author Andrew Baxter
\date   February 16, 2015

Includes Vulkan/Direct3D 12 headers, and sets up a `define` to export/import DLLs

\todo Include Vulkan headers once it's released
*/

#pragma once


#ifdef _WINDLL
#define DYNAMIC __declspec(dllexport) 
#else
#define DYNAMIC __declspec(dllimport) 
#endif

#ifdef _VULKAN

//
//
//

#else
#include <d3d12.h>
#include <dxgi1_4.h>
#ifdef _DEBUG
#include <D3D12SDKLayers.h>
#endif
#endif
#pragma once


#ifdef _WINDLL
#define DYNAMIC __declspec(dllexport) 
#else
#define DYNAMIC __declspec(dllimport) 
#endif

#ifdef _VULKAN
#include <vulkan.h>
#else
#include <d3d12.h>
#include <dxgi1_4.h>
#ifdef _DEBUG
#include <D3D12SDKLayers.h>
#endif
#endif
#include "../include/common.h"

/**
\file   common.cpp
\author Andrew Baxter
\date   February 16, 2015
*/

#ifdef _VULKAN
/*
\todo link with vulkan libraries
*/
#else
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#endif
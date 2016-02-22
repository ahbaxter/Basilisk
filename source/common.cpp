/**
\file   common.cpp
\author Andrew Baxter
\date   February 17, 2015

Links to Direct3D 12 and Vulkan libraries

*/

#include "../include/common.h"


#ifdef ENVIRONMENT64
#pragma comment(lib, "Bin/vulkan-1.lib")
#elif ENVIRONMENT32
#pragma comment(lib, "Bin32/vulkan-1.lib")
#endif

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
/**
\file   common.cpp
\author Andrew Baxter
\date   February 16, 2015

Links to Direct3D 12 libraries

\todo Link with Vulkan libraries once it's released
*/

#include "../include/common.h"

#ifdef _VULKAN

//
//
//

#else
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#endif
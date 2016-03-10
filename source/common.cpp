/**
\file   common.cpp
\author Andrew Baxter
\date   March 9, 2015

Links to Vulkan binaries

*/

#include "../include/common.h"


#ifdef ENVIRONMENT64
#pragma comment(lib, "Bin/vulkan-1.lib")
#elif ENVIRONMENT32
#pragma comment(lib, "Bin32/vulkan-1.lib")
#endif

std::queue<std::string> Basilisk::errors;
std::queue<std::string> Basilisk::warnings;
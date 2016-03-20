/**
\file   instance.cpp
\author Andrew Baxter
\date   March 18, 2016

Defines how Vulkan should start up

*/

#include <sstream>
#include "rendering/backend.h"

using namespace Vulkan;



#ifdef _DEBUG
uint32_t layerCount() {
	return 6;
}
const char *lyrNames[6] = {
	"VK_LAYER_LUNARG_threading",
	"VK_LAYER_LUNARG_draw_state",
	"VK_LAYER_LUNARG_image",
	"VK_LAYER_LUNARG_mem_tracker",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_param_checker"
};
const char **layerNames() {
	return lyrNames;
}
#else
uint32_t layerCount() {
	return 0;
}
const char **layerNames() {
	return nullptr;
}
#endif
uint32_t instExtensionCount() {
	return 2;
}
const char *extNames[2] = {
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	//VK_KHR_DISPLAY_EXTENSION_NAME
};
const char **instExtensionNames() {
	return extNames;
}
uint32_t devExtensionCount() {
	return 1;
}
const char *devExtNames[1] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	//VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME
};
const char **devExtensionNames() {
	return devExtNames;
}



void Instance::Release()
{
	if (m_instance)
	{
		vkDestroyInstance(m_instance, nullptr);
		m_instance = nullptr;
	}
}

Instance::Instance() : m_instance(VK_NULL_HANDLE)
{}


std::shared_ptr<Instance> Vulkan::Initialize(const std::string &appName, uint32_t appVersion)
{
	std::shared_ptr<Instance> out(new Instance,
		[](Instance *ptr) { //Custom deallocator
			ptr->Release();
			delete ptr;
			ptr = nullptr;
		}
	);

	//Should I let them specify application version as well?
	VkApplicationInfo appInfo =
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,          //Must be NULL
		appName.c_str(),  //Application name
		appVersion,       //Application version
		"Basilisk",       //Engine name
		1,                //Engine version
		VK_API_VERSION    //API version
	};

	VkInstanceCreateInfo instanceInfo =
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,               //Must be NULL
		0,                     //No flags
		&appInfo,              //Let the GPU know who we are
		layerCount(),          //Number of layers
		layerNames(),          //Which layers we're using
		instExtensionCount(),  //Number of extensions
		instExtensionNames()   //Which extensions we're using
	};

	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &out->m_instance);
	if (Failed(result))
	{
		Basilisk::errors.push("Vulkan::Initialize() could not create a Vulkan Instance");
		return nullptr;
	}

//Normally I wouldn't use macros, but it actually makes sure I don't mistype the extension string names (in addition to simplifying the code)
#define GET_PROCADDR(name) \
	out->pfn##name = reinterpret_cast<PFN_vk##name>(vkGetInstanceProcAddr(out->m_instance, "vk"#name)); \
	if (!out->pfn##name) { Basilisk::errors.push("Vulkan::Initialize() could not find the proc address for vk"#name); return nullptr; }

	//Store VK_KHR_surface function pointers
	GET_PROCADDR(DestroySurfaceKHR);
	GET_PROCADDR(GetPhysicalDeviceSurfaceSupportKHR);
	GET_PROCADDR(GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_PROCADDR(GetPhysicalDeviceSurfaceFormatsKHR);
	GET_PROCADDR(GetPhysicalDeviceSurfacePresentModesKHR);
	//Store VK_KHR_win32_surface function pointers
	GET_PROCADDR(CreateWin32SurfaceKHR);
	GET_PROCADDR(GetPhysicalDeviceWin32PresentationSupportKHR);

#undef GET_PROCADDR


	return out;
}

uint32_t Instance::FindGpus()
{
	VkResult result;
	uint32_t count;

	//Count the devices
	result = vkEnumeratePhysicalDevices(m_instance, &count, nullptr);
	if (Failed(result))
	{
		Basilisk::errors.push("Vulkan::Instance::::FindGpus() could not count physical devices");
		return 0;
	}
	//Resize our arrays to store all devices
	m_gpus.resize(count);
	m_gpuProps.resize(count);
	//Store the devices
	result = vkEnumeratePhysicalDevices(m_instance, &count, m_gpus.data());
	if (Failed(result))
	{
		Basilisk::errors.push("Vulkan::Instance::::FindGpus() could not list physical devices");
		return 0;
	}

	//Store device details
	for (uint32_t i = 0; i < count; ++i)
	{
		//Get the device's supported features list
		vkGetPhysicalDeviceFeatures(m_gpus[i], &m_gpuProps[i].features);
		//Get the device's general properties
		vkGetPhysicalDeviceProperties(m_gpus[i], &m_gpuProps[i].props);
		//Get the device's memory properties
		vkGetPhysicalDeviceMemoryProperties(m_gpus[i], &m_gpuProps[i].memProps);
		//Get descriptions for the device's queue families
		uint32_t numQueues;
		vkGetPhysicalDeviceQueueFamilyProperties(m_gpus[i], &numQueues, nullptr);
		if (numQueues >= 1)
		{
			m_gpuProps[i].queueDescs.resize(numQueues);
			vkGetPhysicalDeviceQueueFamilyProperties(m_gpus[i], &numQueues, m_gpuProps[i].queueDescs.data());
		}
		//Find an optimal depth buffer format
		std::array<VkFormat, 5> depthFormats = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,  //32-bit depth, 8-bit stencil
			VK_FORMAT_D32_SFLOAT,          //32-bit depth, no stencil
			VK_FORMAT_D24_UNORM_S8_UINT,   //24-bit depth, 8-bit stencil
			VK_FORMAT_D16_UNORM_S8_UINT,   //16-bit depth, 8-bit stencil
			VK_FORMAT_D16_UNORM            //16-bit depth, no stencil
		};
		m_gpuProps[i].depthFormat = VK_FORMAT_UNDEFINED;
		for (VkFormat format : depthFormats)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(m_gpus[i], format, &formatProps);
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				m_gpuProps[i].depthFormat = format;
				m_gpuProps[i].depthTiling = VK_IMAGE_TILING_OPTIMAL;
				break;
			}
		}
		if (VK_FORMAT_UNDEFINED == m_gpuProps[i].depthFormat)
		{
			Basilisk::warnings.push("Vulkan::Instance::::FindGpus() could not find a depth format for GPU at index " + std::to_string(i));
		}
	}

	return count;
}

const GpuProperties *Instance::GetGpuProperties(uint32_t gpuIndex)
{
	if (m_gpuProps.size() < 0 || gpuIndex > m_gpuProps.size() - 1)
	{
		Basilisk::warnings.push("Vulkan::Instance::GetGpuProperties() called on a nonexisted GPU (at index " + std::to_string(gpuIndex) + ")");
		return nullptr;
	}
	else
		return &m_gpuProps[gpuIndex];
}
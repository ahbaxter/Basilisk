/**
\file   backend.cpp
\author Andrew Baxter
\date   March 9, 2016

Defines the behavior of Vulkan rendering backends

*/

#include <sstream>
#include "rendering/backend.h"
#include "rendering/initializers.h"

using namespace Vulkan;

/* Validates an HWND and HINSTANCE. Does not have a home at the moment, but will be useful.
#ifndef BASILISK_FINAL_BUILD
if (WAIT_TIMEOUT != WaitForSingleObject(platformInfo.connection, 0)) //Uses a bit of a hack to figure out if the process is running
{ //With a time-out of zero, anything other than WAIT_TIMEOUT means the process doesn't exist
	Basilisk::errors.push("Vulkan::Instance::Initialize()::platformInfo.connection is not a valid process";
	return Result::IllegalArgument;
}
if (!IsWindow(platformInfo.window))
{
	Basilisk::errors.push("Vulkan::Instance::Initialize().platformInfo.window is not a valid window";
	return Result::IllegalArgument;
}
#endif
*/

#pragma region Configuration

constexpr uint32_t Vulkan::layerCount() {
	return 0;
}
constexpr const char **Vulkan::layerNames() {
	return nullptr;
}

constexpr uint32_t Vulkan::extensionCount() {
	return 5;
}

const char *extNames[Vulkan::extensionCount()] = {
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_DISPLAY_EXTENSION_NAME,
	VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME
};

constexpr const char **Vulkan::extensionNames() {
	return extNames;
}

constexpr uint32_t Vulkan::apiVersion() {
	return VK_API_VERSION;
}
#pragma endregion

Instance::Instance() : m_instance(VK_NULL_HANDLE),
	pfnDestroySurfaceKHR(nullptr),
	pfnGetPhysicalDeviceSurfaceSupportKHR(nullptr),
	pfnGetPhysicalDeviceSurfaceCapabilitiesKHR(nullptr),
	pfnGetPhysicalDeviceSurfaceFormatsKHR(nullptr),
	pfnGetPhysicalDeviceSurfacePresentModesKHR(nullptr),
	pfnCreateWin32SurfaceKHR(nullptr),
	pfnGetPhysicalDeviceWin32PresentationSupportKHR(nullptr)
{
	m_windowTarget = {};
	m_monitorTarget = {};
}

std::shared_ptr<Instance> Vulkan::Initialize(const std::string &appName, uint32_t appVersion)
{
	auto out = std::make_shared<Instance>(new Instance, 
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
		apiVersion()      //API version
	};

	VkInstanceCreateInfo instanceInfo =
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,           //Must be NULL
		0,                 //No flags
		&appInfo,          //Let the GPU know who we are
		layerCount(),      //Number of layers
		layerNames(),      //Which layers we're using
		extensionCount(),  //Number of extensions
		extensionNames()   //Which extensions we're using
	};

	//           Rely on automatic memory allocation ----v
	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &out->m_instance);
	if (Failed(result))
	{
		Basilisk::errors.push("Vulkan::Initialize() could not create a Vulkan Instance");
		return nullptr;
	}

	//Grab Vulkan callbacks for surfaces
	out->pfnDestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(vkGetInstanceProcAddr(out->m_instance, "vkDestroySurfaceKHR"));
	out->pfnGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(vkGetInstanceProcAddr(out->m_instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
	out->pfnGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(vkGetInstanceProcAddr(out->m_instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
	out->pfnGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(vkGetInstanceProcAddr(out->m_instance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
	out->pfnGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(vkGetInstanceProcAddr(out->m_instance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
	//Grab Vulkan callbacks for Win32 surfaces
	out->pfnCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(out->m_instance, "vkCreateWin32SurfaceKHR"));
	out->pfnGetPhysicalDeviceWin32PresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(vkGetInstanceProcAddr(out->m_instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR"));


	return out;
}

void Instance::Release()
{
	//Memory for presentable surfaces is handled under the hood by Vulkan. vkDestroySurfaceKHR() will not work on them.

	if (VK_NULL_HANDLE != m_instance)
	{
		vkDestroyInstance(m_instance, nullptr);
		m_instance = nullptr;
	}
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

std::shared_ptr<Device> Instance::CreateDevice(uint32_t gpuIndex)
{
	if (m_gpus.size() == 0 || gpuIndex > m_gpus.size() - 1)
	{
		Basilisk::errors.push("Vulkan::Instance::::CreateDevice()::gpuIndex is out of GPU array bounds");
		return nullptr;
	}

	if (VK_VERSION_MAJOR(apiVersion()) >= VK_VERSION_MAJOR(m_gpuProps[gpuIndex].props.apiVersion))
	{
		std::stringstream message("Vulkan may not operate properly without compatible API support. Application requires API version ");
		message << VK_VERSION_MAJOR(apiVersion()) << "." << VK_VERSION_MINOR(apiVersion()) << "." << VK_VERSION_PATCH(apiVersion());
		message << " but the selected GPU has version ";
		message << VK_VERSION_MAJOR(m_gpuProps[gpuIndex].props.apiVersion) << "." << VK_VERSION_MINOR(m_gpuProps[gpuIndex].props.apiVersion) << "." << VK_VERSION_PATCH(m_gpuProps[gpuIndex].props.apiVersion);

		Basilisk::warnings.push(message.str());
	}

	//Meets all prerequisites

	auto out = std::make_shared<Device>(new Device,
		[](Device *ptr) {
			ptr->Release();
			delete ptr;
			ptr = nullptr;
		}
	);
	out->m_gpuProps = m_gpuProps[gpuIndex];

	//
	////Grab a VkSurface object from the provided window
	//

	VkResult res;
	VkWin32SurfaceCreateInfoKHR surface_info = {
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		nullptr,                   //Reserved
		0,                         //No flags
		m_platformInfo.connection, //Handle to the process
		m_platformInfo.window      //Handle to the window
	};

	res = pfnCreateWin32SurfaceKHR(m_instance, &surface_info, nullptr, &out->m_windowSurface.surface);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not connect to the provided window";
		return nullptr;
	}

	//
	////Find a graphics queue which supports present
	//

	std::vector<VkBool32>supportsPresent(m_gpuProps[gpuIndex].queueDescs.size());
	for (uint32_t i = 0; i < m_gpuProps[gpuIndex].queueDescs.size(); ++i)
	{
		res = pfnGetPhysicalDeviceSurfaceSupportKHR(m_gpus[gpuIndex], i, out->m_windowSurface.surface, &supportsPresent[i]);
		if (Failed(res))
		{ //Error getting the GPU's surface support
			Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not query the GPU's surface support";
			return nullptr;
		}
	}

	uint32_t presentQueueIndex = std::numeric_limits<uint32_t>::max();
	for (uint32_t i = 0; i < m_gpuProps[gpuIndex].queueDescs.size(); ++i)
	{
		if ((m_gpuProps[gpuIndex].queueDescs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supportsPresent[i])
		{ //Found a queue that fits our criteria
			presentQueueIndex = i;
			break;
		}
	}
	if (std::numeric_limits<uint32_t>::max() == presentQueueIndex)
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not find a present-capable graphics queue");
		return nullptr;
	}

	uint32_t renderQueueIndex = std::numeric_limits<uint32_t>::max();
	for (uint32_t i = 0; i < m_gpuProps[gpuIndex].queueDescs.size(); ++i)
	{
		if ((m_gpuProps[gpuIndex].queueDescs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && i != presentQueueIndex)
		{ //Found a queue that fits our criteria
			renderQueueIndex = i;
			break;
		}
	}
	if (std::numeric_limits<uint32_t>::max() == renderQueueIndex)
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not find a second graphics queue");
		return nullptr;
	}

	//
	////Get the list of VkFormats supported by that surface, and store the one it likes most
	//

	uint32_t formatCount;
	res = pfnGetPhysicalDeviceSurfaceFormatsKHR(m_gpus[gpuIndex], out->m_windowSurface.surface, &formatCount, nullptr);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not count window surface formats");
		return nullptr;
	}

	std::vector<VkSurfaceFormatKHR> surfFormats(formatCount);
	res = pfnGetPhysicalDeviceSurfaceFormatsKHR(m_gpus[gpuIndex], out->m_windowSurface.surface, &formatCount, surfFormats.data());
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not retrieve window surface formats");
		return nullptr;
	}

	if (formatCount == 0)
	{ //Surface isn't playing nice
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not find a preferred format for the window surface");
		return nullptr;
	}
	else if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
	{ //Surface has no preferred format
		out->m_windowSurface.colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{ //Surface has a preferred format
		out->m_windowSurface.colorFormat = surfFormats[0].format;
	}

	//
	////Grab the GPU's surface capabilities and present modes
	//

	res = pfnGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpus[gpuIndex], out->m_windowSurface.surface, &out->m_windowSurface.caps);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not retrieve the GPU's surface capabilities");
		return nullptr;
	}

	uint32_t presentModeCount;
	res = pfnGetPhysicalDeviceSurfacePresentModesKHR(m_gpus[gpuIndex], out->m_windowSurface.surface, &presentModeCount, nullptr);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not count the GPU's present modes");
		return nullptr;
	}
	if (presentModeCount == 0)
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not detect any present modes for the provided GPU");
		return nullptr;
	}

	out->m_windowSurface.presentModes.resize(presentModeCount);

	res = pfnGetPhysicalDeviceSurfacePresentModesKHR(m_gpus[gpuIndex], out->m_windowSurface.surface, &presentModeCount, out->m_windowSurface.presentModes.data());
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not list the GPU's present modes");
		return nullptr;
	}

	//
	////Create the device
	//

	float queue_priorities[1] = { 0.0 };
	VkDeviceQueueCreateInfo queue_info[2] =
	{
		{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,           //Reserved
			0,                 //No flags
			renderQueueIndex,  //This is the rendering queue
			1,                 //Queue count
			queue_priorities   //Queue priorities
		},
		{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,            //Reserved
			0,                  //No flags
			presentQueueIndex,  //This is the presentation queue
			1,                  //Queue count
			queue_priorities    //Queue priorities
		}
	};

	VkDeviceCreateInfo device_info = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,           //Reserved
		0,                 //Flags
		2,                 //Queue count
		queue_info,        //Queue properties
		layerCount(),      //Layer count
		layerNames(),      //Layer types
		extensionCount(),  //Extension count
		extensionNames(),  //Extension names
		nullptr            //Not enabling any device features
	};
	
	if (Failed(vkCreateDevice(m_gpus[gpuIndex], &device_info, nullptr, &out->m_device) ))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() failed to create the device");
		return nullptr;
	}
	//Store Vulkan extension function pointers
	//VK_KHR_swapchain function pointers
	out->pfnCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(vkGetDeviceProcAddr(out->m_device, "vkCreateSwapchainKHR"));
	out->pfnDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(vkGetDeviceProcAddr(out->m_device, "vkDestroySwapchainKHR"));
	out->pfnGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(vkGetDeviceProcAddr(out->m_device, "vkGetSwapchainImagesKHR"));
	out->pfnAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(out->m_device, "vkAcquireNextImageKHR"));
	out->pfnQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(vkGetDeviceProcAddr(out->m_device, "vkQueuePresentKHR"));
	//VK_KHR_display function pointers
	out->pfnGetPhysicalDeviceDisplayPropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPropertiesKHR>(vkGetDeviceProcAddr(out->m_device, "vkGetPhysicalDeviceDisplayPropertiesKHR"));
	out->pfnGetPhysicalDeviceDisplayPlanePropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR>(vkGetDeviceProcAddr(out->m_device, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR"));
	out->pfnGetDisplayPlaneSupportedDisplaysKHR = reinterpret_cast<PFN_vkGetDisplayPlaneSupportedDisplaysKHR>(vkGetDeviceProcAddr(out->m_device, "vkGetDisplayPlaneSupportedDisplaysKHR"));
	out->pfnGetDisplayModePropertiesKHR = reinterpret_cast<PFN_vkGetDisplayModePropertiesKHR>(vkGetDeviceProcAddr(out->m_device, "vkGetDisplayModePropertiesKHR"));
	out->pfnCreateDisplayModeKHR = reinterpret_cast<PFN_vkCreateDisplayModeKHR>(vkGetDeviceProcAddr(out->m_device, "vkCreateDisplayModeKHR"));
	out->pfnGetDisplayPlaneCapabilitiesKHR = reinterpret_cast<PFN_vkGetDisplayPlaneCapabilitiesKHR>(vkGetDeviceProcAddr(out->m_device, "vkGetDisplayPlaneCapabilitiesKHR"));
	out->pfnCreateDisplayPlaneSurfaceKHR = reinterpret_cast<PFN_vkCreateDisplayPlaneSurfaceKHR>(vkGetDeviceProcAddr(out->m_device, "vkCreateDisplayPlaneSurfaceKHR"));
	//VK_KHR_display_swapchain function pointers
	out->pfnCreateSharedSwapchainsKHR = reinterpret_cast<PFN_vkCreateSharedSwapchainsKHR>(vkGetDeviceProcAddr(out->m_device, "vkCreateSharedSwapchainsKHR"));

	//
	////Create the device's command pool(s)
	//

	//Rendering queue
	VkCommandPoolCreateInfo render_pool_info = Init<VkCommandPoolCreateInfo>::Create(renderQueueIndex);

	res = vkCreateCommandPool(out->m_device, &render_pool_info, nullptr, &out->m_commandPools[Device::renderQueue]);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() failed to create the command pool");
		return nullptr;
	}

	//Presentation queue
	VkCommandPoolCreateInfo present_pool_info = Init<VkCommandPoolCreateInfo>::Create(presentQueueIndex);

	res = vkCreateCommandPool(out->m_device, &present_pool_info, nullptr, &out->m_commandPools[Device::presentQueue]);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() failed to create the command pool");
		return nullptr;
	}
	
	//
	////Store the queues we created above in the device
	//

	vkGetDeviceQueue(out->m_device, renderQueueIndex, 0, &out->m_queues[Device::renderQueue]);
	vkGetDeviceQueue(out->m_device, presentQueueIndex, 0, &out->m_queues[Device::presentQueue]);


	return out;
}

#pragma endregion

//////////////////////////
//        DEVICE        //
//////////////////////////


Device::Device() : m_device(VK_NULL_HANDLE), m_parent(nullptr),
	pfnCreateSwapchainKHR(nullptr),
	pfnDestroySwapchainKHR(nullptr),
	pfnGetSwapchainImagesKHR(nullptr),
	pfnAcquireNextImageKHR(nullptr),
	pfnQueuePresentKHR(nullptr),
	pfnGetPhysicalDeviceDisplayPropertiesKHR(nullptr),
	pfnGetPhysicalDeviceDisplayPlanePropertiesKHR(nullptr),
	pfnGetDisplayPlaneSupportedDisplaysKHR(nullptr),
	pfnGetDisplayModePropertiesKHR(nullptr),
	pfnCreateDisplayModeKHR(nullptr),
	pfnGetDisplayPlaneCapabilitiesKHR(nullptr),
	pfnCreateDisplayPlaneSurfaceKHR(nullptr),
	pfnCreateSharedSwapchainsKHR(nullptr)
{
	m_gpuProps = {};
}

void Device::Release() {
	if (VK_NULL_HANDLE != m_device)
	{
		vkDestroyDevice(m_device, nullptr);
		m_device = VK_NULL_HANDLE;
	}
}

std::shared_ptr<SwapChain> Device::CreateSwapChain(const std::shared_ptr<CommandBuffer> &setup, glm::uvec2 resolution, uint32_t numBuffers)
{
#ifndef BASILISK_FINAL_BUILD
	if (nullptr == setup)
	{
		Basilisk::errors.push("Vulkan::Device::CreateSwapChain()::setupBuffer must not be a null pointer");
		return nullptr;
	}
#endif

	//Meets all prerequisites
	
	//
	////Calculate the swap chain's resolution (may differ from the `resolution` argument)
	//

	VkExtent2D swapChainRes;
	if (static_cast<uint32_t>(-1) == m_windowSurface.caps.currentExtent.width)
	{ //Surface size is undefined
		swapChainRes.width = Clamp(resolution.x,
			m_windowSurface.caps.minImageExtent.width,
			m_windowSurface.caps.maxImageExtent.width);

		swapChainRes.height = Clamp(resolution.y,
			m_windowSurface.caps.minImageExtent.height,
			m_windowSurface.caps.maxImageExtent.height);
	}
	else
	{ //Surface size is already defined
		swapChainRes = m_windowSurface.caps.currentExtent;
	}

	//
	////Determine the optimal properties for the swap chain
	//

	//Determine the present mode
	VkPresentModeKHR swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR; //Default to the always-available FIFO present mode
	for (VkPresentModeKHR i : m_windowSurface.presentModes)
	{
		if (VK_PRESENT_MODE_MAILBOX_KHR == i)
		{ //This is the lowest-latency non-tearing present mode
			swapChainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break; //We can't get any better than this
		}
		if (VK_PRESENT_MODE_IMMEDIATE_KHR == i)
		{ //The fastest present mode, but it tears -- hold out for mailbox mode
			swapChainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	//Determine the number of Image backbuffers to use (caller may have to settle)
	numBuffers = Clamp(numBuffers,
		m_windowSurface.caps.minImageCount + 1, //+ 1 enables non-blocking calls to `vkAcquireNextImageKHR()` in mailbox mode
		m_windowSurface.caps.maxImageCount);

	//Make sure the surface has a pre-transform (even if it's just the identity matrix)
	VkSurfaceTransformFlagBitsKHR preTransform;
	if (m_windowSurface.caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	} else {
		preTransform = m_windowSurface.caps.currentTransform;
	}

	//
	////Create the swap chain
	//

	auto out = std::make_shared<SwapChain>(new SwapChain,
		[=](SwapChain *ptr) {
			ptr->Release(m_device);
			delete ptr;
			ptr = nullptr;
		}
	);

	VkSwapchainCreateInfoKHR swapchain_info = 
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,                           //Reserved
		0,                                 //No flags
		m_windowSurface.surface,           //Target surface
		numBuffers,                        //Number of back buffers
		m_windowSurface.colorFormat,       //Surface format
		VK_COLORSPACE_SRGB_NONLINEAR_KHR,  //Color space
		swapChainRes,                      //Resolution
		1,                                 //Image layers
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, //Image usage
		VK_SHARING_MODE_EXCLUSIVE,         //Image sharing mode
		0,                                 //Queue family index count
		nullptr,                           //Queue family indices
		preTransform,                      //Pre-present transformation
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, //Blending. Opaque because back buffers are never blended
		swapChainPresentMode,              //Present mode
		true,                              //Clipped
		VK_NULL_HANDLE                     //Old swap chain
	};

	VkResult res = pfnCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &out->m_swapChain);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateSwapChain() could not create the swap chain");
		return nullptr;
	}

	//
	////Store the back buffers
	//

	res = pfnGetSwapchainImagesKHR(m_device, out->m_swapChain, &numBuffers, nullptr);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateSwapChain() could not count the swap chain's back buffers");
		return nullptr;
	}

	out->m_backBuffers.resize(numBuffers);
	out->m_backBufferViews.resize(numBuffers);

	res = pfnGetSwapchainImagesKHR(m_device, out->m_swapChain, &numBuffers, out->m_backBuffers.data());
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateSwapChain() could not count the swap chain's back buffers");
		return nullptr;
	}

	//
	////Create views for each back buffer
	//

	for (uint32_t i = 0; i < out->m_backBuffers.size(); ++i)
	{
		VkImageViewCreateInfo color_image_view = Init<VkImageViewCreateInfo>::Texture2D(out->m_backBuffers[i], m_windowSurface.colorFormat);

		res = vkCreateImageView(m_device, &color_image_view, nullptr, &out->m_backBufferViews[i]);
		if (Failed(res))
		{
			Basilisk::errors.push("Vulkan::Device::CreateSwapChain() could not create a view for all back buffers");
			return nullptr;
		}

		//Set the image layout to depth stencil optimal
		setup->SetImageLayout(out->m_backBuffers[i],
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}


	return out;
}

std::shared_ptr<FrameBuffer> Device::CreateFrameBuffer(glm::uvec2 resolution, const std::vector<VkFormat> &colorFormats, bool enableDepth)
{
#ifndef BASILISK_FINAL_BUILD
	if (0 == colorFormats.size() && !enableDepth)
	{
		Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() must have at least one image attachment");
		return nullptr;
	}
#endif

	//Meets all prerequisites

	//Preparations...
	auto out = std::make_shared<FrameBuffer>(new FrameBuffer,
		[=](FrameBuffer *ptr) {
			ptr->Release(m_device);
			delete ptr;
			ptr = nullptr;
		}
	);

	out->m_resolution = resolution;

	uint32_t numAttachments = colorFormats.size() + (enableDepth ? 1 : 0);
	out->ResizeVectors(numAttachments);
	std::vector<VkAttachmentDescription> attachmentDescs(numAttachments);
	std::vector<VkAttachmentReference> attachmentRefs(numAttachments);

	uint32_t i;

	//Create and store the images used in the frame buffer
	VkImageCreateInfo image_create_info = Init<VkImageCreateInfo>::Texture2D(resolution, VK_FORMAT_UNDEFINED, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	for (i = 0; i < colorFormats.size(); ++i)
	{ //Color attachment
		image_create_info.format = static_cast<VkFormat>(colorFormats[i]);
		if (Failed( vkCreateImage(m_device, &image_create_info, nullptr, &out->m_images[i]) ))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not create all color images");
			return nullptr;
		}
	}
	for (; i < numAttachments; ++i)
	{ //Depth attachment
		image_create_info.format = static_cast<VkFormat>(colorFormats[i]);
		image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		image_create_info.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		if (Failed( vkCreateImage(m_device, &image_create_info, nullptr, &out->m_images[i]) ))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not create all depth images");
			return nullptr;
		}
	}
	
	//Allocate and store device memory for the images
	VkMemoryAllocateInfo memAlloc = Init<VkMemoryAllocateInfo>::Base();
	VkMemoryRequirements memReqs;
	for (i = 0; i < numAttachments; ++i)
	{ //Does not need to be split up between color and depth stencil attachments
		vkGetImageMemoryRequirements(m_device, out->m_images[i], &memReqs);
		memAlloc.allocationSize = memReqs.size;
		if (!MemoryTypeFromProps(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memAlloc.memoryTypeIndex))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not determine appropriate memory type for all images");
			return nullptr;
		}
		if (Failed( vkAllocateMemory(m_device, &memAlloc, nullptr, &out->m_memory[i]) ))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not allocate memory for all images");
			return nullptr;
		}
		if (Failed( vkBindImageMemory(m_device, out->m_images[i], out->m_memory[i], 0) ))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not bind memory for all images");
			return nullptr;
		}
	}

	//Create and store the image views
	VkImageViewCreateInfo view_create_info = Init<VkImageViewCreateInfo>::Texture2D(VK_NULL_HANDLE, VK_FORMAT_UNDEFINED);
	for (i = 0; i < colorFormats.size(); ++i)
	{ //Color attachment
		view_create_info.image = out->m_images[i];
		view_create_info.format = static_cast<VkFormat>(colorFormats[i]);
		if (Failed( vkCreateImageView(m_device, &view_create_info, nullptr, &out->m_views[i]) ))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not create all color image views");
			return nullptr;
		}
	}
	for (; i < numAttachments; ++i)
	{ //Depth attachment
		view_create_info.image = out->m_images[i];
		view_create_info.format = static_cast<VkFormat>(m_gpuProps.depthFormat);
		view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		if (Failed( vkCreateImageView(m_device, &view_create_info, nullptr, &out->m_views[i]) ))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not create all the depth stencil image view");
			return nullptr;
		}
	}

	//Create the attachment descriptions, and store the formats
	for (i = 0; colorFormats.size(); ++i)
	{ //Color attachment
		attachmentDescs[i] = Init<VkAttachmentDescription>::Color(static_cast<VkFormat>(colorFormats[i]));
		attachmentRefs[i] = { i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		out->m_formats[i] = static_cast<VkFormat>(colorFormats[i]);
	}
	for (; i < numAttachments; ++i)
	{ //Depth attachment
		attachmentDescs[i] = Init<VkAttachmentDescription>::DepthStencil(m_gpuProps.depthFormat);
		attachmentRefs[i] = { i, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		out->m_formats[i] = m_gpuProps.depthFormat;
	}

	//Create the render pass
	VkSubpassDescription subpassDesc = Init<VkSubpassDescription>::Base();
	subpassDesc.colorAttachmentCount = colorFormats.size();
	subpassDesc.pColorAttachments = &attachmentRefs[0];
	if (enableDepth)
		subpassDesc.pDepthStencilAttachment = &attachmentRefs[colorFormats.size() - 1]; //The last attachment is depth
	
	
	VkRenderPassCreateInfo rp_create_info = Init<VkRenderPassCreateInfo>::Base();
	rp_create_info.attachmentCount = numAttachments;
	rp_create_info.pAttachments = attachmentDescs.data();
	rp_create_info.subpassCount = 1;
	rp_create_info.pSubpasses = &subpassDesc;
	
	if (Failed( vkCreateRenderPass(m_device, &rp_create_info, nullptr, &out->m_renderPass) ))
	{
		Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not create the render pass");
		return nullptr;
	}

	VkFramebufferCreateInfo fb_create_info = Init<VkFramebufferCreateInfo>::Create(out->m_renderPass, resolution, out->m_views);
	
	if (Failed(vkCreateFramebuffer(m_device, &fb_create_info, nullptr, &out->m_frameBuffer)))
	{
		Basilisk::errors.push("Basilisk::Device::CreateFrameBuffer() could not create the frame buffer");
		return nullptr;
	}


	return out;
}

bool Device::MemoryTypeFromProps(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex)
{
	//Search memoryTypes to find first index with those properties
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {
			//Type is available; does it match user properties?
			if ((m_gpuProps.memProps.memoryTypes[i].propertyFlags &
				requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	//No memory types matched; return failure
	return false;
}

/* Separate into Device::CreatePipelineLayout and Device::CreateShader functions
std::shared_ptr<GraphicsPipeline> Device::CreateGraphicsPipeline(const std::shared_ptr<PipelineLayout> &layout, const std::vector<Shader> &shaders)
{
#ifndef BASILISK_FINAL_BUILD

#endif

	//Passes all prerequisites

	auto out = std::make_shared<GraphicsPipeline>(new GraphicsPipeline,
		[=](GraphicsPipeline *ptr) {
			ptr->Release(m_device);
			delete ptr;
			ptr = nullptr;
		}
	);
	VkResult res;

	std::vector<VkDescriptorSetLayoutBinding> bindings(layout->m_descriporsLayout.size());
	for (uint32_t i = 0; i < bindings.size(); ++i)
	{
		bindings[i].binding = layout[i].bindPoint;
		bindings[i].descriptorType = static_cast<VkDescriptorType>(layout[i].type);
		bindings[i].descriptorCount = 1;
		bindings[i].stageFlags = static_cast<VkShaderStageFlags>(layout[i].visibility);
		bindings[i].pImmutableSamplers = nullptr;
	}

	VkDescriptorSetLayoutCreateInfo desc_layout_info = Init<VkDescriptorSetLayoutCreateInfo>::Create(bindings);
	res = vkCreateDescriptorSetLayout(m_device, &desc_layout_info, nullptr, &out->m_descriptorSetLayout);
	if (Failed(res))
	{
		Basilisk::errors.push("Basilisk::Device::CreateGraphicsPipeline() could not create the descriptor set layout";
		SafeRelease(out);
		return nullptr;
	}

	VkPipelineLayoutCreateInfo pipeline_layout_info = Init<VkPipelineLayoutCreateInfo>::Create({ out->m_descriptorSetLayout });
	res = vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr, &out->m_pipelineLayout);
	if (Failed(res))
	{
		Basilisk::errors.push("Basilisk::Device::CreateGraphicsPipeline() could not create the pipeline layout";
		SafeRelease(out);
		return nullptr;
	}


	return Result::Success;
}
*/
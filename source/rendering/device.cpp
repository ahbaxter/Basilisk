/**
\file   device.cpp
\author Andrew Baxter
\date   February 28, 2016

Defines the behavior of Vulkan and D3D12 rendering backends

*/

#include <memory> //Used for smart pointers
#include "rendering/device.h"

using namespace Basilisk;


//////////////////////////
//      INSTANCE        //
//////////////////////////


//If you add a layer, don't forget to change the layer count
const char * const* VulkanInstance::layerNames = nullptr;

//If you add an extension, don't forget to change the extension count
const char* VulkanInstance::extensionNames[VulkanInstance::extensionCount] = {
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_DISPLAY_EXTENSION_NAME
};

D3D12Instance::D3D12Instance() {
	m_platformInfo = { };
}

VulkanInstance::VulkanInstance() : m_instance(nullptr) {
	m_platformInfo = {};
}

Result D3D12Instance::Initialize(const PlatformInfo &platformInfo, const std::string &appName)
{
#ifndef BASILISK_FINAL_BUILD
	if (WAIT_TIMEOUT != WaitForSingleObject(platformInfo.connection, 0)) //Uses a bit of a hack to figure out if the process is running
	{ //With a time-out of zero, anything other than WAIT_TIMEOUT means the process doesn't exist
		Basilisk::errorMessage = "Basilisk::D3D12Instance::Initialize()::platformInfo.connection is not a valid process";
		return Result::IllegalArgument;
	}
	if (!IsWindow(platformInfo.window))
	{
		Basilisk::errorMessage = "Basilisk::D3D12Instance::Initialize().platformInfo.window is not a valid window";
		return Result::IllegalArgument;
	}
#endif

	//Store the process and window handles
	m_platformInfo.connection = platformInfo.connection;
	m_platformInfo.window = platformInfo.window;

	//Create the DirectX graphics interface factory
	if (Failed( CreateDXGIFactory1(__uuidof(IDXGIFactory4), reinterpret_cast<void**>(&m_factory)) ))
	{
		Basilisk::errorMessage = "Basilisk::D3D12Instance::Initialize() could not create a DirectX Graphics Interface Factory";
		return Result::ApiError;
	}

	return Result::Success;
}

Result VulkanInstance::Initialize(const PlatformInfo &platformInfo, const std::string &appName)
{
#ifndef BASILISK_FINAL_BUILD
	if (WAIT_TIMEOUT != WaitForSingleObject(platformInfo.connection, 0)) //Uses a bit of a hack to figure out if the process is running
	{ //With a time-out of zero, anything other than WAIT_TIMEOUT means the process doesn't exist
		Basilisk::errorMessage = "Basilisk::VulkanInstance::Initialize()::platformInfo.connection is not a valid process";
		return Result::IllegalArgument;
	}
	if (!IsWindow(platformInfo.window))
	{
		Basilisk::errorMessage = "Basilisk::VulkanInstance::Initialize().platformInfo.window is not a valid window";
		return Result::IllegalArgument;
	}
#endif

	//Store the process and window handles
	m_platformInfo.connection = platformInfo.connection;
	m_platformInfo.window = platformInfo.window;


	//Should I let them specify application version as well?
	VkApplicationInfo appInfo =
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,         //Must be NULL
		appName.c_str(), //Application name
		1,               //Application version
		"Basilisk",      //Engine name
		1,               //Engine version
		apiVersion       //API version
	};

	VkInstanceCreateInfo instanceInfo =
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,         //Must be NULL
		0,               //No flags
		&appInfo,        //Let the GPU know who we are
		layerCount,      //Number of layers
		layerNames,      //Which layers we're using
		extensionCount,  //Number of extensions
		extensionNames   //Which extensions we're using
	};

	//           Rely on automatic memory allocation ----v
	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &m_instance);
	if (Failed(result))
	{
		Basilisk::errorMessage = "Basilisk::VulkanInstance::Initialize() could not create a Vulkan Instance";
		return Result::ApiError;
	}

	return Result::Success;
}

void D3D12Instance::Release()
{
	SafeReleaseCom(m_factory);

	for (GPU device : m_gpus)
	{
		SafeReleaseCom(device.adapter);
		device.adapter = nullptr;
	}
}

void VulkanInstance::Release()
{
	if (nullptr != m_instance)
	{
		vkDestroyInstance(m_instance, nullptr);
		m_instance = nullptr;
	}
}

Result D3D12Instance::FindGpus(uint32_t *count)
{
#ifndef BASILISK_FINAL_BUILD
	if (nullptr == m_factory) //Check if Initialize() has been called
	{
		Basilisk::errorMessage = "Basilisk::D3D12Instance::FindGpus() was called before the instance was successfully initialized";
		return Result::IllegalState;
	}
	else if (nullptr == count) //Caller messed up
	{
		Basilisk::errorMessage = "Basilisk::D3D12Instance::FindGpus()::count cannot be a null pointer";
		return Result::IllegalArgument;
	}
#endif

	//Meets all prerequisites

	//Free memory from the last time we scanned for GPUs
	for (GPU i : m_gpus)
	{
		SafeReleaseCom(i.adapter);
		i.adapter = nullptr;
	}

	IDXGIAdapter *adapter;
	DXGI_ADAPTER_DESC adapterDesc;
	HRESULT result;

	//Count the number of GPUs
	for ((*count) = 0; DXGI_ERROR_NOT_FOUND != m_factory->EnumAdapters((*count), &adapter); ++(*count))
	{
		adapter->Release();
		adapter = nullptr;
	}

	m_gpus.resize(*count);

	//Get details for the GPUs, and store in m_gpus
	//No longer using adapter or adapterDesc

	for (uint32_t i = 0; i < (*count); ++i)
	{
		//Don't check for errors here; we just enumerated the adapters
		m_factory->EnumAdapters(i, &m_gpus[i].adapter);

		//Get the video card's details
		result = adapter->GetDesc(&m_gpus[i].desc);
		if (Failed(result))
		{
			Basilisk::errorMessage = "Basilisk::D3D12Instance::FindGpus() could not retrieve GPU details";
			return Result::ApiError;
		}
	}

	return Result::Success;
}

Result VulkanInstance::FindGpus(uint32_t *count)
{
#ifndef BASILISK_FINAL_BUILD
	if (nullptr == m_instance) //Check if Initialize() has been called
	{
		Basilisk::errorMessage = "Basilisk::VulkanInstance::FindGpus() was called before the instance was successfully initialized";
		return Result::IllegalState;
	}
	else if (nullptr == count) //Caller messed up
	{
		Basilisk::errorMessage = "Basilisk::VulkanInstance::FindGpus()::count cannot be a null pointer";
		return Result::IllegalArgument;
	}
#endif

	//Meets all prerequisites

	if (Failed(vkEnumeratePhysicalDevices(m_instance, count, nullptr)))
	{
		Basilisk::errorMessage = "Basilisk::VulkanInstance::FindGpus()'s could not count physical devices";
		return Result::ApiError;
	}

	m_gpus.resize(*count);
	m_gpuProps.resize(*count);
	
	if (Failed(vkEnumeratePhysicalDevices(m_instance, count, m_gpus.data())))
	{
		Basilisk::errorMessage = "Basilisk::VulkanInstance::FindGpus()'s could not list physical devices";
		return Result::ApiError;
	}

	for (uint32_t i = 0; i < (*count); ++i)
	{
		//Store details
		
		vkGetPhysicalDeviceProperties(m_gpus[i], &m_gpuProps[i].props);
		vkGetPhysicalDeviceMemoryProperties(m_gpus[i], &m_gpuProps[i].memoryProps);

		uint32_t numQueues;
		vkGetPhysicalDeviceQueueFamilyProperties(m_gpus[i], &numQueues, nullptr);
		if (numQueues >= 1)
		{
			m_gpuProps[i].queueDescs.resize(numQueues);
			vkGetPhysicalDeviceQueueFamilyProperties(m_gpus[i], &numQueues, m_gpuProps[i].queueDescs.data());
		}
		
		//Get optimal depth buffer format
		//Since all depth formats may be optional, we need to find a suitable depth format to use
		//Start with the highest precision packed format
		std::vector<VkFormat> depthFormats = { 
			VK_FORMAT_D32_SFLOAT_S8_UINT, 
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT, 
			VK_FORMAT_D16_UNORM_S8_UINT, 
			VK_FORMAT_D16_UNORM 
		};
		m_gpuProps[i].depthFormat = VK_FORMAT_UNDEFINED;
		for (VkFormat format : depthFormats)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(m_gpus[i], format, &formatProps);
			
			if (formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{ //Ideally, we get the highest format with the most efficient tiling method
				m_gpuProps[i].depthFormat = format;
				m_gpuProps[i].depthTiling = VK_IMAGE_TILING_LINEAR;
				break;
			}
			else if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{ //But we can settle for an implementation-specific tiling method if necessary
				m_gpuProps[i].depthFormat = format;
				m_gpuProps[i].depthTiling = VK_IMAGE_TILING_OPTIMAL;
				break;
			}
		}
		if (VK_FORMAT_UNDEFINED == m_gpuProps[i].depthFormat)
		{
			Basilisk::errorMessage = "Basilisk::VulkanInstance::FindGpus() could not find a depth format for all GPUs";
			return Result::ApiError;
		}
	}

	return Result::Success;
}

template<> Result D3D12Instance::CreateDevice<D3D12Device>(D3D12Device *&out, uint32_t gpuIndex)
{
#ifndef BASILISK_FINAL_BUILD
	if (nullptr == m_factory)
	{
		Basilisk::errorMessage = "Basilisk::D3D12Instance::CreateDevice() was called before the instance was successfully initialized";
		return Result::IllegalState;
	}
#endif

	//Meets all prerequisites

	out = new D3D12Device();

	if (Failed(D3D12CreateDevice(m_gpus[gpuIndex].adapter, featureLevel, _uuidof(ID3D12Device), reinterpret_cast<void**>(&out->m_device))))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::D3D12Instance::CreateDevice()'s call to D3D12CreateDevice() was unsucessful";
		return Result::ApiError;
	}

	return Result::Success;
}

template<> Result VulkanInstance::CreateDevice<VulkanDevice>(VulkanDevice *&out, uint32_t gpuIndex)
{
#ifndef BASILISK_FINAL_BUILD
	if (nullptr == m_instance)
	{
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() was called before the instance was successfully initialized";
		return Result::IllegalState;
	}
#endif
	if (VK_VERSION_MAJOR(apiVersion) >= VK_VERSION_MAJOR(m_gpuProps[gpuIndex].props.apiVersion))
	{
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() cannot complete without proper API support";
		return Result::ApiError;
	}

	//Meets all prerequisites

	out = new VulkanDevice();
	out->m_depthFormat = m_gpuProps[gpuIndex].depthFormat;
	out->m_depthTiling = m_gpuProps[gpuIndex].depthTiling;

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

	res = vkCreateWin32SurfaceKHR(m_instance, &surface_info, nullptr, &out->m_windowSurface.surface);
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() could not connect to the provided window";
		return Result::ApiError;
	}

	//
	////Find a graphics queue which supports present
	//

	auto supportsPresent = std::make_unique<VkBool32[]>(m_gpuProps[gpuIndex].queueDescs.size());
	for (uint32_t i = 0; i < m_gpuProps[gpuIndex].queueDescs.size(); ++i)
	{
		res = vkGetPhysicalDeviceSurfaceSupportKHR(m_gpus[gpuIndex], i, out->m_windowSurface.surface, &supportsPresent[i]);
		if (Failed(res))
		{ //Error getting the GPU's surface support
			SafeRelease(out);
			Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() could not query the GPU's surface support";
			return Result::ApiError;
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
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() could not find a present-capable graphics queue";
		return Result::ApiError;
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
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() could not find a second graphics queue";
		return Result::ApiError;
	}

	//
	////Get the list of VkFormats supported by that surface, and store the one it likes most
	//

	uint32_t formatCount;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpus[gpuIndex], out->m_windowSurface.surface, &formatCount, nullptr);
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() could not count window surface formats";
		return Result::ApiError;
	}

	auto surfFormats = std::make_unique <VkSurfaceFormatKHR[]>(formatCount);
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpus[gpuIndex], out->m_windowSurface.surface, &formatCount, surfFormats.get());
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() could not retrieve window surface formats";
		return Result::ApiError;
	}

	if (formatCount == 0)
	{ //Surface isn't playing nice
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() could not find a preferred format for the window surface";
		return Result::ApiError;
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

	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpus[gpuIndex], out->m_windowSurface.surface, &out->m_windowSurface.caps);
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() could not retrieve the GPU's surface capabilities";
		return Result::ApiError;
	}

	uint32_t presentModeCount;
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpus[gpuIndex], out->m_windowSurface.surface, &presentModeCount, nullptr);
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() could not count the GPU's present modes";
		return Result::ApiError;
	}
	if (presentModeCount == 0)
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() could not detect any present modes for the provided GPU";
		return Result::ApiError;
	}

	out->m_windowSurface.presentModes.resize(presentModeCount);

	res = vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpus[gpuIndex], out->m_windowSurface.surface, &presentModeCount, out->m_windowSurface.presentModes.data());
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() could not list the GPU's present modes";
		return Result::ApiError;
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
		nullptr,         //Reserved
		0,               //Flags
		2,               //Queue count
		queue_info,      //Queue properties
		layerCount,      //Layer count
		layerNames,      //Layer types
		extensionCount,  //Extension count
		extensionNames,  //Extension names
		nullptr          //Not enabling any device features
	};
	
	out = new VulkanDevice();
	if (Failed(vkCreateDevice(m_gpus[gpuIndex], &device_info, nullptr, &out->m_device) ))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() failed to create the device";
		return Result::ApiError;
	}

	//
	////Create the device's command pools
	//

	//Rendering queue
	VkCommandPoolCreateInfo render_pool_info =
	{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,                                          //Reserved
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,  //Command buffers created from this pool can be reset
		presentQueueIndex                                 //All command buffers from this pool must be submitted to the render queue
	};

	res = vkCreateCommandPool(out->m_device, &render_pool_info, nullptr, &out->m_commandPools[VulkanDevice::render]);
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() failed to create the command pool";
		return Result::ApiError;
	}

	//Presentation queue
	VkCommandPoolCreateInfo present_pool_info =
	{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,                                          //Reserved
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,  //Command buffers created from this pool can be reset
		presentQueueIndex                                 //All command buffers from this pool must be submitted to the present queue
	};

	res = vkCreateCommandPool(out->m_device, &present_pool_info, nullptr, &out->m_commandPools[VulkanDevice::present]);
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() failed to create the command pool";
		return Result::ApiError;
	}
	
	//
	////Store the queues we created above in the device
	//

	vkGetDeviceQueue(out->m_device, renderQueueIndex, 0, &out->m_queues[VulkanDevice::render]);
	vkGetDeviceQueue(out->m_device, presentQueueIndex, 0, &out->m_queues[VulkanDevice::present]);


	return Result::Success;
}


//////////////////////////
//        DEVICE        //
//////////////////////////


D3D12Device::D3D12Device() : m_device(nullptr), m_commandQueue(nullptr), m_commandAllocator(nullptr) {
}

VulkanDevice::VulkanDevice() : m_device(nullptr) {
	m_windowSurface = { };
}

void D3D12Device::Release() {
	SafeReleaseCom(m_device);
}

void VulkanDevice::Release() {
	if (nullptr != m_device)
	{
		vkDestroyDevice(m_device, nullptr);
		m_device = nullptr;
	}
}


template<> Result VulkanDevice::CreateSwapChain<VulkanSwapChain, VulkanCmdBuffer>(VulkanSwapChain *&out, VulkanCmdBuffer &cmdBuffer, Bounds2D<uint32_t> resolution, uint32_t numBuffers)
{
	//All prerequisites were checked in `VulkanInstance::CreateDevice()`
	
	//
	////Calculate the swap chain's resolution (may differ from the `resolution` argument)
	//

	VkExtent2D swapChainRes;
	if (static_cast<uint32_t>(-1) == m_windowSurface.caps.currentExtent.width)
	{ //Surface size is undefined
		swapChainRes.width = Clamp(resolution.width,
			m_windowSurface.caps.minImageExtent.width,
			m_windowSurface.caps.maxImageExtent.width);

		swapChainRes.height = Clamp(resolution.height,
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

	out = new VulkanSwapChain();

	VkSwapchainCreateInfoKHR swapchain_info = 
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,                           //Reserved
		0,                                 //No flags
		m_windowSurface.surface,           //Target surface
		numBuffers,                        //Number of back buffers
		m_windowSurface.colorFormat,            //Surface format
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

	VkResult res = vkCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &out->m_swapChain);
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateSwapChain() could not create the swap chain";
		return Result::ApiError;
	}

	//
	////Store the back buffers
	//

	res = vkGetSwapchainImagesKHR(m_device, out->m_swapChain, &numBuffers, nullptr);
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateSwapChain() could not count the swap chain's back buffers";
		return Result::ApiError;
	}

	out->m_backBuffers.resize(numBuffers);
	out->m_backBufferViews.resize(numBuffers);

	res = vkGetSwapchainImagesKHR(m_device, out->m_swapChain, &numBuffers, out->m_backBuffers.data());
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateSwapChain() could not count the swap chain's back buffers";
		return Result::ApiError;
	}

	//
	////Create views for each back buffer
	//

	VkComponentMapping components =
	{
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};

	VkImageSubresourceRange range =
	{
		0,  //Aspect mask
		0,  //Base mip level
		1,  //Level count
		0,  //Base array layer
		1   //Layer count
	};

	for (uint32_t i = 0; i < out->m_backBuffers.size(); ++i)
	{
		VkImageViewCreateInfo color_image_view =
		{
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			nullptr,                      //Reserved
			0,                            //No flags
			out->m_backBuffers[i],        //Image
			VK_IMAGE_VIEW_TYPE_2D,        //Image view type
			m_windowSurface.colorFormat,  //Image format
			components,                   //Swizzle RGBA components
			range                         //Subresource range
		};

		res = vkCreateImageView(m_device, &color_image_view, nullptr, &out->m_backBufferViews[i]);
		if (Failed(res))
		{
			SafeRelease(out);
			Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateSwapChain() could not create a view for all back buffers";
			return Result::ApiError;
		}

		//Set the image layout to depth stencil optimal
		cmdBuffer.SetImageLayout(out->m_backBuffers[i],
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}


	return Result::Success;
}

Result VulkanDevice::CreateRenderPass(VulkanRenderPass *&out, uint32_t numColorBuffers, bool enableDepth)
{
#ifndef BASILISK_FINAL_BUILD
	if (0 == numColorBuffers && !enableDepth)
	{
		Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateRenderPass() must have at least one image attachment";
		return Result::IllegalArgument;
	}
#endif

	//Meets all prerequisites

	uint32_t numAttachments = numColorBuffers + (enableDepth ? 1 : 0);
	auto attachments = std::make_unique<VkAttachmentDescription[]>(numAttachments);
	auto attachmentRefs = std::make_unique<VkAttachmentReference[]>(numAttachments);


	uint32_t i = 0;
	
	//Describe the color attachments
	for ( ; i < numColorBuffers; ++i)
	{
		attachments[i].format = m_windowSurface.colorFormat;
		attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		attachmentRefs[i].attachment = i;
		attachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	
	//Describe the depth attachments
	for ( ; i < numAttachments; ++i)
	{
		attachments[i].format = m_depthFormat;
		attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[i].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		
		attachmentRefs[i].attachment = i;
		attachmentRefs[i].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	
	//
	////Create the render pass
	//

	VkSubpassDescription subpass = 
	{
		0,                   //Flags
		VK_PIPELINE_BIND_POINT_GRAPHICS,  //Pipeline bind point
		0,                   //Input attachment count
		nullptr,             //Input attachments
		numColorBuffers,     //Color attachment count
		&attachmentRefs[0],  //Color attachments
		nullptr,             //Resolve attachment
		enableDepth ? 	&attachmentRefs[numColorBuffers] : nullptr, //Depth buffer attachment
		0,                   //Preserve attachment count
		nullptr              //Preserve attachments
	};
	
	out = new VulkanRenderPass();
	
	VkRenderPassCreateInfo render_pass_info =
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,            //Reserved
		0,                  //No flags
		numAttachments,     //Attachment count
		attachments.get(),  //Attachments
		1,                  //Subpass count
		&subpass,           //Subpasses
		0,                  //Dependency count
		nullptr             //Dependencies
	};
	
	if (Failed( vkCreateRenderPass(m_device, &render_pass_info, nullptr, &out->m_renderPass) ))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateRenderPass() could not create the render pass";
		return Result::ApiError;
	}
	
	
	return Result::Success;
}

bool VulkanDevice::MemoryTypeFromProps(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex)
{
	//Search memoryTypes to find first index with those properties
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {
			//Type is available; does it match user properties?
			if ((m_memoryProps.memoryTypes[i].propertyFlags &
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

Result VulkanDevice::CreateDepthBuffer(VulkanImage *&out, VulkanCmdBuffer &cmdBuffer, Bounds2D<uint32_t> resolution, uint32_t numSamples)
{
#ifndef BASILISK_FINAL_BUILD
	if (0 == numSamples || numSamples > 64 || !PowerOfTwo(numSamples))
	{
		Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateDepthBuffer()::numSamples must be a power of two between 1 and 64";
		return Result::IllegalArgument;
	}
#endif

	//Meets all prerequisites

	out = new VulkanImage();
	out->m_format = m_depthFormat;
	VkResult res;

	//Create the image

	VkImageCreateInfo image_info =
	{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,                   //Reserved
		0,                         //Flags
		VK_IMAGE_TYPE_2D,          //Image type
		m_depthFormat,             //Format
		{resolution.width, resolution.height},           //Extent
		1,                         //Mip levels
		1,                         //Array layers
		static_cast<VkSampleCountFlagBits>(numSamples),  //Sample count
		m_depthTiling,             //Tiling
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,     //Usage
		VK_SHARING_MODE_EXCLUSIVE, //Sharing mode
		0,                         //Queue family index count
		nullptr,                   //Queue family indices
		VK_IMAGE_LAYOUT_UNDEFINED  //Initial layout
	};

	res = vkCreateImage(m_device, &image_info, nullptr, &out->m_image);
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateDepthBuffer() could not create the target image";
		return Result::ApiError;
	}

	//Determine memory requirements

	VkMemoryRequirements mem_reqs;
	vkGetImageMemoryRequirements(m_device, out->m_image, &mem_reqs);


	//Allocate the memory

	VkMemoryAllocateInfo mem_alloc =
	{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,        //Reserved
		mem_reqs.size,  //Allocation size
		0               //Memory type index
	};

	//Use the memory properties to determine the type of memory required
	if (!MemoryTypeFromProps(mem_reqs.memoryTypeBits, 0, &mem_alloc.memoryTypeIndex))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateDepthBuffer() could not determine the type of memory required by the target image";
		return Result::ApiError;
	}

	res = vkAllocateMemory(m_device, &mem_alloc, NULL, &out->m_memory);
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateDepthBuffer() could not allocate memory for the target image";
		return Result::ApiError;
	}

	//Bind the memory

	res = vkBindImageMemory(m_device, out->m_image, out->m_memory, 0);
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateDepthBuffer() could not bind depth buffer memory";
		return Result::ApiError;
	}

	//Set the image layout to depth stencil optimal
	cmdBuffer.SetImageLayout(out->m_image,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	
	//Create image view

	VkImageViewCreateInfo view_info = { };

	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = nullptr;
	view_info.image = out->m_image;
	view_info.format = m_depthFormat;
	view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.flags = 0;

	if (m_depthFormat == VK_FORMAT_D16_UNORM_S8_UINT ||
		m_depthFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
		m_depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) {
		view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	res = vkCreateImageView(m_device, &view_info, nullptr, &out->m_view);
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateDepthBuffer() could not create an image view for the target image";
		return Result::ApiError;
	}
}

/*
Result D3D12Device::Initialize(HWND window, Bounds2D<uint32_t> resolution, bool fullscreen, bool vsync)
{
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_0;
	HRESULT result;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	IDXGIFactory4 *factory;
	IDXGIAdapter *adapter;
	IDXGIOutput *adapterOutput;
	uint32_t numModes, i = 0, renderTargetViewDescriptorSize;
	size_t stringLen;
	DXGI_MODE_DESC *displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int32_t error;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	IDXGISwapChain *swapChain;
	D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc;
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle;

	m_vsync = vsync;
	m_fullscreen = fullscreen;

	//Create the DirectX graphics interface factory
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)&factory);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not create DirectX Graphics Interface Factory";
		return Result::APIFailure;
	}

	//Loop through all adapters until we find one that supports Direct3D 12
	for (uint32_t adapterIndex = 0; ; ++adapterIndex)
	{
		if (DXGI_ERROR_NOT_FOUND == factory->EnumAdapters(adapterIndex, &adapter))
			break; //No more adapters

		if (SUCCEEDED(D3D12CreateDevice(adapter, featureLevel, _uuidof(ID3D12Device), nullptr)))
			break; //Stop searching if this adapter supports Direct3D 12

		adapter->Release(); //Prepare for the next iteration
	}
	if (result != S_OK) //Ran out of adapters
	{
		Basilisk::errorMessage = "Could not find a video card which supports the Direct3D 12 runtime";
		return Result::APIFailure;
	}

	//Create the Direct3D device
	result = D3D12CreateDevice(adapter, featureLevel, __uuidof(ID3D12Device), (void**)&m_device);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Unexpected error creating the Direct3D 12 device";
		return Result::APIFailure;
	}

	//\todo Are multi-monitor displays typically supported?
	//Connect to the primary monitor
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not locate the primary monitor";
		return Result::APIFailure;
	}

	//List supported display modes matching DXGI_FORMAT_R8G8B8A8_UNORM
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not enumerate primary monitor's display modes";
		return Result::APIFailure;
	}

	//Create a list holding all the display modes for this monitor/gpu combination
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		Basilisk::errorMessage = "Ran out of memory while listing primary monitor's display modes";
		return Result::APIFailure;
	}

	//Fill the list of display modes
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not retrieve primary monitor's display modes";
		return Result::APIFailure;
	}
	/* \todo Store all resolutions, so it can be changed quickly at runtime
	for (i = 0; i < numModes; ++i)
	{
		displayModeList[i].Height
		displayModeList[i].Width
	}* /

	//Get the video card description
	result = adapter->GetDesc(&adapterDesc);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not retrieve video card description";
		return Result::APIFailure;
	}

	//Store the video card's memory (in megabytes)
	m_videoCardMemory = static_cast<uint32_t>(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
	//Store the video card's name
	error = wcstombs_s(&stringLen, m_videoCardDesc, DESC_LEN, adapterDesc.Description, DESC_LEN);
	if (error != 0)
	{
		Basilisk::errorMessage = "Could not store video card name";
		return Result::APIFailure;
	}

	//Clean up
	delete[] displayModeList;
	displayModeList = nullptr;
	adapterOutput->Release();
	adapterOutput = nullptr;
	adapter->Release();
	adapter = nullptr;

	return Result::Success;
}

void D3D12Device::Release()
{
	/*

	//Set the swap chain to windowed before releasing it to avoid stupid exceptions
	if (m_swapChain)
		m_swapChain->SetFullscreenState(false, nullptr);

	//Close the handle to the fence event
	if (CloseHandle(m_fenceEvent) == 0)
		Basilisk::errorMessage = "Failed to close fence";

	//Release the fence
	safeRelease(m_fence);
	//Release the empty pipeline state
	//safeRelease(m_pipelineState);
	//Release the command list
	safeRelease(m_commandList);
	//Release the command allocator
	safeRelease(m_commandAllocator);
	//Release the back buffer render target views
	safeRelease(m_backBufferRenderTarget[0]);
	safeRelease(m_backBufferRenderTarget[1]);
	//Release the render target view heap
	safeRelease(m_renderTargetViewHeap);
	//Release the swap chain
	safeRelease(m_swapChain);
	//Release the command queue
	safeRelease(m_commandQueue);
	
	* /
	//Release the device
	safeRelease(m_device);
}
*/

/*

	//Create the command queue
	commandQueueDesc = {};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;
	result = m_device->CreateCommandQueue(&commandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&m_commandQueue);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not create Direct3D 12 command queue";
		return Result::APIFailure;
	}

	//Describe the swap chain

	swapChainDesc = {}; //Zero the memory
	swapChainDesc.BufferCount = 2; //Use double buffering
	swapChainDesc.BufferDesc.Width = resolution.width; //Set width
	swapChainDesc.BufferDesc.Height = resolution.height; //Set height
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //32-bit back buffers
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //Set the back buffers to be used as render targets
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //Throw out old back buffer contents
	swapChainDesc.OutputWindow = window; //Give it a handle to the window
	swapChainDesc.Windowed = !fullscreen; //Fullscreen vs windowed
	//Auto-detect the refresh rate
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;

	//@todo msaa toggling
	//No multisampling for now
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	//Set the scan line ordering and scaling to unspecified
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//Allow to switch between windowed and fullscreen modes
	//Also changes the monitor resolution to match the width and height of the window
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//Create the swap chain
	result = factory->CreateSwapChain(m_commandQueue, &swapChainDesc, &swapChain);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not create swap chain";
		return Result::APIFailure;
	}

	//Now shoehorn the swap chain into a IDXGISwapChain3 structure
	result = swapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&m_swapChain);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not upgrade the swap chain to version 3";
		return Result::APIFailure;
	}

	//More housekeeping
	swapChain = nullptr;
	factory->Release();
	factory = nullptr;


	//Describe the back buffers as render target views
	renderTargetViewHeapDesc = {};
	renderTargetViewHeapDesc.NumDescriptors = 2; //two back buffers, one is presented at any time
	renderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	renderTargetViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//Create the render target view heap
	result = m_device->CreateDescriptorHeap(&renderTargetViewHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_renderTargetViewHeap);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not create back buffers as render target views";
		return Result::APIFailure;
	}

	//Get a handle to the starting memory location in the render target view heap to identify where the render target views will be located for the two back buffers
	renderTargetViewHandle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	//Get the size of the memory location for the render target view descriptors
	renderTargetViewDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	
	//Get a pointer to the first back buffer from the swap chain
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D12Resource), (void**)&m_backBufferRenderTarget[0]);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not retrieve first back buffer from the swap chain";
		return Result::APIFailure;
	}
	//Create a render target view for the first back buffer
	m_device->CreateRenderTargetView(m_backBufferRenderTarget[0], nullptr, renderTargetViewHandle);

	//Increment the view handle to the next descriptor location in the render target view heap
	renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;
	//Get a pointer to the second back buffer from the swap chain
	result = m_swapChain->GetBuffer(1, __uuidof(ID3D12Resource), (void**)&m_backBufferRenderTarget[1]);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not retrieve second back buffer from the swap chain";
		return Result::APIFailure;
	}
	//Create a render target view for the second back buffer
	m_device->CreateRenderTargetView(m_backBufferRenderTarget[1], nullptr, renderTargetViewHandle);

	//Store the location of the current back buffer
	m_bufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	//Create a command allocator
	result = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_commandAllocator);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not create command allocator";
		return Result::APIFailure;
	}
	//Create a command list
	result = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, nullptr, __uuidof(ID3D12GraphicsCommandList), (void**)&m_commandList);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not create command list";
		return Result::APIFailure;
	}
	//Close the command list during initialization as it is created in a recording state
	result = m_commandList->Close();
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not close command list";
		return Result::APIFailure;
	}

	//Create a fence for GPU synchronization
	result = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_fence);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not create fence";
		return Result::APIFailure;
	}
	//Create an event object for the fence
	m_fenceEvent = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (m_fenceEvent == nullptr)
	{
		Basilisk::errorMessage = "Could not create event object for fence";
		return Result::APIFailure;
	}
	//Set a starting value for the fence
	m_fenceValue = 1; 

	return Result::Success;
}

bool D3D12Device::beginFrame(unsigned long long fenceValue)
{
	HRESULT result;

	//Wait for the last frame to finish
	if (m_fence->GetCompletedValue() < fenceValue)
	{
		result = m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		if (result != S_OK)
		{
			Basilisk::errorMessage = "Could not set the fence's completion behavior";
			return false;
		}
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	//Use the command allocator and list
	result = m_commandAllocator->Reset();
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not reset the command allocator";
		return false;
	}
	result = m_commandList->Reset(m_commandAllocator, m_pipelineState);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not reset the command list";
		return false;
	}
	return true;
}

bool D3D12Device::execute(ID3D12CommandList **commandLists)
{
	HRESULT result;
	uint32_t numCommands = sizeof(commandLists) / sizeof(commandLists[0]);
	D3D12_RESOURCE_BARRIER barrier;
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle;
	uint32_t renderTargetViewDescriptorSize;
	ID3D12CommandList* ppCommandLists[1];
	float color[4];

	//Record commands in the command list

	//Pull the backbuffer from the screen so we can write to it
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_backBufferRenderTarget[m_bufferIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_commandList->ResourceBarrier(1, &barrier);

	//Get the render target view handle for the current back buffer
	renderTargetViewHandle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	renderTargetViewDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	if (m_bufferIndex == 1)
		renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;

	//Set the back buffer as the render target
	m_commandList->OMSetRenderTargets(1, &renderTargetViewHandle, FALSE, nullptr);

	//clear the window to gray
	color[0] = 0.5f;
	color[1] = 0.5f;
	color[2] = 0.5f;
	color[3] = 1.0f;
	m_commandList->ClearRenderTargetView(renderTargetViewHandle, color, 0, nullptr);

	//Present the back buffer
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_commandList->ResourceBarrier(1, &barrier);

	//Close the command list, submit it to the queue, and execute the queue
	result = m_commandList->Close();
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not close command list";
		return false;
	}
	ppCommandLists[0] = m_commandList;
	m_commandQueue->ExecuteCommandLists(1, ppCommandLists);

	return true;
}

unsigned long long D3D12Device::present()
{
	HRESULT result;

	//Present the frame through the swap chain
	if (m_vsync)
	{
		result = m_swapChain->Present(1, 0);
		if (result != S_OK)
		{
			Basilisk::errorMessage = "Could not present swap chain (with vsync)";
			return 0;
		}
	}
	else
	{
		result = m_swapChain->Present(0, 0);
		if (result != S_OK)
		{
			Basilisk::errorMessage = "Could not present the swap chain (without vsync)";
			_com_error msg(result);
			LPCSTR str = msg.ErrorMessage();
			OutputDebugString(str);
			return 0;
		}
	}

	//Set the fence value
	result = m_commandQueue->Signal(m_fence, m_fenceValue);
	if (result != S_OK)
	{
		Basilisk::errorMessage = "Could not signal the fence";
		return 0;
	}

	//Alternate the back buffer index each frame
	m_bufferIndex = (m_bufferIndex == 0 ? 1 : 0);

	return m_fenceValue++;
}
*/
/**
\file   device.cpp
\author Andrew Baxter
\date   February 26, 2016

Defines the behavior of Vulkan and D3D12 rendering backends

*/

#include <memory> //Used for smart pointers
#include "rendering/device.h"

using namespace Basilisk;


//If you add a layer, don't forget to change the layer count
const char * const* VulkanInstance::layerNames = nullptr;

//If you add an extension, don't forget to change the extension count
const char* VulkanInstance::extensionNames[VulkanInstance::extensionCount] = {
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME
};

D3D12Instance::D3D12Instance() {
	m_platformInfo = { };
}

VulkanInstance::VulkanInstance() : m_instance(nullptr) {
	m_platformInfo = {};
}

Result D3D12Instance::Initialize(const PlatformInfo &platformInfo, const std::string &appName)
{
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
	//Not sure if I can get away with passing nullptr as the second argument
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
		Basilisk::errorMessage = "Basilisk::VulkanInstance::FindGpus()'s could not enumerate physical devices";
		return Result::ApiError;
	}

	m_gpus.resize(*count);

	for (uint32_t i = 0; i < (*count); ++i)
	{
		vkGetPhysicalDeviceProperties(m_gpus[i].device, &m_gpus[i].props);
		vkGetPhysicalDeviceMemoryProperties(m_gpus[i].device, &m_gpus[i].memoryProps);

		uint32_t numQueues;
		vkGetPhysicalDeviceQueueFamilyProperties(m_gpus[i].device, &numQueues, nullptr);
		if (numQueues >= 1)
		{
			m_gpus[i].queueDescs.resize(numQueues);
			vkGetPhysicalDeviceQueueFamilyProperties(m_gpus[i].device, &numQueues, m_gpus[i].queueDescs.data());
		}
	}
	
	return Result::Success;
}

template<> Result D3D12Instance::CreateDevice<D3D12Device>(uint32_t gpuIndex, D3D12Device *&out)
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

	if (Failed( D3D12CreateDevice(m_gpus[gpuIndex].adapter, featureLevel, _uuidof(ID3D12Device), reinterpret_cast<void**>(&out->m_device)) ))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::D3D12Instance::CreateDevice()'s call to D3D12CreateDevice() was unsucessful";
		return Result::ApiError;
	}

	return Result::Success;
}

template<> Result VulkanInstance::CreateDevice<VulkanDevice>(uint32_t gpuIndex, VulkanDevice *&out)
{
#ifndef BASILSIK_FINAL_BUILD
	if (nullptr == m_instance)
	{
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() was called before the instance was successfully initialized";
		return Result::IllegalState;
	}
	if (0 == m_platformInfo.connection)
	{
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() was called with an invalid process handle";
		return Result::IllegalState;
	}
	if (0 == m_platformInfo.window)
	{
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() was called with an invalid window handle";
		return Result::IllegalState;
	}
#endif

	//Meets all prerequisites


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

	res = vkCreateWin32SurfaceKHR(m_instance, &surface_info, nullptr, &m_gpus[gpuIndex].windowSurface);
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() could not connect to the provided window";
		return Result::ApiError;
	}

	//
	////Find a graphics queue which supports present
	//

	auto supportsPresent = std::make_unique<VkBool32[]>(m_gpus[gpuIndex].queueDescs.size());
	for (uint32_t i = 0; i < m_gpus[gpuIndex].queueDescs.size(); ++i)
	{
		res = vkGetPhysicalDeviceSurfaceSupportKHR(m_gpus[gpuIndex].device, i, m_gpus[gpuIndex].windowSurface, &supportsPresent[i]);
		if (Failed(res))
		{
			//Error getting the GPU's surface support
			SafeRelease(out);
			Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() could not query the GPU's surface support";
			return Result::ApiError;
		}
	}

	uint32_t graphicsQueueNodeIndex = std::numeric_limits<uint32_t>::max();
	for (uint32_t i = 0; i < m_gpus[gpuIndex].queueDescs.size(); ++i)
	{
		if ((m_gpus[gpuIndex].queueDescs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0
			&& supportsPresent[i])
		{
			//Found a queue that fits our criteria
			graphicsQueueNodeIndex = i;
			break;
		}
	}

	if (std::numeric_limits<uint32_t>::max() == graphicsQueueNodeIndex)
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() could not locate a present-capable graphics queue";
		return Result::ApiError;
	}

	/*
	// Get the list of VkFormats that are supported:
	uint32_t formatCount;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(info.gpus[0], info.surface,
		&formatCount, NULL);
	assert(res == VK_SUCCESS);
	VkSurfaceFormatKHR *surfFormats =
		(VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(info.gpus[0], info.surface,
		&formatCount, surfFormats);
	assert(res == VK_SUCCESS);
	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
		info.format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else {
		assert(formatCount >= 1);
		info.format = surfFormats[0].format;
	}
	*/

	//
	////Create the device
	//
	
	VkDeviceQueueCreateInfo queue_info = { };

	float queue_priorities[1] = { 0.0 };
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.pNext = NULL;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = queue_priorities;
	queue_info.queueFamilyIndex = graphicsQueueNodeIndex;

	VkDeviceCreateInfo device_info = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,         //Reserved
		0,               //Flags
		1,               //Queue count
		&queue_info,     //Queue properties
		layerCount,      //Layer count
		layerNames,      //Layer types
		extensionCount,  //Extension count
		extensionNames,  //Extension names
		nullptr          //Not enabling any device features yet
	};
	
	out = new VulkanDevice();
	if (Failed(vkCreateDevice(m_gpus[gpuIndex].device, &device_info, nullptr, &out->m_device) ))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanInstance::CreateDevice() failed to create the device";
		return Result::ApiError;
	}
	
	return Result::Success;
}



D3D12Device::D3D12Device() : m_device(nullptr) {
}

VulkanDevice::VulkanDevice() : m_device(nullptr) {
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


template<> Result D3D12Device::CreateSwapChain<D3D12SwapChain>(D3D12SwapChain *&out, HINSTANCE connection, HWND window, Bounds2D<uint16_t> resolution, uint8_t numBuffers, uint8_t numSamples)
{
	
}

/*
template<> Result VulkanDevice::CreateSwapChain<VulkanSwapChain>(VulkanSwapChain *&out, HINSTANCE connection, HWND window, Bounds2D<uint16_t> resolution, uint8_t numBuffers, uint8_t numSamples)
{
	out = new VulkanSwapChain();
	VkResult res;

	//
	////
	//Store a handle to the window surface as a render target
	////
	//

	VkWin32SurfaceCreateInfoKHR surfaceInfo = {
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		nullptr,     //Reserved
		0,           //No flags allowed
		connection,  //Handle to the process
		window       //Handle to the window
	};
	
	res = vkCreateWin32SurfaceKHR(m_instance, &surfaceInfo, nullptr, &out->m_windowSurface);
	if (Failed(res))
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateSwapChain() could not connect to the provided window";
		return Result::ApiError;
	}
	else
	{
		auto supportsPresent = std::make_unique<VkBool32[]>(m_queues.size());
		for (uint32_t i = 0; i < m_queues.size(); ++i) //Loop through the list of queues
		{
			res = vkGetPhysicalDeviceSurfaceSupportKHR(m_gpu, i, out->m_windowSurface, &supportsPresent[i]);
			if (Failed(res)) //Error getting the GPU's surface support
			{
				SafeRelease(out);
				Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateSwapChain() could not query the GPU's surface support";
				return Result::ApiError;
			}
		}

		//Find a queue which supports graphics and present
		uint32_t graphicsQueueNodeIndex = std::numeric_limits<uint32_t>::max();
		for (uint32_t i = 0; i < m_queues.size(); ++i)
		{
			if ((m_queues[i].props.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0
				&& supportsPresent[i])
			{
				graphicsQueueNodeIndex = i;
				break;
			}
		}
		if (std::numeric_limits<uint32_t>::max() == graphicsQueueNodeIndex)
		{
			SafeRelease(out);
			Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateSwapChain() could not locate a present-capable graphics queue";
			return Result::ApiError;
		}
	}

	VkSurfaceCapabilitiesKHR surfaceCaps;
	uint32_t presentModeCount;
	auto presentModes = std::make_unique<VkPresentModeKHR[]>(presentModeCount);// VkPresentModeKHR[presentModeCount];
	VkExtent2D swapChainExtent;
	constexpr VkPresentModeKHR swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	VkSurfaceTransformFlagBitsKHR preTransform;
	VkSwapchainCreateInfoKHR swapChainInfo;

	
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, out->m_windowSurface, &surfaceCaps);
	if (Failed(res)) //Could not get surface capabilities
	{
		SafeRelease(out);
		Basilisk::errorMessage = "Basilisk::VulkanDevice::CreateSwapChain()'s call to vkGetPhysicalDeviceSurfaceCapabilities() failed";
		return Result::ApiError;
	}
	else //Got surface capabilities
	{
		//Ran into design problems here
		//Will funish resolving tomorrow
	}
}
*/

/*
Result D3D12Device::Initialize(HWND window, Bounds2D<uint16_t> resolution, bool fullscreen, bool vsync)
{
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0; //We can actually get away with using the D3D12 API on GPUs that only support D3D11
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

	//List supported display modes matáqching DXGI_FORMAT_R8G8B8A8_UNORM
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

template<class T> void safeRelease(T *obj)
{
	if (obj != nullptr)
	{
		obj->Release();
		obj = nullptr;
	}
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
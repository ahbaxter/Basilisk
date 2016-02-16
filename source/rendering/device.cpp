#include "../include/rendering/device.h"

using namespace Basilisk;

#ifndef _VULKAN
#include <vector>
#include <comdef.h>
#endif
#include <chrono>

D3D12Device::D3D12Device()
{
	memset(&m_videoCardDesc[0], 0, DESC_LEN);
	m_device = nullptr;
	m_commandQueue = nullptr;
	m_swapChain = nullptr;
	m_renderTargetViewHeap = nullptr;
	m_backBufferRenderTarget[0] = nullptr;
	m_backBufferRenderTarget[1] = nullptr;
	m_commandAllocator = nullptr;
	m_commandList = nullptr;
	m_pipelineState = nullptr;
	m_fence = nullptr;
	m_fenceEvent = nullptr;
}

D3D12Device::~D3D12Device()
{
	//Handled in the Shutdown function
}

bool D3D12Device::initialize(HWND window, unsigned int width, unsigned int height, bool fullscreen, bool vsync)
{
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	HRESULT result = S_OK;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	IDXGIFactory4 *factory;
	IDXGIAdapter *adapter;
	IDXGIOutput *adapterOutput;
	unsigned int numModes, i = 0, renderTargetViewDescriptorSize;
	size_t stringLen;
	DXGI_MODE_DESC *displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
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
		Basilisk::error = "Could not create DirectX Graphics Interface Factory";
		return false;
	}

	//Loop through all adapters until we find one that supports Direc3D 12
	for (unsigned int adapterIndex = 0; ; ++adapterIndex)
	{
		if (DXGI_ERROR_NOT_FOUND == factory->EnumAdapters(adapterIndex, &adapter))
			break; //No more adapters

		if (SUCCEEDED(D3D12CreateDevice(adapter, featureLevel, _uuidof(ID3D12Device), nullptr)))
			break; //Stop searching if this adapter supports Direct3D 12

		adapter->Release(); //Prepare for the next iteration
	}
	if (result != S_OK) //Ran out of adapters
	{
		Basilisk::error = "Could not find a video card which supports the Direct3D 12 runtime";
		return false;
	}

	//Create the Direct3D device
	result = D3D12CreateDevice(adapter, featureLevel, __uuidof(ID3D12Device), (void**)&m_device);
	if (result != S_OK)
	{
		Basilisk::error = "Unexpected error creating the Direct3D 12 device";
		return false;
	}

	//Create the command queue
	commandQueueDesc = {};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;
	result = m_device->CreateCommandQueue(&commandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&m_commandQueue);
	if (result != S_OK)
	{
		Basilisk::error = "Could not create Direct3D 12 command queue";
		return false;
	}

	//@todo Are multi-monitor displays typically supported?
	//Connect to the primary monitor
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (result != S_OK)
	{
		Basilisk::error = "Could not locate the primary monitor";
		return false;
	}

	//List supported display modes matching DXGI_FORMAT_R8G8B8A8_UNORM
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
	if (result != S_OK)
	{
		Basilisk::error = "Could not enumerate primary monitor's display modes";
		return false;
	}

	//Create a list holding all the display modes for this monitor/gpu combination
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		Basilisk::error = "Ran out of memory while listing primary monitor's display modes";
		return false;
	}

	//Fill the list of display modes
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (result != S_OK)
	{
		Basilisk::error = "Could not retrieve primary monitor's display modes";
		return false;
	}
	/* @todo Store all resolutions, so it can be changed quickly at runtime
	for (i = 0; i < numModes; ++i)
	{
		displayModeList[i].Height
		displayModeList[i].Width
	}*/

	//Get the video card description
	result = adapter->GetDesc(&adapterDesc);
	if (result != S_OK)
	{
		Basilisk::error = "Could not retrieve video card description";
		return false;
	}

	//Store the video card's memory (in megabytes)
	m_videoCardMemory = static_cast<unsigned int>(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
	//Store the video card's name
	error = wcstombs_s(&stringLen, m_videoCardDesc, DESC_LEN, adapterDesc.Description, DESC_LEN);
	if (error != 0)
	{
		Basilisk::error = "Could not store video card name";
		return false;
	}

	//Clean up
	delete[] displayModeList;
	displayModeList = nullptr;
	adapterOutput->Release();
	adapterOutput = nullptr;
	adapter->Release();
	adapter = nullptr;

	//Describe the swap chain

	swapChainDesc = {}; //Zero the memory
	swapChainDesc.BufferCount = 2; //Use double buffering
	swapChainDesc.BufferDesc.Width = width; //Set width
	swapChainDesc.BufferDesc.Height = height; //Set height
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
		Basilisk::error = "Could not create swap chain";
		return false;
	}

	//Now shoehorn the swap chain into a IDXGISwapChain3 structure
	result = swapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&m_swapChain);
	if (result != S_OK)
	{
		Basilisk::error = "Could not upgrade the swap chain to version 3";
		return false;
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
		Basilisk::error = "Could not create back buffers as render target views";
		return false;
	}

	//Get a handle to the starting memory location in the render target view heap to identify where the render target views will be located for the two back buffers
	renderTargetViewHandle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	//Get the size of the memory location for the render target view descriptors
	renderTargetViewDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	
	//Get a pointer to the first back buffer from the swap chain
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D12Resource), (void**)&m_backBufferRenderTarget[0]);
	if (result != S_OK)
	{
		Basilisk::error = "Could not retrieve first back buffer from the swap chain";
		return false;
	}
	//Create a render target view for the first back buffer
	m_device->CreateRenderTargetView(m_backBufferRenderTarget[0], nullptr, renderTargetViewHandle);

	//Increment the view handle to the next descriptor location in the render target view heap
	renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;
	//Get a pointer to the second back buffer from the swap chain
	result = m_swapChain->GetBuffer(1, __uuidof(ID3D12Resource), (void**)&m_backBufferRenderTarget[1]);
	if (result != S_OK)
	{
		Basilisk::error = "Could not retrieve second back buffer from the swap chain";
		return false;
	}
	//Create a render target view for the second back buffer
	m_device->CreateRenderTargetView(m_backBufferRenderTarget[1], nullptr, renderTargetViewHandle);

	//Store the location of the current back buffer
	m_bufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	//Create a command allocator
	result = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_commandAllocator);
	if (result != S_OK)
	{
		Basilisk::error = "Could not create command allocator";
		return false;
	}
	//Create a command list
	result = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, nullptr, __uuidof(ID3D12GraphicsCommandList), (void**)&m_commandList);
	if (result != S_OK)
	{
		Basilisk::error = "Could not create command list";
		return false;
	}
	//Close the command list during initialization as it is created in a recording state
	result = m_commandList->Close();
	if (result != S_OK)
	{
		Basilisk::error = "Could not close command list";
		return false;
	}

	//Create a fence for GPU synchronization
	result = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_fence);
	if (result != S_OK)
	{
		Basilisk::error = "Could not create fence";
		return false;
	}
	//Create an event object for the fence
	m_fenceEvent = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (m_fenceEvent == nullptr)
	{
		Basilisk::error = "Could not create event object for fence";
		return false;
	}
	//Set a starting value for the fence
	m_fenceValue = 1; 

	return true;
}

template<class T> void safeRelease(T *obj)
{
	if (obj != nullptr)
	{
		obj->Release();
		obj = nullptr;
	}
}

void D3D12Device::release()
{
	//Set the swap chain to windowed before releasing it to avoid stupid exceptions
	if (m_swapChain)
		m_swapChain->SetFullscreenState(false, nullptr);

	//Close the handle to the fence event
	if (CloseHandle(m_fenceEvent) == 0)
		Basilisk::error = "Failed to close fence";

	//Release the fence
	safeRelease(m_fence);
	//Release the empty pipe line state
	safeRelease(m_pipelineState);
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
	//Release the device
	safeRelease(m_device);
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
			Basilisk::error = "Could not set the fence's completion behavior";
			return false;
		}
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	//Use the command allocator and list
	result = m_commandAllocator->Reset();
	if (result != S_OK)
	{
		Basilisk::error = "Could not reset the command allocator";
		return false;
	}
	result = m_commandList->Reset(m_commandAllocator, m_pipelineState);
	if (result != S_OK)
	{
		Basilisk::error = "Could not reset the command list";
		return false;
	}
	return true;
}

bool D3D12Device::execute(ID3D12CommandList **commandLists)
{
	HRESULT result;
	unsigned int numCommands = sizeof(commandLists) / sizeof(commandLists[0]);
	D3D12_RESOURCE_BARRIER barrier;
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle;
	unsigned int renderTargetViewDescriptorSize;
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
		Basilisk::error = "Could not close command list";
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
			Basilisk::error = "Could not present swap chain (with vsync)";
			return 0;
		}
	}
	else
	{
		result = m_swapChain->Present(0, 0);
		if (result != S_OK)
		{
			Basilisk::error = "Could not present the swap chain (without vsync)";
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
		Basilisk::error = "Could not signal the fence";
		return 0;
	}

	//Alternate the back buffer index each frame
	m_bufferIndex = (m_bufferIndex == 0 ? 1 : 0);

	return m_fenceValue++;
}

bool D3D12Device::vsyncEnabled()
{
	return m_vsync;
}
void D3D12Device::setVsyncEnabled(bool enable)
{
	m_vsync = enable;
}

bool D3D12Device::isFullscreen()
{
	return m_fullscreen;
}
void D3D12Device::setFullscreen(bool fullscreen)
{
	m_swapChain->SetFullscreenState(fullscreen, nullptr);
	m_fullscreen = fullscreen;
}

inline unsigned int D3D12Device::getVideoCardMemory()
{
	return m_videoCardMemory;
}
inline const char *D3D12Device::getVideoCardName()
{
	return m_videoCardDesc;
}
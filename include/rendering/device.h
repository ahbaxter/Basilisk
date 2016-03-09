/**
\file   device.h
\author Andrew Baxter
\date   March 8, 2016

The virtual interface with the selected graphics API

\todo Break up GraphicsPipeline into smaller pieces
\todo Figure out what I'm doing with render and present queues
\todo Look into debug/validation layers
\todo Boot up Vulkan without a window target

*/

#ifndef BASILISK_DEVICE_H
#define BASILISK_DEVICE_H

#include "common.h"
#include "pipeline.h"
#include "swap_chain.h"
#include "command_buffer.h"
#include "frame_buffer.h"
#include "image.h"

namespace Basilisk
{
	/**
	Stores information about a single GPU

	\todo Can Direct3D 12 check what type of device it is (discrete, integrated, etc)?
	\todo Remove?
	*/
	struct PhysicalDevice
	{
		uint32_t memory; //In MB
		uint32_t vendorId;
		uint32_t deviceId;
		std::string name;
		//bool supportsApi;
	};

#ifdef _WIN32
	/**
	Stores information about the platform 
	*/
	struct PlatformInfo
	{
		HINSTANCE connection;
		HWND window;
	};
#else
#error Basilisk has not been ported to non-Windows systems yet
#endif

	/**
	Uses CRTP abstraction to represent an ambiguous device

	\tparam Impl Sets up the Curiously Recurring Template Pattern
	*/
	template<class Impl>
	class Device abstract
	{
	public:
		/**
		Gets this class's CRTP implementation
		\return This class's CRTP implementation
		*/
		inline const Impl &GetImplementation() {
			return static_cast<Impl&>(*this);
		}
		
		/** Cleans up after itself */
		inline void Release() {
			return GetImplementation().Release();
		}

		/**
		Creates a graphics pipeline
		\warning After creation, Basilisk is not responsible for memory management of the resultant object

		\param[out] out Where to store the resulting pipeline
		\param[in] layout What information this pipeline expects to receive
		\param[in] shaders A list of shaders to bind to different stages of the pipeline
		\tparam Specifies which API to use, and what type of pipeline to create
		\return Details about potential failure

		\todo Multiple creation?
		*/
		template<class PipelineType>
		inline Result CreateGraphicsPipeline(PipelineType *&out, const std::vector<Descriptor> &layout, const std::vector<ShaderInfo> &shaders) {
			return GetImplementation().CreateGraphicsPipeline(out, layout, shaders);
		}
		/**
		Creates a swap chain
		\warning After creation, Basilisk is not responsible for memory management of the resultant object

		\param[out] out Where to store the resulting swap chain
		\param[in] resolution The resolution to use. May be ignored, depending on GPU quirks
		\param[in] numBuffers The number of buffers to use. Defaults to 2 (double-buffered). May be clamped to fit the GPU's capabilities
		\tparam SwapChainType Specifies which API to create the swap chain for
		\tparam CmdBufferType Must match the API set in `SwapChainType`
		\return Details about potential failure

		\todo Expand to non-Windows sytems
		*/
		template<class SwapChainType, class CmdBufferType>
		Result CreateSwapChain(SwapChainType *&out, CmdBufferType *cmdBuffer, Bounds2D<uint32_t> resolution, uint32_t numBuffers = 2) {
			return GetImplementation().CreateSwapChain(out, cmdBuffer, resolution, numBuffers);
		}


		/**
		Creates a command buffer
		\warning After creation, Basilisk is not responsible for memory management of the resultant object
		
		\param[out] out Where to store the resulting command buffer
		\param[in] bundle Bundles are small command buffers which can quickly be added to larger command buffers, but cannot be executed directly
		\tparam CmdBuffType Specifies which API to create the command buffer for
		\return Details about potential failure
		*/
		template<class CmdBuffType>
		inline Result CreateCommandBuffer(CmdBuffType *&out, bool bundle = false) {
			return GetImplementation().CreateCommandBuffer(out, bundle);
		}
		
		/**
		Creates a frame buffer
		\warning After creation, Basilisk is not responsible for memory management of the resultant object
		
		\param[out] Where to store the resulting render pass
		\tparam FrameBufferType Specifies which API to create the render pass for
		\return Details abot potential failure
		*/
		template<class FrameBufferType>
		inline Result CreateFrameBuffer(FrameBufferType *&out) {
			return GetImplementation().CreateFrameBuffer(out);
		}
		
		/**
		Presents the swap chain we've rendered to (if any)

		\todo Flesh out
		*/
		inline void Present() {
			return GetImplementation().Present();
		}
	};

	/** Implements the `Device` interface for Direct3D 12 */
	class D3D12Device : public Device<D3D12Device>
	{
	public:
		friend class D3D12Instance; //Make sure that this can be instantiated through a proper `Instance` object

		/** Gives an error message when idiots try to create a Vulkan pipeline with a D3D12 device */
		template<class PipelineType> Result CreateGraphicsPipeline(PipelineType *&out, const std::vector<Descriptor> &layout, const std::vector<ShaderInfo> &shaders) {}
		/**
		Partial specialization of `Device::CreateGraphicsPipeline` for D3D12 graphics pipelines
		\warning After creation, Basilisk is not responsible for memory management of the resultant object

		\param[out] out A pointer to an empty pipeline object
		\param[in] layout What information this pipeline expects to receive
		\param[in] shaders A list of shaders to bind to different stages of the pipeline
		\return Details about potential failure
		*/
		template<> Result CreateGraphicsPipeline(D3D12GraphicsPipeline *&out, const std::vector<Descriptor> &layout, const std::vector<ShaderInfo> &shaders);

		/**
		Gives an error message when idiots try to create a Vulkan swap chain with a D3D12 device
		\tparam SwapChainType Anything that isn't `D3D12SwapChain` will resolve here
		\tparam CmdBufferType Anything that isn't `D3D12CmdBuffer` will resolve here
		*/
		template<class SwapChainType, class CmdBufferType>
		Result CreateSwapChain(SwapChainType *&out, CmdBufferType &cmdBuffer, Bounds2D<uint32_t> resolution, uint32_t numBuffers = 2) {}
		/**
		\brief Partial specialization of `Device::CreateSwapChain`, working only with compatible API objects
		Really, this can be done without a device in D3D12, but Vulkan needs it so I'll just be consistent
		\warning May fall back to fewer buffers than requested, depending on hardware capabilities

		\param[out] out Where to store the resulting swap chain
		\param[in] resolution The resolution to use. May be ignored, depending on GPU quirks
		\param[in] numBuffers The number of buffers to use. Defaults to 2 (double-buffered). May be clamped to fit the GPU's capabilities
		\tparam Specifies which API to create the swap chain for

		\todo Expand to non-Windows sytems
		*/
		template<> Result CreateSwapChain(D3D12SwapChain *&out, D3D12CmdBuffer &cmdBuffer, Bounds2D<uint32_t> resolution, uint32_t numBuffers);

		/** Swaps out backbuffers */
		void Present();

		/** Cleans up after itself */
		void Release();
	private:

		D3D12Device();
		~D3D12Device() = default;

		ID3D12Device *m_device;
		ID3D12CommandQueue *m_commandQueue;
		ID3D12CommandAllocator *m_commandAllocator;
		
		/* Remove from this class:
		ID3D12Fence *m_fence;
		HANDLE m_fenceEvent;
		uint64_t m_fenceValue;
		*/
	};

	/**
	Implements the `Device` interface for Vulkan

	\todo CCompute operations
	*/
	class VulkanDevice : public Device<VulkanDevice>
	{
	public:
		friend class VulkanInstance;
		struct WindowSurface
		{
			VkSurfaceCapabilitiesKHR caps;
			VkSurfaceKHR surface;
			VkFormat colorFormat;
			std::vector<VkPresentModeKHR> presentModes;
		};

		/**
		Gives an error message when idiots try to create a D3D12 pipeline with a Vulkan device
		\tparam Anything other than `VulkanGraphicsPipeline` will resolve here
		*/
		template<class PipelineType>
		Result CreateGraphicsPipeline(PipelineType *&out, const std::vector<Descriptor> &layout, const std::vector<ShaderInfo> &shaders) {}
		/**
		Partial specialization of `Device::CreateGraphicsPipeline`, working only with compatible API objects

		\param[out] out A pointer to an empty pipeline object
		\param[in] layout What information this pipeline expects to receive
		\param[in] shaders A list of shaders to bind to different stages of the pipeline
		\return Details about potential failure
		*/
		template<> Result CreateGraphicsPipeline(VulkanGraphicsPipeline *&out, const std::vector<Descriptor> &layout, const std::vector<ShaderInfo> &shaders);


		/**
		Gives an error message when idiots try to create a D3D12 swap chain with a Vulkan device
		\tparam SwapChainType Anything that isn't `VulkanSwapChain` will resolve here
		\tparam CmdBufferType Anything that isn't `VulkanCmdBuffer` will resolve here
		*/
		template<class SwapChainType, class CmdBufferType>
		Result CreateSwapChain(SwapChainType *&out, CmdBufferType *cmdBuffer, Bounds2D<uint32_t> resolution, uint32_t numBuffers = 2) {}
		/**
		Partial specialization of `Device::CreateSwapChain`, working only with compatible API objects
		
		\param[out] out Where to store the resulting swap chain
		\param[in] cmdBuffer Helps to initialize the swap chain. Must be open for writing when passed.
		\param[in] resolution The resolution to use. May be ignored, depending on GPU quirks
		\param[in] numBuffers The number of buffers to use. Defaults to 2 (double-buffered). May be clamped to fit the GPU's capabilities

		\todo Expand to non-Windows sytems
		*/
		template<> Result CreateSwapChain(VulkanSwapChain *&out, VulkanCmdBuffer *setup, Bounds2D<uint32_t> resolution, uint32_t numBuffers);
		
		/**
		Creates a command buffer
		\warning After creation, Basilisk is not responsible for memory management of the resultant object

		\param[out] out Where to store the resulting command buffer
		\param[in] bundle Bundles are small command buffers which can quickly be added to larger command buffers, but cannot be executed directly
		\tparam CmdBuffType Specifies which API to create the command buffer for
		\return Details about potential failure
		*/
		template<class CmdBuffType>
		inline Result CreateCommandBuffer(CmdBuffType *&out, bool bundle = false) {}

		/**
		Creates a command buffer
		\warning After creation, Basilisk is not responsible for memory management of the resultant object

		\param[out] out Where to store the resulting command buffer
		\param[in] bundle Bundles are small command buffers which can quickly be added to larger command buffers, but cannot be executed directly
		\return Details about potential failure

		\todo Queue index parrameter?
		*/
		inline Result CreateCommandBuffer(VulkanCmdBuffer *&out, bool bundle);

		/**
		Creates a frame buffer, intended for offscreen rendering
		
		\param[out] out Where to store the resulting frame buffer
		\param[in] resolution The size of 
		\param[in] colorFormats Which formats to create each image with. Also used to determine 
		\param[in] enableDepth Should we expect a depth buffer? Defaults to yes
		\return Details about potential failure
		*/
		Result CreateFrameBuffer(VulkanFrameBuffer *&out, Bounds2D<uint32_t> resolution, const std::vector<ImageFormat> &colorFormats = {ImageFormat::R8G8B8A8UNorm}, bool enableDepth = true);

		/**
		Executes pre-recorded commands stored in a command bundle

		\return Details about potential failure
		*/
		Result ExecuteCommands(const VulkanCmdBuffer *commands);


		/** Swaps out backbuffers */
		void Present();

		/** Cleans up after itself */
		void Release();

		static constexpr uint32_t render = 0; //Queue index for present operations
		static constexpr uint32_t present = 1;  //Queue index for render operations
		static constexpr uint32_t compute = 2; //Queue index for compute operations
		//TODO: Are queues thread-friendly?

	private:
		VulkanDevice();
		~VulkanDevice() = default;

		bool MemoryTypeFromProps(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

		std::vector<VkQueue> m_queues;
		VkDevice m_device;
		VkCommandPool m_commandPools[2]; //TODO: Multiple command pools for multithreading

		WindowSurface m_windowSurface;
		
		//TODO: Just store the entire GpuProperties struct in here

		VkFormat m_depthFormat;
		VkImageTiling m_depthTiling;
		VkPhysicalDeviceMemoryProperties m_memoryProps;


		/////////////////////////////////
		// Extention Function Pointers //
		/////////////////////////////////
		void GetExtensionProcs();
		
		//VK_KHR_swapchain
		PFN_vkCreateSwapchainKHR pfnCreateSwapchainKHR;
		PFN_vkDestroySwapchainKHR pfnDestroySwapchainKHR;
		PFN_vkGetSwapchainImagesKHR pfnGetSwapchainImagesKHR;
		PFN_vkAcquireNextImageKHR pfnAcquireNextImageKHR;
		PFN_vkQueuePresentKHR pfnQueuePresentKHR;

		//VK_KHR_display_swapchain
		PFN_vkCreateSharedSwapchainsKHR pfnCreateSharedSwapchainsKHR;
		
	};


	/**
	Uses CRTP abstraction to represent an ambiguous API instance

	\tparam Impl Sets up the Curiously Recurring Template Pattern
	*/
	template<class Impl>
	class Instance abstract
	{
	public:
		/**
		Gets this class's CRTP implementation
		\return This class's CRTP implementation
		*/
		inline const Impl &GetImplementation() {
			return static_cast<Impl&>(*this);
		}

		/**
		Initializes the selected API
		
		\param[in] platformInfo Stores information about the target platform
		\param[in] appName The title of your application
		\return Details about potential failure
		*/
		inline Result Initialize(const PlatformInfo &platformInfo, const std::string &appName = "") {
			return GetImplementation().Initialize(platformInfo, appName);
		}

		/** Cleans up after itself */
		inline void Release() {
			GetImplementation().Release();
		}

		/**
		Counts and stores all connected GPUs
		\param[out] count Where to store the number of connected GPUs
		*/
		inline Result FindGpus(uint32_t *&count) {
			return GetImplementation().FindGpus(count);
		}

		/**
		Creates an API device on the specified GPU

		\param[out] out Where to store the resultant device
		\param[in] gpuIndex Which GPU to use
		\return Details about potential failure
		*/
		template<class DeviceType>
		inline Result CreateDevice(DeviceType *&out, uint32_t gpuIndex = 0) {
			return GetImplementation().CreateDevice(out, gpuIndex);
		}
	protected:
		PlatformInfo m_platformInfo;
	};

	/** Implements the `Instance` interface for Direct3D 12 */
	class D3D12Instance : Instance<D3D12Instance>
	{
	public:
		D3D12Instance();
		~D3D12Instance() = default;

		/**
		Boots up D3D12
		
		\param[in] platformInfo Stores information about the target platform
		\param[in] appName Not used in D3D12
		\return Details about potential failure
		*/
		inline Result Initialize(const PlatformInfo &platformInfo, const std::string &appName = "");
		
		/** Cleans up after itself */
		void Release();

		/**
		Counts and stores all connected GPUs
		\param[out] count Where to store the number of connected GPUs
		*/
		Result FindGpus(uint32_t *&count);

		/** Gives an error message when idiots try to create a Vulkan device with a D3D12 instance */
		template<class DeviceType>
		Result CreateDevice(DeviceType *&out, uint32_t gpuIndex = 0) {}
		
		/**
		Creates a Direct3D 12 device on the specified GPU

		\param[out] out Where to store the resultant device
		\param[in] gpuIndex Which GPU to use
		\return Details about potential failure
		*/
		template<> Result CreateDevice(D3D12Device *&out, uint32_t gpuIndex);

		struct GPU
		{
			IDXGIAdapter *adapter;
			DXGI_ADAPTER_DESC desc;
		};
	private:
		IDXGIFactory4 *m_factory;
		std::vector<GPU> m_gpus;

		static constexpr D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_0;
		static constexpr size_t MAX_DESC_LEN = 128; //Obtained from dxgi.h, line 195
	};

	/** Implements the `Instance` interface for Vulkan */
	class VulkanInstance : Instance<VulkanInstance>
	{
	public:
		VulkanInstance();
		~VulkanInstance() = default;

		/**
		Boots up Vulkan
		
		\param[in] platformInfo Stores information about the target platform
		\param[in] appName The title of your application
		\return Details about potential failure
		*/
		inline Result Initialize(const PlatformInfo &platformInfo, const std::string &appName = "");

		/** Cleans up after itself */
		void Release();

		/**
		Counts and stores all connected GPUs
		\param[out] count Where to store the number of connected GPUs
		
		\todo Check out VkPhysicalDeviceMemoryProperties
		*/
		Result FindGpus(uint32_t *&count);

		/** Gives an error message when idiots try to create a D3D12 device with a Vulkan instance */
		template<class DeviceType>
		Result CreateDevice(DeviceType *&out, uint32_t gpuIndex) {}

		/**
		Creates a Vulkan device on the specified GPU

		\param[out] out Where to store the resultant device
		\param[in] gpuIndex Which GPU to use
		\return Details about potential failure

		\todo Multiple command pools
		*/
		template<> Result CreateDevice(VulkanDevice *&out, uint32_t gpuIndex);

		struct GpuProperties
		{
			VkPhysicalDeviceProperties props;
			std::vector<VkQueueFamilyProperties> queueDescs;
			VkPhysicalDeviceMemoryProperties memoryProps;
			VkFormat depthFormat;
			VkImageTiling depthTiling;
		};

	private:
		VkInstance m_instance;
		std::vector<GpuProperties> m_gpuProps;
		std::vector<VkPhysicalDevice> m_gpus;
		
		std::vector<VkSurfaceKHR> m_surfacesCreated;


		/////////////////////////////////
		// Extention Function Pointers //
		/////////////////////////////////

		//VK_KHR_surface
		PFN_vkDestroySurfaceKHR pfnDestroySurfaceKHR;
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR pfnGetPhysicalDeviceSurfaceSupportKHR;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR pfnGetPhysicalDeviceSurfaceCapabilitiesKHR;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR pfnGetPhysicalDeviceSurfaceFormatsKHR;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR pfnGetPhysicalDeviceSurfacePresentModesKHR;

		//VK_KHR_win32_surface
		PFN_vkCreateWin32SurfaceKHR pfnCreateWin32SurfaceKHR;
		PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR pfnGetPhysicalDeviceWin32PresentationSupportKHR;

		static constexpr uint32_t layerCount = 0;
		static const char* const* layerNames;
		
		static constexpr uint32_t extensionCount = 4;
		static const char* extensionNames[extensionCount];

		static constexpr uint32_t apiVersion = VK_API_VERSION;
	};
}

#endif
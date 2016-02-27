/**
\file   device.h
\author Andrew Baxter
\date   February 26, 2016

The virtual interface with the selected graphics API

\todo Don't shirk off creating the pipeline object

*/

#ifndef BASILISK_DEVICE_H
#define BASILISK_DEVICE_H

#include "common.h"
#include "pipeline.h"
#include "swap_chain.h"
#include "command_buffer.h"

namespace Basilisk
{
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
		\tparam Specifies which API to use, and what type of pipeline to create
		\todo Multiple creation?
		*/
		template<class PipelineType>
		inline Result CreateGraphicsPipeline(PipelineType *&out) {
			return GetImplementation().CreateGraphicsPipeline(out);
		}
		/**
		Creates a swap chain
		\warning After creation, Basilisk is not responsible for memory management of the resultant object
		\warning May fall back to fewer buffers than requested, depending on hardware capabilities

		\param[out] out Where to store the resulting swap chain
		\param[in] connection The platform-specific connection to the process
		\param[in] window The window to render to
		\param[in] numBuffers The number of buffers to use. Defaults to 2 (double-buffered)
		\param[in] resolution The resolution to use. Defaults to native resolution
		\param[in] numSamples Number of samples in hardware-level antialiasing. Defaults to 1 (off)
		\tparam Specifies which API to create the swap chain for

		\todo Expand to non-Windows sytems
		*/
		template<class SwapChainType>
		inline Result CreateSwapChain(SwapChainType *&out, HINSTANCE connection, HWND window, Bounds2D<uint16_t> resolution = { 0, 0 }, uint8_t numBuffers = 2, uint8_t numSamples = 1) {
			return GetImplementation().CreateSwapChain(out, connection, window, resolution, numBuffers, numSamples);
		}
	};

	/** Implements the `Device` interface for Direct3D 12 */
	class D3D12Device : public Device<D3D12Device>
	{
	public:
		friend class D3D12Instance; //Make sure that this can be instantiated through a proper `Instance` object

		/** Gives an error message when idiots try to create a Vulkan pipeline with a D3D12 device */
		template<class PipelineType> Result CreateGraphicsPipeline(PipelineType *&out) {
			static_assert(false, "Basilisk::D3D12Device::CreateGraphicsPipeline() is not specialized for the provided type");
		}
		/**
		Partial specialization of `Device::CreateGraphicsPipeline` for D3D12 graphics pipelines
		\warning After creation, Basilisk is not responsible for memory management of the resultant object

		\param[out] out A pointer to an empty pipeline object
		\return Details about potential failure
		*/
		template<> Result CreateGraphicsPipeline<D3D12GraphicsPipeline>(D3D12GraphicsPipeline *&out);

		/**
		Gives an error message when idiots try to create a Vulkan swap chain with a D3D12 device
		\tparam SwapChainType Anything that isn't `D3D12SwapChain` will resolve here
		*/
		template<class SwapChainType> Result CreateSwapChain(SwapChainType *&out, HINSTANCE connection, HWND window, Bounds2D<uint16_t> resolution = { 0, 0 }, uint8_t numBuffers = 2, uint8_t numSamples = 1) {
			static_assert(false, "Basilisk::D3D12Device::CreateSwapChain() is not specialized for the provided type");
		}
		/**
		\brief Partial specialization of `Device::CreateSwapChain`, working only with compatible API objects
		Really, this can be done without a device in D3D12, but Vulkan needs it so I'll just be consistent
		\warning May fall back to fewer buffers than requested, depending on hardware capabilities

		\param[out] out Where to store the resulting swap chain
		\param[in] connection The platform-specific connection to the process
		\param[in] window The window to render to
		\param[in] numBuffers The number of buffers to use. Defaults to 2 (double-buffered)
		\param[in] resolution The resolution to use. Defaults to native resolution
		\param[in] numSamples Number of samples in hardware-level antialiasing. Defaults to 1 (off)
		\tparam Specifies which API to create the swap chain for

		\todo Expand to non-Windows sytems
		*/
		template<> Result CreateSwapChain<D3D12SwapChain>(D3D12SwapChain *&out, HINSTANCE connection, HWND window, Bounds2D<uint16_t> resolution, uint8_t numBuffers, uint8_t numSamples);


		/** Cleans up after itself */
		void Release();
	private:

		D3D12Device();
		~D3D12Device() = default; //All handled in the `Release()` function

		ID3D12Device *m_device;
		ID3D12CommandQueue *m_commandQueue;
		ID3D12CommandAllocator *m_commandAllocator;
		IDXGIFactory4 *m_factory;
		
		/* Remove from this class:
		ID3D12GraphicsCommandList *m_commandList;
		ID3D12Fence *m_fence;
		HANDLE m_fenceEvent;
		uint64_t m_fenceValue;|
		*/
	};

	/** Implements the `Device` interface for Vulkan */
	class VulkanDevice : public Device<VulkanDevice>
	{
	public:
		friend class VulkanInstance; //Make sure that this can be instantiated through a proper `Instance` object

		/** Gives an error message when idiots try to create a D3D12 pipeline with a Vulkan device */
		template<class PipelineType> Result CreateGraphicsPipeline(PipelineType *&out) {
			static_assert(false, "Basilisk::VulkanDevice::CreateGraphicsPipeline() is not specialized for the provided type");
		}
		/**
		Partial specialization of `Device::CreateGraphicsPipeline`, working only with compatible API objects

		\param[out] out A pointer to an empty pipeline object
		\return Details about potential failure
		*/
		template<> Result CreateGraphicsPipeline<VulkanGraphicsPipeline>(VulkanGraphicsPipeline *&out);


		/**
		Gives an error message when idiots try to create a D3D12 swap chain with a Vulkan device
		\tparam SwapChainType Anything that isn't `VulkanSwapChain` will resolve here
		*/
		template<class SwapChainType> Result CreateSwapChain(SwapChainType *&out, HINSTANCE connection, HWND window, Bounds2D<uint16_t> resolution = { 0, 0 }, uint8_t numBuffers = 2, uint8_t numSamples = 1) {
			static_assert(false, "Basilisk::D3D12Device::CreateSwapChain() is not specialized for the provided type");
		}
		/**
		Partial specialization of `Device::CreateSwapChain`, working only with compatible API objects
		
		\warning May fall back to fewer buffers than requested, depending on hardware capabilities

		\param[out] out Where to store the resulting swap chain
		\param[in] connection The platform-specific connection to the process
		\param[in] window The window to render to
		\param[in] numBuffers The number of buffers to use. Defaults to 2 (double-buffered)
		\param[in] resolution The resolution to use. Defaults to native resolution
		\param[in] numSamples Number of samples in hardware-level antialiasing. Defaults to 1 (off)
		\tparam Specifies which API to create the swap chain for

		\todo Expand to non-Windows sytems
		*/
		template<> Result CreateSwapChain<VulkanSwapChain>(VulkanSwapChain *&out, HINSTANCE connection, HWND window, Bounds2D<uint16_t> resolution, uint8_t numBuffers, uint8_t numSamples);

		/** Cleans up after itself */
		void Release();

		struct Queue
		{
			VkQueue queue;
			VkQueueFamilyProperties props;
		};

	private:
		VulkanDevice();
		~VulkanDevice() = default; //All handled in the `Release()` function

		VkDevice m_device;
		std::vector<Queue> m_queues;

		VkPhysicalDevice m_gpu; //Needed to create swap chains
		VkInstance m_instance; //Needed to create swap chains
	};


	/**
	Stores information about a single GPU

	\todo Can Direct3D 12 check what type of device it is (discrete, integrated, etc)?
	*/
	struct PhysicalDevice
	{
		uint32_t memory; //In MB
		uint32_t vendorId;
		uint32_t deviceId;
		std::string name;
		//bool supportsApi;
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
		inline const Impl &GetImplementation()
		{
			return static_cast<Impl&>(*this);
		}

		/**
		Initializes the selected API
		
		\param[in] appName The title of your application
		\return Details about potential failure
		*/
		inline Result Initialize(const std::string &appName = "") {
			return GetImplementation().Initialize(appName);
		}

		/** Cleans up after itself */
		inline void Release() {
			GetImplementation().Release();
		}

		/**
		Counts and stores all connected GPUs
		\param[out] count Where to store the number of connected GPUs
		*/
		inline Result FindGpus(uint32_t *count) {
			return GetImplementation().FindGpus(count);
		}

		/**
		Creates an API device on the specified GPU

		\param[in] gpuIndex Which GPU to use
		\param[out] out Where to store the resultant device
		\return Details about potential failure
		*/
		template<class DeviceType>
		inline Result CreateDevice(uint32_t gpuIndex, DeviceType *&out) {
			return GetImplementation().CreateDevice(gpuIndex, out);
		}
	};

	/** Implements the `Instance` interface for Direct3D 12 */
	class D3D12Instance : Instance<D3D12Instance>
	{
	public:
		D3D12Instance();
		~D3D12Instance() = default;

		/**
		Boots up D3D12
		\param[in] appName Not used in D3D12
		*/
		Result Initialize(const std::string &appName = "");
		
		/** Cleans up after itself */
		void Release();

		/**
		Counts and stores all connected GPUs
		\param[out] count Where to store the number of connected GPUs
		*/
		Result FindGpus(uint32_t *count);

		/** Gives an error message when idiots try to create a Vulkan device with a D3D12 instance */
		template<class DeviceType>
		Result CreateDevice(uint32_t gpuIndex, DeviceType *&out) {
			static_assert(false, "Basilisk::D3D12Instance::CreateDevice() is not specialized for the provided type");
		}
		
		/**
		Creates a Direct3D 12 device on the specified GPU

		\param[in] gpuIndex Which GPU to use
		\param[out] out Where to store the resultant device
		\return Details about potential failure
		*/
		template<> Result CreateDevice<D3D12Device>(uint32_t gpuIndex, D3D12Device *&out);

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
		\param[in] appName Lets GPU drivers know who you are

		\todo Look into custom memory allocation
		*/
		Result Initialize(const std::string &appName = "");

		/** Cleans up after itself */
		void Release();

		/**
		Counts and stores all connected GPUs
		\param[out] count Where to store the number of connected GPUs
		
		\todo Check out VkPhysicalDeviceMemoryProperties
		*/
		Result FindGpus(uint32_t *count);

		/** Gives an error message when idiots try to create a D3D12 device with a Vulkan instance */
		template<class DeviceType>
		Result CreateDevice(uint32_t gpuIndex, DeviceType *&out) {
			static_assert(false, "Basilisk::VulkanInstance::CreateDevice() is not specialized for the provided type");
		}

		/**
		Creates a Vulkan device on the specified GPU

		\param[in] gpuIndex Which GPU to use
		\param[out] out Where to store the resultant device
		\return Details about potential failure
		*/
		template<> Result CreateDevice<VulkanDevice>(uint32_t gpuIndex, VulkanDevice *&out);

		struct GPU
		{
			VkPhysicalDevice device;
			VkPhysicalDeviceProperties props;
			std::vector<VkQueueFamilyProperties> queueDescs;
			VkPhysicalDeviceMemoryProperties memoryProps;
		};

	private:
		VkInstance m_instance;
		static constexpr uint32_t layerCount = 0;
		static const char* const* layerNames;
		
		static constexpr uint32_t extensionCount = 3;
		static const char* extensionNames[extensionCount];

		std::vector<GPU> m_gpus;
		
		static constexpr uint32_t apiVersion = 1;
	};
}

#endif
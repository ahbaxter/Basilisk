/**
\file   device.h
\author Andrew Baxter
\date   February 21, 2016

The virtual interface with the selected graphics API

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
	Uses CRTP abstraction to represent an API-ambiguous device

	\tparam Impl Sets up the Curiously Recurring Template Pattern
	*/
	template<class Impl>
	class Device abstract
	{
	public:
		/**
		Gets this class's RCTP implementation
		\return This class's RCTP implementation
		*/
		inline const Impl &GetImplementation() {
			return static_cast<Impl&>(*this);
		}
		
		/**
		Cleans up after itself
		*/
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

		\param[out] out Where to store the resulting swap chain
		\param[in] window The window to render to
		\param[in] numBuffers The number of buffers to use. Defaults to 2 (double-buffered)
		\param[in] resolution The resolution to use. Defaults to native resolution
		\tparam Specifies which API to create the swap chain for

		\todo Expand to non-Windows sytems
		*/
		template<class SwapChainType>
		inline Result CreateSwapChain(SwapChainType *&out, HWND window, uint8_t numBuffers = 2, Bounds2D<uint16_t> resolution = { 0, 0 }, uint8_t antiAliasing = 1) {
			return GetImplementation().CreateSwapChain(out, window, numBuffers, resolution, antiAliasing);
		}
	};

	/**
	Implements the `Device` interface for Direct3D 12
	*/
	class D3D12Device : public Device<D3D12Device>
	{
	public:
		friend class D3D12Instance; //Make sure that this can be instantiated through a proper `Instance` object

		/**
		Gives an error message when idiots try to create a Vulkan pipeline with a D3D12 device
		*/
		template<class PipelineType> Result CreateGraphicsPipeline(PipelineType *&out) {
			static_assert(false, "Basilisk::D3D12Device::CreateGraphicsPipeline() is not specialized for the provided type.");
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
		template<class SwapChainType> Result CreateSwapChain(SwapChainType *&out) {
			static_assert(false, "Basilisk::D3D12Device::CreateSwapChain() is not specialized for the provided type.");
		}
		/**
		\brief Partial specialization of `Device::CreateSwapChain`, working only with compatible API objects
		Really, this can be done without a device in D3D12, but Vulkan needs it so I'll just be consistent
		\warning After creation, Basilisk is not responsible for memory management of the resultant object

		\param[out] out A pointer to an empty swap chain object
		\return Details about potential failure
		*/
		template<> Result CreateSwapChain<D3D12SwapChain>(D3D12SwapChain *&out);


		/**
		Cleans up after itself
		*/
		void Release();
	private:

		D3D12Device();
		~D3D12Device() = default; //All handled in the `Release()` function

		ID3D12Device *m_device;

		/*
		ID3D12CommandQueue *m_commandQueue;
		ID3D12CommandAllocator *m_commandAllocator; //Remove from device class?
		ID3D12GraphicsCommandList *m_commandList; //Remove from device class?
		ID3D12Fence *m_fence;
		HANDLE m_fenceEvent;
		uint64_t m_fenceValue;
		*/
	};

	/**
	Implements the `Device` interface for Vulkan
	*/
	class VulkanDevice : public Device<VulkanDevice>
	{
	public:
		friend class VulkanInstance; //Make sure that this can be instantiated through a proper `Instance` object

		/**
		Gives an error message when idiots try to create a D3D12 pipeline with a Vulkan device
		*/
		template<class PipelineType> Result CreateGraphicsPipeline(PipelineType *&out) {
			static_assert(false, "Basilisk::VulkanDevice::CreateGraphicsPipeline() is not specialized for the provided type.");
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
		template<class SwapChainType> Result CreateSwapChain(SwapChainType *&out) {
			static_assert(false, "Basilisk::D3D12Device::CreateSwapChain() is not specialized for the provided type.");
		}
		/**
		\brief Partial specialization of `Device::CreateSwapChain`, working only with compatible API objects

		\param[out] out A pointer to an empty swap chain object
		\return Details about potential failure
		*/
		template<> Result CreateSwapChain<VulkanSwapChain>(VulkanSwapChain *&out);

		/**
		Cleans up after itself
		*/
		void Release();
	private:
		VulkanDevice();
		~VulkanDevice() = default; //All handled in the `Release()` function

		VkDevice m_device;
	};


	/**

	\todo Can Direct3D 12 check what type of device it is (discrete, integrated, etc)?
	*/
	struct PhysicalDevice
	{
		uint32_t memory; //In MB
		uint32_t vendorId;
		uint32_t deviceId;
		std::string name;
		bool supportsApi;
	};



	/**
	Uses CRTP abstraction to represent an API-ambiguous instance

	\tparam Impl Sets up the Curiously Recurring Template Pattern
	*/
	template<class Impl>
	class Instance abstract
	{
	public:
		/**
		Gets this class's RCTP implementation
		\return This class's RCTP implementation
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

		/**
		Counts and/or lists the number of connected GPUs
		\param[out] count Where to store the number of connected GPUs
		\param[out] details Where to store the details of each connected GPU
		*/
		inline Result EnumeratePhysicalDevices(uint32_t *count, PhysicalDevice *details) {
			return GetImplementation().EnumeratePhysicalDevices(count, details);
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

	/**
	Implements the `Instance` interface for Direct3D 12
	*/
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
		
		/**
		Counts and/or lists the number of connected GPUs
		\param[out] count Where to store the number of connected GPUs
		\param[out] details Where to store the details of each connected GPU
		*/
		Result EnumeratePhysicalDevices(uint32_t *count, PhysicalDevice *details);

		/**
		Gives an error message when idiots try to create a Vulkan device with a D3D12 instance
		*/
		template<class DeviceType>
		Result CreateDevice(uint32_t gpuIndex, DeviceType *&out) {
			static_assert(false, "Basilisk::D3D12Instance::CreateDevice() is not specialized for the provided type.");
		}
		
		/**
		Creates a Direct3D 12 device on the specified GPU

		\param[in] gpuIndex Which GPU to use
		\param[out] out Where to store the resultant device
		\return Details about potential failure
		*/
		template<> Result CreateDevice<D3D12Device>(uint32_t gpuIndex, D3D12Device *&out);
	private:
		IDXGIFactory4 *m_factory;
		static constexpr D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_0;
	};

	/**
	Implements the `Instance` interface for Vulkan
	*/
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

		/**
		Counts and/or lists the number of connected GPUs
		\param[out] count Where to store the number of connected GPUs
		\param[out] details Where to store the details of each connected GPU
		
		\todo Check out VkPhysicalDeviceMemoryProperties
		*/
		Result EnumeratePhysicalDevices(uint32_t *count, PhysicalDevice *details);

		/**
		Gives an error message when idiots try to create a D3D12 device with a Vulkan instance
		*/
		template<class DeviceType>
		Result CreateDevice(uint32_t gpuIndex, DeviceType *&out) {
			static_assert(false, "Basilisk::VulkanInstance::CreateDevice() is not specialized for the provided type.");
		}

		/**
		Creates a Vulkan device on the specified GPU

		\param[in] gpuIndex Which GPU to use
		\param[out] out Where to store the resultant device
		\return Details about potential failure
		*/
		template<> Result CreateDevice<D3D12Device>(uint32_t gpuIndex, D3D12Device *&out);
	private:
		VkInstance m_instance;
		static constexpr uint32_t layerCount = 0;
		static const char* const* layerNames;
		
		static constexpr uint32_t extensionCount = 3;
		static const char* extensionNames[extensionCount];
	};
}

#endif
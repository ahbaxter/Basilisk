/**
\file   device.h
\author Andrew Baxter
\date   February 19, 2016

The virtual interface with the selected graphics API

\todo Vulkan has too many pipelines for each to hav their own function. Use templates.

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
		Gets the derivative class of this object
		\return The derivative class of this object
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
		\brief Creates a graphics pipeline
		After creation, Basilisk is not responsible for memory management of the resultant object
		\param[out] out Where to store the resulting pipeline
		\tparam Specifies which API to create the pipeline for
		\todo Multiple creation?
		*/
		template<class PipelineType>
		inline Result CreateGraphicsPipeline(PipelineType **out) {
			return GetImplementation().CreateGraphicsPipeline(out);
		}
		/**
		\brief Creates a compute pipeline
		After creation, Basilisk is not responsible for memory management of the resultant object
		\param[out] out Where to store the resulting pipeline
		\tparam Specifies which API to create the pipeline for
		\todo Multiple creation?
		*/
		template<class PipelineType>
		inline Result CreateComputePipeline(PipelineType **out) {
			return GetImplementation().CreateComputePipeline(out);
		}
		/**
		\brief Creates a swap chain
		After creation, Basilisk is not responsible for memory management of the resultant object

		\param[out] out Where to store the resulting swap chain
		\param[in] window The window to render to
		\param[in] numBuffers The number of buffers to use. Defaults to 2 (double-buffered)
		\param[in] resolution The resolution to use. Defaults to native resolution
		\tparam Specifies which API to create the swap chain for

		\todo Expand to non-Windows sytems
		*/
		template<class SwapChainType>
		inline Result CreateSwapChain(SwapChainType **out, HWND window, uint8_t numBuffers = 2, Bounds2D<uint16_t> resolution = { 0, 0 }, uint8_t antiAliasing = 1) {
			return GetImplementation().CreateSwapChain(out, window, numBuffers, resolution, antiAliasing);
		}



		/**
		Changes the active graphics pipeline
		\param[in] pipeline The pipeline to use. If `nullptr`, a default pipeline is used
		\tparam PipelineType Specifies which API to use
		*/
		template<class PipelineType>
		inline Result BindGraphicsPipeline(PipelineType *pipeline) {
			return GetImplementation().BindGraphicsPipeline(pipeline);
		}
		/**
		Changes the active graphics pipeline
		\param[in] pipeline The pipeline to use. If `nullptr`, a default pipeline is used
		\tparam PipelineType Specifies which API to use
		*/
		template<class PipelineType>
		inline Result BindComputePipeline(PipelineType *pipeline) {
			return GetImplementation().BindComputePipeline(pipeline);
		}
		/**
		Changes the active swap chain
		\param[in] swapChain The swap chain to use. If `nullptr`, a default pipeline is used
		\tparam PipelineType Specifies which API to use
		*/
		template<class SwapChainType>
		inline Result BindSwapChain(SwapChainType *swapChain) {
			return GetImplementation().BindSwapChain(swapChain);
		}

	protected:
		Device() = 0;
		~Device() = 0;
	};

	class D3D12Device : public Device<D3D12Device>
	{
	public:
		friend class D3D12Instance; //Make sure that this can be instantiated through a proper `Instance` object

		/**
		Gives an error message when idiots ty to create a Vulkan pipeline with a D3D12 device
		*/
		template<class PipelineType> Result CreateGraphicsPipeline(PipelineType **out) {
			static_assert("Basilisk::D3D12Device::CreateGraphicsPipeline is not specialized for the provided type.");
		}
		/**
		Partial specialization of `Device::CreateGraphicsPipeline`, working only with compatible API objects

		\param[out] out A pointer to an empty pipeline object
		\return Details about potential failure
		*/
		template<> Result CreateGraphicsPipeline<D3D12GraphicsPipeline>(D3D12GraphicsPipeline **out);

		/**
		Gives an error message when idiots ty to create a Vulkan pipeline with a D3D12 device
		*/
		template<class PipelineType> Result CreateSwapChain(SwapChainType **out)
		{
			static_assert("Basilisk::D3D12Device::CreateSwapChain is not specialized for the provided type.");
		}
		/**
		\brief Partial specialization of `Device::CreateGraphicsPipeline`, working only with compatible API objects
		Technically, this can be done without a device handle in D3D12, but 

		\param[out] out A pointer to an empty pipeline object
		\return Details about potential failure
		*/
		template<> Result CreateGraphicsPipeline<D3D12GraphicsPipeline>(D3D12GraphicsPipeline **out);


		/**
		Cleans up after itself
		*/
		void Release();
	private:

		D3D12Device();
		~D3D12Device() = default; //All handled in the `Release()` function

		ID3D12Device *m_device;
		/*
		uint64_t m_fenceValue;
		ID3D12CommandQueue *m_commandQueue;
		ID3D12CommandAllocator *m_commandAllocator; //@todo remove from device class
		ID3D12GraphicsCommandList *m_commandList; //@todo remove from device class
		ID3D12PipelineState *m_pipelineState;
		ID3D12Fence *m_fence;
		HANDLE m_fenceEvent;
		*/
	};

	class VulkanDevice : public Device<VulkanDevice>
	{
	public:
		friend class VulkanInstance; //Make sure that this can be instantiated through a proper `Instance` object

		/**
		Gives an error message when idiots ty to create a D3D12 pipeline with a Vulkan device
		*/
		template<class PipelineType> Result CreateGraphicsPipeline(PipelineType **out)
		{
			static_assert("Basilisk::VulkanDevice::CreateGraphicsPipeline is not specialized for the provided type.");
		}
		/**
		Partial specialization of `Device::CreateGraphicsPipeline`, working only with compatible API objects

		\param[out] out A pointer to an empty pipeline object
		\return Details about potential failure
		*/
		template<> Result CreateGraphicsPipeline<VulkanGraphicsPipeline>(VulkanGraphicsPipeline **out);

		/**
		Cleans up after itself
		*/
		void Release();
	private:
		VulkanDevice();
		~VulkanDevice() = default; //All handled in the `Release()` function

		VkDevice m_device;
	};


	struct PhysicalDevice
	{
		uint32_t memory; //In MB
		std::string name;
		std::string vendor;
	};

	template<class Impl>
	class Instance abstract
	{
	public:
		inline const Impl &GetImplementation()
		{
			return static_cast<Impl&>(*this);
		}

		/**
		Initializes the selected API to a 
		*/
		inline Result Initialize() {
			return GetImplementation().Initialize();
		}

		/**
		Counts and/or lists the number of connected GPUs
		\param[out] count Where to store the number of connected GPUs
		\param[out] details Where to store the details of each connected GPU
		*/
		inline Result EnumeratePhysicalDevices(uint8_t **count, PhysicalDevice **details) {
			return GetImplementation().EnumeratePhysicalDevices(count, details);
		}

		/**
		\param[in] gpuIndex
		\param[out] out
		*/
		template<class DeviceType>
		inline Result CreateDevice(uint8_t gpuIndex, DeviceType **out) {
			return GetImplementation().CreateDevice(gpuIndex, out);
		}
	};

	class D3D12Instance : Instance<D3D12Instance>
	{
	public:

	};

	class VulkanInstance : Instance<VulkanInstance>
	{
	public:

	};
}

#endif
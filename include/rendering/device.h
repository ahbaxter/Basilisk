/**
\file   device.h
\author Andrew Baxter
\date   February 19, 2016

The virtual interface with the selected graphics API
I honestly have no idea if this design is going to work

\todo Do Devices need to know resolution to initialize? Don't think so.

*/

#ifndef BASILISK_DEVICE_H
#define BASILISK_DEVICE_H

#include "common.h"
#include "pipeline.h"
//#include "swap_chain.h"

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
		Starts up the desired API with a few starting parameters

		\param[in] window The Win32 Window to render to
		\param[in] resolution The resolution to 
		\return Details about potential failure

		\todo Add support for non-Windows systems
		*/
		inline Result Initialize(HWND window, Bounds2D<uint16_t> resolution = { 0, 0 }) {
			return GetImplementation().Initialize(window, resolution, fullscreen, vsync);
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
		inline Result CreateGraphicsPipeline(PipelineType *out) {
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
		inline Result CreateComputePipeline(PipelineType *out) {
			return GetImplementation().CreateComputePipeline(out);
		}

		/**
		\brief Creates a swap chain
		After creation, Basilisk is not responsible for memory management of the resultant object
		\param[out] out Where to store the resulting swap chain
		\tparam Specifies which API to create the swap chain for
		*/
		template<class SwapChainType>
		inline Result CreateSwapChain(SwapChainType *out) {
			return GetImplementation().CreateSwapChain(out);
		}
	protected:
		Device() = 0;
		~Device() = 0;
	};

	class D3D12Device : public Device<D3D12Device>
	{
	public:
		friend class D3D12Instance; //Make sure that this can be instantiated through a proper `Instance` object

		Result Initialize(HWND window, Bounds2D<uint16_t> resolution = { 0, 0 });

		/**
		Gives an error message when idiots ty to create a Vulkan pipeline with a D3D12 device
		*/
		template<class PipelineType> Result CreateGraphicsPipeline(PipelineType *out)
		{
			static_assert("Basilisk::D3D12Device::CreateGraphicsPipeline is not specialized for the provided type.");
		}
		/**
		Partial specialization of `Device::CreateGraphicsPipeline`, working only with compatible API objects

		\param[out] out A pointer to an empty pipeline object
		\return Details about potential failure
		*/
		template<> Result CreateGraphicsPipeline<D3D12GraphicsPipeline>(D3D12GraphicsPipeline *out);
		



		
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
		IDXGISwapChain3 *m_swapChain;
		ID3D12DescriptorHeap *m_renderTargetViewHeap;
		ID3D12Resource *m_backBufferRenderTarget[2];
		uint32_t m_bufferIndex;
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

		Result Initialize(HWND window, Bounds2D<uint16_t> resolution = { 0, 0 });
		/**
		Gives an error message when idiots ty to create a D3D12 pipeline with a Vulkan device
		*/
		template<class PipelineType> Result CreateGraphicsPipeline(PipelineType *out)
		{
			static_assert("Basilisk::VulkanDevice::CreateGraphicsPipeline is not specialized for the provided type.");
		}
		/**
		Partial specialization of `Device::CreateGraphicsPipeline`, working only with compatible API objects

		\param[out] out A pointer to an empty pipeline object
		\return Details about potential failure
		*/
		template<> Result CreateGraphicsPipeline<VulkanGraphicsPipeline>(VulkanGraphicsPipeline *out);

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
		inline const Impl &GetImplementation()
		{
			return static_cast<Impl&>(*this);
		}

		/**
		\param[out] count
		\param[out] details
		*/
		inline Result EnumeratePhysicalDevices(uint8_t *count, PhysicalDevice **details) {
			return GetImplementation().EnumeratePhysicalDevices(count, details);
		}

		/**
		\param[in] gpuIndex
		\param[out] out
		*/
		template<class DeviceType>
		inline Result CreateDevice(uint8_t gpuIndex, DeviceType *out) {
			return GetImplementation().CreateDevice(gpuIndex, out);
		}
	};

	class D3D12Instance : Instance<D3D12Instance>
	{

	};

	class VulkanInstance : Instance<VulkanInstance>
	{

	};

	/*
	class D3D12Device final
	{
	public:
		bool BeginFrame(unsigned long long fenceValue);
		bool Execute(ID3D12CommandList **commandLists);
		/**
		\return The fence value indicating completion in BeginFrame()
		/
		unsigned long long Present();
		
	private:
	};
	*/
}

#endif
/**
\file   device.h
\author Andrew Baxter
\date   February 19, 2016

The virtual interface with the selected graphics API
I honestly have no idea if this design is going to work

\todo Devices do too much right now -- distribute to multiple classes
\todo Add an `Instance` class, to make sure we're not automatically restricted to a single GPU at a time

*/

#ifndef BASILISK_DEVICE_H
#define BASILISK_DEVICE_H

#include "common.h"
#include "pipeline.h"


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
		inline Result Initialize(HWND window, Bounds2D<uint16_t> resolution = { 0, 0 }, bool fullscreen = true, bool vsync = false) {
			return GetImplementation().Initialize(window, width, height, fullscreen, vsync);
		}
		
		/**
		Cleans up after itself
		*/
		inline void Release() {
			return GetImplementation().Release();
		}

		/**
		\brief Creates a graphics pipeline
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
		\param[out] out Where to store the resulting pipeline
		\tparam Specifies which API to create the pipeline for
		\todo Multiple creation?
		*/
		template<class PipelineType>
		inline Result CreateComputePipeline(PipelineType *out) {
			return GetImplementation().CreateComputePipeline(out);
		}

		/**
		Checks if vsync is enabled
		\return `true` if vsync is enabled; `false` otherwise
		*/
		inline bool IsVsyncEnabled() { return m_vsync; }
		/**
		Checks if the target window is fullscreen
		\return `true` if the appication is fullscreen; `false` otherwise
		*/
		inline bool IsFullscreen() { return m_fullscreen; }
		/**
		Gets the video card name
		\return the video card name
		*/
		inline const char *GetVideoCardName() { return m_videoCardDesc; }
		/**
		Gets the total amount of GPU memory, in megabytes
		\return The total amount of GPU memory, in megabytes
		*/
		inline uint32_t GetVideoCardMemory() { return m_videoCardMemory; } //In megabytes

		/**
		Turns vsync on or off
		\param[in] vsync `true` to enable vsync; `false` to disable
		*/
		inline void SetVsyncEnabled(bool vsync) {
			GetImplementation().SetVsyncEnabled(vsync)
		}
		/**
		Turns vsync on or off
		\param[in] vsync `true` to enable vsync; `false` to disable
		*/
		inline void SetFullscreenEnabled(bool fullscreen) {
			GetImplementation().SetFullscreenEnabled(fullscreen);
		}

		//Only allow derivative classes to be instantiated
		//Makes sure that CRTP is not sidestepped
		friend class D3D12Device;
		friend class VulkanDevice;
	protected:
		Device() = 0;
		~Device() = 0;

		static constexpr size_t DESC_LEN = 128;
		char m_videoCardDesc[DESC_LEN];
		uint32_t m_videoCardMemory;

		bool m_vsync; //Migrate to a SwapChain object
		bool m_fullscreen; //Migrate to a SwapChain object
	};

	class D3D12Device : public Device<D3D12Device>
	{
	public:
		Result Initialize(HWND window, Bounds2D<uint16_t> resolution = { 0, 0 }, bool fullscreen = true, bool vsync = false);
		
		void SetVsyncEnabled(bool vsync);
		void SetFullscreenEnabled(bool fullscreen);

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
	private:
		VulkanDevice();
		~VulkanDevice() = default; //All handled in the `Release()` function -- once written

		VkDevice m_device;
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
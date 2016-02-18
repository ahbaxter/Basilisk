/**
\file   device.h
\author Andrew Baxter
\date   February 17, 2016

The virtual interface with the selected graphics API

\todo Document
\todo Finish
\todo Make into factories for pipelines

*/

#ifndef BASILISK_DEVICE_H
#define BASILISK_DEVICE_H

#include "common.h"
#include "pipeline.h"


namespace Basilisk
{
	template<class Impl>
	class Device
	{
	public:
		inline const Impl &GetImplementation()
		{
			return static_cast<Impl&>(*this);
		}

		//Allow implementations, and only implementations to instanciate a Device object
		//Makes sure that CRTP is not sidestepped
		friend class D3D12Device;
		friend class VulkanDevice;
	private:
		Device();
		~Device();


	};

	class D3D12Device : public Device<D3D12Device>
	{

	};

	class VulkanDevice : public Device<VulkanDevice>
	{

	};


	class D3D12Device final
	{
	public:
		D3D12Device();
		~D3D12Device();

		bool initialize(HWND window, unsigned int width=0, unsigned int height=0, bool fullscreen=false, bool vsync=false);
		void release();
		bool beginFrame(unsigned long long fenceValue);
		bool execute(ID3D12CommandList **commandLists);
		/**
		@return The fence value indicating completion in beginFrame()
		*/
		unsigned long long present();

		//bool createResource();
		//bool deleteResource();
		//bool readbackResource();
		
		inline bool vsyncEnabled();
		void setVsyncEnabled(bool enable);

		inline bool isFullscreen();
		void setFullscreen(bool fullscreen); //@todo buggy

		
		inline unsigned int getVideoCardMemory();
		inline const char *getVideoCardName();
		
	private:
		static constexpr size_t DESC_LEN = 128;

		bool m_vsync;
		bool m_fullscreen;

		ID3D12Device *m_device;
		ID3D12CommandQueue *m_commandQueue;
		IDXGISwapChain3 *m_swapChain;
		ID3D12DescriptorHeap *m_renderTargetViewHeap;
		ID3D12Resource *m_backBufferRenderTarget[2];
		unsigned int m_bufferIndex;
		ID3D12CommandAllocator *m_commandAllocator; //@todo remove from device class
		ID3D12GraphicsCommandList *m_commandList; //@todo remove from device class
		ID3D12PipelineState *m_pipelineState;
		ID3D12Fence *m_fence;
		HANDLE m_fenceEvent;
		unsigned long long m_fenceValue;
		char m_videoCardDesc[DESC_LEN];
		unsigned int m_videoCardMemory; //in megabytes
	};
}

#endif
#pragma once
#include "../common.h"


namespace Basilisk
{
	class DYNAMIC D3D12Device final
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

#ifdef _VULKAN
	typedef VulkanDevice Device;
#else
	typedef D3D12Device Device;
#endif
}
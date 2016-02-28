/**
\file   swap_chain.h
\author Andrew Baxter
\date February 27, 2016

Implements swap chains in Vulkan and Direct3D 12

\todo Add support for fullscreen rendering

*/

#ifndef BASILISK_SWAP_CHAIN_H
#define BASILISK_SWAP_CHAIN_H

#include "common.h"
#include "image.h"

namespace Basilisk
{
	/**
	Uses CRTP abstraction to represent an ambiguous swap chain

	\tparam Impl Sets up the Curiously Recurring Template Pattern
	*/
	template<class Impl>
	class SwapChain abstract
	{
	public:
		/**
		Gets this class's CRTP implementation
		\return This class's CRTP implementation
		*/
		inline Impl &GetImplementation() {
			return static_cast<Impl&>(*this);
		}

	//	/**
	//	Checks if vsync is enabled
	//	\return `true` if vsync is enabled; `false` otherwise
	//	*/
	//	inline bool IsVsyncEnabled() {
	//		return m_vsync;
	//	}
	//	/**
	//	Checks if the target window is fullscreen
	//	\return `true` if the appication is fullscreen; `false` otherwise
	//	*/
	//	inline bool IsFullscreen() {
	//		return m_fullscreen;
	//	}
	//	/**
	//	Turns vsync on or off
	//	\param[in] vsync `true` to enable vsync; `false` to disable
	//	*/
	//	inline void SetVsyncEnabled(bool vsync) {
	//		GetImplementation().SetVsyncEnabled(vsync)
	//	}
	//	/**
	//	Turns vsync on or off
	//	\param[in] vsync `true` to enable vsync; `false` to disable
	//	*/
	//	inline void SetFullscreenEnabled(bool fullscreen) {
	//		GetImplementation().SetFullscreenEnabled(fullscreen);
	//	}

	//protected:
	//	bool m_vsync, m_fullscreen;
	};

	/**
	Implements the `SwapChain` interface for Direct3D 12
	*/
	class D3D12SwapChain : public SwapChain<D3D12SwapChain>
	{
	public:
		friend class D3D12Device;

	private:
		D3D12SwapChain();
		~D3D12SwapChain() = default;

		IDXGISwapChain3 *m_swapChain;
		ID3D12DescriptorHeap *m_renderTargetViewHeap; //No idea what this is
		ID3D12Resource **m_backBufferRenderTarget; //Back buffers
		uint32_t m_bufferIndex;
	};

	/**
	Implements the `SwapChain` interface for Vulkan
	*/
	class VulkanSwapChain : public SwapChain<VulkanSwapChain>
	{
	public:
		friend class VulkanDevice;
		friend class VulkanCmdBuffer;

	private:
		VulkanSwapChain();
		~VulkanSwapChain() = default;

		VkSwapchainKHR m_swapChain;
		std::vector<VkImage> m_backBuffers;
		std::vector<VkImageView> m_backBufferViews;

		uint32_t m_bufferIndex;
	};
}

#endif
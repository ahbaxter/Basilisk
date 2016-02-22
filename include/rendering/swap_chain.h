/**
\file   swap_chain.h
\author Andrew Baxter
\date February 21, 2016

Implements swap chains in Vulkan and Direct3D 12

*/

#ifndef BASILISK_SWAP_CHAIN_H
#define BASILISK_SWAP_CHAIN_H

#include "common.h"

namespace Basilisk
{
	/**
	Uses CRTP abstraction to represent an API-ambiguous swap chain

	\tparam Impl Sets up the Curiously Recurring Template Pattern
	*/
	template<class Impl>
	class SwapChain abstract
	{
	public:
		/**
		Gets this class's RCTP implementation
		\return This class's RCTP implementation
		*/
		inline Impl &GetImplementation() {
			return static_cast<Impl&>(*this);
		}

		/**
		Checks if vsync is enabled
		\return `true` if vsync is enabled; `false` otherwise
		*/
		inline bool IsVsyncEnabled() {
			return m_vsync;
		}
		/**
		Checks if the target window is fullscreen
		\return `true` if the appication is fullscreen; `false` otherwise
		*/
		inline bool IsFullscreen() {
			return m_fullscreen;
		}
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

	protected:
		bool m_vsync, m_fullscreen;
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
		ID3D12DescriptorHeap *m_renderTargetViewHeap;
		ID3D12Resource **m_backBufferRenderTarget;
		uint32_t m_bufferIndex;
	};

	/**
	Implements the `SwapChain` interface for Vulkan
	*/
	class VulkanSwapChain : public SwapChain<VulkanSwapChain>
	{
	public:
		friend class VulkanDevice;
	private:
		VulkanSwapChain();
		~VulkanSwapChain() = default;

		VkSwapchainKHR m_swapChain;
	};
}

#endif
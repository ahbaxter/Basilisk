/**
\file   swap_chain.h
\author Andrew Baxter
\date February 20, 2016



*/

#ifndef BASILISK_SWAP_CHAIN_H
#define BASILISK_SWAP_CHAIN_H

namespace Basilisk
{
	template<class Impl>
	class SwapChain
	{
	public:
		inline Impl &GetImplementation()
		{
			return static_cast<Impl&>(*this);
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
		bool m_vsync;
		bool m_fullscreen;
	};

	class D3D12SwapChain : public SwapChain<D3D12SwapChain>
	{
	public:
		friend class D3D12Device;
	};

	class VulkanSwapChain : public SwapChain<VulkanSwapChain>
	{
	public:
		friend class VulkanDevice;
	};
}

#endif
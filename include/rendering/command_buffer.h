/**
\file   command_buffer.h
\author Andrew Baxter
\date   February 28, 2016

Encapsulates a command buffers (or command lists in D3D12)

*/

#ifndef BASILISK_COMMAND_BUFFER_H
#define BASILISK_COMMAND_BUFFER_H

#include "common.h"
#include "swap_chain.h"

namespace Basilisk
{
	/**
	Uses CRTP abstraction to represent an ambiguous command buffer

	\tparam Impl Sets up the Curiously Recurring Template Pattern
	*/
	template<class Impl>
	class CmdBuffer abstract
	{
	public:
		/**
		Gets this class's CRTP implementation
		\return This class's CRTP implementation
		*/
		inline const Impl &GetImplementation() {
			return static_cast<Impl&>(*this);
		}

		/**
		Start recording commands
		
		\param[in] disposable 
		\return Details about potential failure
		*/
		inline Result Begin(bool disposable) {
			return GetImplementation.Begin(disposable);
		}

		/**
		Stop recording commands

		\return Details about potential failure
		*/
		inline Result End() {
			return GetImplementation().End();
		}

		/**
		Writes a bundle to the command buffer

		\param[in] bundle The bundle to write
		\return Details about potential failure
		*/
		inline void WriteBundle(const Impl &bundle) {
			GetImplementation().WriteBundle(bundle);
		}

		/**
		Changes the active graphics pipeline

		\param[in] pipeline The pipeline to use
		\tparam PipelineType Specifies which API to use, and what type of pipeline to create
		*/
		template<class PipelineType>
		inline void BindGraphicsPipeline(const PipelineType &pipeline) {
			GetImplementation().BindGraphicsPipeline(pipeline);
		}

		/**
		Changes the active compute pipeline

		\param[in] pipeline The pipeline to use
		\tparam PipelineType Specifies which API to use, and what type of pipeline to create
		*/
		template<class PipelineType>
		inline void BindComputePipeline(const PipelineType &pipeline) {
			GetImplementation().BindComputePipeline(pipeline);
		}
		
		/**
		Changes the active swap chain

		\param[in] swapChain The swap chain to use
		\tparam PipelineType Specifies which API to use
		*/
		template<class SwapChainType>
		inline void BindSwapChain(const SwapChainType &swapChain) {
			GetImplementation().BindSwapChain(swapChain);
		}

		/** Completely wipes the contents of the command list */
		inline void Reset() {
			GetImplementation().Reset();
		}

		/** Cleans up after itself */
		inline void Release() {
			GetImplementation().Release();
		}
	};

	/** Implements the `CmdBuffer` interface for Direct3D 12 */
	class D3D12CmdBuffer : public CmdBuffer<D3D12CmdBuffer>
	{
	public:
		friend class D3D12Device;

	private:
		D3D12CmdBuffer();
		~D3D12CmdBuffer() = default;

		ID3D12GraphicsCommandList *m_commandList;
	};

	/** Implements the `CmdBuffer` interface for Vulkan */
	class VulkanCmdBuffer : public CmdBuffer<VulkanCmdBuffer>
	{
	public:
		friend class VulkanDevice;

		Result Begin(bool disposable);

		void WriteBundle(const VulkanCmdBuffer &bundle);

		Result End();

		/**
		Before use, Vulkan swap chains need a little more preparation
		
		\param[in] swapChain The swap chain to prepare
		*/
		void PrepareSwapChain(const VulkanSwapChain &swapChain);

	private:
		VulkanCmdBuffer();
		~VulkanCmdBuffer() = default;

		VkCommandBuffer m_commandBuffer;
	};

}

#endif
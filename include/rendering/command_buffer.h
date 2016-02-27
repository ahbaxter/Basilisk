/**
\file   command_buffer.h
\author Andrew Baxter
\date   February 21, 2016

Encapsulates an API-generic command buffer

\todo Add Bundles

*/

#ifndef BASILISK_COMMAND_BUFFER_H
#define BASILISK_COMMAND_BUFFER_H

namespace Basilisk
{
	template<class Impl>
	class CommandBuffer abstract
	{
	public:
		/**
		Gets this class's CRTP implementation
		\return This class's CRTP implementation
		*/
		inline const Impl &GetImplementation()
		{
			return static_cast<Impl&>(*this);
		}

		/**
		Changes the active pipeline

		\param[in] pipeline The pipeline to use. If `nullptr`, a default pipeline is used
		\return Details about potential failure
		\tparam PipelineType Specifies which API to use, and what type of pipeline to create
		*/
		template<class PipelineType>
		inline void BindPipeline(const PipelineType *pipeline) {
			return GetImplementation().BindPipeline(pipeline);
		}
		/**
		Changes the active swap chain

		\param[in] swapChain The swap chain to use. If `nullptr`, a default swap chain is used
		\tparam PipelineType Specifies which API to use

		\todo Can you even have a "default swap chain"? What would it do?
		*/
		template<class SwapChainType>
		inline void BindSwapChain(const SwapChainType *swapChain) {
			return GetImplementation().BindSwapChain(swapChain);
		}

		/**
		Completely wipes the contents of the command list
		\return Details about potential failure
		*/
		inline void Clear() {
			return GetImplementation().Clear();
		}
	};

	class D3D12CommandBuffer : public CommandBuffer<D3D12CommandBuffer>
	{
	public:

	private:
		ID3D12GraphicsCommandList *m_commandList;
	};

	class VulkanCommandBuffer : public CommandBuffer<VulkanCommandBuffer>
	{
	public:

	private:
		VkCommandBuffer m_commandBuffer;
	};

}

#endif
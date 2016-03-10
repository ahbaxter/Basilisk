/**
\file   command_buffer.h
\author Andrew Baxter
\date   February 29, 2016

Encapsulates a command buffers (or command lists in D3D12)

*/

#ifndef BASILISK_COMMAND_BUFFER_H
#define BASILISK_COMMAND_BUFFER_H

#include "common.h"
#include "swap_chain.h"

namespace Vulkan
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
		inline void WriteBundle(const Impl *bundle) {
			GetImplementation().WriteBundle(bundle);
		}

		/**
		Changes the active graphics pipeline

		\param[in] pipeline The pipeline to use
		\tparam PipelineType Specifies which API to use
		*/
		template<class PipelineType>
		inline void BindGraphicsPipeline(const PipelineType *pipeline) {
			GetImplementation().BindGraphicsPipeline(pipeline);
		}

		/**
		Changes the active compute pipeline

		\param[in] pipeline The pipeline to use
		\tparam PipelineType Specifies which API to use
		*/
		template<class PipelineType>
		inline void BindComputePipeline(const PipelineType *pipeline) {
			GetImplementation().BindComputePipeline(pipeline);
		}
		
		/**
		Enables drawing commands

		\taparam RenderPassType Specifies which API to use
		*/
		template<class RenderPassType>
		inline void BeginRenderPass(const RenderPassType *renderPass) {
			GetImplementation().beginRenderPass(renderPass);
		}

		/**
		Disables drawing commands
		*/
		inline void EndRenderPass() {
			GetImplementation.EndRenderPass();
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

	/** Implements the `CmdBuffer` interface for Vulkan */
	class VulkanCmdBuffer : public CmdBuffer<VulkanCmdBuffer>
	{
	public:
		friend class VulkanDevice;

		Result Begin(bool disposable);

		void WriteBundle(const VulkanCmdBuffer *bundle);

		Result End();

	private:
		VulkanCmdBuffer();
		~VulkanCmdBuffer() = default;

		/**
		Before use, many Vulkan images require extra processing in a command buffer

		\param[in] image The image to modify
		\param[in] aspectMask The aspect mask
		\param[in] oldLayout The image's prior layout
		\param[in] newLayout What layout to change the image to
		\param[in] oldQueueFamilyIndex Where to the image was previously viewable from
		\param[in] newQueueFamilyIndex Where to the image will be viewable from
		*/
		void SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t oldQueueFamilyIndex = VulkanDevice::render, uint32_t newQueueFamilyIndex = VulkanDevice::render);

		VkCommandBuffer m_commandBuffer;
	};

}

#endif
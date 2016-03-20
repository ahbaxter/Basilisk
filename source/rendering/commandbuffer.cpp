/**
\file   commandbuffer.cpp
\author Andrew Baxter
\date March 19, 2016
*/

#include "rendering\backend.h"
using namespace Vulkan;

CommandBuffer::CommandBuffer() : m_commandBuffer(VK_NULL_HANDLE) { }

void CommandBuffer::Release(VkDevice device, VkCommandPool pool)
{
	if (m_commandBuffer)
	{
		vkFreeCommandBuffers(device, pool, 1, &m_commandBuffer);
		m_commandBuffer = VK_NULL_HANDLE;
	}
}

std::shared_ptr<CommandBuffer> Device::CreateCommandBuffer(uint32_t poolIndex, bool bundle)
{
	std::shared_ptr<CommandBuffer> out(new CommandBuffer,
		[=](CommandBuffer *ptr) {
			ptr->Release(m_device, m_commandPools[poolIndex]);
			delete ptr;
			ptr = nullptr;
		}
	);

	VkCommandBufferAllocateInfo cmd_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,                    //Reserved
		m_commandPools[poolIndex],  //Command pool
		bundle ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY, //Buffer level
		1                           //Command buffer count
	};

	VkResult res = vkAllocateCommandBuffers(m_device, &cmd_info, &out->m_commandBuffer);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateCommandBuffer() could not create the command buffer");
		return nullptr;
	}


	return out;
}

bool CommandBuffer::Begin(bool reusable)
{
	VkCommandBufferUsageFlags flags = reusable ? 0 : VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkCommandBufferBeginInfo begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,  //Reserved
		flags,    //Flags
		nullptr   //Inheritance info
	};

	VkResult res = vkBeginCommandBuffer(m_commandBuffer, &begin_info);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::CommandBuffer::Begin() could not begin writing to the command buffer");
		return false;
	}

	return true;
}

void CommandBuffer::BeginRendering(const std::shared_ptr<FrameBuffer> &target, bool allowBundles)
{
	VkRenderPassBeginInfo rp_info = {
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		nullptr,
		target->m_renderPass,
		target->m_frameBuffer,
		target->m_renderArea,
		static_cast<uint32_t>(target->m_clearValues.size()),
		target->m_clearValues.data()
	};

	vkCmdBeginRenderPass(m_commandBuffer, &rp_info, allowBundles ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::BindGraphicsPipeline(const std::shared_ptr<GraphicsPipeline> &pipeline)
{
	if (pipeline)
		vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->m_pipeline);
	else
		Basilisk::errors.push("Vulkan::CommandBuffer::BindGraphicsPipeline()::pipeline must not be a null pointer");
}

void CommandBuffer::SetLineWidth(float width)
{
	vkCmdSetLineWidth(m_commandBuffer, width);
}

void CommandBuffer::SetViewport(const VkViewport &viewport)
{
	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
}

void CommandBuffer::SetScissor(const VkRect2D &scissor)
{
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void CommandBuffer::Blit(const std::shared_ptr<FrameBuffer> &src, const std::shared_ptr<SwapChain> &dst)
{
	SetImageLayout(src->m_images[0], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	VkImageBlit region;
	region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.srcSubresource.mipLevel = 0;
	region.srcSubresource.baseArrayLayer = 0;
	region.srcSubresource.layerCount = 1;
	region.srcOffsets[0] = { 0, 0, 0 };
	region.srcOffsets[1] = { static_cast<int32_t>(src->m_renderArea.extent.width), static_cast<int32_t>(src->m_renderArea.extent.height), 1 };
	region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.dstSubresource.mipLevel = 0;
	region.dstSubresource.baseArrayLayer = 0;
	region.dstSubresource.layerCount = 1;
	region.dstOffsets[0] = { 0, 0, 0 };
	region.dstOffsets[1] = { static_cast<int32_t>(src->m_renderArea.extent.width), static_cast<int32_t>(src->m_renderArea.extent.height), 1 };

	vkCmdBlitImage(m_commandBuffer, src->m_images.back(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->m_images[*dst->GetBufferIndex()], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_LINEAR);

	SetImageLayout(src->m_images.back(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void CommandBuffer::DrawIndexed(uint32_t count)
{
	vkCmdDrawIndexed(m_commandBuffer, count, 1, 0, 0, 1);
}



void CommandBuffer::EndRendering()
{
	vkCmdEndRenderPass(m_commandBuffer);
}

bool CommandBuffer::End()
{
	VkResult res = vkEndCommandBuffer(m_commandBuffer);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::CommandBuffer::Begin() could not close the command buffer");
		return false;
	}

	return true;
}

void CommandBuffer::SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageSubresourceRange range = {
		aspectMask,  //Aspect mask
		0,  //Base mip level
		1,  //Level count
		0,  //Base array layer
		1   //Layer count
	};

	//TODO: Depth buffer attachments
	VkImageMemoryBarrier image_memory_barrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,    //Reserved
		0,          //Source access mask
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,  //Destination access mask
		oldLayout,  //Old layout
		newLayout,  //New layout
		0,          //Source queue family index
		0,          //Destination queue family index
		image,      //Image
		range       //Subresource range
	};

	VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(m_commandBuffer, src_stages, dest_stages, 0, 0, nullptr, 0, nullptr,
		1, &image_memory_barrier);
}

void CommandBuffer::WriteBundle(const std::shared_ptr<CommandBuffer> &bundle)
{
	vkCmdExecuteCommands(m_commandBuffer, 1, &bundle->m_commandBuffer);
}
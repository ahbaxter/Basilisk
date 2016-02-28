/**
\file   command_buffer.cpp
\author Andrew Baxter
\date   February 27, 2016

Defines the behavior of Vulkan and D3D12 command buffers

*/

#include "rendering/command_buffer.h"
#include "rendering/device.h"

using namespace Basilisk;

D3D12CmdBuffer::D3D12CmdBuffer() : m_commandList(nullptr) {
}

VulkanCmdBuffer::VulkanCmdBuffer() : m_commandBuffer(nullptr) {
}


Result VulkanCmdBuffer::Begin(bool disposable)
{
	VkCommandBufferUsageFlags flags = disposable ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;

	VkCommandBufferBeginInfo begin_info = 
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,  //Reserved
		flags,    //Flags
		nullptr   //Inheritance info
	};

	if (Failed( vkBeginCommandBuffer(m_commandBuffer, &begin_info) ))
	{
		Basilisk::errorMessage = "Basilisk::VulkanCmdBuffer::Begin() could not begin writing to the command buffer";
		return Result::ApiError;
	}

	return Result::Success;
}

Result VulkanCmdBuffer::End()
{
	if (Failed(vkEndCommandBuffer(m_commandBuffer)))
	{
		Basilisk::errorMessage = "Basilisk::VulkanCmdBuffer::Begin() could not close the command buffer";
		return Result::ApiError;
	}

	return Result::Success;
}

void VulkanCmdBuffer::PrepareSwapChain(const VulkanSwapChain &swapChain)
{
	VkImageSubresourceRange range = 
	{
		VK_IMAGE_ASPECT_COLOR_BIT, //Aspect mask
		0,  //Base mip level
		1,  //Level count
		0,  //Base array layer
		1   //Layer count
	};

	for (uint32_t i = 0; i < swapChain.m_backBuffers.size(); ++i)
	{
		VkImageMemoryBarrier image_memory_barrier =
		{
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			nullptr,                                  //Reserved
			0,                                        //Source access mask
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,      //Destination access mask
			VK_IMAGE_LAYOUT_UNDEFINED,                //Old layout
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, //New layout
			VulkanDevice::render,                     //Source queue family index
			VulkanDevice::render,                     //Destination queue family index
			swapChain.m_backBuffers[i],               //Image
			range                                     //Subresource range
		};

		VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		vkCmdPipelineBarrier(m_commandBuffer, src_stages, dest_stages, 0, 0, nullptr, 0, nullptr,
			1, &image_memory_barrier);
	}
}
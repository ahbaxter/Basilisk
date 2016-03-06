/**
\file   frame_buffer.h
\author Andrew Baxter
\date   March 5, 2016

Encapsulates a Vulkan frame buffer, with its own render pass

\todo Check if D3D12 has an equivalent -- sure it does
*/

#ifndef BASILISK_FRAME_BUFFER_H
#define BASILISK_FRAME_BUFFER_H

#include "common.h"

namespace Basilisk
{
	class VulkanFrameBuffer
	{
	public:
		friend class VulkanDevice;
	private:
		VulkanFrameBuffer() = default;
		~VulkanFrameBuffer() = default;

		void Allocate(uint32_t size);
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_views;
		std::vector<VkDeviceMemory> m_memory;
		std::vector<VkFormat> m_formats;

		VkFramebuffer m_frameBuffer;
		VkRenderPass m_renderPass;
		
		Bounds2D<uint32_t> m_size;
	};
}

#endif
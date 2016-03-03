/**
\file   frame_buffer.h
\author Andrew Baxter
\date   March 3, 2016

Encapsulates a Vulkan frame buffer, with its own render pass

\todo Check if D3D12 has an equivalent -- probably does
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

		VkFramebuffer m_frameBuffer;
		VkRenderPass m_renderPass;
		VkImageSet m_images;
	};
}

#endif
/**
\file   frame_buffer.h
\author Andrew Baxter
\date   February 29, 2016

Encapsulates a Vulkan frame buffer

\todo Check if D3D12 has an equivalent -- probably does
*/

#ifndef BASILISK_FRAME_BUFFER_H
#define BASILISK_FRAME_BUFFER_H

#include "common.h"

namespace Basilisk
{
	class VulkanFrameBufferSet
	{
	public:
		friend class VulkanDevice;

		inline size_t Count() {
			return m_frameBuffers.size();
		}

	private:
		VulkanFrameBufferSet() = default;
		~VulkanFrameBufferSet() = default;

		std::vector<VkFramebuffer> m_frameBuffers;
	};
}

#endif
/**
\file   frame_buffer.cpp
\author Andrew Baxter
\date   March 5, 2016

Sets up Render pass and Frame buffers

*/

#include "rendering/frame_buffer.h"

using namespace Basilisk;

void VulkanFrameBuffer::Allocate(uint32_t size)
{
	m_images.resize(size);
	m_views.resize(size);
	m_memory.resize(size);
	m_formats.resize(size);
}
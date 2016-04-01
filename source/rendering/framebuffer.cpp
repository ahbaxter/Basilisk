/**
\file   framebuffer.cpp
\author Andrew Baxter
\date   March 18, 2016

Defines the behavior of a Vulkan::SwapChain, from creation to destruction

*/

#include "rendering/backend.h"
using namespace Vulkan;


VkAttachmentDescription Vulkan::AttachmentDescription(VkFormat format, VkAttachmentLoadOp loadOp)
{
	bool isDepth = VK_FORMAT_D16_UNORM <= format && format <= VK_FORMAT_D32_SFLOAT_S8_UINT;
	bool hasStencil = isDepth && format >= VK_FORMAT_S8_UINT;
	return {
		0, //Flags: none
		format,
		VK_SAMPLE_COUNT_1_BIT,
		loadOp,
		VK_ATTACHMENT_STORE_OP_STORE,
		hasStencil ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		hasStencil ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
		isDepth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, //Initial layout
		isDepth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL //Final layout
	};
}

FrameBuffer::FrameBuffer() : m_frameBuffer(VK_NULL_HANDLE), m_renderPass(VK_NULL_HANDLE)
{
	m_renderArea = { 0, 0, 0, 0 };
}

void FrameBuffer::ResizeVectors(uint32_t size)
{
	m_images.resize(size);
	m_views.resize(size);
	m_formats.resize(size);
	m_memory.resize(size);
	m_clearValues.resize(size);
}

void FrameBuffer::Release(VkDevice device)
{
	if (m_frameBuffer)
	{
		vkDestroyFramebuffer(device, m_frameBuffer, nullptr);
		m_frameBuffer = VK_NULL_HANDLE;
	}
	if (m_renderPass)
	{
		vkDestroyRenderPass(device, m_renderPass, nullptr);
		m_renderPass = VK_NULL_HANDLE;
	}

	for (uint32_t i = 0; i < m_images.size(); ++i)
	{
		if (m_views[i]) {
			vkDestroyImageView(device, m_views[i], nullptr);
			m_views[i] = VK_NULL_HANDLE;
		}

		if (m_memory[i]) {
			vkFreeMemory(device, m_memory[i], nullptr);
			m_memory[i] = VK_NULL_HANDLE;
		}

		if (m_images[i]) {
			vkDestroyImage(device, m_images[i], nullptr);
			m_images[i] = VK_NULL_HANDLE;
		}
	}
}

bool FrameBuffer::SetClearValues(std::vector<VkClearValue> clearValues)
{
	if (clearValues.size() == m_clearValues.size())
	{
		std::copy(clearValues.begin(), clearValues.end(), m_clearValues.begin());
		return true;
	}
	else
	{
		Basilisk::errors.push("Vulkan::FrameBuffer::SetClearValues()::clearValues does not contain the correct amount of values");
		return false;
	}
}

std::shared_ptr<FrameBuffer> Device::CreateFrameBuffer(std::vector<AttachmentInfo> colorAttachments, bool depthBuffer)
{
	if (colorAttachments.size() == 0 && !depthBuffer)
	{
		Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() must have at least one attachment");
		return nullptr;
	}

	std::shared_ptr<FrameBuffer> out(new FrameBuffer,
		[=](FrameBuffer *&ptr) {
			ptr->Release(m_device);
			delete ptr;
			ptr = nullptr;
		}
	);
	out->m_renderArea = { 0, 0, colorAttachments[0].image.extent.width, colorAttachments[0].image.extent.height };

	uint32_t numAttachments = static_cast<uint32_t>( colorAttachments.size() + (depthBuffer ? 1 : 0) );
	std::vector<VkAttachmentDescription> allAttachments(numAttachments);
	std::vector<VkAttachmentReference> attachmentRefs(numAttachments);
	out->ResizeVectors(numAttachments);

	VkMemoryAllocateInfo memAlloc = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr};
	VkMemoryRequirements memReqs = {};
	VkImageViewCreateInfo view_create_info = ImageViewCreateInfo(VK_NULL_HANDLE, VK_FORMAT_UNDEFINED, VK_IMAGE_ASPECT_COLOR_BIT);
	VkSubpassDescription subpassDesc = {
		0,        //Flags
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		0,        //Input attachment count
		nullptr,  //Input attachments
		static_cast<uint32_t>(colorAttachments.size()),
		attachmentRefs.data(),    //Color attachments: the first n entries in attachmentRefs, where n = colorAttachments.size()
		nullptr,  //Resolve attachment
		depthBuffer ? &attachmentRefs.back() : nullptr,  //Depth buffer attachment: if it exists, it is the last attachmentin attachmentRefs
		0,        //Preserve attachment count
		nullptr   //Preserve attachments
	};
	VkRenderPassCreateInfo rp_create_info = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,  //Flags
		numAttachments,
		allAttachments.data(),
		1,  //Subpass count
		&subpassDesc,  //Subpasses
		0,  //Dependency count
		nullptr  //Dependencies
	};
	VkFramebufferCreateInfo fb_create_info = {
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		nullptr,  //Next: reserved
		0,  //Flags
		nullptr,  //Render pass: fill in later
		numAttachments,
		nullptr, //Attachment image views: fill in later
		colorAttachments[0].image.extent.width,
		colorAttachments[0].image.extent.height,
		1  //Layers
	};
	VkResult res = VK_SUCCESS;
	uint32_t i = 0;



	for (i = 0; i < colorAttachments.size(); ++i) //Create color buffers
	{
		//Store image format and attachment data
		out->m_formats[i] = colorAttachments[i].image.format;
		allAttachments[i] = colorAttachments[i].attachment;
		attachmentRefs[i] = { i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		//Create image
		res = vkCreateImage(m_device, &colorAttachments[i].image, nullptr, &out->m_images[i]);
		if (Failed(res))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not create all color images");
			return nullptr;
		}

		//Allocate memory for image
		vkGetImageMemoryRequirements(m_device, out->m_images[i], &memReqs);
		memAlloc.allocationSize = memReqs.size;
		if (!MemoryTypeFromProps(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memAlloc.memoryTypeIndex))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not determine appropriate memory type for all color images");
			return nullptr;
		}
		res = vkAllocateMemory(m_device, &memAlloc, nullptr, &out->m_memory[i]);
		if (Failed(res))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not allocate memory for all color images");
			return nullptr;
		}
		vkBindImageMemory(m_device, out->m_images[i], out->m_memory[i], 0);
		if (Failed(res))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not bind memory for all color images");
			return nullptr;
		}

		//Create image view
		view_create_info.image = out->m_images[i];
		view_create_info.format = out->m_formats[i];
		res = vkCreateImageView(m_device, &view_create_info, nullptr, &out->m_views[i]);
		if (Failed(res))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not create an image view for all color buffers");
			return nullptr;
		}
	}
	for (; i < numAttachments; ++i) //Create image for depth buffer (if any)
	{
		//Store image format and attachment data
		out->m_formats[i] = m_gpuProps.depthFormat;
		allAttachments[i] = AttachmentDescription(m_gpuProps.depthFormat, VK_ATTACHMENT_LOAD_OP_CLEAR);
		attachmentRefs[i] = { i, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		//Create image
		VkImageCreateInfo depth_image_info = ImageCreateInfo(VK_IMAGE_TYPE_2D, m_gpuProps.depthFormat, { colorAttachments[0].image.extent.width, colorAttachments[0].image.extent.height, 1 }, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		res = vkCreateImage(m_device, &depth_image_info, nullptr, &out->m_images[i]);
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not creat the depth stencil image");
			return nullptr;
		}

		//Allocate memory for image
		vkGetImageMemoryRequirements(m_device, out->m_images[i], &memReqs);
		memAlloc.allocationSize = memReqs.size;
		if (!MemoryTypeFromProps(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memAlloc.memoryTypeIndex))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not determine appropriate memory type for the depth stencil image");
			return nullptr;
		}
		res = vkAllocateMemory(m_device, &memAlloc, nullptr, &out->m_memory[i]);
		if (Failed(res))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not allocate memory for the depth stencil image");
			return nullptr;
		}
		vkBindImageMemory(m_device, out->m_images[i], out->m_memory[i], 0);
		if (Failed(res))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not bind memory for the depth stencil image");
			return nullptr;
		}

		//Create image view
		view_create_info.image = out->m_images[i];
		view_create_info.format = out->m_formats[i];
		res = vkCreateImageView(m_device, &view_create_info, nullptr, &out->m_views[i]);
		if (Failed(res))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not create an image view for all color buffers");
			return nullptr;
		}
	}

	res = vkCreateRenderPass(m_device, &rp_create_info, nullptr, &out->m_renderPass);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not create the render pass");
		return nullptr;
	}


	fb_create_info.renderPass = out->m_renderPass;
	fb_create_info.pAttachments = out->m_views.data();
	res = vkCreateFramebuffer(m_device, &fb_create_info, nullptr, &out->m_frameBuffer);
	if (Failed(res))
	{
		Basilisk::errors.push("Basilisk::Device::CreateFrameBuffer() could not create the frame buffer");
		return nullptr;
	}


	return out;
}
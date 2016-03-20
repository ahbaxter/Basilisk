/**
\file   image.cpp
\author Andrew Baxter
\date   March 18, 2016

Defines the behavior of the half-built Vulkan::Image class, and helps with 

*/

#include "rendering/backend.h"
using namespace Vulkan;

VkImageCreateInfo Vulkan::ImageCreateInfo(VkImageType dimensionality, VkFormat format, VkExtent3D resolution, VkImageUsageFlags usage, VkImageLayout initialLayout)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr, //Next (reserved)
		0, //Flags
		dimensionality,
		format,
		resolution,
		1, //Mip levels
		1, //Array levels
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0, //Queue family index count
		nullptr, //Queue family indices
		initialLayout
	};
}

VkImageViewCreateInfo Vulkan::ImageViewCreateInfo(VkImage image, VkFormat format, VkImageAspectFlags aspect)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr, //Next: reserved
		0, //Flags
		image,
		VK_IMAGE_VIEW_TYPE_2D,
		format,
		{VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
		{aspect, 0, 1, 0, 1}
	};
}

VkImageViewCreateInfo ViewCreateInfo(VkImage image, VkFormat format)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,                //Next (reserved)
		0,                      //Flags
		image,                  //Image handle
		VK_IMAGE_VIEW_TYPE_2D,  //Image view type
		format,                 //Format
		{
			VK_COMPONENT_SWIZZLE_R,  //Everything is
			VK_COMPONENT_SWIZZLE_G,  //ordered the same
			VK_COMPONENT_SWIZZLE_B,  //in the shaders
			VK_COMPONENT_SWIZZLE_A,  //as it is here
		},  //Components
		{
			VK_IMAGE_ASPECT_COLOR_BIT, //Aspect mask
			0,  //Base mip level
			1,  //Mip level count
			0,  //Base array layer
			1   //Array layer count
		}  //Subresource range
	};
}

Image::Image() : m_image(VK_NULL_HANDLE), m_view(VK_NULL_HANDLE), m_memory(VK_NULL_HANDLE), m_format(VK_FORMAT_UNDEFINED)
{
	m_size = {};
}

void Image::Release(VkDevice device)
{
	if (m_view)
	{
		vkDestroyImageView(device, m_view, nullptr);
		m_view = VK_NULL_HANDLE;
	}
	if (m_memory)
	{
		vkFreeMemory(device, m_memory, nullptr);
		m_memory = VK_NULL_HANDLE;
	}
	if (m_image)
	{
		vkDestroyImage(device, m_image, nullptr);
		m_image = VK_NULL_HANDLE;
	}
	m_format = VK_FORMAT_UNDEFINED;
}
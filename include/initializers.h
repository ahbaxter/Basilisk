/**
\file   initializers.h
\author Andrew Baxter
\date   March 4, 2016

Provides default initializers for common D3D12 and Vulkan objects

*/

#ifndef BASILISK_INITIALIZERS_H
#define BASILISK_INITIALIZERS_H

#include "common.h"

namespace Basilisk
{
	template<typename T>
	struct Init;
	
	template<> struct Init<VkImageCreateInfo>
	{
		/**
		Assumes:
			2D image
			Format type of VK_FORMAT_R8G8B8A8_UNORM
			Usage of VK_IMAGE_USAGE_SAMPLED_BIT
			Initial layout of VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			No multisampling
			No mipmaps
			Non-array image
			Not shared between queues
			Optimal tiling mode
		Does not supply:
			A reasonable resolution
		*/
		static VkImageCreateInfo Base();
		static VkImageCreateInfo Texture1D(uint32_t width, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uint32_t arrayLayers = 1, uint32_t mipLevels = 1);
		static VkImageCreateInfo Texture2D(Bounds2D<uint32_t> dimensions, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uint32_t arrayLayers = 1, uint32_t mipLevels = 1);
		static VkImageCreateInfo Texture3D(Bounds3D<uint32_t> dimensions, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uint32_t arrayLayers = 1, uint32_t mipLevels = 1);
		static VkImageCreateInfo CubeMap(uint32_t sideLength, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uint32_t arrayLayers = 1, uint32_t mipLevels = 1);
		static VkImageCreateInfo DepthStencil(Bounds2D<uint32_t> dimensions, VkFormat format, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	};
	
	template<> struct Init<VkImageViewCreateInfo>
	{
		/**
		Assumes:
			2D image
			Format of VK_FORMAT_R8G8B8A8_UNORM
			Aspect mask of VK_IMAGE_ASPECT_COLOR_BIT
			RGBA swizzle
			No mipmaps
			Non-array image
		Does not supply:
			Image handle
		*/
		static VkImageViewCreateInfo Base();
		static VkImageViewCreateInfo Texture1D(VkImage image, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, uint32_t arrayLayers = 1, uint32_t baseArrayLayer = 0, uint32_t mipLevels = 1, uint32_t baseMipLevel = 0);
		static VkImageViewCreateInfo Texture2D(VkImage image, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, uint32_t arrayLayers = 1, uint32_t baseArrayLayer = 0, uint32_t mipLevels = 1, uint32_t baseMipLevel = 0);
		static VkImageViewCreateInfo Texture3D(VkImage image, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, uint32_t mipLevels = 1, uint32_t baseMipLevel = 0);
		static VkImageViewCreateInfo CubeMap(VkImage image, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, uint32_t arrayLayers = 1, uint32_t baseArrayLayer = 0, uint32_t mipLevels = 1, uint32_t baseMipLevel = 0);
		static VkImageViewCreateInfo DepthStencil(VkImage image, VkFormat format);
	};
	
	template<> struct Init<VkCommandPoolCreateInfo>
	{
		/**
		Assumes:
			Allows manual reset
		*/
		static VkCommandPoolCreateInfo Base(uint32_t queueFamilyIndex);
	};

	
	//UNFINISHED:
	
	template<> struct Init<VkSamplerCreateInfo>
	{
		static VkSamplerCreateInfo Base();
	};

	template<> struct Init<VkRenderPassBeginInfo>
	{
		static VkRenderPassBeginInfo Base();
	};

	template<> struct Init<VkFramebufferCreateInfo>
	{
		template<uint32_t NumAttachments>
		static VkFramebufferCreateInfo Base(VkRenderPass renderPass, Bounds2D<uint32_t> resolution, const std::array<VkImageView, NumAttachments> attachments);
	};

	template<> struct Init<VkCommandBufferAllocateInfo>
	{
		static VkCommandBufferAllocateInfo Base();
	};

	template<> struct Init<VkCommandBufferBeginInfo>
	{
		static VkCommandBufferBeginInfo Base();
	};

	//VkMemoryAllocateInfo?

	*/
}

#endif
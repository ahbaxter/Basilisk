/**
\file   initializers.h
\author Andrew Baxter
\date   March 8, 2016

Provides default initializers for common D3D12 and Vulkan objects

\todo Add push constants to `Init<VkPipelineLayoutCreateInfo>`

*/

#ifndef BASILISK_INITIALIZERS_H
#define BASILISK_INITIALIZERS_H

#include "common.h"

#define GET_INSTANCE_PROCADDR(instance, function) \
	pfn##function = reinterpret_cast<PFN_vk##function>(vkGetInstanceProcAddr(instance, "vk"#function))

#define GET_DEVICE_PROCADDR(device, function) \
	pfn##function = reinterpret_cast<PFN_vk##function>(vkGetDeviceProcAddr(device, "vk"#function))

namespace Basilisk
{
	char *ReadBinaryFile(const std::string &filename, size_t *sizeOut);

	std::string ReadTextFile(const std::string &filename);

	template<typename T>
	struct Init;
	
	///////////////////////////////////////
	//                                   //
	//    Images, views, and samplers    //
	//                                   //
	///////////////////////////////////////

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
		static constexpr VkImageCreateInfo Base();
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
		static constexpr VkImageViewCreateInfo Base();
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
		static VkCommandPoolCreateInfo Create(uint32_t queueFamilyIndex);
	};

	template<> struct Init<VkSamplerCreateInfo>
	{
		/**
		Assumes:
			Linear texel filtering
			Linear mipmap filtering
			Samples clamped to border
			No mipmap LOD bias
			No anisotropy
			No compare operations
			Min and max LOD of 0
			Opaque white border
		*/
		static constexpr VkSamplerCreateInfo Default();
		static VkSamplerCreateInfo Create(VkFilter filter = VK_FILTER_LINEAR, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, VkCompareOp = VK_COMPARE_OP_NEVER);
	};

	template<> struct Init<VkMemoryAllocateInfo>
	{
		/**
		Does not provide:
			Allocation size
			Memory type index
		*/
		static constexpr VkMemoryAllocateInfo Base();
		static VkMemoryAllocateInfo Create(VkDeviceSize allocationSize, uint32_t memoryTypeIndex);
	};

	///////////////////////////////////////
	//                                   //
	//  Frame buffers and render passes  //
	//                                   //
	///////////////////////////////////////

	template<> struct Init<VkAttachmentDescription>
	{
		static VkAttachmentDescription Color(VkFormat format);
		static VkAttachmentDescription DepthStencil(VkFormat format);
	};

	template<> struct Init<VkSubpassDescription>
	{
		/**
		Assumes:
			Graphics pipeline bind point
			Absolutely no attachments
		*/
		static constexpr VkSubpassDescription Base();
	};

	template<> struct Init<VkRenderPassCreateInfo>
	{
		/**
		Assumes:
			Absolutely no attachmentes
			No subpasses
			No dependencies
		*/
		static constexpr VkRenderPassCreateInfo Base();
	};

	template<> struct Init<VkFramebufferCreateInfo>
	{
		/**
		Assumes:
			Single-layer
		Does not supply:
			Render pass
			Resolution
			Attachments
		*/
		static constexpr VkFramebufferCreateInfo Base();
		static VkFramebufferCreateInfo Create(VkRenderPass renderPass, Bounds2D<uint32_t> resolution, const std::vector<VkImageView> &attachments);
	};

	///////////////////////////////////////
	//                                   //
	//  Descriptor (set)s and pipelines  //
	//                                   //
	///////////////////////////////////////

	template<> struct Init<VkDescriptorSetLayoutBinding>
	{
		/**
		Assumes:
			Descriptor type of VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
			Visibility of VK_SHADER_STAGE_ALL_GRAPHICS
			Bound to slot 0
			Count of 1
		*/
		static constexpr VkDescriptorSetLayoutBinding Base();
		static VkDescriptorSetLayoutBinding Create(uint32_t slot, VkDescriptorType type, VkShaderStageFlags visibility);
	};

	template<> struct Init<VkDescriptorSetLayoutCreateInfo>
	{
		static VkDescriptorSetLayoutCreateInfo Create(const std::vector<VkDescriptorSetLayoutBinding> &bindings);
	};

	template<> struct Init<VkPipelineLayoutCreateInfo>
	{
		/**
		Assumes:
			
		*/
		static VkPipelineLayoutCreateInfo Create(const std::vector<VkDescriptorSetLayout> &layouts);
	};

	struct VulkanGraphicsPipelineState
	{
		VkPipelineVertexInputStateCreateInfo vertexInputState;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		VkPipelineTessellationStateCreateInfo tesselationState;
		VkPipelineViewportStateCreateInfo viewportState;
		VkPipelineRasterizationStateCreateInfo rasterizationState;
		VkPipelineMultisampleStateCreateInfo multisampleState;
		VkPipelineDepthStencilStateCreateInfo depthStencilState;
		VkPipelineColorBlendStateCreateInfo colorBlendState;
		VkPipelineDynamicStateCreateInfo dynamicState;
	};

	template<> struct Init<VulkanGraphicsPipelineState>
	{
		/**
		Assumes:
			Triangle list topology
			Dynamic viewports and scissors (not provided at creation-time)
			Filled polygons, cull back-facing polygons, line width of 1px
			No multisampling
			Depth enabled, stencil and depth bounds disabled
		Does not provide:
			Vertex input state data
			Tesselation state
		*/
		static constexpr VulkanGraphicsPipelineState Base(const std::vector<VkDynamicState> &dynamicStateEnables, const std::vector<VkPipelineColorBlendAttachmentState> &blendAttachments);
	};

	template<> struct Init<VkShaderModuleCreateInfo>
	{
		static VkShaderModuleCreateInfo FromSPIRV(size_t codeSize, uint32_t *shaderCode);
		/**
		\warning Basilisk is not responsible for the memory in `spirvCode` after the function call
		*/
		static VkShaderModuleCreateInfo FromGLSL(const std::string &source, VkShaderStageFlagBits stage, uint32_t *&spirvCode);
	};

	template<> struct Init<VkPipelineShaderStageCreateInfo>
	{
		/**
		Assumes:
			Entry point is called "main"
		Does not provide:
			Module handle
			Shader stage bit
		*/
		static constexpr VkPipelineShaderStageCreateInfo Base();
		static VkPipelineShaderStageCreateInfo FromModule(VkShaderModule module, VkShaderStageFlagBits stage, const std::string &entryFunc = "main");
	};


	//UNFINISHED:

	template<> struct Init<VkGraphicsPipelineCreateInfo>
	{
		static constexpr VkGraphicsPipelineCreateInfo Base();
		static VkGraphicsPipelineCreateInfo Create(const std::vector<VkPipelineShaderStageCreateInfo> &stages, const VulkanGraphicsPipelineState &state, VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpassIndex = 0);
		static VkGraphicsPipelineCreateInfo Parent(const std::vector<VkPipelineShaderStageCreateInfo> &stages, const VulkanGraphicsPipelineState &state, VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpassIndex = 0);
		static VkGraphicsPipelineCreateInfo Child(VkPipeline parent);
	};

	template<> struct Init<VkCommandBufferAllocateInfo>
	{
		static VkCommandBufferAllocateInfo Base();
	};

	template<> struct Init<VkCommandBufferBeginInfo>
	{
		static VkCommandBufferBeginInfo Base();
	};
}

#endif
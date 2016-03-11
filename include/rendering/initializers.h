/**
\file   initializers.h
\author Andrew Baxter
\date   March 9, 2016

Provides default CreateInfo structs for common Vulkan objects

\todo Add push constants to `Init<VkPipelineLayoutCreateInfo>`
\todo Send to hell. I don't know what I was thinking; this is disgusting.

*/

#ifndef BASILISK_INITIALIZERS_H
#define BASILISK_INITIALIZERS_H

#include "common.h"

namespace Vulkan
{

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
		static constexpr VkImageCreateInfo Base() {
			return{
				VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				nullptr,                    //Reserved
				0,                          //Flags
				VK_IMAGE_TYPE_2D,           //Image type
				VK_FORMAT_R8G8B8A8_UNORM,   //Format
				{ 1, 1, 1 },                //Extent
				1,                          //Mip levels
				1,                          //Array layers
				VK_SAMPLE_COUNT_1_BIT,      //Sample count
				VK_IMAGE_TILING_OPTIMAL,    //Tiling
				VK_IMAGE_USAGE_SAMPLED_BIT, //Usage
				VK_SHARING_MODE_EXCLUSIVE,  //Sharing mode
				0,                          //Queue family index count
				nullptr,                    //Queue family indices
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL //Initial layout
			};
		}
		static VkImageCreateInfo Texture1D(uint32_t width, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uint32_t arrayLayers = 1, uint32_t mipLevels = 1);
		static VkImageCreateInfo Texture2D(glm::uvec2 dimensions, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uint32_t arrayLayers = 1, uint32_t mipLevels = 1);
		static VkImageCreateInfo Texture3D(glm::uvec3 dimensions, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uint32_t arrayLayers = 1, uint32_t mipLevels = 1);
		static VkImageCreateInfo CubeMap(uint32_t sideLength, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uint32_t arrayLayers = 1, uint32_t mipLevels = 1);
		static VkImageCreateInfo DepthStencil(glm::uvec2 dimensions, VkFormat format, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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
		static constexpr VkImageViewCreateInfo Base() {
			return {
				VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				nullptr,                      //Reserved
				0,                            //Flags
				VK_NULL_HANDLE,               //Image handle
				VK_IMAGE_VIEW_TYPE_2D,        //Image view type
				VK_FORMAT_R8G8B8A8_UNORM,     //Format
				{
					VK_COMPONENT_SWIZZLE_R,   //Everything is
					VK_COMPONENT_SWIZZLE_G,   //ordered the same
					VK_COMPONENT_SWIZZLE_B,   //in the shaders
					VK_COMPONENT_SWIZZLE_A,   //as it is here
				},                            ///Components
				{
					VK_IMAGE_ASPECT_COLOR_BIT, //Aspect mask
					0,                         //Base mip level
					1,                         //Mip level count
					0,                         //Base array layer
					1                          //Array layer count
				}                              ///Subresource range
			};
		}
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
		static constexpr VkSamplerCreateInfo Default() {
			return {
				VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
				nullptr,             //Reserved
				0,                   //Flags: reserved
				VK_FILTER_LINEAR,    //Mag filter
				VK_FILTER_LINEAR,    //Min filter
				VK_SAMPLER_MIPMAP_MODE_LINEAR, //Mipmap mode
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, //Address mode U
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, //Address mode V
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, //Address mode W
				0.0f,                //Mip lod bias
				VK_FALSE,            //Enable anistropy
				0,                   //Max anistopy
				VK_FALSE,            //Enable compare
				VK_COMPARE_OP_NEVER, //Compare operation,
				0.0f,                //Min lod
				0.0f,                //Max lod
				VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE //Border color
			};
		}
		static VkSamplerCreateInfo Create(VkFilter filter = VK_FILTER_LINEAR, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, VkCompareOp compareOp = VK_COMPARE_OP_NEVER);
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
		static constexpr VkSubpassDescription Base() {
			return {
				0,        //Flags
				VK_PIPELINE_BIND_POINT_GRAPHICS,  //Pipeline bind point
				0,        //Input attachment count
				nullptr,  //Input attachments
				0,        //Color attachment count
				nullptr,  //Color attachments
				nullptr,  //Resolve attachment
				nullptr,  //Depth buffer attachment
				0,        //Preserve attachment count
				nullptr   //Preserve attachments
			};
		}
	};

	template<> struct Init<VkRenderPassCreateInfo>
	{
		/**
		Assumes:
			Absolutely no attachmentes
			No subpasses
			No dependencies
		*/
		static constexpr VkRenderPassCreateInfo Base() {
			return {
				VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
				nullptr,  //Reserved
				0,        //No flags
				0,        //Attachment count
				nullptr,  //Attachments
				0,        //Subpass count
				nullptr,  //Subpasses
				0,        //Dependency count
				nullptr   //Dependencies
			};
		}
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
		static constexpr VkFramebufferCreateInfo Base() {
			return {
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				nullptr,        //Reserved
				0,              //No flags: reserved
				VK_NULL_HANDLE, //Render pass
				0,              //Attachment count
				nullptr,        //Attachments
				0,              //Width
				0,              //Height
				1               //Layers
			};
		}
		static VkFramebufferCreateInfo Create(VkRenderPass renderPass, glm::uvec2 resolution, const std::vector<VkImageView> &attachments);
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
		static constexpr VkDescriptorSetLayoutBinding Base() {
			return {
				0,                                  //Binding
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  //Descriptor type
				1,                                  //Descriptor count
				VK_SHADER_STAGE_ALL_GRAPHICS,       //Stage flags
				nullptr                             //Immutable samplers
			};
		}
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
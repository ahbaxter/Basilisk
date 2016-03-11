/**
\file   initializers.cpp
\author Andrew Baxter
\date   March 6, 2016

Implements the default initializers declared in `initializers.h`

*/

#include <fstream>
#include <assert.h>
#include <memory>
#include "rendering/initializers.h"

using namespace Vulkan;

#pragma region VkImageCreateInfo

VkImageCreateInfo Init<VkImageCreateInfo>::Texture1D(uint32_t width, VkFormat format, VkImageUsageFlags usage, VkImageLayout initialLayout, uint32_t arrayLayers, uint32_t mipLevels)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,                    //Reserved
		0,                          //Flags
		VK_IMAGE_TYPE_1D,           //Image type
		format,                     //Format
		{ width, 1, 1 },            //Extent
		mipLevels,                  //Mip levels
		arrayLayers,                //Array layers
		VK_SAMPLE_COUNT_1_BIT,      //Sample count
		VK_IMAGE_TILING_OPTIMAL,    //Tiling
		usage,                      //Usage
		VK_SHARING_MODE_EXCLUSIVE,  //Sharing mode
		0,                          //Queue family index count
		nullptr,                    //Queue family indices
		initialLayout               //Initial layout
	};
}

VkImageCreateInfo Init<VkImageCreateInfo>::Texture2D(glm::uvec2 dimensions, VkFormat format, VkImageUsageFlags usage, VkImageLayout initialLayout, uint32_t arrayLayers, uint32_t mipLevels)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,                    //Reserved
		0,                          //Flags
		VK_IMAGE_TYPE_2D,           //Image type
		format,                     //Format
		{ dimensions.x, dimensions.y, 1 },  //Extent
		mipLevels,                  //Mip levels
		arrayLayers,                //Array layers
		VK_SAMPLE_COUNT_1_BIT,      //Sample count
		VK_IMAGE_TILING_OPTIMAL,    //Tiling
		usage,                      //Usage
		VK_SHARING_MODE_EXCLUSIVE,  //Sharing mode
		0,                          //Queue family index count
		nullptr,                    //Queue family indices
		initialLayout               //Initial layout
	};
}

VkImageCreateInfo Init<VkImageCreateInfo>::Texture3D(glm::uvec3 dimensions, VkFormat format, VkImageUsageFlags usage, VkImageLayout initialLayout, uint32_t arrayLayers, uint32_t mipLevels)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,                    //Reserved
		0,                          //Flags
		VK_IMAGE_TYPE_2D,           //Image type
		format,                     //Format
		{ dimensions.x, dimensions.y, dimensions.z },  //Extent
		mipLevels,                  //Mip levels
		arrayLayers,                //Array layers
		VK_SAMPLE_COUNT_1_BIT,      //Sample count
		VK_IMAGE_TILING_OPTIMAL,    //Tiling
		usage,                      //Usage
		VK_SHARING_MODE_EXCLUSIVE,  //Sharing mode
		0,                          //Queue family index count
		nullptr,                    //Queue family indices
		initialLayout               //Initial layout
	};
}

VkImageCreateInfo Init<VkImageCreateInfo>::DepthStencil(glm::uvec2 dimensions, VkFormat format, VkImageLayout initialLayout)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,                    //Reserved
		0,                          //Flags
		VK_IMAGE_TYPE_2D,           //Image type
		format,                     //Format
		{ dimensions.x, dimensions.y, 1 },  //Extent
		1,                          //Mip levels
		1,                          //Array layers
		VK_SAMPLE_COUNT_1_BIT,      //Sample count
		VK_IMAGE_TILING_OPTIMAL,    //Tiling
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, //Usage
		VK_SHARING_MODE_EXCLUSIVE,  //Sharing mode
		0,                          //Queue family index count
		nullptr,                    //Queue family indices
		initialLayout               //Initial layout
	};
}

VkImageCreateInfo Init<VkImageCreateInfo>::CubeMap(uint32_t sideLength, VkFormat format, VkImageUsageFlags usage, VkImageLayout initialLayout, uint32_t arrayLayers, uint32_t mipLevels)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,                    //Reserved
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, //Flags
		VK_IMAGE_TYPE_2D,           //Image type
		format,                     //Format
		{ sideLength, sideLength, 1 },  //Extent
		mipLevels,                  //Mip levels
		arrayLayers * 6,            //Array layers
		VK_SAMPLE_COUNT_1_BIT,      //Sample count
		VK_IMAGE_TILING_OPTIMAL,    //Tiling
		usage,                      //Usage
		VK_SHARING_MODE_EXCLUSIVE,  //Sharing mode
		0,                          //Queue family index count
		nullptr,                    //Queue family indices
		initialLayout               //Initial layout
	};
}

#pragma endregion

#pragma region VkImageViewCreateInfo

VkImageViewCreateInfo Init<VkImageViewCreateInfo>::Texture1D(VkImage image, VkFormat format, uint32_t arrayLayers, uint32_t baseArrayLayer, uint32_t mipLevels, uint32_t baseMipLevel)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,                      //Reserved
		0,                            //Flags
		image,                        //Image handle
		arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D, //Image view type
		format,                       //Format
		{
			VK_COMPONENT_SWIZZLE_R,   //Everything is
			VK_COMPONENT_SWIZZLE_G,   //ordered the same
			VK_COMPONENT_SWIZZLE_B,   //in the shaders
			VK_COMPONENT_SWIZZLE_A,   //as it is here
		},                            ///Components
		{
			VK_IMAGE_ASPECT_COLOR_BIT, //Aspect mask
			baseMipLevel,              //Base mip level
			mipLevels,                 //Mip level count
			baseArrayLayer,            //Base array layer
			arrayLayers                //Array layer count
		}                              ///Subresource range
	};
}

VkImageViewCreateInfo Init<VkImageViewCreateInfo>::Texture2D(VkImage image, VkFormat format, uint32_t arrayLayers, uint32_t baseArrayLayer, uint32_t mipLevels, uint32_t baseMipLevel)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,                      //Reserved
		0,                            //Flags
		image,                        //Image handle
		arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, //Image view type
		format,                       //Format
		{
			VK_COMPONENT_SWIZZLE_R,   //Everything is
			VK_COMPONENT_SWIZZLE_G,   //ordered the same
			VK_COMPONENT_SWIZZLE_B,   //in the shaders
			VK_COMPONENT_SWIZZLE_A,   //as it is here
		},                            ///Components
		{
			VK_IMAGE_ASPECT_COLOR_BIT, //Aspect mask
			baseMipLevel,              //Base mip level
			mipLevels,                 //Mip level count
			baseArrayLayer,            //Base array layer
			arrayLayers                //Array layer count
		}                              ///Subresource range
	};
}

VkImageViewCreateInfo Init<VkImageViewCreateInfo>::Texture3D(VkImage image, VkFormat format, uint32_t mipLevels, uint32_t baseMipLevel)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,                      //Reserved
		0,                            //Flags
		image,                        //Image handle
		VK_IMAGE_VIEW_TYPE_3D,        //Image view type
		format,                       //Format
		{
			VK_COMPONENT_SWIZZLE_R,   //Everything is
			VK_COMPONENT_SWIZZLE_G,   //ordered the same
			VK_COMPONENT_SWIZZLE_B,   //in the shaders
			VK_COMPONENT_SWIZZLE_A,   //as it is here
		},                            ///Components
		{
			VK_IMAGE_ASPECT_COLOR_BIT, //Aspect mask
			baseMipLevel,              //Base mip level
			mipLevels,                 //Mip level count
			0,                         //Base array layer
			1                          //Array layer count
		}                              ///Subresource range
	};
}

VkImageViewCreateInfo Init<VkImageViewCreateInfo>::CubeMap(VkImage image, VkFormat format, uint32_t arrayLayers, uint32_t baseArrayLayer, uint32_t mipLevels, uint32_t baseMipLevel)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,                      //Reserved
		0,                            //Flags
		image,                        //Image handle
		arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE, //Image view type
		format,                       //Format
		{
			VK_COMPONENT_SWIZZLE_R,   //Everything is
			VK_COMPONENT_SWIZZLE_G,   //ordered the same
			VK_COMPONENT_SWIZZLE_B,   //in the shaders
			VK_COMPONENT_SWIZZLE_A,   //as it is here
		},                            ///Components
		{
			VK_IMAGE_ASPECT_COLOR_BIT, //Aspect mask
			baseMipLevel,              //Base mip level
			mipLevels,                 //Mip level count
			baseArrayLayer,            //Base array layer
			arrayLayers                //Array layer count
		}                              ///Subresource range
	};
}

VkImageViewCreateInfo Init<VkImageViewCreateInfo>::DepthStencil(VkImage image, VkFormat format)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,                      //Reserved
		0,                            //Flags
		image,                        //Image handle
		VK_IMAGE_VIEW_TYPE_2D,        //Image view type
		format,                       //Format
		{
			VK_COMPONENT_SWIZZLE_R,   //Everything is
			VK_COMPONENT_SWIZZLE_G,   //ordered the same
			VK_COMPONENT_SWIZZLE_B,   //in the shaders
			VK_COMPONENT_SWIZZLE_A,   //as it is here
		},                            ///Components
		{
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, //Aspect mask
			0,                        //Base mip level
			1,                        //Mip level count
			0,                        //Base array layer
			1                         //Array layer count
		}                              ///Subresource range
	};
}

#pragma endregion

#pragma region VkSamplerCreateInfo

 VkSamplerCreateInfo Init<VkSamplerCreateInfo>::Create(VkFilter filter, VkSamplerAddressMode addressMode, VkSamplerMipmapMode mipmapMode, VkCompareOp compareOp)
{
	return {
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		nullptr,      //Reserved
		0,            //Flags: reserved
		filter,       //Mag filter
		filter,       //Min filter
		mipmapMode,   //Mipmap mode
		addressMode,  //Address mode U
		addressMode,  //Address mode V
		addressMode,  //Address mode W
		0.0f,         //Mip lod bias
		VK_FALSE,     //Enable anistropy
		0,            //Max anistopy
		VK_FALSE,     //Enable compare
		compareOp,    //Compare operation,
		0.0f,         //Min lod
		0.0f,         //Max lod
		VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE  //Border color
	};
}

#pragma endregion

#pragma region VkCommandPoolCreateInfo

VkCommandPoolCreateInfo Init<VkCommandPoolCreateInfo>::Create(uint32_t queueFamilyIndex)
{
	return {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,                                          //Reserved
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,  //Command buffers created from this pool can be reset manually
		queueFamilyIndex                                  //All command buffers from this pool must be submitted to this queue
	};
}

#pragma endregion

#pragma region VkAttachmentDescription

VkAttachmentDescription Init<VkAttachmentDescription>::Color(VkFormat format)
{
	return {
		0, //No flags
		format, //Format
		VK_SAMPLE_COUNT_1_BIT, //Sample count
		VK_ATTACHMENT_LOAD_OP_CLEAR, //Load operation
		VK_ATTACHMENT_STORE_OP_STORE, //Store operation
		VK_ATTACHMENT_LOAD_OP_DONT_CARE, //Stencil load operation
		VK_ATTACHMENT_STORE_OP_DONT_CARE, //Stencil store operation
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, //Initial layout
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL //Final layout
	};
}

VkAttachmentDescription Init<VkAttachmentDescription>::DepthStencil(VkFormat format)
{
	return {
		0, //No flags
		format, //Format
		VK_SAMPLE_COUNT_1_BIT, //Sample count
		VK_ATTACHMENT_LOAD_OP_CLEAR, //Load operation
		VK_ATTACHMENT_STORE_OP_STORE, //Store operation
		VK_ATTACHMENT_LOAD_OP_CLEAR, //Stencil load operation
		VK_ATTACHMENT_STORE_OP_STORE, //Stencil store operation
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, //Initial layout
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL //Final layout
	};
}

#pragma endregion

#pragma region VkFramebufferCreateInfo

VkFramebufferCreateInfo Init<VkFramebufferCreateInfo>::Create(VkRenderPass renderPass, glm::uvec2 resolution, const std::vector<VkImageView> &attachments)
{
	return {
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		nullptr,             //Reserved
		0,                   //No flags: reserved
		renderPass,          //Render pass
		static_cast<uint32_t>(attachments.size()),  //Attachment count
		attachments.data(),  //Attachments
		resolution.x,        //Width
		resolution.y,        //Height
		1                   //Layers
	};
}

#pragma endregion

#pragma region VkDescriptorSetLayoutBinding

VkDescriptorSetLayoutBinding Init<VkDescriptorSetLayoutBinding>::Create(uint32_t slot, VkDescriptorType type, VkShaderStageFlags visibility)
{
	return {
		slot,       //Binding
		type,       //Descriptor type
		1,          //Descriptor count
		visibility, //Stage flags
		nullptr     //Immutable samplers
	};
}

#pragma endregion

#pragma region VkDescriptorSetLayoutCreateInfo

VkDescriptorSetLayoutCreateInfo Init<VkDescriptorSetLayoutCreateInfo>::Create(const std::vector<VkDescriptorSetLayoutBinding> &bindings)
{
	return {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,          //Reserved
		0,                //No flags: reserved
		static_cast<uint32_t>(bindings.size()),  //Binding count. Narrowed from 64-bits, but no GPU can come close to handling that amount of descriptor sets anyways.
		bindings.data()   //Bindings
	};
}

#pragma endregion

#pragma region VkPipelineLayoutCreateInfo

VkPipelineLayoutCreateInfo Init<VkPipelineLayoutCreateInfo>::Create(const std::vector<VkDescriptorSetLayout> &layouts)
{
	return {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,        //Reserved,
		0,              //No flags: reserved
		static_cast<uint32_t>(layouts.size()), //Set layout count
		layouts.data(), //Set layouts
		0,              //Push constant range count
		nullptr         //Push constant ranges
	};
}

#pragma endregion

#pragma region VkShaderModuleCreateInfo

VkShaderModuleCreateInfo Init<VkShaderModuleCreateInfo>::FromSPIRV(size_t codeSize, uint32_t *shaderCode)
{
	return {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr,    //Reserved
		0,          //No flags: reserved
		codeSize,   //Code size
		shaderCode  //Code
	};
}

VkShaderModuleCreateInfo Init<VkShaderModuleCreateInfo>::FromGLSL(const std::string &source, VkShaderStageFlagBits stage, uint32_t *&spirvCode)
{
	//Pretty much a black box solution. Taken from the Vulkan SDK samples.

	const char *shaderCode = source.c_str();
	size_t codeSize = strlen(shaderCode);
	spirvCode = reinterpret_cast<uint32_t*>(new char[sizeof(uint32_t)*3 + codeSize + 1]);

	spirvCode[0] = 0x07230203;
	spirvCode[1] = 0;
	spirvCode[2] = stage;
	memcpy(&spirvCode[3], shaderCode, codeSize + 1);

	return {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr,   //Reserved
		0,         //No flags: reserved
		codeSize,  //Code size
		spirvCode  //Code
	};
}

#pragma endregion

#pragma region VkPipelineShaderStageCreateInfo

constexpr VkPipelineShaderStageCreateInfo Init<VkPipelineShaderStageCreateInfo>::Base()
{
	return {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		nullptr,         //Reserved
		0,               //No flags: reserved
		static_cast<VkShaderStageFlagBits>(0), //Shader stage: intentionally invalid
		VK_NULL_HANDLE,  //Module
		"main",          //Entry point
		nullptr          //Specialization info
	};
}

VkPipelineShaderStageCreateInfo Init<VkPipelineShaderStageCreateInfo>::FromModule(VkShaderModule module, VkShaderStageFlagBits stage, const std::string &entryFunc)
{
	return {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		nullptr,            //Reserved
		0,                  //No flags: reserved
		stage,              //Shader stage
		module,             //Module
		entryFunc.c_str(),  //Entry point
		nullptr             //Specialization info
	};
}

#pragma endregion
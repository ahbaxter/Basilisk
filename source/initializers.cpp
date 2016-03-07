/**
\file   initializers.cpp
\author Andrew Baxter
\date   March 6, 2016

Implements the default initializers declared in `initializers.h`

*/

#include <fstream>
#include <assert.h>
#include <memory>
#include "initializers.h"

using namespace Basilisk;

char *ReadBinaryFile(const std::string &filename, size_t *sizeOut)
{
#ifndef BASILISK_FINAL_BUILD
	if (nullptr == sizeOut)
	{
		Basilisk::errorMessage = "ReadBinaryFile()::sizeOut must not be a null pointer";
		assert("ReadBinaryFile()::sizeOut must not be a null pointer" && false);
	}
#endif
	//Open the file
	std::ifstream reader(filename, std::ios::in | std::ios::binary);
	if (!reader)
	{
		Basilisk::errorMessage = "ReadBinaryFile() could not open " + filename;
		*sizeOut = 0;
		return nullptr;
	}

	//Get size of file
	std::streampos start = reader.tellg();
	reader.seekg(0, std::ios::end);
	*sizeOut = reader.tellg() - start;

	//Read from the file
	char *data = new char[*sizeOut];
	reader.seekg(start);
	reader.get(data, *sizeOut);
	reader.close();

	return data;
}

std::string ReadTextFile(const std::string &filename)
{
	//Open the file
	std::ifstream reader(filename, std::ios::in);
	if (!reader.is_open())
	{
		Basilisk::errorMessage = "ReadTextFile() could not open " + filename;
		return "";
	}

	//Read from the file
	std::string contents, line = "";
	while (!reader.eof())
	{
		getline(reader, line);
		contents.append(line + "\n");
	}
	reader.close();

	return contents;
}

#pragma region VkImageCreateInfo

VkImageCreateInfo Init<VkImageCreateInfo>::Base()
{
	return {
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

VkImageCreateInfo Init<VkImageCreateInfo>::Texture2D(Bounds2D<uint32_t> dimensions, VkFormat format, VkImageUsageFlags usage, VkImageLayout initialLayout, uint32_t arrayLayers, uint32_t mipLevels)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,                    //Reserved
		0,                          //Flags
		VK_IMAGE_TYPE_2D,           //Image type
		format,                     //Format
		{ dimensions.width, dimensions.height, 1 },  //Extent
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

VkImageCreateInfo Init<VkImageCreateInfo>::Texture3D(Bounds3D<uint32_t> dimensions, VkFormat format, VkImageUsageFlags usage, VkImageLayout initialLayout, uint32_t arrayLayers, uint32_t mipLevels)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,                    //Reserved
		0,                          //Flags
		VK_IMAGE_TYPE_2D,           //Image type
		format,                     //Format
		{ dimensions.width, dimensions.height, dimensions.depth },  //Extent
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

VkImageCreateInfo Init<VkImageCreateInfo>::DepthStencil(Bounds2D<uint32_t> dimensions, VkFormat format, VkImageLayout initialLayout)
{
	return {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,                    //Reserved
		0,                          //Flags
		VK_IMAGE_TYPE_2D,           //Image type
		format,                     //Format
		{ dimensions.width, dimensions.height, 1 },  //Extent
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

VkImageViewCreateInfo Init<VkImageViewCreateInfo>::Base()
{
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

VkSamplerCreateInfo Init<VkSamplerCreateInfo>::Default()
{
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

#pragma region VkSubpassDescription

VkSubpassDescription Init<VkSubpassDescription>::Base()
{
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

#pragma endregion

#pragma region VkRenderPassCreateInfo

VkRenderPassCreateInfo Init<VkRenderPassCreateInfo>::Base()
{
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

#pragma endregion

#pragma region VkFramebufferCreateInfo

VkFramebufferCreateInfo Init<VkFramebufferCreateInfo>::Base()
{
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

VkFramebufferCreateInfo Init<VkFramebufferCreateInfo>::Create(VkRenderPass renderPass, Bounds2D<uint32_t> resolution, const std::vector<VkImageView> &attachments)
{
	return {
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		nullptr,             //Reserved
		0,                   //No flags: reserved
		renderPass,          //Render pass
		attachments.size(),  //Attachment count
		attachments.data(),  //Attachments
		resolution.width,    //Width
		resolution.height,   //Height
		1                    //Layers
	};
}

#pragma endregion

#pragma region VkDescriptorSetLayoutBinding

VkDescriptorSetLayoutBinding Init<VkDescriptorSetLayoutBinding>::Base()
{
	return {
		0,                                  //Binding
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  //Descriptor type
		1,                                  //Descriptor count
		VK_SHADER_STAGE_ALL_GRAPHICS,       //Stage flats
		nullptr                             //Immutable samplers
	};
}

VkDescriptorSetLayoutBinding Init<VkDescriptorSetLayoutBinding>::Create(uint32_t slot, VkDescriptorType type, VkShaderStageFlags visibility)
{
	return {
		slot,       //Binding
		type,       //Descriptor type
		1,          //Descriptor count
		visibility, //Stage flats
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
		bindings.size(),  //Binding count
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
		layouts.size(), //Set layout count
		layouts.data(), //Set layouts
		0,              //Push constant range count
		nullptr         //Push constant ranges
	};
}

#pragma endregion

#pragma region VulkanGraphicsPipelineState

VulkanGraphicsPipelineState Init<VulkanGraphicsPipelineState>::Base(const std::vector<VkDynamicState> &dynamicStateEnables, const std::vector<VkPipelineColorBlendAttachmentState> &blendAttachments)
{
	return {
		{ //Vertex input state
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			nullptr,  //Reserved
			0,        //No flags: reserved
			0,        //Binding description count
			nullptr,  //Binding descriptions
			0,        //Attribute description count
			nullptr   //Attribute descriptions
		},
		{ //Input assembly state
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			nullptr, //Reserved
			0, //No flags: reserved
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, //Primitive topology
			VK_FALSE, //Enable primitive restart
		},
		{ //Tesselation state
			VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
			nullptr, //Reserved
			0,       //No flags: reserved
			0,       //Patch control points: purposely invalid
		},
		{ //Viewport state
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			nullptr,  //Reserved
			0,        //No flags: reserved
			1,        //Viewport count
			nullptr,  //Viewports
			1,        //Scissor count
			nullptr   //Scissors
		},
		{ //Rasterization state
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			nullptr,                  //Reserved
			0,                        //No flags: reserved
			VK_TRUE,                  //Enable depth clamp
			VK_FALSE,                 //Enable rasterizer discard
			VK_POLYGON_MODE_FILL,     //Polygon mode: [fill], wireframe, dots
			VK_CULL_MODE_BACK_BIT,    //Cull mode: cull back-facing polygons
			VK_FRONT_FACE_CLOCKWISE,  //Front face: discard zero-area polygons
			VK_FALSE,                 //Enable depth bias
			0.0f,                     //Depth bias constant factor
			0.0f,                     //Depth bias clamp
			0.0f,                     //Depth bias slope factor
			1.0f                      //Line width
		},
		{ //Multisample state
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			nullptr,                //Reserved
			0,                      //No flags: reserved
			VK_SAMPLE_COUNT_1_BIT,  //Rasterization samples: 1
			VK_FALSE,               //Enable sample shading
			0.0f,                   //Min sample shading
			nullptr,                //Sample mask
			VK_FALSE,               //Enable alpha-to-coverage
			VK_FALSE                //Enable alpha-to-one
		},
		{ //Depth stencil state
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			nullptr,   //Reserved
			0,         //No flags: reserved
			VK_TRUE,   //Enable depth test
			VK_TRUE,   //Enable depth write
			VK_COMPARE_OP_LESS_OR_EQUAL, //Depth compare operation
			VK_FALSE,  //Enable depth bounds testing
			VK_FALSE,  //Enable stencil testing
			{          //Front-facing polygon stencil behavior
				VK_STENCIL_OP_KEEP,     //On fail: keep the old value
				VK_STENCIL_OP_KEEP,     //On pass: keep the old value
				VK_STENCIL_OP_KEEP,     //On depth fail: keep the old value
				VK_COMPARE_OP_ALWAYS,   //Compare operation: really doesn't matter, since pass and fail have the same result
				0,     //Compare mask
				0,     //Write mask
				0,     //Reference
			},
			{          //Back-facing polygon stencil behavior (identical to front, since stencil is disabled)
				VK_STENCIL_OP_KEEP,     //On fail: keep the old value
				VK_STENCIL_OP_KEEP,     //On pass: keep the old value
				VK_STENCIL_OP_KEEP,     //On depth fail: keep the old value
				VK_COMPARE_OP_ALWAYS,   //Compare operation: really doesn't matter, since pass and fail have the same result
				0,     //Compare mask
				0,     //Write mask
				0,     //Reference
			},
			0,         //Min depth bounds
			1          //Max depth bounds
		},
		{ //Color blend state
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			nullptr,                  //Reserved
			0,                        //No flags: reserved
			VK_FALSE,                 //Enable logic operation
			VK_LOGIC_OP_CLEAR,        //Logic operation: doesn't matter, since it's disabled
			blendAttachments.size(),  //Color blend attachment count
			blendAttachments.data(),  //Color blend attachments
			{0.0f, 0.0f, 0.0f, 0.0f}  //Blend constants
		},
		{ //Dynamic state
			VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			nullptr,                     //Reserved
			0,                           //No flags: reserved
			dynamicStateEnables.size(),  //Dynamic state count
			dynamicStateEnables.data()   //Dynamic states
		},
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

VkPipelineShaderStageCreateInfo Init<VkPipelineShaderStageCreateInfo>::Base()
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
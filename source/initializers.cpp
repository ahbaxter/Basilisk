/**
\file   initializers.cpp
\author Andrew Baxter
\date   March 5, 2016

Implements the default initializers declared in `initializers.h`

*/

#include "initializers.h"

using namespace Basilisk;

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
	return
	{
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

VkSamplerCreateInfo Init<VkSamplerCreateInfo>::Base()
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

VkCommandPoolCreateInfo Init<VkCommandPoolCreateInfo>::Base(uint32_t queueFamilyIndex)
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
	return{
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
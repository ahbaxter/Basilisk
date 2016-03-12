/**
\file   backend.cpp
\author Andrew Baxter
\date   March 11, 2016

Defines the behavior of the Vulkan rendering backend

*/

#include <sstream>
#include "rendering/backend.h"
#include "rendering/initializers.h"

using namespace Vulkan;

#pragma region Configuration

constexpr uint32_t Vulkan::layerCount() {
	return 0;
}
constexpr const char **Vulkan::layerNames() {
	return nullptr;
}

constexpr uint32_t Vulkan::extensionCount() {
	return 5;
}

const char *extNames[Vulkan::extensionCount()] = {
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_DISPLAY_EXTENSION_NAME,
	VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME
};

constexpr const char **Vulkan::extensionNames() {
	return extNames;
}

constexpr uint32_t Vulkan::apiVersion() {
	return VK_API_VERSION;
}
#pragma endregion

void SwapChain::Release(VkDevice device, PFN_vkDestroySwapchainKHR func)
{
	//The images and image views are managed under the hood by Vulkan
	if (m_swapChain)
	{
		func(device, m_swapChain, nullptr);
		m_swapChain = VK_NULL_HANDLE;
	}
}

void Shader::Release(VkDevice device)
{
	if (m_module)
	{
		vkDestroyShaderModule(device, m_module, nullptr);
		m_module = VK_NULL_HANDLE;
	}
}

void PipelineLayout::Release(VkDevice device)
{
	if (m_layout)
	{
		vkDestroyPipelineLayout(device, m_layout, nullptr);
		m_layout = VK_NULL_HANDLE;
	}
	if (m_setLayout)
	{
		vkDestroyDescriptorSetLayout(device, m_setLayout, nullptr);
		m_setLayout = VK_NULL_HANDLE;
	}
}

void GraphicsPipeline::Release(VkDevice device)
{
	if (m_pipeline)
	{
		vkDestroyPipeline(device, m_pipeline, nullptr);
		m_pipeline = VK_NULL_HANDLE;
	}
}

#pragma region FrameBuffer
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

void FrameBuffer::ResizeVectors(uint32_t size)
{
	m_images.resize(size);
	m_views.resize(size);
	m_formats.resize(size);
	m_memory.resize(size);
}

#pragma endregion

#pragma region CommandBuffer
void CommandBuffer::Release(VkDevice device, VkCommandPool pool)
{
	if (m_commandBuffer)
	{
		vkFreeCommandBuffers(device, pool, 1, &m_commandBuffer);
		m_commandBuffer = VK_NULL_HANDLE;
	}
}

CommandBuffer::CommandBuffer() : m_commandBuffer(nullptr) {
}


bool CommandBuffer::Begin(bool disposable)
{
	VkCommandBufferUsageFlags flags = disposable ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;

	VkCommandBufferBeginInfo begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,  //Reserved
		flags,    //Flags
		nullptr   //Inheritance info
	};

	if (Failed(vkBeginCommandBuffer(m_commandBuffer, &begin_info)))
	{
		Basilisk::errors.push("Vulkan::CommandBuffer::Begin() could not begin writing to the command buffer");
		return false;
	}

	return true;
}

bool CommandBuffer::End()
{
	if (Failed(vkEndCommandBuffer(m_commandBuffer)))
	{
		Basilisk::errors.push("Vulkan::CommandBuffer::Begin() could not close the command buffer");
		return false;
	}

	return true;
}

void CommandBuffer::SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t oldQueueFamilyIndex, uint32_t newQueueFamilyIndex)
{
	VkImageSubresourceRange range = {
		aspectMask,  //Aspect mask
		0,  //Base mip level
		1,  //Level count
		0,  //Base array layer
		1   //Layer count
	};

	//TODO: Depth buffer attachments
	VkImageMemoryBarrier image_memory_barrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,                              //Reserved
		0,                                    //Source access mask
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,  //Destination access mask
		oldLayout,                            //Old layout
		newLayout,                            //New layout
		oldQueueFamilyIndex,                  //Source queue family index
		newQueueFamilyIndex,                  //Destination queue family index
		image,                                //Image
		range                                 //Subresource range
	};

	VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(m_commandBuffer, src_stages, dest_stages, 0, 0, nullptr, 0, nullptr,
		1, &image_memory_barrier);
}

void CommandBuffer::WriteBundle(const std::shared_ptr<CommandBuffer> &bundle)
{
	vkCmdExecuteCommands(m_commandBuffer, 1, &bundle->m_commandBuffer);
}

#pragma endregion

#pragma region Device
void Device::Release() {
	for (uint32_t i = 0; i < m_commandPools.size(); ++i)
	{
		vkDestroyCommandPool(m_device, m_commandPools[i], nullptr);
	}
	if (m_device)
	{
		vkDestroyDevice(m_device, nullptr);
		m_device = VK_NULL_HANDLE;
	}
}

Device::Device() : m_parent(nullptr), m_device(VK_NULL_HANDLE),
	pfnCreateSwapchainKHR(nullptr),
	pfnDestroySwapchainKHR(nullptr),
	pfnGetSwapchainImagesKHR(nullptr),
	pfnAcquireNextImageKHR(nullptr),
	pfnQueuePresentKHR(nullptr),
	pfnGetPhysicalDeviceDisplayPropertiesKHR(nullptr),
	pfnGetPhysicalDeviceDisplayPlanePropertiesKHR(nullptr),
	pfnGetDisplayPlaneSupportedDisplaysKHR(nullptr),
	pfnGetDisplayModePropertiesKHR(nullptr),
	pfnCreateDisplayModeKHR(nullptr),
	pfnGetDisplayPlaneCapabilitiesKHR(nullptr),
	pfnCreateDisplayPlaneSurfaceKHR(nullptr),
	pfnCreateSharedSwapchainsKHR(nullptr)
{
	m_gpuProps = {};
}

void Device::Join()
{
	vkDeviceWaitIdle(m_device);
}

std::shared_ptr<CommandBuffer> Device::CreateCommandBuffer(bool bundle, uint32_t poolIndex)
{
	std::shared_ptr<CommandBuffer> out(new CommandBuffer,
		[=](CommandBuffer *ptr)
		{
			ptr->Release(m_device, m_commandPools[poolIndex]);
			delete ptr;
			ptr = nullptr;
		}
	);

	VkCommandBufferAllocateInfo cmd_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,                    //Reserved
		m_commandPools[poolIndex],  //Command pool
		bundle ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY, //Buffer level
		1                           //Command buffer count
	};

	VkResult res = vkAllocateCommandBuffers(m_device, &cmd_info, &out->m_commandBuffer);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateCommandBuffer() could not create the command buffer");
	}
}

std::shared_ptr<SwapChain> Device::CreateSwapChain(const std::shared_ptr<CommandBuffer> &setup, glm::uvec2 resolution, uint32_t numBuffers)
{
#ifndef BASILISK_FINAL_BUILD
	if (nullptr == setup)
	{
		Basilisk::errors.push("Vulkan::Device::CreateSwapChain()::setupBuffer must not be a null pointer");
		return nullptr;
	}
	else if (nullptr == m_parent->GetPresentTarget())
	{
		Basilisk::errors.push("Vulkan::Device::CreateSwapChain() cannot be called before hooking into a window or monitor");
		return nullptr;
	}
#endif

	//Meets all prerequisites
	
	//
	////Calculate the swap chain's resolution (may differ from the `resolution` argument)
	//

	VkExtent2D swapChainRes;
	if (static_cast<uint32_t>(-1) == m_parent->GetPresentTarget()->caps.currentExtent.width)
	{ //Surface size is undefined
		swapChainRes.width = Clamp(resolution.x,
			m_parent->GetPresentTarget()->caps.minImageExtent.width,
			m_parent->GetPresentTarget()->caps.maxImageExtent.width);

		swapChainRes.height = Clamp(resolution.y,
			m_parent->GetPresentTarget()->caps.minImageExtent.height,
			m_parent->GetPresentTarget()->caps.maxImageExtent.height);
	}
	else
	{ //Surface size is already defined
		swapChainRes = m_parent->GetPresentTarget()->caps.currentExtent;
	}

	//
	////Determine the optimal properties for the swap chain
	//

	//Determine the present mode
	VkPresentModeKHR swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR; //Default to the always-available FIFO present mode
	for (VkPresentModeKHR i : m_parent->GetPresentTarget()->presentModes)
	{
		if (VK_PRESENT_MODE_MAILBOX_KHR == i)
		{ //This is the lowest-latency non-tearing present mode
			swapChainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break; //We can't get any better than this
		}
		if (VK_PRESENT_MODE_IMMEDIATE_KHR == i)
		{ //The fastest present mode, but it tears -- hold out for mailbox mode
			swapChainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	//Determine the number of Image backbuffers to use (caller may have to settle)
	numBuffers = Clamp(numBuffers,
		m_parent->GetPresentTarget()->caps.minImageCount + 1, //+ 1 enables non-blocking calls to `vkAcquireNextImageKHR()` in mailbox mode
		m_parent->GetPresentTarget()->caps.maxImageCount);

	//Make sure the surface has a pre-transform (even if it's just the identity matrix)
	VkSurfaceTransformFlagBitsKHR preTransform;
	if (m_parent->GetPresentTarget()->caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	} else {
		preTransform = m_parent->GetPresentTarget()->caps.currentTransform;
	}

	//
	////Create the swap chain
	//

	std::shared_ptr<SwapChain> out(new SwapChain,
		[=](SwapChain *ptr) {
			ptr->Release(m_device, pfnDestroySwapchainKHR);
			delete ptr;
			ptr = nullptr;
		}
	);

	VkSwapchainCreateInfoKHR swapchain_info = 
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,               //Reserved
		0,                     //No flags
		m_parent->GetPresentTarget()->surface,      //Target surface
		numBuffers,            //Number of back buffers
		m_parent->GetPresentTarget()->colorFormat,  //Surface format
		VK_COLORSPACE_SRGB_NONLINEAR_KHR,           //Color space
		swapChainRes,          //Resolution
		1,                     //Image layers
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, //Image usage
		VK_SHARING_MODE_EXCLUSIVE,                  //Image sharing mode
		0,                     //Queue family index count
		nullptr,               //Queue family indices
		preTransform,          //Pre-present transformation
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,          //Blending. Opaque because back buffers are never blended
		swapChainPresentMode,  //Present mode
		true,                  //Clipped
		VK_NULL_HANDLE         //Old swap chain
	};

	VkResult res = pfnCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &out->m_swapChain);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateSwapChain() could not create the swap chain");
		return nullptr;
	}

	//
	////Store the back buffers
	//

	res = pfnGetSwapchainImagesKHR(m_device, out->m_swapChain, &numBuffers, nullptr);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateSwapChain() could not count the swap chain's back buffers");
		return nullptr;
	}

	out->m_backBuffers.resize(numBuffers);
	out->m_backBufferViews.resize(numBuffers);

	res = pfnGetSwapchainImagesKHR(m_device, out->m_swapChain, &numBuffers, out->m_backBuffers.data());
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateSwapChain() could not count the swap chain's back buffers");
		return nullptr;
	}

	//
	////Create views for each back buffer
	//

	for (uint32_t i = 0; i < out->m_backBuffers.size(); ++i)
	{
		VkImageViewCreateInfo color_image_view = Init<VkImageViewCreateInfo>::Texture2D(out->m_backBuffers[i], m_parent->GetPresentTarget()->colorFormat);

		res = vkCreateImageView(m_device, &color_image_view, nullptr, &out->m_backBufferViews[i]);
		if (Failed(res))
		{
			Basilisk::errors.push("Vulkan::Device::CreateSwapChain() could not create a view for all back buffers");
			return nullptr;
		}

		//Set the image layout to depth stencil optimal
		setup->SetImageLayout(out->m_backBuffers[i],
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			renderQueue, renderQueue);
	}


	return out;
}

std::shared_ptr<FrameBuffer> Device::CreateFrameBuffer(glm::uvec2 resolution, const std::vector<VkFormat> &colorFormats, bool enableDepth)
{
#ifndef BASILISK_FINAL_BUILD
	if (0 == colorFormats.size() && !enableDepth)
	{
		Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() must have at least one image attachment");
		return nullptr;
	}
#endif

	//Meets all prerequisites

	//Preparations...
	std::shared_ptr<FrameBuffer> out(new FrameBuffer,
		[=](FrameBuffer *ptr) {
			ptr->Release(m_device);
			delete ptr;
			ptr = nullptr;
		}
	);

	out->m_resolution = resolution;

	uint32_t numAttachments = static_cast<uint32_t>(colorFormats.size()) + (enableDepth ? 1 : 0);
	out->ResizeVectors(numAttachments);
	std::vector<VkAttachmentDescription> attachmentDescs(numAttachments);
	std::vector<VkAttachmentReference> attachmentRefs(numAttachments);

	uint32_t i;

	//Create and store the images used in the frame buffer
	VkImageCreateInfo image_create_info = Init<VkImageCreateInfo>::Texture2D(resolution, VK_FORMAT_UNDEFINED, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	for (i = 0; i < colorFormats.size(); ++i)
	{ //Color attachment
		image_create_info.format = static_cast<VkFormat>(colorFormats[i]);
		if (Failed( vkCreateImage(m_device, &image_create_info, nullptr, &out->m_images[i]) ))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not create all color images");
			return nullptr;
		}
	}
	for (; i < numAttachments; ++i)
	{ //Depth attachment
		image_create_info.format = static_cast<VkFormat>(colorFormats[i]);
		image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		image_create_info.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		if (Failed( vkCreateImage(m_device, &image_create_info, nullptr, &out->m_images[i]) ))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not create all depth images");
			return nullptr;
		}
	}
	
	//Allocate and store device memory for the images
	VkMemoryAllocateInfo memAlloc = {};
	VkMemoryRequirements memReqs;
	for (i = 0; i < numAttachments; ++i)
	{ //Does not need to be split up between color and depth stencil attachments
		vkGetImageMemoryRequirements(m_device, out->m_images[i], &memReqs);
		memAlloc.allocationSize = memReqs.size;
		if (!MemoryTypeFromProps(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memAlloc.memoryTypeIndex))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not determine appropriate memory type for all images");
			return nullptr;
		}
		if (Failed( vkAllocateMemory(m_device, &memAlloc, nullptr, &out->m_memory[i]) ))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not allocate memory for all images");
			return nullptr;
		}
		if (Failed( vkBindImageMemory(m_device, out->m_images[i], out->m_memory[i], 0) ))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not bind memory for all images");
			return nullptr;
		}
	}

	//Create and store the image views
	VkImageViewCreateInfo view_create_info = Init<VkImageViewCreateInfo>::Texture2D(VK_NULL_HANDLE, VK_FORMAT_UNDEFINED);
	for (i = 0; i < colorFormats.size(); ++i)
	{ //Color attachment
		view_create_info.image = out->m_images[i];
		view_create_info.format = static_cast<VkFormat>(colorFormats[i]);
		if (Failed( vkCreateImageView(m_device, &view_create_info, nullptr, &out->m_views[i]) ))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not create all color image views");
			return nullptr;
		}
	}
	for (; i < numAttachments; ++i)
	{ //Depth attachment
		view_create_info.image = out->m_images[i];
		view_create_info.format = static_cast<VkFormat>(m_gpuProps.depthFormat);
		view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		if (Failed( vkCreateImageView(m_device, &view_create_info, nullptr, &out->m_views[i]) ))
		{
			Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not create all the depth stencil image view");
			return nullptr;
		}
	}

	//Create the attachment descriptions, and store the formats
	for (i = 0; colorFormats.size(); ++i)
	{ //Color attachment
		attachmentDescs[i] = Init<VkAttachmentDescription>::Color(static_cast<VkFormat>(colorFormats[i]));
		attachmentRefs[i] = { i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		out->m_formats[i] = static_cast<VkFormat>(colorFormats[i]);
	}
	for (; i < numAttachments; ++i)
	{ //Depth attachment
		attachmentDescs[i] = Init<VkAttachmentDescription>::DepthStencil(m_gpuProps.depthFormat);
		attachmentRefs[i] = { i, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		out->m_formats[i] = m_gpuProps.depthFormat;
	}

	//Create the render pass
	VkSubpassDescription subpassDesc = Init<VkSubpassDescription>::Base();
	subpassDesc.colorAttachmentCount = static_cast<uint32_t>(colorFormats.size());
	subpassDesc.pColorAttachments = &attachmentRefs[0];
	if (enableDepth)
		subpassDesc.pDepthStencilAttachment = &attachmentRefs[colorFormats.size() - 1]; //The last attachment is depth
	
	
	VkRenderPassCreateInfo rp_create_info = Init<VkRenderPassCreateInfo>::Base();
	rp_create_info.attachmentCount = numAttachments;
	rp_create_info.pAttachments = attachmentDescs.data();
	rp_create_info.subpassCount = 1;
	rp_create_info.pSubpasses = &subpassDesc;
	
	if (Failed( vkCreateRenderPass(m_device, &rp_create_info, nullptr, &out->m_renderPass) ))
	{
		Basilisk::errors.push("Vulkan::Device::CreateFrameBuffer() could not create the render pass");
		return nullptr;
	}

	VkFramebufferCreateInfo fb_create_info = Init<VkFramebufferCreateInfo>::Create(out->m_renderPass, resolution, out->m_views);
	
	if (Failed(vkCreateFramebuffer(m_device, &fb_create_info, nullptr, &out->m_frameBuffer)))
	{
		Basilisk::errors.push("Basilisk::Device::CreateFrameBuffer() could not create the frame buffer");
		return nullptr;
	}


	return out;
}

bool Device::MemoryTypeFromProps(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex)
{
	//Search memoryTypes to find first index with those properties
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {
			//Type is available; does it match user properties?
			if ((m_gpuProps.memProps.memoryTypes[i].propertyFlags &
				requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	//No memory types matched; return failure
	return false;
}

std::shared_ptr<PipelineLayout> Device::CreatePipelineLayout(const std::vector<Descriptor> &bindings)
{
	std::shared_ptr<PipelineLayout> out(new PipelineLayout,
		[=](PipelineLayout *ptr) {
			ptr->Release(m_device);
			delete ptr;
			ptr = nullptr;
		}
	);
	
	std::vector<VkDescriptorSetLayoutBinding> layout_bindings(bindings.size());
	for (uint32_t i = 0; i < bindings.size(); ++i)
	{
		layout_bindings[i] = Init<VkDescriptorSetLayoutBinding>::Create(bindings[i].bindPoint, bindings[i].type, bindings[i].visibility);
	}
	
	VkDescriptorSetLayoutCreateInfo set_info = Init<VkDescriptorSetLayoutCreateInfo>::Create(layout_bindings);

	VkResult res = vkCreateDescriptorSetLayout(m_device, &set_info, nullptr,
		&out->m_setLayout);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreatePipelineLayout() could not create the descriptor set layout");
		return nullptr;
	}
	
	VkPipelineLayoutCreateInfo pipeline_info = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,                 //Reserved
		0,                       //No flags: reserved
		static_cast<uint32_t>(layout_bindings.size()),  //Descriptor set layout count
		&out->m_setLayout,       //Descriptor set layouts
		0,                       //Push constant range count
		VK_NULL_HANDLE           //Push constant ranges
	};
	
	res = vkCreatePipelineLayout(m_device, &pipeline_info, nullptr, &out->m_layout);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulan::Device::CreatePipelineLayout() could not create the pipeline layout");
		return nullptr;
	}
	
	
	return out;
}

std::shared_ptr<Shader> Device::CreateShaderFromSPIRV(const std::vector<uint32_t> &bytecode)
{
	std::shared_ptr<Shader> out(new Shader,
		[=](Shader *ptr) {
			ptr->Release(m_device);
			delete ptr;
			ptr = nullptr;
		}
	);
	VkResult res;
	
	VkShaderModuleCreateInfo module_info = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr,          //Reserved
		0,                //No flags: reserved
		bytecode.size(),  //Bytecode size in bytes
		bytecode.data()   //Bytecode data
	};

	res = vkCreateShaderModule(m_device, &module_info, nullptr, &out->m_module);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateShaderFromSPIRV() could not create the shader module");
		return nullptr;
	}


	return out;
}

std::shared_ptr<Shader> Device::CreateShaderFromGLSL(const std::string &source, VkShaderStageFlagBits stage)
{
	std::shared_ptr<Shader> out(new Shader,
		[=](Shader *ptr) {
			ptr->Release(m_device);
			delete ptr;
			ptr = nullptr;
		}
	);
	VkResult res;
	
	uint32_t spirvSize = static_cast<uint32_t>( 3 * sizeof(uint32_t) + source.size() + 1 );
	uint32_t *spirvCode = new uint32_t[spirvSize];
	spirvCode[0] = 0x07230203;
	spirvCode[1] = 0;
	spirvCode[2] = stage;
	memcpy(&spirvCode[3], source.c_str(), spirvSize + 1); //1 more byte for escape character
	
	VkShaderModuleCreateInfo module_info = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr,    //Reserved
		0,          //No flags: reserved
		spirvSize,  //Bytecode size in bytes
		spirvCode   //Bytecode data
	};

	res = vkCreateShaderModule(m_device, &module_info, nullptr, &out->m_module);
	delete[] spirvCode;
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateShaderFromGLSL() could not create the shader module");
		return nullptr;
	}


	return out;
}

std::shared_ptr<GraphicsPipeline> Device::CreateGraphicsPipeline(const std::shared_ptr<FrameBuffer> &frameBuffer, const std::shared_ptr<PipelineLayout> &layout, const std::vector<Shader> &shaders, uint32_t subpassIndex, uint32_t patchCtrlPoints)
{
	VkGraphicsPipelineCreateInfo pipeline_info;


	std::vector<VkPipelineShaderStageCreateInfo> stage_info(shaders.size());
	for (uint32_t i = 0; i < shaders.size(); ++i)
	{
		stage_info[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_info[i].pNext = nullptr;
		stage_info[i].flags = 0;
		stage_info[i].stage = shaders[i].m_stage;
		stage_info[i].module = shaders[i].m_module;
		stage_info[i].pName = shaders[i].m_entryPoint.c_str();
		stage_info[i].pSpecializationInfo = nullptr;
	}
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext = nullptr;
	pipeline_info.flags = 0;
	pipeline_info.stageCount = shaders.size();
	pipeline_info.pStages = stage_info.data();


	VkPipelineVertexInputStateCreateInfo vertex_input = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,  //Reserved
		0,        //No flags: reserved
		0,        //Binding description count
		nullptr,  //Binding descriptions
		0,        //Attribute description count
		nullptr   //Attribute descriptions
	};
	pipeline_info.pVertexInputState = &vertex_input;
	

	VkPipelineInputAssemblyStateCreateInfo input_assembly = { //Input assembly state
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		nullptr, //Reserved
		0, //No flags: reserved
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, //Primitive topology
		VK_FALSE, //Enable primitive restart
	};
	pipeline_info.pInputAssemblyState = &input_assembly;


	VkPipelineTessellationStateCreateInfo tesselation = {
		VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		nullptr,         //Reserved
		0,               //No flags: reserved
		patchCtrlPoints  //Patch control points
	};
	if (patchCtrlPoints > 0)
		pipeline_info.pTessellationState = &tesselation;
	else
		pipeline_info.pTessellationState = nullptr;


	VkPipelineViewportStateCreateInfo viewport = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,  //Reserved
		0,        //No flags: reserved
		1,        //Viewport count
		nullptr,  //Viewports
		1,        //Scissor count
		nullptr   //Scissors
	};
	pipeline_info.pViewportState = &viewport;


	VkPipelineRasterizationStateCreateInfo rasterization = {
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
	};
	pipeline_info.pRasterizationState = &rasterization;


	VkPipelineMultisampleStateCreateInfo multisample = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,                //Reserved
		0,                      //No flags: reserved
		VK_SAMPLE_COUNT_1_BIT,  //Rasterization samples: 1
		VK_FALSE,               //Enable sample shading
		0.0f,                   //Min sample shading
		nullptr,                //Sample mask
		VK_FALSE,               //Enable alpha-to-coverage
		VK_FALSE                //Enable alpha-to-one
	};
	pipeline_info.pMultisampleState = &multisample;


	VkPipelineDepthStencilStateCreateInfo depth_stencil = {
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
	};
	pipeline_info.pDepthStencilState = &depth_stencil;


	std::vector<VkPipelineColorBlendAttachmentState> blend_attachments(frameBuffer->NumAttachments());
	for (auto &iter : blend_attachments)
	{
		iter.blendEnable = VK_FALSE;
		iter.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		iter.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		iter.colorBlendOp = VK_BLEND_OP_ADD;
		iter.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		iter.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		iter.alphaBlendOp = VK_BLEND_OP_ADD;
		iter.colorWriteMask = 0xF;
	}
	VkPipelineColorBlendStateCreateInfo color_blend = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		nullptr,                    //Reserved
		0,                          //No flags: reserved
		VK_FALSE,                   //Enable logic operation
		VK_LOGIC_OP_CLEAR,          //Logic operation: doesn't matter, since it's disabled
		frameBuffer->NumAttachments(),  //Color blend attachment count
		blend_attachments.data(),   //Color blend attachments
		{ 0.0f, 0.0f, 0.0f, 0.0f }  //Blend constants
	};
	pipeline_info.pColorBlendState = &color_blend;


	std::array<VkDynamicState, 3> dynamic_enables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamic = {
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		nullptr,                  //Reserved
		0,                        //No flags: reserved
		dynamic_enables.size(),  //Dynamic state count
		dynamic_enables.data()   //Dynamic state enables
	};
	pipeline_info.pDynamicState = &dynamic;

	pipeline_info.layout = layout->m_layout;
	pipeline_info.renderPass = frameBuffer->m_renderPass;
	pipeline_info.subpass = subpassIndex;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = -1;

	std::shared_ptr<GraphicsPipeline> out(new GraphicsPipeline,
		[=](GraphicsPipeline *ptr) {
			ptr->Release(m_device);
			delete ptr;
			ptr = nullptr;
		}
	);

	VkResult res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &out->m_pipeline);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateGraphicsPipeline() could not create the graphics pipeline");
		return nullptr;
	}


	return out;
}


#pragma endregion

#pragma region Instance
void Instance::Release()
{
	//Memory for presentable surfaces is handled under the hood by Vulkan, but our pointer needs to be deallocated
	if (m_presentTarget)
	{
		delete m_presentTarget;
		m_presentTarget = nullptr;
	}

	if (m_instance)
	{
		vkDestroyInstance(m_instance, nullptr);
		m_instance = nullptr;
	}
}

Instance::Instance() : m_instance(VK_NULL_HANDLE)
{}


std::shared_ptr<Instance> Vulkan::Initialize(const std::string &appName, uint32_t appVersion)
{
	std::shared_ptr<Instance> out(new Instance,
		[](Instance *ptr) { //Custom deallocator
			ptr->Release();
			delete ptr;
			ptr = nullptr;
		}
	);

	//Should I let them specify application version as well?
	VkApplicationInfo appInfo =
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,          //Must be NULL
		appName.c_str(),  //Application name
		appVersion,       //Application version
		"Basilisk",       //Engine name
		1,                //Engine version
		apiVersion()      //API version
	};

	VkInstanceCreateInfo instanceInfo =
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,           //Must be NULL
		0,                 //No flags
		&appInfo,          //Let the GPU know who we are
		layerCount(),      //Number of layers
		layerNames(),      //Which layers we're using
		extensionCount(),  //Number of extensions
		extensionNames()   //Which extensions we're using
	};

	//           Rely on automatic memory allocation ----v
	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &out->m_instance);
	if (Failed(result))
	{
		Basilisk::errors.push("Vulkan::Initialize() could not create a Vulkan Instance");
		return nullptr;
	}

	//Grab Vulkan callbacks for surfaces
	out->pfnDestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(vkGetInstanceProcAddr(out->m_instance, "vkDestroySurfaceKHR"));
	out->pfnGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(vkGetInstanceProcAddr(out->m_instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
	out->pfnGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(vkGetInstanceProcAddr(out->m_instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
	out->pfnGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(vkGetInstanceProcAddr(out->m_instance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
	out->pfnGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(vkGetInstanceProcAddr(out->m_instance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
	//Grab Vulkan callbacks for Win32 surfaces
	out->pfnCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(out->m_instance, "vkCreateWin32SurfaceKHR"));
	out->pfnGetPhysicalDeviceWin32PresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(vkGetInstanceProcAddr(out->m_instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR"));


	return out;
}

uint32_t Instance::FindGpus()
{
	VkResult result;
	uint32_t count;

	//Count the devices
	result = vkEnumeratePhysicalDevices(m_instance, &count, nullptr);
	if (Failed(result))
	{
		Basilisk::errors.push("Vulkan::Instance::::FindGpus() could not count physical devices");
		return 0;
	}
	//Resize our arrays to store all devices
	m_gpus.resize(count);
	m_gpuProps.resize(count);
	//Store the devices
	result = vkEnumeratePhysicalDevices(m_instance, &count, m_gpus.data());
	if (Failed(result))
	{
		Basilisk::errors.push("Vulkan::Instance::::FindGpus() could not list physical devices");
		return 0;
	}

	//Store device details
	for (uint32_t i = 0; i < count; ++i)
	{
		//Get the device's supported features list
		vkGetPhysicalDeviceFeatures(m_gpus[i], &m_gpuProps[i].features);
		//Get the device's general properties
		vkGetPhysicalDeviceProperties(m_gpus[i], &m_gpuProps[i].props);
		//Get the device's memory properties
		vkGetPhysicalDeviceMemoryProperties(m_gpus[i], &m_gpuProps[i].memProps);
		//Get descriptions for the device's queue families
		uint32_t numQueues;
		vkGetPhysicalDeviceQueueFamilyProperties(m_gpus[i], &numQueues, nullptr);
		if (numQueues >= 1)
		{
			m_gpuProps[i].queueDescs.resize(numQueues);
			vkGetPhysicalDeviceQueueFamilyProperties(m_gpus[i], &numQueues, m_gpuProps[i].queueDescs.data());
		}
		//Find an optimal depth buffer format
		std::array<VkFormat, 5> depthFormats = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,  //32-bit depth, 8-bit stencil
			VK_FORMAT_D32_SFLOAT,          //32-bit depth, no stencil
			VK_FORMAT_D24_UNORM_S8_UINT,   //24-bit depth, 8-bit stencil
			VK_FORMAT_D16_UNORM_S8_UINT,   //16-bit depth, 8-bit stencil
			VK_FORMAT_D16_UNORM            //16-bit depth, no stencil
		};
		m_gpuProps[i].depthFormat = VK_FORMAT_UNDEFINED;
		for (VkFormat format : depthFormats)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(m_gpus[i], format, &formatProps);
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				m_gpuProps[i].depthFormat = format;
				m_gpuProps[i].depthTiling = VK_IMAGE_TILING_OPTIMAL;
				break;
			}
		}
		if (VK_FORMAT_UNDEFINED == m_gpuProps[i].depthFormat)
		{
			Basilisk::warnings.push("Vulkan::Instance::::FindGpus() could not find a depth format for GPU at index " + std::to_string(i));
		}
	}

	return count;
}

const GpuProperties *Instance::GetGpuProperties(uint32_t gpuIndex)
{
	if (m_gpuProps.size() < 0 || gpuIndex > m_gpuProps.size() - 1)
	{
		Basilisk::warnings.push("Vulkan::Instance::GetGpuProperties() called on a nonexisted GPU (at index " + std::to_string(gpuIndex) + ")");
		return nullptr;
	}
	else
		return &m_gpuProps[gpuIndex];
}

std::shared_ptr<Device> Instance::CreateDevice(uint32_t gpuIndex)
{
	if (m_gpus.size() == 0 || gpuIndex > m_gpus.size() - 1)
	{
		Basilisk::errors.push("Vulkan::Instance::::CreateDevice()::gpuIndex is out of GPU array bounds");
		return nullptr;
	}

	if (VK_VERSION_MAJOR(apiVersion()) != VK_VERSION_MAJOR(m_gpuProps[gpuIndex].props.apiVersion))
	{
		std::stringstream message("Vulkan may not operate properly without compatible API support. Application requires API version ");
		message << VK_VERSION_MAJOR(apiVersion()) << "." << VK_VERSION_MINOR(apiVersion()) << "." << VK_VERSION_PATCH(apiVersion());
		message << " but the selected GPU is using version ";
		message << VK_VERSION_MAJOR(m_gpuProps[gpuIndex].props.apiVersion) << "." << VK_VERSION_MINOR(m_gpuProps[gpuIndex].props.apiVersion) << "." << VK_VERSION_PATCH(m_gpuProps[gpuIndex].props.apiVersion);

		Basilisk::warnings.push(message.str());
	}

	//Meets all prerequisites

	std::shared_ptr<Device> out(new Device,
		[](Device *ptr) {
			ptr->Release();
			delete ptr;
			ptr = nullptr;
		}
	);
	out->m_parent = this;
	out->m_gpuProps = m_gpuProps[gpuIndex];
	VkResult res;

	//
	////Create the device
	//

	float queue_priorities[1] = { 1.0 };
	VkDeviceQueueCreateInfo queue_info[2] =
	{
		{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,           //Reserved
			0,                 //No flags
			m_presentTarget->renderQueueIndex,  //This is the rendering queue
			1,                 //Queue count
			queue_priorities   //Queue priorities
		},
		{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,            //Reserved
			0,                  //No flags
			m_presentTarget->presentQueueIndex,  //This is the presentation queue
			1,                  //Queue count
			queue_priorities    //Queue priorities
		}
	};

	VkDeviceCreateInfo device_info = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,           //Reserved
		0,                 //Flags
		2,                 //Queue count
		queue_info,        //Queue properties
		layerCount(),      //Layer count
		layerNames(),      //Layer types
		extensionCount(),  //Extension count
		extensionNames(),  //Extension names
		nullptr            //Not enabling any device features
	};

	if (Failed(vkCreateDevice(m_gpus[gpuIndex], &device_info, nullptr, &out->m_device)))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() failed to create the device");
		return nullptr;
	}
	//Store Vulkan extension function pointers
	//VK_KHR_swapchain function pointers
	out->pfnCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(vkGetDeviceProcAddr(out->m_device, "vkCreateSwapchainKHR"));
	out->pfnDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(vkGetDeviceProcAddr(out->m_device, "vkDestroySwapchainKHR"));
	out->pfnGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(vkGetDeviceProcAddr(out->m_device, "vkGetSwapchainImagesKHR"));
	out->pfnAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(out->m_device, "vkAcquireNextImageKHR"));
	out->pfnQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(vkGetDeviceProcAddr(out->m_device, "vkQueuePresentKHR"));
	//VK_KHR_display function pointers
	out->pfnGetPhysicalDeviceDisplayPropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPropertiesKHR>(vkGetDeviceProcAddr(out->m_device, "vkGetPhysicalDeviceDisplayPropertiesKHR"));
	out->pfnGetPhysicalDeviceDisplayPlanePropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR>(vkGetDeviceProcAddr(out->m_device, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR"));
	out->pfnGetDisplayPlaneSupportedDisplaysKHR = reinterpret_cast<PFN_vkGetDisplayPlaneSupportedDisplaysKHR>(vkGetDeviceProcAddr(out->m_device, "vkGetDisplayPlaneSupportedDisplaysKHR"));
	out->pfnGetDisplayModePropertiesKHR = reinterpret_cast<PFN_vkGetDisplayModePropertiesKHR>(vkGetDeviceProcAddr(out->m_device, "vkGetDisplayModePropertiesKHR"));
	out->pfnCreateDisplayModeKHR = reinterpret_cast<PFN_vkCreateDisplayModeKHR>(vkGetDeviceProcAddr(out->m_device, "vkCreateDisplayModeKHR"));
	out->pfnGetDisplayPlaneCapabilitiesKHR = reinterpret_cast<PFN_vkGetDisplayPlaneCapabilitiesKHR>(vkGetDeviceProcAddr(out->m_device, "vkGetDisplayPlaneCapabilitiesKHR"));
	out->pfnCreateDisplayPlaneSurfaceKHR = reinterpret_cast<PFN_vkCreateDisplayPlaneSurfaceKHR>(vkGetDeviceProcAddr(out->m_device, "vkCreateDisplayPlaneSurfaceKHR"));
	//VK_KHR_display_swapchain function pointers
	out->pfnCreateSharedSwapchainsKHR = reinterpret_cast<PFN_vkCreateSharedSwapchainsKHR>(vkGetDeviceProcAddr(out->m_device, "vkCreateSharedSwapchainsKHR"));

	//
	////Create the device's command pool(s)
	//

	//Rendering queue
	VkCommandPoolCreateInfo render_pool_info = Init<VkCommandPoolCreateInfo>::Create(m_presentTarget->renderQueueIndex);

	res = vkCreateCommandPool(out->m_device, &render_pool_info, nullptr, &out->m_commandPools[Device::renderQueue]);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() failed to create the command pool");
		return nullptr;
	}

	//Presentation queue
	VkCommandPoolCreateInfo present_pool_info = Init<VkCommandPoolCreateInfo>::Create(m_presentTarget->presentQueueIndex);

	res = vkCreateCommandPool(out->m_device, &present_pool_info, nullptr, &out->m_commandPools[Device::presentQueue]);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() failed to create the command pool");
		return nullptr;
	}

	//
	////Store the queues we created above in the device
	//

	vkGetDeviceQueue(out->m_device, m_presentTarget->renderQueueIndex, 0, &out->m_queues[Device::renderQueue]);
	vkGetDeviceQueue(out->m_device, m_presentTarget->presentQueueIndex, 0, &out->m_queues[Device::presentQueue]);


	return out;
}

bool Instance::HookWin32Window(uint32_t gpuIndex, HWND hWnd, HINSTANCE hInstance)
{
	if (m_gpus.size() == 0 || gpuIndex > m_gpus.size() - 1)
	{
		Basilisk::errors.push("Vulkan::Instance::HookWin32Window()::gpuIndex is out of range");
		return false;
	}
	if (WAIT_TIMEOUT != WaitForSingleObject(hInstance, 0)) //Uses a bit of a hack to figure out if the process is running
	{ //With a time-out of zero, anything other than WAIT_TIMEOUT means the process doesn't exist
		Basilisk::errors.push("Vulkan::Instance::HookWin32Window()::hInstance is not a valid instance");
		return false;
	}
	if (!IsWindow(hWnd))
	{
		Basilisk::errors.push("Vulkan::Instance::HookWin32Window():hWnd is not a valid window");
		return false;
	}
	if (!m_presentTarget)
		m_presentTarget = new PresentableSurface;


	//
	////Grab a VkSurface object from the provided window
	//

	VkResult res;
	VkWin32SurfaceCreateInfoKHR surface_info = {
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		nullptr,    //Reserved
		0,          //No flags
		hInstance,  //Handle to the process
		hWnd        //Handle to the window
	};

	res = pfnCreateWin32SurfaceKHR(m_instance, &surface_info, nullptr, &m_presentTarget->surface);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::HookWin32Window() could not connect to the provided window");
		return nullptr;
	}

	//
	////Find a graphics queue which supports present
	//

	std::vector<VkBool32>supportsPresent(m_gpuProps[gpuIndex].queueDescs.size());
	for (uint32_t i = 0; i < m_gpuProps[gpuIndex].queueDescs.size(); ++i)
	{
		res = pfnGetPhysicalDeviceSurfaceSupportKHR(m_gpus[gpuIndex], i, m_presentTarget->surface, &supportsPresent[i]);
		if (Failed(res))
		{ //Error getting the GPU's surface support
			Basilisk::errors.push("Vulkan::Instance::HookWin32Window() could not query the GPU's surface support");
			return nullptr;
		}
	}

	m_presentTarget->presentQueueIndex = std::numeric_limits<uint32_t>::max();
	for (uint32_t i = 0; i < m_gpuProps[gpuIndex].queueDescs.size(); ++i)
	{
		if ((m_gpuProps[gpuIndex].queueDescs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supportsPresent[i])
		{ //Found a queue that fits our criteria
			m_presentTarget->presentQueueIndex = i;
			break;
		}
	}
	if (std::numeric_limits<uint32_t>::max() == m_presentTarget->presentQueueIndex)
	{
		Basilisk::errors.push("Vulkan::Instance::HookWin32Window() could not find a present-capable graphics queue");
		return nullptr;
	}

	m_presentTarget->renderQueueIndex = std::numeric_limits<uint32_t>::max();
	for (uint32_t i = 0; i < m_gpuProps[gpuIndex].queueDescs.size(); ++i)
	{
		if ((m_gpuProps[gpuIndex].queueDescs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && i != m_presentTarget->presentQueueIndex)
		{ //Found a queue that fits our criteria
			m_presentTarget->renderQueueIndex = i;
			break;
		}
	}
	if (std::numeric_limits<uint32_t>::max() == m_presentTarget->renderQueueIndex)
	{
		Basilisk::errors.push("Vulkan::Instance::HookWin32Window() could not find a second graphics queue");
		return nullptr;
	}

	//
	////Get the list of VkFormats supported by that surface, and store the one it likes most
	//

	uint32_t formatCount;
	res = pfnGetPhysicalDeviceSurfaceFormatsKHR(m_gpus[gpuIndex], m_presentTarget->surface, &formatCount, nullptr);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::HookWin32Window() could not count window surface formats");
		return nullptr;
	}

	std::vector<VkSurfaceFormatKHR> surfFormats(formatCount);
	res = pfnGetPhysicalDeviceSurfaceFormatsKHR(m_gpus[gpuIndex], m_presentTarget->surface, &formatCount, surfFormats.data());
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::HookWin32Window() could not retrieve window surface formats");
		return nullptr;
	}

	if (formatCount == 0)
	{ //Surface isn't playing nice
		Basilisk::errors.push("Vulkan::Instance::HookWin32Window() could not find a preferred format for the window surface");
		return nullptr;
	}
	else if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
	{ //Surface has no preferred format
		m_presentTarget->colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{ //Surface has a preferred format
		m_presentTarget->colorFormat = surfFormats[0].format;
	}

	//
	////Grab the GPU's surface capabilities and present modes
	//

	res = pfnGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpus[gpuIndex], m_presentTarget->surface, &m_presentTarget->caps);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::HookWin32Window() could not retrieve the GPU's surface capabilities");
		return nullptr;
	}

	uint32_t presentModeCount;
	res = pfnGetPhysicalDeviceSurfacePresentModesKHR(m_gpus[gpuIndex], m_presentTarget->surface, &presentModeCount, nullptr);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::HookWin32Window() could not count the GPU's present modes");
		return nullptr;
	}
	if (presentModeCount == 0)
	{
		Basilisk::errors.push("Vulkan::Instance::HookWin32Window() could not detect any present modes for the provided GPU");
		return nullptr;
	}

	m_presentTarget->presentModes.resize(presentModeCount);

	res = pfnGetPhysicalDeviceSurfacePresentModesKHR(m_gpus[gpuIndex], m_presentTarget->surface, &presentModeCount, m_presentTarget->presentModes.data());
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::HookWin32Window() could not list the GPU's present modes");
		return nullptr;
	}
}

#pragma endregion

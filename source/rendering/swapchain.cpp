/**
\file   swapchain.cpp
\author Andrew Baxter
\date   March 18, 2016

Defines the behavior of a Vulkan::SwapChain object, from creation to destruction

*/

#include "rendering/backend.h"
using namespace Vulkan;

SwapChain::SwapChain() : m_swapChain(VK_NULL_HANDLE), pfnAcquireNextImage(nullptr)
{
	m_desc = {};
}

void SwapChain::NextBuffer()
{
	pfnAcquireNextImage(&m_currentImage);
}

void SwapChain::Release(VkDevice device, PFN_vkDestroySwapchainKHR func)
{
	//The images and image views are managed under the hood by Vulkan
	if (m_swapChain)
	{
		func(device, m_swapChain, nullptr);
		m_swapChain = VK_NULL_HANDLE;
	}
}

VkSwapchainCreateInfoKHR Vulkan::SwapChainCreateInfo(glm::tvec2<uint32_t> res, uint32_t numBuffers)
{
	return {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,               //Reserved
		0,                     //No flags
		VK_NULL_HANDLE,        //Target surface
		numBuffers,            //Number of back buffers
		VK_FORMAT_UNDEFINED,   //Surface format
		VK_COLORSPACE_SRGB_NONLINEAR_KHR,           //Color space
		{res.x, res.y},        //Resolution
		1,                     //Image layers
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, //Image usage
		VK_SHARING_MODE_EXCLUSIVE,                  //Image sharing mode
		0,                     //Queue family index count
		nullptr,               //Queue family indices
		VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,      //Pre-present transformation flags
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,          //Blending. Opaque because back buffers are never blended
		VK_PRESENT_MODE_FIFO_KHR,  //Present mode
		true,                  //Clipped
		VK_NULL_HANDLE         //Old swap chain
	};
}

std::shared_ptr<SwapChain> Device::CreateSwapChain(VkSwapchainCreateInfoKHR &swapchain_info)
{
	//Make sure the requested swap chain has a reasonable resolution
	if (static_cast<uint32_t>(-1) == m_targetSurface.caps.currentExtent.width)
	{
		//Surface size is undefined; snap to requested resolution
		swapchain_info.imageExtent.width = glm::clamp(swapchain_info.imageExtent.width,
			m_targetSurface.caps.minImageExtent.width,
			m_targetSurface.caps.maxImageExtent.width);

		swapchain_info.imageExtent.height = glm::clamp(swapchain_info.imageExtent.height,
			m_targetSurface.caps.minImageExtent.height,
			m_targetSurface.caps.maxImageExtent.height);
	}
	else //Surface size is already defined and cannot change
		swapchain_info.imageExtent = m_targetSurface.caps.currentExtent;


	//Determine the present mode
	swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR; //Default to the universally-supported FIFO present mode
	for (VkPresentModeKHR i : m_targetSurface.presentModes)
	{
		if (VK_PRESENT_MODE_MAILBOX_KHR == i) //This is the lowest-latency non-tearing present mode
		{
			swapchain_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			swapchain_info.minImageCount = std::max(swapchain_info.minImageCount, m_targetSurface.caps.minImageCount + 1); //Clamp minImageCount to at least the surface minimum + 1, for guaranteed non-blocking calls to `vkAcquireNextImageKHR()` in mailbox mode
			break; //We can't get any better than this; call it a day
		}
		else if (VK_PRESENT_MODE_IMMEDIATE_KHR == i) //The fastest present mode, but it tears -- prioritize mailbox mode when available
			swapchain_info.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	//Make sure the requested swap chain has a reasonable backbuffer count
	swapchain_info.minImageCount = glm::clamp(swapchain_info.minImageCount, m_targetSurface.caps.minImageCount, m_targetSurface.caps.maxImageCount);

	//Provide an identity pre-transform matrix if available
	if (m_targetSurface.caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	else //If the identity transform is unavailable, let it do its own thing
		swapchain_info.preTransform = m_targetSurface.caps.currentTransform;

	//Fill in surface details
	swapchain_info.surface = m_targetSurface.surface;
	swapchain_info.imageFormat = m_targetSurface.colorFormat;

	//Create the swap chain
	std::shared_ptr<SwapChain> out(new SwapChain,
		[=](SwapChain *ptr) {
			ptr->Release(m_device, pfnDestroySwapchainKHR);
			delete ptr;
			ptr = nullptr;
		}
	);

	out->m_width = swapchain_info.imageExtent.width;
	out->m_height = swapchain_info.imageExtent.height;
	out->m_desc.attachment = {
		0,  //No flags
		m_targetSurface.colorFormat,      //Format
		VK_SAMPLE_COUNT_1_BIT,            //Sample count
		VK_ATTACHMENT_LOAD_OP_CLEAR,      //Load op
		VK_ATTACHMENT_STORE_OP_STORE,     //Store op
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,  //Stencil load op
		VK_ATTACHMENT_STORE_OP_DONT_CARE, //Stencil store op
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, //Initial layout
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  //Final layout
	};
	out->m_desc.image = ImageCreateInfo(VK_IMAGE_TYPE_2D, m_targetSurface.colorFormat, { out->m_width, out->m_height, 1 });

	VkResult res = pfnCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &out->m_swapChain);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateSwapChain() could not create the swap chain");
		return nullptr;
	}

	//Store the back buffers

	res = pfnGetSwapchainImagesKHR(m_device, out->m_swapChain, &swapchain_info.minImageCount, nullptr);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateSwapChain() could not count the swap chain's back buffers");
		return nullptr;
	}

	out->m_images.resize(swapchain_info.minImageCount);

	res = pfnGetSwapchainImagesKHR(m_device, out->m_swapChain, &swapchain_info.minImageCount, out->m_images.data());
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateSwapChain() could not count the swap chain's back buffers");
		return nullptr;
	}

	out->pfnAcquireNextImage = std::bind(pfnAcquireNextImageKHR, m_device, out->m_swapChain, UINT64_MAX, m_presentComplete, static_cast<VkFence>(VK_NULL_HANDLE), std::placeholders::_1);
	
	
	return out;
}
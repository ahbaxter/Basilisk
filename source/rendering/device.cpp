/**
\file   device.cpp
\author Andrew Baxter
\date   March 18, 2016



*/

#include <sstream>
#include "rendering/backend.h"
using namespace Vulkan;

extern uint32_t layerCount();
extern const char **layerNames();
extern uint32_t devExtensionCount();
extern const char **devExtensionNames();

Device::Device() : m_device(VK_NULL_HANDLE),
	m_renderComplete(VK_NULL_HANDLE), m_presentComplete(VK_NULL_HANDLE),
	m_cmdPrePresent(VK_NULL_HANDLE), m_cmdPostPresent(VK_NULL_HANDLE),
	pfnCreateSwapchainKHR(nullptr),
	pfnDestroySwapchainKHR(nullptr),
	pfnGetSwapchainImagesKHR(nullptr),
	pfnAcquireNextImageKHR(nullptr),
	pfnQueuePresentKHR(nullptr)/*,
	pfnGetPhysicalDeviceDisplayPropertiesKHR(nullptr),
	pfnGetPhysicalDeviceDisplayPlanePropertiesKHR(nullptr),
	pfnGetDisplayPlaneSupportedDisplaysKHR(nullptr),
	pfnGetDisplayModePropertiesKHR(nullptr),
	pfnCreateDisplayModeKHR(nullptr),
	pfnGetDisplayPlaneCapabilitiesKHR(nullptr),
	pfnCreateDisplayPlaneSurfaceKHR(nullptr),
	pfnCreateSharedSwapchainsKHR(nullptr)*/
{
	m_targetSurface = {};
	m_gpuProps = {};
	m_queues = {};
	m_commandPools = {};
}

void Device::Release() {
	//Release semaphores
	if (m_presentComplete)
	{
		vkDestroySemaphore(m_device, m_presentComplete, nullptr);
		m_presentComplete = VK_NULL_HANDLE;
	}
	if (m_renderComplete)
	{
		vkDestroySemaphore(m_device, m_renderComplete, nullptr);
		m_renderComplete = VK_NULL_HANDLE;
	}
	//Release command buffers
	if (m_cmdPrePresent)
	{
		vkFreeCommandBuffers(m_device, m_commandPools[graphicsIndex], 1, &m_cmdPrePresent);
		m_cmdPrePresent = VK_NULL_HANDLE;
	}
	if (m_cmdPostPresent)
	{
		vkFreeCommandBuffers(m_device, m_commandPools[graphicsIndex], 1, &m_cmdPostPresent);
		m_cmdPostPresent = VK_NULL_HANDLE;
	}
	//Release command pools
	for (auto &iter : m_commandPools)
	{
		if (iter)
		{
			vkDestroyCommandPool(m_device, iter, nullptr);
			iter = VK_NULL_HANDLE;
		}
	}
	//Queues self-destruct
	if (m_device)
	{
		vkDestroyDevice(m_device, nullptr);
		m_device = VK_NULL_HANDLE;
	}
}

void Device::Join()
{
	vkDeviceWaitIdle(m_device);
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

std::shared_ptr<Device> Instance::CreateDeviceOnWindow(uint32_t gpuIndex, HWND hWnd, HINSTANCE hInstance)
{
	if (m_gpus.size() == 0 || gpuIndex > m_gpus.size() - 1)
	{
		Basilisk::errors.push("Vulkan::Instance::::CreateDevice()::gpuIndex is out of GPU array bounds");
		return nullptr;
	}
	if (VK_VERSION_MAJOR(VK_API_VERSION) != VK_VERSION_MAJOR(m_gpuProps[gpuIndex].props.apiVersion))
	{
		std::stringstream message("Vulkan may not operate properly without compatible API support. Application requires API version ");
		message << VK_VERSION_MAJOR(VK_API_VERSION) << "." << VK_VERSION_MINOR(VK_API_VERSION) << "." << VK_VERSION_PATCH(VK_API_VERSION);
		message << " but the selected GPU is using version ";
		message << VK_VERSION_MAJOR(m_gpuProps[gpuIndex].props.apiVersion) << "." << VK_VERSION_MINOR(m_gpuProps[gpuIndex].props.apiVersion) << "." << VK_VERSION_PATCH(m_gpuProps[gpuIndex].props.apiVersion);

		Basilisk::warnings.push(message.str());
	}
	if (!IsWindow(hWnd))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDeviceOnWindow():hWnd is not a valid window");
		return false;
	}

	//Meets all prerequisites

	std::shared_ptr<Device> out(new Device,
		[](Device *ptr) {
		ptr->Release();
		delete ptr;
		ptr = nullptr;
	}
	);
	out->m_gpuProps = m_gpuProps[gpuIndex];
	VkResult res;


	//Grab a VkSurface object from the provided window

	VkWin32SurfaceCreateInfoKHR surface_info = {
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		nullptr,    //Reserved
		0,          //No flags
		hInstance,  //Handle to the process
		hWnd        //Handle to the window
	};

	res = pfnCreateWin32SurfaceKHR(m_instance, &surface_info, nullptr, &out->m_targetSurface.surface);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDeviceOnWindow() could not connect to the provided window");
		return false;
	}

	//Find a graphics queue which supports present

	std::vector<VkBool32>supportsPresent(m_gpuProps[gpuIndex].queueDescs.size());
	for (uint32_t i = 0; i < m_gpuProps[gpuIndex].queueDescs.size(); ++i)
	{
		res = pfnGetPhysicalDeviceSurfaceSupportKHR(m_gpus[gpuIndex], i, out->m_targetSurface.surface, &supportsPresent[i]);
		if (Failed(res))
		{ //Error getting the GPU's surface support
			Basilisk::errors.push("Vulkan::Instance::CreateDeviceOnWindow() could not query the GPU's surface support");
			return false;
		}
	}

	out->m_targetSurface.queueIndex = std::numeric_limits<uint32_t>::max();
	for (uint32_t i = 0; i < m_gpuProps[gpuIndex].queueDescs.size(); ++i)
	{
		if ((m_gpuProps[gpuIndex].queueDescs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supportsPresent[i])
		{ //Found a queue that fits our criteria
			out->m_targetSurface.queueIndex = i;
			break;
		}
	}
	if (std::numeric_limits<uint32_t>::max() == out->m_targetSurface.queueIndex)
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDeviceOnWindow() could not find a present-capable graphics queue");
		return false;
	}

	//Get the list of VkFormats supported by that surface, and store the one it likes most

	uint32_t formatCount;
	res = pfnGetPhysicalDeviceSurfaceFormatsKHR(m_gpus[gpuIndex], out->m_targetSurface.surface, &formatCount, nullptr);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDeviceOnWindow() could not count window surface formats");
		return false;
	}

	std::vector<VkSurfaceFormatKHR> surfFormats(formatCount);
	res = pfnGetPhysicalDeviceSurfaceFormatsKHR(m_gpus[gpuIndex], out->m_targetSurface.surface, &formatCount, surfFormats.data());
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDeviceOnWindow() could not retrieve window surface formats");
		return false;
	}

	if (formatCount == 0)
	{ //Surface isn't playing nice
		Basilisk::errors.push("Vulkan::Instance::CreateDeviceOnWindow() could not find a preferred format for the window surface");
		return false;
	}
	else if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
	{ //Surface has no preferred format
		out->m_targetSurface.colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{ //Surface has a preferred format
		out->m_targetSurface.colorFormat = surfFormats[0].format;
	}

	//Grab the GPU's surface capabilities and present modes

	res = pfnGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpus[gpuIndex], out->m_targetSurface.surface, &out->m_targetSurface.caps);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDeviceOnWindow() could not retrieve the GPU's surface capabilities");
		return false;
	}

	uint32_t presentModeCount;
	res = pfnGetPhysicalDeviceSurfacePresentModesKHR(m_gpus[gpuIndex], out->m_targetSurface.surface, &presentModeCount, nullptr);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDeviceOnWindow() could not count the GPU's present modes");
		return false;
	}
	if (presentModeCount == 0)
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDeviceOnWindow() could not detect any present modes for the provided GPU");
		return false;
	}

	out->m_targetSurface.presentModes.resize(presentModeCount);

	res = pfnGetPhysicalDeviceSurfacePresentModesKHR(m_gpus[gpuIndex], out->m_targetSurface.surface, &presentModeCount, out->m_targetSurface.presentModes.data());
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDeviceOnWindow() could not list the GPU's present modes");
		return false;
	}

	//Create the device

	float queue_priorities[1] = { 1.0 };
	VkDeviceQueueCreateInfo queue_info[1] =
	{
		{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,           //Reserved
		0,                 //No flags
		out->m_targetSurface.queueIndex,  //This is the rendering queue
		1,                 //Queue count
		queue_priorities   //Queue priorities
		}
	};

	VkDeviceCreateInfo device_info = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,              //Reserved
		0,                    //Flags
		1,                    //Queue count
		queue_info,           //Queue properties
		layerCount(),         //Layer count
		layerNames(),         //Layer types
		devExtensionCount(),  //Extension count
		devExtensionNames(),  //Extension names
		nullptr               //Not enabling any device features
	};

	res = vkCreateDevice(m_gpus[gpuIndex], &device_info, nullptr, &out->m_device);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() failed to create the device");
		return nullptr;
	}
	//Store the queues we created above in the device
	vkGetDeviceQueue(out->m_device, out->m_targetSurface.queueIndex, 0, &out->m_queues[graphicsIndex]);

	//Create the device's graphics command pool
	VkCommandPoolCreateInfo render_pool_info = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,                                          //Reserved
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,  //Command buffers created from this pool can be reset manually
		out->m_targetSurface.queueIndex                   //All command buffers from this pool must be submitted to this queue
	};

	res = vkCreateCommandPool(out->m_device, &render_pool_info, nullptr, &out->m_commandPools[graphicsIndex]);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() failed to create the graphics command pool");
		return nullptr;
	}

	//Create built-in semaphores for rendering and presentation
	VkSemaphoreCreateInfo semaphore_info = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,  //Reserved
		0         //No flags: reserved
	};

	res = vkCreateSemaphore(out->m_device, &semaphore_info, nullptr, &out->m_renderComplete);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not create the render semaphore");
		return nullptr;
	}
	res = vkCreateSemaphore(out->m_device, &semaphore_info, nullptr, &out->m_presentComplete);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not create the presentation semaphore");
		return nullptr;
	}

	//Create pre-present and post-present command buffers
	VkCommandBufferAllocateInfo cmd_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		out->m_commandPools[graphicsIndex],
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1
	};
	res = vkAllocateCommandBuffers(out->m_device, &cmd_info, &out->m_cmdPrePresent);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not create the pre-present command buffer");
		return nullptr;
	}
	res = vkAllocateCommandBuffers(out->m_device, &cmd_info, &out->m_cmdPostPresent);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not create the post-present command buffer");
		return nullptr;
	}
	//Store submit info for graphics commands
	out->m_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	out->m_submitInfo.pNext = nullptr;
	out->m_submitInfo.waitSemaphoreCount = 1;
	out->m_submitInfo.pWaitSemaphores = &out->m_presentComplete;
	out->m_submitInfo.pWaitDstStageMask = nullptr;
	//Command buffer count and command buffers will be updated each frame
	out->m_submitInfo.signalSemaphoreCount = 1;
	out->m_submitInfo.pSignalSemaphores = &out->m_renderComplete;

	//Normally I stray away from macros, but here it actually makes sure I don't mistype the extension string names
#define GET_PROCADDR(name) \
	out->pfn##name = reinterpret_cast<PFN_vk##name>(vkGetDeviceProcAddr(out->m_device, "vk"#name)); \
	if (!out->pfn##name) { Basilisk::errors.push("Vulkan::Instance::CreateDevice() could not find the proc address for vk"#name); return nullptr; }

	//Store VK_KHR_swapchain function pointers
	GET_PROCADDR(CreateSwapchainKHR);
	GET_PROCADDR(DestroySwapchainKHR);
	GET_PROCADDR(GetSwapchainImagesKHR);
	GET_PROCADDR(AcquireNextImageKHR);
	GET_PROCADDR(QueuePresentKHR);

	/*Get VK_KHR_display function pointers
	GET_PROCADDR(GetPhysicalDeviceDisplayPropertiesKHR);
	GET_PROCADDR(GetPhysicalDeviceDisplayPlanePropertiesKHR);
	GET_PROCADDR(GetDisplayPlaneSupportedDisplaysKHR);
	GET_PROCADDR(GetDisplayModePropertiesKHR);
	GET_PROCADDR(CreateDisplayModeKHR);
	GET_PROCADDR(GetDisplayPlaneCapabilitiesKHR);
	GET_PROCADDR(CreateDisplayPlaneSurfaceKHR);
	//Get VK_KHR_display_swapchain function pointers
	GET_PROCADDR(CreateSharedSwapchainsKHR);*/

#undef GET_PROCADDR


	return out;
}

/*
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
} */

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

	uint32_t spirvSize = static_cast<uint32_t>(3 * sizeof(uint32_t) + source.size() + 1);
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

std::shared_ptr<GraphicsPipeline> Device::CreateGraphicsPipeline(const std::shared_ptr<FrameBuffer> &frameBuffer, const std::shared_ptr<PipelineLayout> &layout, const std::vector<ShaderStage> &shaders, uint32_t patchCtrlPoints)
{
	VkGraphicsPipelineCreateInfo pipeline_info;


	std::vector<VkPipelineShaderStageCreateInfo> stage_info(shaders.size());
	for (uint32_t i = 0; i < shaders.size(); ++i)
	{
		stage_info[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_info[i].pNext = nullptr;
		stage_info[i].flags = 0;
		stage_info[i].stage = shaders[i].stage;
		stage_info[i].module = shaders[i].shader->m_module;
		stage_info[i].pName = shaders[i].entryPoint.c_str();
		stage_info[i].pSpecializationInfo = nullptr;
	}
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext = nullptr;
	pipeline_info.flags = 0;
	pipeline_info.stageCount = static_cast<uint32_t>(shaders.size());
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
		static_cast<uint32_t>(dynamic_enables.size()),  //Dynamic state count
		dynamic_enables.data()   //Dynamic state enables
	};
	pipeline_info.pDynamicState = &dynamic;

	pipeline_info.layout = layout->m_layout;
	pipeline_info.renderPass = frameBuffer->m_renderPass;
	pipeline_info.subpass = 0;
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

bool Device::ExecuteCommands(const std::vector<std::shared_ptr<CommandBuffer>> &commands)
{
	VkResult res;
	if (commands.size() > 0)
	{
		std::vector<VkCommandBuffer> vk_commands(commands.size());
		for (uint32_t i = 0; i < commands.size(); ++i)
			vk_commands[i] = commands[i]->m_commandBuffer;

		m_submitInfo.commandBufferCount = static_cast<uint32_t>(vk_commands.size());
		m_submitInfo.pCommandBuffers = vk_commands.data();

		res = vkQueueSubmit(m_queues[graphicsIndex], 1, &m_submitInfo, VK_NULL_HANDLE);
		if (Failed(res))
		{
			Basilisk::errors.push("Vulkan::Device::ExecuteCommands() could not submit the commands to the graphics queue");
			return false;
		}
		else return true;
	}
	else
	{
		Basilisk::errors.push("Vulkan::Device::ExecuteCommands()::commands must not be empty");
		return false;
	}
}

bool Device::PrePresent(const std::shared_ptr<SwapChain> &swapChain)
{
	VkCommandBufferBeginInfo begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr, 0, nullptr
	};

	VkImageMemoryBarrier barrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,  //Reserved
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,      //Source access mask
		0,        //Destination access mask
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  //Old layout
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,           //New layout
		(~0ui32), (~0ui32),  //Source, destination queue family index
		swapChain->m_images[*swapChain->GetBufferIndex()],     //Image
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }  //Subresource range
	};

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = nullptr;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &m_cmdPrePresent;

	VkResult res = vkBeginCommandBuffer(m_cmdPrePresent, &begin_info);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::PrePresent() could not begin the command buffer");
		return false;
	}

	vkCmdPipelineBarrier(m_cmdPrePresent,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,           //No flags
		0, nullptr,  //No memory barriers
		0, nullptr,  //No buffer barriers
		1, &barrier  //One image barrier
		);

	res = vkEndCommandBuffer(m_cmdPrePresent);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::PrePresent() could not end the command buffer");
		return false;
	}

	res = vkQueueSubmit(m_queues[graphicsIndex], 1, &submit_info, VK_NULL_HANDLE);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::PrePresent() could not submit the command buffer");
		return false;
	}

	return true;
}

bool Device::Present(const std::shared_ptr<SwapChain> &swapChain)
{
	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pNext = nullptr;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swapChain->m_swapChain;
	present_info.pImageIndices = swapChain->GetBufferIndex();

	if (Failed(pfnQueuePresentKHR(m_queues[graphicsIndex], &present_info)))
	{
		Basilisk::errors.push("Failed to present swap chain");
		return false;
	}
	else return true;
}

bool Device::PostPresent(const std::shared_ptr<SwapChain> &swapChain)
{
	VkCommandBufferBeginInfo begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr, 0, nullptr
	};

	VkImageMemoryBarrier barrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,  //Reserved
		0,        //Source access mask
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,      //Destination access mask
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,           //Old layout
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  //New layout
		(~0ui32), (~0ui32),  //Source, destination queue family index
		swapChain->m_images[*swapChain->GetBufferIndex()],     //Image
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }  //Subresource range
	};

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = nullptr;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &m_cmdPostPresent;

	VkResult res = vkBeginCommandBuffer(m_cmdPostPresent, &begin_info);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::PostPresent() could not begin the command buffer");
		return false;
	}

	vkCmdPipelineBarrier(m_cmdPostPresent,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,           //No flags
		0, nullptr,  //No memory barriers
		0, nullptr,  //No buffer barriers
		1, &barrier  //One image barrier
		);

	res = vkEndCommandBuffer(m_cmdPostPresent);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::PostPresent() could not end the command buffer");
		return false;
	}

	res = vkQueueSubmit(m_queues[graphicsIndex], 1, &submit_info, VK_NULL_HANDLE);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::PrePresent() could not submit the command buffer");
		return false;
	}

	return true;
}
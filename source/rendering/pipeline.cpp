/**
\file   pipeline.cpp
\author Andrew Baxter
\date   March 19, 2016

Defines the behavior of Vulkan::Shader and Vulkan::GraphicsPipeline objects, from creation to destruction

\todo Implement compute pipelines

*/

#include <bitset>
#include "rendering/backend.h"
using namespace Vulkan;

Shader::Shader() : m_module(VK_NULL_HANDLE) {
}

void Shader::Release(VkDevice device)
{
	if (m_module)
	{
		vkDestroyShaderModule(device, m_module, nullptr);
		m_module = VK_NULL_HANDLE;
	}
}

std::shared_ptr<Shader> Device::CreateShaderFromSPIRV(const std::vector<uint32_t> &bytecode)
{
	std::shared_ptr<Shader> out(new Shader, 
		[=](Shader *&ptr) {
			ptr->Release(m_device);
			delete ptr;
			ptr = nullptr;
		}
	);

	VkShaderModuleCreateInfo create_info = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr, //Next: reserved
		0, //Flags: reserved
		static_cast<uint32_t>(bytecode.size()),
		bytecode.data()
	};

	VkResult res = vkCreateShaderModule(m_device, &create_info, nullptr, &out->m_module);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateShaderFromSPIRV() failed");
		return nullptr;
	}

	return out;
}

std::shared_ptr<Shader> Device::CreateShaderFromGLSL(const std::string &source, VkShaderStageFlagBits stage)
{
	//Make sure `stage` only has one bit set
	std::bitset<sizeof(VkShaderStageFlagBits)*8> bits(stage);
	if (bits.count() != 1)
	{
		Basilisk::errors.push("Vulkan::Device::CreateShaderFromGLSL()::stage must have a single bit set");
		return nullptr;
	}

	std::shared_ptr<Shader> out(new Shader,
		[=](Shader *&ptr) {
			ptr->Release(m_device);
			delete ptr;
			ptr = nullptr;
		}
	);

	size_t len = source.size() + 1; //String length in characters. +1 is for the null terminator \0
	len += (len % sizeof(uint32_t)); //Round up to the nearest sizeof(uint32_t)
	len *= sizeof(std::string::allocator_type); //String length in bytes. Accounts for the possibility of wide-character strings -- not sure if the shader compiler will like that, though
	
	std::vector<uint32_t> bytecode(3 + len);
	bytecode[0] = 0x07230203;
	bytecode[1] = 0;
	bytecode[2] = stage;
	std::copy(source.begin(), source.end(), bytecode.begin() + 3);

	VkShaderModuleCreateInfo create_info = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr, //Next: reserved
		0, //Flags: reserved
		static_cast<uint32_t>(len),
		bytecode.data()
	};

	VkResult res = vkCreateShaderModule(m_device, &create_info, nullptr, &out->m_module);
	if (Failed(res))
	{
		Basilisk::errors.push("Vulkan::Device::CreateShaderFromGLSL() failed");
		return nullptr;
	}

	return out;
}

PipelineLayout::PipelineLayout() : m_layout(VK_NULL_HANDLE), m_setLayout(VK_NULL_HANDLE) { }

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


GraphicsPipeline::GraphicsPipeline() : m_pipeline(VK_NULL_HANDLE) { }

void GraphicsPipeline::Release(VkDevice device)
{
	if (m_pipeline)
	{
		vkDestroyPipeline(device, m_pipeline, nullptr);
		m_pipeline = VK_NULL_HANDLE;
	}
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
		[=](GraphicsPipeline *&ptr) {
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
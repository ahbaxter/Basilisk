/**
\file   pipeline.cpp
\author Andrew Baxter
\date   March 19, 2016

Defines the behavior of Vulkan::Shader and Vulkan::GraphicsPipeline objects, from creation to destruction

\todo Implement compute pipelines

*/

#include "rendering/backend.h"
using namespace Vulkan;

Shader::Shader() : m_module(VK_NULL_HANDLE) { }

void Shader::Release(VkDevice device)
{
	if (m_module)
	{
		vkDestroyShaderModule(device, m_module, nullptr);
		m_module = VK_NULL_HANDLE;
	}
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
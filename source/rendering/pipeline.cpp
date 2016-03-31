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
		[=](Shader *ptr) {
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
		[=](Shader *ptr) {
			ptr->Release(m_device);
			delete ptr;
			ptr = nullptr;
		}
	);

	size_t len = source.size() + 1; //String length in characters. +1 is for the null terminator \0
	len += (len % sizeof(uint32_t)); //Round up to the nearest sizeof(uint32_t)
	len *= sizeof(std::string::allocator_type); //String length in bytes. Accounts for the possibility of wide-character strings -- not sure if the shader compiler will like that, though
	len /= sizeof(uint32_t); //String length in uint32_t's
	
	std::vector<uint32_t> bytecode(3 + len);
	bytecode[0] = 0x07230203;
	bytecode[1] = 0;
	bytecode[2] = stage;
	std::copy(source.begin(), source.end(), bytecode.begin() + 3);

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
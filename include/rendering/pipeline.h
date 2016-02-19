/**
\file   pipeline.h
\author Andrew Baxter
\date   February 17, 2015

Controls shader pipelines and (eventually) compute operations

\todo Implement compute pipelines

*/

#ifndef BASILISK_PIPELINE_H
#define BASILISK_PIPELINE_H

#include <vector>
#include "common.h"

namespace Basilisk
{
	enum class InputFormat : uint8_t
	{
		Unknown = 0,

		Typeless32,
		Float32,
		UInt32,
		SInt32,
		UNorm32,
		SNorm32,

		Typeless16,
		Float16,
		UInt16,
		SInt16,
		UNorm16,
		SNorm16,

		Typeless8,
		Float8,
		UInt8,
		SInt8,
		UNorm8,
		SNorm8,
	};

	/**
	A direct mirror of Vulkan's `VkShaderStageFlagBits` structure
	Will be translated at runtime for use in a D3D12 pipeline
	*/
	enum class ShaderStage : uint32_t
	{
		Vertex = 0x00000001,
		TesselationControl = 0x00000002,
		TesselationEvaluation = 0x00000004,
		Geometry = 0x00000008,
		Fragment = 0x00000010,
		Compute = 0x00000020,
		AllGraphics = 0x1F,
		All = 0x7FFFFFFF
	};
	

	/**
	Uses CRTP abstraction to represent an ambiguous device

	\tparam Impl Sets up the Curiously Recurring Template Pattern
	*/
	template<class Impl>
	class GraphicsPipeline abstract
	{
	public:
		/**
		Gets the derivative class of this object
		\return The derivative class of this object
		*/
		inline const Impl &GetImplementation() {
			return static_cast<Impl&>(*this);
		}

		inline Result AddShaderFromSource(const std::string &text, ShaderStage stage) {
			return GetImplementation().AddShaderFromSource(text, stage);
		}
		inline Result AddShaderFromFile(const std::string &filename, ShaderStage stage) {
			return GetImplementation().AddShaderFromFile(filename, stage);
		}
		inline Result MapAttribute(const std::string &name, InputFormat format, uint8_t count = 1) {//Return type is incorrect for sure
			return GetImplementation().MapAttribute(name, format, count);
		}
		inline Result MapUniform(const std::string &name, InputFormat format, uint8_t count = 1) {//Return type is incorrect for sure
			return GetImplementation().MapUniform(name, format, count);
		}
		/**
		\brief Compiles the shaders into a complete pipeline
		`AddShader`, `MapAttribute`, and `MapUniform` will fail after `Compile` has been called

		\return Details about potential failure
		*/
		inline Result Compile() {
			return GetImplementation().Compile();
		}
		/**
		\brief Sets this pipeline as active in the given command buffer
		\param[in] commandBuffer The command buffer to bind to

		\return Details about potential failure
		*/
		template<class CmdBufferType>
		inline Result SetAsActive(const CmdBufferType &commandBuffer) {
			return GetImplementation().SetAsActive(commandBuffer)
		}

		/**
		Cleans up after itself
		*/
		inline void Release() {
			GetImplementation().Release();
		}

		//Allow Devices, and only Devices, to create Pipeline objects
		//Makes sure that CRTP and factory design is not sidestepped
		template<class DeviceImpl> friend class Device;
	private:
		Pipeline() = 0;
		~Pipeline() = 0;
	};


	class D3D12GraphicsPipeline : public GraphicsPipeline<D3D12GraphicsPipeline>
	{
	private:
		D3D12GraphicsPipeline();
		~D3D12GraphicsPipeline() = default;

		std::vector<ID3DBlob*> m_shaders;
		ID3DBlob *signature, *error;
		ID3D12PipelineState *m_pso;
	};


	class VulkanGraphicsPipeline : public GraphicsPipeline<VulkanGraphicsPipeline>
	{
	private:
		VulkanGraphicsPipeline();
		~VulkanGraphicsPipeline() = default;
	};


	/*
	template<class Impl>
	class ComputePipeline abstract
	{
	public:

		template<DeviceImpl> friend class
	protected:
		ComputePipeline() = 0;
		~ComputePipeline() = 0;
	};
	*/
}

#endif
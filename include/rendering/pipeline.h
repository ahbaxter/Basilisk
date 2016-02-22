/**
\file   pipeline.h
\author Andrew Baxter
\date   February 19, 2015

Controls shader pipelines and (eventually) compute operations

\todo Implement compute pipelines

*/

#ifndef BASILISK_PIPELINE_H
#define BASILISK_PIPELINE_H

#include <vector>
#include "common.h"

namespace Basilisk
{
	
	/**
	\brief The size and type of a shader's input component
	Does not track the number of components
	*/
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
	Represents an API-ambiguous graphics pipeline

	\tparam Impl Sets up the Curiously Recurring Template Pattern

	\todo Make an AddSampler() function, or equivalent
	\todo Rework the MapAttribute/MapUniform system
	*/
	template<class Impl>
	class GraphicsPipeline abstract
	{
	public:
		/**
		Gets this class's RCTP implementation
		\return This class's RCTP implementation
		*/
		inline const Impl &GetImplementation() {
			return static_cast<Impl&>(*this);
		}

		/**
		Loads in a shader from source code
		\param[in] text The GLSL/HLSL source code
		\param[in] stage The pipeline stage to add the shader to
		*/
		inline Result AddShaderFromSource(const std::string &text, ShaderStage stage) {
			return GetImplementation().AddShaderFromSource(text, stage);
		}
		/**
		Loads in a shader from a local file
		\param[in] filename The location of the GLSL/HLSL source code
		\param[in] stage The pipeline stage to add the shader to
		*/
		inline Result AddShaderFromFile(const std::string &filename, ShaderStage stage) {
			return GetImplementation().AddShaderFromFile(filename, stage);
		}
		/**
		Lets the engine know about a per-vertex attribute
		\param[in] name The name of the in-shader variable name
		\param[in] format The data format of the variable
		\param[in] count The number of components, if the variable is a vector type

		\todo Return offset?
		*/
		inline Result MapAttribute(const std::string &name, InputFormat format, uint8_t count = 1) {
			return GetImplementation().MapAttribute(name, format, count);
		}
		/**
		Lets the engine know about a uniform variable
		\param[in] name The name of the in-shader variable name
		\param[in] format The data format of the variable
		\param[in] count The number of components, if the variable is a vector type

		\todo Return offset?
		*/
		inline Result MapUniform(const std::string &name, InputFormat format, uint8_t count = 1) {
			return GetImplementation().MapUniform(name, format, count);
		}
		/**
		\brief Compiles the shaders into a fully baked pipeline
		\warning `AddShader`, `MapAttribute`, and `MapUniform` will fail after `Compile` has been called

		\return Details about potential failure
		*/
		inline Result Compile() {
			return GetImplementation().Compile();
		}

		/**
		Cleans up after itself
		*/
		inline void Release() {
			GetImplementation().Release();
		}
	};

	/**
	Implements the `GraphicsPipeline` interface for Direct3D 12
	*/
	class D3D12GraphicsPipeline : public GraphicsPipeline<D3D12GraphicsPipeline>
	{
	public:
		//Allow Devices, and only Devices, to create Pipeline objects
		//Makes sure that CRTP and factory design are not sidestepped
		friend class D3D12Device;
	private:
		/**
		Just zeroes all the memory
		*/
		D3D12GraphicsPipeline();
		/**
		All the clean-up is handled in `Release()`
		*/
		~D3D12GraphicsPipeline() = default;

		std::vector<ID3DBlob*> m_shaders;  //Stores compiled (but unlinked) shaders
		ID3DBlob *signature;               //D3D12's version of uniforms and attributes, pretty sure
		ID3DBlob *error;                   //Keep track of errors while ompiling
		ID3D12PipelineState *m_pipeline;   //A D3D12 pipeline state object (PSO)
		ID3D12Device *m_device;            //Stores the device which created it, because the API object hasn't actually been created until `Compile()` is called
	};

	/**
	Implements the `GraphicsPipeline` interface for Vulkan
	*/
	class VulkanGraphicsPipeline : public GraphicsPipeline<VulkanGraphicsPipeline>
	{
	public:
		//Allow Devices, and only Devices, to create Pipeline objects
		//Makes sure that CRTP and factory design are not sidestepped
		friend class VulkanDevice;
	private:
		/**
		Just zeroes all the memory
		*/
		VulkanGraphicsPipeline();
		/**
		All the clean-up is handled in `Release()`
		*/
		~VulkanGraphicsPipeline() = default;
		
		VkPipeline m_pipeline; //A Vulkan pipeline object
	};
}

#endif
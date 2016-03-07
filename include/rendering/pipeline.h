/**
\file   pipeline.h
\author Andrew Baxter
\date   March 6, 2015

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
	The size and type of a shader's input component
	* /
	enum class InputFormat : uint8_t
	{
		Unknown = 0,

		//Scalars
		
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
		
		//2D Vectors
		
		Typeless32_2,
		Float32_2,
		UInt32_2,
		SInt32_2,
		UNorm32_2,
		SNorm32_2,

		Typeless16_2,
		Float16_2,
		UInt16_2,
		SInt16_2,
		UNorm16_2,
		SNorm16_2,

		Typeless8_2,
		Float8_2,
		UInt8_2,
		SInt8_2,
		UNorm8_2,
		SNorm8_2,
		
		//3D Vectors
		
		Typeless32_3,
		Float32_3,
		UInt32_3,
		SInt32_3,
		UNorm32_3,
		SNorm32_3,

		Typeless16_3,
		Float16_3,
		UInt16_3,
		SInt16_3,
		UNorm16_3,
		SNorm16_3,

		Typeless8_3,
		Float8_3,
		UInt8_3,
		SInt8_3,
		UNorm8_3,
		SNorm8_3,
		
		//4D Vectors
		
		Typeless32_4,
		Float32_4,
		UInt32_4,
		SInt32_4,
		UNorm32_4,
		SNorm32_4,

		Typeless16_4,
		Float16_4,
		UInt16_4,
		SInt16_4,
		UNorm16_4,
		SNorm16_4,

		Typeless8_4,
		Float8_4,
		UInt8_4,
		SInt8_4,
		UNorm8_4,
		SNorm8_4,
		
		//2x2 Matrices
		
		Typeless32_2x2,
		Float32_2x2,
		UInt32_2x2,
		SInt32_2x2,
		UNorm32_2x2,
		SNorm32_2x2,

		Typeless16_2x2,
		Float16_2x2,
		UInt16_2x2,
		SInt16_2x2,
		UNorm16_2x2,
		SNorm16_2x2,

		Typeless8_2x2,
		Float8_2x2,
		UInt8_2x2,
		SInt8_2x2,
		UNorm8_2x2,
		SNorm8_2x2,
		
		//3x3 Matrices
		
		Typeless32_3x3,
		Float32_3x3,
		UInt32_3x3,
		SInt32_3x3,
		UNorm32_3x3,
		SNorm32_3x3,

		Typeless16_3x3,
		Float16_3x3,
		UInt16_3x3,
		SInt16_3x3,
		UNorm16_3x3,
		SNorm16_3x3,

		Typeless8_3x3,
		Float8_3x3,
		UInt8_3x3,
		SInt8_3x3,
		UNorm8_3x3,
		SNorm8_3x3,
		
		//4x4 Matrices
		
		Typeless32_4x4,
		Float32_4x4,
		UInt32_4x4,
		SInt32_4x4,
		UNorm32_4x4,
		SNorm32_4x4,

		Typeless16_4x4,
		Float16_4x4,
		UInt16_4x4,
		SInt16_4x4,
		UNorm16_4x4,
		SNorm16_4x4,

		Typeless8_4x4,
		Float8_4x4,
		UInt8_4x4,
		SInt8_4x4,
		UNorm8_4x4,
		SNorm8_4x4,
	}; */

	/**
	A direct mirror of Vulkan's `VkShaderStageFlagBits` structure
	Will be translated at runtime for use in a D3D12 pipeline
	*/
	enum class ShaderStage : uint32_t
	{
		Vertex = 1 << 0,
		TesselationControl = 1 << 1,
		TesselationEvaluation = 1 << 2,
		Geometry = 1 << 3,
		Fragment = 1 << 4,
		Compute = 1 << 5,
		AllGraphics = Vertex | TesselationControl | TesselationEvaluation | Geometry | Fragment,
		All = 0x7FFFFFFF
	};
	

	/**
	Uses CRTP to represent an ambiguous graphics pipeline

	\tparam Impl Sets up the Curiously Recurring Template Pattern

	\todo Make an AddSampler() function, or equivalent
	\todo Rework the MapAttribute/MapUniform system
	*/
	template<class Impl>
	class GraphicsPipeline abstract
	{
	public:
		/**
		Gets this class's CRTP implementation
		\return This class's CRTP implementation
		*/
		inline const Impl &GetImplementation() {
			return static_cast<Impl&>(*this);
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
		
		VkPipeline m_pipeline;
		VkPipelineLayout m_layout;
	};
}

#endif
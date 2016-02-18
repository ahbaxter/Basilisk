/**
\file   pipeline.h
\author Andrew Baxter
\date   February 17, 2015



*/

#ifndef BASILISK_PIPELINE_H
#define BASILISK_PIPELINE_H

#include "../common.h"
#include <vector>

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

	/*
	enum class ShaderFormat
	{

	};
	*/

	/**
	\brief 
	To create a `Pipeline` object, you must use the `Device::CreatePipeline` method

	\tparam Impl Sets up the Curiously Recurring Template Pattern
	*/
	template<class Impl>
	class Pipeline
	{
	public:
		inline const Impl &GetImplementation()
		{
			return static_cast<Impl&>(*this);
		}

		//Result AddShader(const std::string &text);
		//Result AddShader(const std::string &fileName);
		Result DescribeAttribute(const std::string &name, InputFormat format, uint8_t count = 1);
		Result DescribeUniform(const std::string &name, InputFormat format, uint8_t count = 1);
		Result Compile();

		Result Release();

		//Allow implementations, and only implementations to instanciate a Device object
		//Makes sure that CRTP is not sidestepped
		friend class D3D12Pipeline;
		friend class VulkanPipeline;
	private:
		Pipeline();
		~Pipeline();
	};


	class D3D12Pipeline : public Pipeline<D3D12Pipeline>
	{
	public:
		friend class D3D12Device; //Can I do this?
		inline ID3D12PipelineState *getPipelineState();
	private:
		std::vector<ID3DBlob*> m_shaders;
		ID3DBlob *signature, *error;
		ID3D12PipelineState *m_pso;
	};


	class VulkanPipeline : public Pipeline<VulkanPipeline>
	{
	public:
		friend class VulkanDevice; //Can I do this?
	};

}

#endif
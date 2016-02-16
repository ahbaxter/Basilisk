#pragma once
#include "../Common.h"
#include <vector>

namespace Basilisk
{
	class D3D12Device;

	enum InputFormat
	{
		INPUT_FORMAT_UNKOWN = 0,
		INPUT_FORMAT_TYPELESS32,
		INPUT_FORMAT_FLOAT32,
		INPUT_FORMAT_UINT32,
		INPUT_FORMAT_SINT32,
		INPUT_FORMAT_UNORM32,
		INPUT_FORMAT_SNORM32,
		INPUT_FORMAT_TYPELESS16,
		INPUT_FORMAT_FLOAT16,
		INPUT_FORMAT_UINT16,
		INPUT_FORMAT_SINT16,
		INPUT_FORMAT_UNORM16,
		INPUT_FORMAT_SNORM16,
		INPUT_FORMAT_TYPELESS8,
		INPUT_FORMAT_FLOAT8,
		INPUT_FORMAT_UINT8,
		INPUT_FORMAT_SINT8,
		INPUT_FORMAT_UNORM8,
		INPUT_FORMAT_SNORM8
	};

	class D3D12Pipeline
	{
	public:
		D3D12Pipeline(D3D12Device *device);

		bool addShader(const char *fileName, const char *target, const char *entryPoint);
		void addAttribute(const char *name, InputFormat format, unsigned int count = 1);
		void addUniform(const char *name, InputFormat format, unsigned int count = 1);
		bool compile();

		void release();

		//D3D12-specific functions

		inline ID3D12PipelineState *getPipelineState();
	private:

		std::vector<ID3DBlob*> m_shaders;
		ID3DBlob *signature, *error;
		ID3D12PipelineState *m_pso;
	};

	class VulkanPipeline
	{
	};

#ifdef _VULKAN
	typedef VulkanPipeline Pipeline;
#else
	typedef D3D12Pipeline Pipeline;
#endif

}
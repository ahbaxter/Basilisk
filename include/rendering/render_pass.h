/**
\file   render_target.h
\author Andrew Baxter
\date   February 28, 2016

Encapsulates an ambiguous render pass

*/

#ifndef BASILISK_RENDER_TARGET_H
#define BASILISK_RENDER_TARGET_H

#include "common.h"

namespace Basilisk
{
	template<class Impl>
	class RenderPass
	{
		
	};
	
	class D3D12RenderPass : public RenderPass<D3D12RenderPass>
	{
	public:
		friend class D3D12Device;
			
	private:
		// ?
	};
	
	class VulkanRenderPass : public RenderPass<VulkanRenderPass>
	{
	public:
		friend class VulkanDevice;
	private:
		VkRenderPass m_renderPass;
	};
	
}

#endif
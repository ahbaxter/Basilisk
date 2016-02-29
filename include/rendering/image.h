/**
\file   image.h
\author Andrew Baxter
\date February 28, 2016

An in-progress representation of multidimentional images

*/

#ifndef BASILISK_IMAGE_H
#define BASILISK_IMAGE_H

#include "common.h"

namespace Basilisk
{
	template<class Impl>
	class Image abstract
	{
	public:
		const inline Impl &GetImplementation() {
			return static_cast<Impl&>(*this);
		}
	};



	class D3D12Image : public Image<D3D12Image>
	{
	public:
		friend class D3D12Device;

	private:
		D3D12Image();
		~D3D12Image() = default;


	};


	class VulkanImage : public Image<VulkanImage>
	{
	public:
		friend class VulkanDevice;

	private:
		VulkanImage();
		~VulkanImage() = default;

		VkImage m_image;
		VkImageView m_view;
		VkDeviceMemory m_memory;
		VkFormat m_format;
	};
}

#endif
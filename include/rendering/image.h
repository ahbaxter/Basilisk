/**
\file   image.h
\author Andrew Baxter
\date   February 28, 2016

An in-progress representation of multidimentional images

*/

#ifndef BASILISK_IMAGE_H
#define BASILISK_IMAGE_H

#include "common.h"

namespace Basilisk
{
	template<class Impl>
	class ImageSet abstract
	{
	public:
		const inline Impl &GetImplementation() {
			return static_cast<Impl&>(*this);
		}
	};



	class D3D12ImageSet : public ImageSet<D3D12ImageSet>
	{
	public:
		friend class D3D12Device;

	private:
		D3D12ImageSet();
		~D3D12ImageSet() = default;


	};


	class VulkanImageSet : public ImageSet<VulkanImageSet>
	{
	public:
		friend class VulkanDevice;

	private:
		VulkanImageSet();
		~VulkanImageSet() = default;
		
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_views;
		std::vector<VkDeviceMemory> m_memory;
		std::vector<VkFormat> m_formats;
	};
}

#endif
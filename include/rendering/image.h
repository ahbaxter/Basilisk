/**
\file   image.h
\author Andrew Baxter
\date February 23, 2016



\todo Investigate usage types. How much should be specified compile-time?

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



	class D3D12Image2D : public Image<D3D12Image2D>
	{
	public:

	private:
		D3D12Image2D();
		~D3D12Image2D() = default;
	};

	class D3D12Image3D : public Image<D3D12Image3D>
	{
	public:

	private:
		D3D12Image3D();
		~D3D12Image3D() = default;
	};



	class VulkanImage2D : public Image<VulkanImage2D>
	{
	public:

	private:
		VulkanImage2D();
		~VulkanImage2D() = default;
	};

	class VulkanImage3D : public Image<VulkanImage3D>
	{
	public:

	private:
		VulkanImage3D();
		~VulkanImage3D() = default;
	};

}

#endif
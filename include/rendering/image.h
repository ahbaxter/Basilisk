/**
\file   image.h
\author Andrew Baxter
\date   March 4, 2016

An in-progress representation of multidimentional images

*/

#ifndef BASILISK_IMAGE_H
#define BASILISK_IMAGE_H

#include "common.h"

namespace Basilisk
{
	
	enum class ImageFormat
	{
		Undefined = 0,
		R4G4UNormPack8,
		R4G4B4A4UNormPack16,
		B4G4R4A4UNormPack16,
		R5G6B5UNormPack16,
		B5G6R5UNormPack16,
		R5G5B5A1UNormPack16,
		B5G5R5A1UNormPack16,
		A1B5G5R5UNormPack16,
		R8UNorm,
		R8SNorm,
		R8UScaled,
		R8SScaled,
		R8UInt,
		R8SInt,
		R8SRGB,
		R8G8UNorm,
		R8G8SNorm,
		R8G8UScaled,
		R8G8SScaled,
		R8G8UInt,
		R8G8SInt,
		R8G8SRGB,
		R8G8B8UNorm,
		R8G8B8SNorm,
		R8G8B8UScaled,
		R8G8B8SScaled,
		R8G8B8UInt,
		R8G8B8SInt,
		R8G8B8SRGB,
		B8G8R8UNorm,
		B8G8R8SNorm,
		B8G8R8UScaled,
		B8G8R8SScaled,
		B8G8R8UInt,
		B8G8R8SInt,
		B8G8R8SRGB,
		R8G8B8A8UNorm,
		R8G8B8A8SNorm,
		R8G8B8A8UScaled,
		R8G8B8A8SScaled,
		R8G8B8A8UInt,
		R8G8B8A8SInt,
		R8G8B8A8SRGB,
		B8G8R8A8UNorm,
		B8G8R8A8SNorm,
		B8G8R8A8UScaled,
		B8G8R8A8SScaled,
		B8G8R8A8UInt,
		B8G8R8A8SInt,
		B8G8R8A8SRGB,
		A8R8G8B8UNormPack32,
		A8R8G8B8SNormPack32,
		A8R8G8B8UScaledPack32,
		A8R8G8B8SScaledPack32,
		A8R8G8B8UIntPack32,
		A8R8G8B8SIntPack32,
		A8R8G8B8SRGBPack32,
		A2R10G10B10UNormPack32,
		A2R10G10B10SNormPack32,
		A2R10G10B10UScaledPack32,
		A2R10G10B10SScaledPack32,
		A2R10G10B10UIntPack32,
		A2R10G10B10SIntPack32,
		A2B10G10R10UNormPack32,
		A2B10G10R10SNormPack32,
		A2B10G10R10UScaledPack32,
		A2B10G10R10SScaledPack32,
		A2B10G10R10UIntPack32,
		A2B10G10R10SIntPack32,
		R16UNorm,
		R16SNorm,
		R16UScaled,
		R16SScaled,
		R16UInt,
		R16SInt,
		R16SFloat,
		R16G16UNorm,
		R16G16SNorm,
		R16G16UScaled,
		R16G16SScaled,
		R16G16UInt,
		R16G16SInt,
		R16G16SFloat,
		R16G16B16UNorm,
		R16G16B16SNorm,
		R16G16B16UScaled,
		R16G16B16SScaled,
		R16G16B16UInt,
		R16G16B16SInt,
		R16G16B16SFloat,
		R16G16B16A16UNorm,
		R16G16B16A16SNorm,
		R16G16B16A16UScaled,
		R16G16B16A16SScaled,
		R16G16B16A16UInt,
		R16G16B16A16SInt,
		R16G16B16A16SFloat,
		R32UInt,
		R32SInt,
		R32SFloat,
		R32G32UInt,
		R32G32SInt,
		R32G32SFloat,
		R32G32B32UInt,
		R32G32B32SInt,
		R32G32B32SFloat,
		R32G32B32A32G32B32UInt,
		R32G32B32A32G32B32SInt,
		R32G32B32A32G32B32SFloat,
		R64UInt,
		R64SInt,
		R64SFloat,
		R64G64UInt,
		R64G64SInt,
		R64G64SFloat,
		R64G64B64UInt,
		R64G64B64SInt,
		R64G64B64SFloat,
		R64G64B64A64G64B64UInt,
		R64G64B64A64G64B64SInt,
		R64G64B64A64G64B64SFloat,
		B10G11R11UFloatPack32,
		E5B9G9R9UFloatPack32,
		D16UNorm,
		X8D24UnormPack32,
		D32SFloat,
		S8UInt,
		D16UNormS8UInt,
		D24UNormS8UInt,
		D32SFloatS8UInt,
		BC1RGBUNormBlock,
		BC1RGBSRGBBlock,
		BC1RGBAUNormBlock,
		BC1RGBAUNormBlock,
		BC1RGBASRGBBlock,
		BC2UNormBlock,
		BV2SRGBBlock,
		BC3UNormBlock,
		BC3SRGBBlock,
		BC4UNormBlock,
		BC4SNormBlock,
		BC5UNormBlock,
		BC5SNormBlock,
		BC6HUFloatBlock,
		BC6HSFloatBlock,
		BC7UNormBlock,
		BC7SRGBBlock,
		ETC2R8G8B8UNormBlock,
		ETC2R8G8B8SRGBBlock,
		ETC2R8G8B8A1UNormBlock,
		ETC2R8G8B8A1SRGBBlock,
		ETC2R8G8B8A8UNormBlock,
		ETC2R8G8B8A8SRGBBlock,
		EACR11UNormBlock,
		EACR11SNormBlock,
		EACR11G11UNormBlock,
		EACR11G11SNormBlock,
		ASTC4x4UNormBlock,
		ASTC4x4SRGBBlock,
		ASTC5x4UNormBlock,
		ASTC5x4SRGBBlock,
		ASTC5x5UNormBlock,
		ASTC5x5SRGBBlock,
		ASTC6x5UNormBlock,
		ASTC6x5SRGBBlock,
		ASTC6x6UNormBlock,
		ASTC6x6SRGBBlock,
		ASTC8x5UNormBlock,
		ASTC8x5SRGBBlock,
		ASTC8x6UNormBlock,
		ASTC8x6SRGBBlock,
		ASTC8x8UNormBlock,
		ASTC8x8SRGBBlock,
		ASTC10x5UNormBlock,
		ASTC10x5SRGBBlock,
		ASTC10x6UNormBlock,
		ASTC10x6SRGBBlock,
		ASTC10x8UNormBlock,
		ASTC10x8SRGBBlock,
		ASTC10x10UNormBlock,
		ASTC10x10SRGBBlock,
		ASTC12x10UNormBlock,
		ASTC12x10SRGBBlock,
		ASTC12x12UNormBlock,
		ASTC12x12SRGBBlock
	};

	struct D3D12Image
	{
	public:
		friend class D3D12Device;

	private:
		D3D12Image();
		~D3D12Image() = default;


	};


	template<uint32_t count>
	struct VulkanImage
	{
	public:
		friend class VulkanDevice;
		
		static Result LoadFromFile(VkImage out, VkDevice device, const std::string &filename, VkFormat format);
		static Result LoadFromData(VkImage out, VkDevice device, const unsigned char *bytes, VkFormat format);
		
	private:
		VulkanImage();
		~VulkanImage() = default;
		
		std::array<VkImage, count> m_images;
		std::array<VkImageView, count> m_views;
		std::array<VkDeviceMemory, count> m_memory;
		std::array<VkFormat, count> m_formats;
		std::array<VkSampler, count> m_samplers;
	};
}

#endif
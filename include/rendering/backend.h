/**
\file   backend.h
\author Andrew Baxter
\date   March 9, 2016

The virtual interface with the Vulkan API

\todo Figure out what I'm doing with render and present queues
\todo Look into debug/validation layers
\todo Boot up Vulkan without a render output, or with a monitor target
\todo Contiguous FrameBuffer image memory

*/

#ifndef BASILISK_BACKEND_H
#define BASILISK_BACKEND_H

#include <array>
#include "common.h"
#include "command_buffer.h"


namespace Vulkan
{
	//Configuration details
	extern constexpr uint32_t layerCount();
	extern constexpr const char **layerNames();

	extern constexpr uint32_t extensionCount();
	extern constexpr const char **extensionNames();

	extern constexpr uint32_t apiVersion();


	template<uint32_t count>
	struct ImageSet
	{
	public:
		friend class Device;

		static VkImage LoadFromFile(VkDevice device, const std::string &filename, VkFormat format);
		static VkImage LoadFromData(VkDevice device, const char *bytes, VkFormat format);

	private:
		ImageSet();
		~ImageSet() = default;

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		std::array<VkImage, count> m_images;
		std::array<VkImageView, count> m_views;
		std::array<VkDeviceMemory, count> m_memory;
		std::array<VkFormat, count> m_formats;
		std::array<VkSampler, count> m_samplers;

		glm::uvec3 m_size; //Any unused dimension should be set to 1
	};
	typedef ImageSet<1> Image;

	class SwapChain
	{
	public:
		friend class Device;
	private:
		SwapChain();
		~SwapChain() = default;

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		VkSwapchainKHR m_swapChain;

		//List of all backbuffers
		std::vector<VkImage> m_backBuffers;
		std::vector<VkImageView> m_backBufferViews;
		std::vector<VkFramebuffer> m_frameBuffers;

		uint32_t m_bufferIndex; //Which buffer to write to this frame
	};

	class Shader
	{
	public:
		friend class Device;

	private:
		Shader();
		~Shader() = default;

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		/**
		Loads a shader from SPIR-V bytecode

		\param[in] device The Vulkan device to use for creation
		\param[in] data The bytecode to compile from
		\param[in] stage Which pipeline stage this module will be bound to
		\return If successful, a pointer to the resulting shader module. If failed, `nullptr`.
		*/
		static VkShaderModule FromSPIRVBlob(VkDevice device, char *data, VkShaderStageFlagBits stage);
		/**
		Loads a shader from a SPIR-V file

		\param[in] device The Vulkan device to use for creation
		\param[in] filename The location of the SPIR-V file
		\param[in] stage Which pipeline stage this module will be bound to
		\return If successful, a pointer to the resulting shader module. If failed, `nullptr`.
		*/
		static VkShaderModule FromSPIRVFile(VkDevice device, const std::string &filename, VkShaderStageFlags stage);
		/**
		Loads a shader from GLSL source

		\param[in] device The Vulkan device to use for creation
		\param[in] source The source to compile from
		\param[in] stage Which pipeline stage this module will be bound to
		\return If successful, a pointer to the resulting shader module. If failed, `nullptr`.
		*/
		static VkShaderModule FromGLSLSource(VkDevice device, const std::string &source, VkShaderStageFlags stage);
		/**
		Loads a shader from a GLSL file

		\param[in] device The Vulkan device to use for creation
		\param[in] filename The location of the GLSL file
		\param[in] stage Which pipeline stage this module will be bound to
		\return If successful, a pointer to the resulting shader module. If failed, `nullptr`.
		*/
		static VkShaderModule FromGLSLFile(VkDevice device, const std::string &filename, VkShaderStageFlags stage);

		VkShaderModule m_module;
	};
	
	/*
	struct Descriptor
	{
		uint32_t bindPoint;
		DescriptorType type;
		VkShaderStageFlagBits visibility;
	};
	*/
	
	class PipelineLayout
	{
	public:
		friend class Device;

	private:
		PipelineLayout();
		~PipelineLayout() = default;

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		VkDescriptorSetLayout m_descriporsLayout;
		VkPipelineLayout m_layout;
	};

	class GraphicsPipeline
	{
	public:
		friend class Device;

	private:
		GraphicsPipeline();
		~GraphicsPipeline() = default;

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		PipelineLayout m_layout; //Shared; do not deallocate
		VkPipeline m_pipeline;
	};

	class ComputePipeline
	{
	public:
		friend class Device;

	private:
		ComputePipeline();
		~ComputePipeline() = default;

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used
	};


	class FrameBuffer
	{
	public:
		friend class Device;
	private:
		FrameBuffer();
		~FrameBuffer() = default;

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		void ResizeVectors(uint32_t size);
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_views;
		std::vector<VkFormat> m_formats;
		std::vector<VkDeviceMemory> m_memory; //I think I can just allocate a solid block large enough for all of them

		VkFramebuffer m_frameBuffer;
		VkRenderPass m_renderPass;

		glm::uvec2 m_resolution;
	};

	class CommandBuffer
	{
	public:
		friend class Device;

		bool Begin(bool disposable);

		void WriteBundle(const VulkanCmdBuffer *bundle);

		bool End();

		/**
		Changes the layout of an image

		\param[in] image The image to modify
		\param[in] aspectMask The aspect mask
		\param[in] oldLayout The image's prior layout
		\param[in] newLayout What layout to change the image to
		\param[in] oldQueueFamilyIndex Where to the image was previously viewable from
		\param[in] newQueueFamilyIndex Where to the image will be viewable from
		*/
		void SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t oldQueueFamilyIndex = Device::renderQueue, uint32_t newQueueFamilyIndex = Device::renderQueue);


	private:
		CommandBuffer();
		~CommandBuffer() = default;

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		VkCommandBuffer m_commandBuffer;
	};
	
	struct GpuProperties
	{
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceProperties props;
		VkPhysicalDeviceMemoryProperties memProps;

		VkFormat depthFormat;
		VkImageTiling depthTiling;

		std::vector<VkQueueFamilyProperties> queueDescs;
	};

	class Device
	{
	public:
		friend class Instance;

		/**
		Creates a graphics pipeline

		\param[in] layout What information this pipeline expects to receive when used
		\param[in] shaders A list of shaders to bind to different stages of the pipeline
		\return If successful, a pointer to the resulting graphics pipeline. If unsuccessfull, `nullptr`.
		*/
		std::shared_ptr<GraphicsPipeline> CreateGraphicsPipeline(const std::shared_ptr<PipelineLayout> &layout, const std::vector<Shader> &shaders);

		/**
		Creates a swap chain
		
		\param[in] setupCmdBuffer Helps to initialize the swap chain. Must be open for writing when passed.
		\param[in] resolution The resolution to use. May be ignored, depending on GPU quirks
		\param[in] numBuffers The number of buffers to use. Defaults to 2 (double-buffered). May be clamped to fit the GPU's capabilities
		\return If successful, a pointer to the resulting swap chain. If failed, `nullptr`.
		*/
		std::shared_ptr<SwapChain> CreateSwapChain(const std::shared_ptr<CommandBuffer> &setupCmdBuffer, glm::uvec2 resolution, uint32_t numBuffers);
		
		/**
		Creates a command buffer

		\param[in] bundle Is this command buffer a secondary command buffer? Defaults to false.
		\param[in] poolIndex Which pool to allocate from. Defaults to 0.
		\return If successful, a pointer to the resulting command buffer. If failed, `nullptr`.

		\todo Queue index parameter
		*/
		std::shared_ptr<CommandBuffer> CreateCommandBuffer(bool bundle = false, uint32_t poolIndex = 0);

		/**
		Creates a frame buffer
		
		\param[in] resolution The size of 
		\param[in] colorFormats Which formats to create each image with
		\param[in] enableDepth Should we expect a depth buffer? Defaults to yes
		\return If successful, a pointer to the resulting frame buffer. If failed, `nullptr`.
		*/
		std::shared_ptr<FrameBuffer> CreateFrameBuffer(glm::uvec2 resolution, const std::vector<VkFormat> &colorFormats = {VK_FORMAT_R8G8B8A8_UNORM}, bool enableDepth = true);

		/**
		Executes pre-recorded commands stored in a command bundle

		\return If successful, `true`. If failed, `false`.
		*/
		bool ExecuteCommands(const std::vector<CommandBuffer> &commands);


		/** Swaps out backbuffers */
		void Present();

		static constexpr uint32_t renderQueue = 0; //Queue index for render operations
		static constexpr uint32_t presentQueue = 1; //Queue index for present operations
		static constexpr uint32_t computeQueue = 2; //Queue index for compute operations
		//TODO: Are queues thread-friendly? Probably not.

	private:
		Device();
		~Device() = default;

		void Release(); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		Instance *m_parent; //Devices do not own instances, so use a regular pointer. Primarily used to present to the screen.
		GpuProperties m_gpuProps;

		VkDevice m_device;
		std::vector<VkQueue> m_queues;
		std::array<VkCommandPool, 2> m_commandPools; //Separate pools for render and present queues
		
		//VK_KHR_swapchain function pointers
		PFN_vkCreateSwapchainKHR pfnCreateSwapchainKHR;
		PFN_vkDestroySwapchainKHR pfnDestroySwapchainKHR;
		PFN_vkGetSwapchainImagesKHR pfnGetSwapchainImagesKHR;
		PFN_vkAcquireNextImageKHR pfnAcquireNextImageKHR;
		PFN_vkQueuePresentKHR pfnQueuePresentKHR;
		//VK_KHR_display function pointers
		PFN_vkGetPhysicalDeviceDisplayPropertiesKHR pfnGetPhysicalDeviceDisplayPropertiesKHR;
		PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR pfnGetPhysicalDeviceDisplayPlanePropertiesKHR;
		PFN_vkGetDisplayPlaneSupportedDisplaysKHR pfnGetDisplayPlaneSupportedDisplaysKHR;
		PFN_vkGetDisplayModePropertiesKHR pfnGetDisplayModePropertiesKHR;
		PFN_vkCreateDisplayModeKHR pfnCreateDisplayModeKHR;
		PFN_vkGetDisplayPlaneCapabilitiesKHR pfnGetDisplayPlaneCapabilitiesKHR;
		PFN_vkCreateDisplayPlaneSurfaceKHR pfnCreateDisplayPlaneSurfaceKHR;
		//VK_KHR_display_swapchain function pointers
		PFN_vkCreateSharedSwapchainsKHR pfnCreateSharedSwapchainsKHR;
		

		//Helper functions
		bool MemoryTypeFromProps(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);
	};

	struct PresentableSurface
	{
		VkSurfaceCapabilitiesKHR caps;
		VkSurfaceKHR surface;
		VkFormat colorFormat;
		std::vector<VkPresentModeKHR> presentModes;
	};

	/**
	\brief Wraps a Vulkan instance
	Can only be instantiated through `Vulkan::Initialize()`
	*/
	class Instance
	{
	public:
		friend std::shared_ptr<Instance> Initialize(const std::string &appName, uint32_t appVersion = 1);

		/**
		Counts and internally stores all connected GPUs

		\return 
		*/
		uint32_t FindGpus();

		/**
		Fetches 
		*/
		const GpuProperties &GetGpuProperties(uint32_t index);
		
		/**
		Creates a Vulkan device on the specified GPU

		\param[in] gpuIndex Which GPU to use
		\return If successful, 

		\todo Multiple command pools
		*/
		std::shared_ptr<Device> CreateDevice(uint32_t gpuIndex);

		/**

		*/
		bool HookWin32Window(HWND hWnd, HINSTANCE hInstance);

		/**

		*/
		bool HookMonitor(uint32_t index);

	private:
		Instance();
		~Instance() = default;

		void Release(); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		VkInstance m_instance;
		std::vector<GpuProperties> m_gpuProps;
		std::vector<VkPhysicalDevice> m_gpus;

		//Add getters later. Not sure what information the device needs to present.
		PresentableSurface m_windowTarget;
		PresentableSurface m_monitorTarget;

		//VK_KHR_surface function pointers
		PFN_vkDestroySurfaceKHR pfnDestroySurfaceKHR;
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR pfnGetPhysicalDeviceSurfaceSupportKHR;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR pfnGetPhysicalDeviceSurfaceCapabilitiesKHR;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR pfnGetPhysicalDeviceSurfaceFormatsKHR;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR pfnGetPhysicalDeviceSurfacePresentModesKHR;

		//VK_KHR_win32_surface function pointers
		PFN_vkCreateWin32SurfaceKHR pfnCreateWin32SurfaceKHR;
		PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR pfnGetPhysicalDeviceWin32PresentationSupportKHR;
	};


	/**
	Boots up Vulkan

	\param[in] appName The title of your application
	\param[in] appVersion The version of your application
	\return The number of GPUs connected to this computer. 0 indicates failure.
	*/
	std::shared_ptr<Instance> Initialize(const std::string &appName, uint32_t appVersion = 1);
}

#endif
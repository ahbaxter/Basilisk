/**
\file   backend.h
\author Andrew Baxter
\date   March 10, 2016

The virtual interface with the Vulkan API

\todo Integrate command buffers
\todo Figure out what I'm doing with render and present queues
\todo Look into debug/validation layers
<<<<<<< HEAD
\todo Boot up Vulkan with a monitor target
=======
\todo Boot up Vulkan without a render output, or with a monitor target
>>>>>>> 433ed478d12f6824d8a7d2954fc70e6d342e675f
\todo Can FrameBuffer image memory be contiguous?

*/

#ifndef BASILISK_BACKEND_H
#define BASILISK_BACKEND_H

#include <map>
#include "common.h"


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
		~ImageSet() = default;
		friend class Device;

		static VkImage LoadFromFile(VkDevice device, const std::string &filename, VkFormat format);
		static VkImage LoadFromData(VkDevice device, const char *bytes, VkFormat format);

	private:
		ImageSet();

		void Release(VkDevice device) //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used
		{
			for (uint32_t i = 0; i < m_images.size(); ++i)
			{
				m_formats[i] = VK_FORMAT_UNDEFINED;

				if (m_samplers[i]) {
					vkDestroySampler(device, m_samplers[i], nullptr);
					m_samplers[i] = VK_NULL_HANDLE;
				}

				if (m_views[i]) {
					vkDestroyImageView(device, m_views[i], nullptr);
					m_views[i] = VK_NULL_HANDLE;
				}

				if (m_memory[i]) {
					vkFreeMemory(device, m_memory[i], nullptr);
					m_memory[i] = VK_NULL_HANDLE;
				}

				if (m_images[i]) {
					vkDestroyImage(device, m_images[i], nullptr);
					m_images[i] = VK_NULL_HANDLE;
				}
			}
		}

		std::array<VkImage, count> m_images;
		std::array<VkImageView, count> m_views;
		std::array<VkDeviceMemory, count> m_memory;
		std::array<VkFormat, count> m_formats;
		std::array<VkSampler, count> m_samplers;

		glm::uvec3 m_resolution; //Any unused dimension should be set to 1
	};
	typedef ImageSet<1> Image;

	class SwapChain
	{
	public:
		~SwapChain() = default;
		friend class Device;
	private:
		SwapChain();

		void Release(VkDevice device, PFN_vkDestroySwapchainKHR func); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		VkSwapchainKHR m_swapChain;

		//List of all backbuffers
		std::vector<VkImage> m_backBuffers;
		std::vector<VkImageView> m_backBufferViews;

		uint32_t m_bufferIndex; //Which buffer to write to this frame
	};

	class Shader
	{
	public:
		~Shader() = default;
		friend class Device;

	private:
		Shader();

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		VkShaderModule m_module;
	};
	
	struct Descriptor
	{
		uint32_t bindPoint;
		VkDescriptorType type;
		VkShaderStageFlagBits visibility;
	};
	
	class PipelineLayout
	{
	public:
		~PipelineLayout() = default;
		friend class Device;

	private:
		PipelineLayout();

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used
		
<<<<<<< HEAD
		VkDescriptorSetLayout m_setLayout;
=======
		VkDescriptorSetLayoutInfo m_setLayout;
>>>>>>> 433ed478d12f6824d8a7d2954fc70e6d342e675f
		VkPipelineLayout m_layout;
	};

	struct GraphicsPipelineState
	{
		static GraphicsPipelineState Create(const std::vector<VkDynamicState> &dynamicStateEnables, const std::vector<VkPipelineColorBlendAttachmentState> &blendAttachments);

		VkPipelineVertexInputStateCreateInfo vertexInputState;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		VkPipelineTessellationStateCreateInfo tesselationState;
		VkPipelineViewportStateCreateInfo viewportState;
		VkPipelineRasterizationStateCreateInfo rasterizationState;
		VkPipelineMultisampleStateCreateInfo multisampleState;
		VkPipelineDepthStencilStateCreateInfo depthStencilState;
		VkPipelineColorBlendStateCreateInfo colorBlendState;
		VkPipelineDynamicStateCreateInfo dynamicState;
	};

	class GraphicsPipeline
	{
	public:
		~GraphicsPipeline() = default;
		friend class Device;

	private:
		GraphicsPipeline();

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		VkPipeline m_pipeline;
	};

	class ComputePipeline
	{
	public:
		~ComputePipeline() = default;
		friend class Device;

	private:
		ComputePipeline();

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used
	};


	class FrameBuffer
	{
	public:
		~FrameBuffer() = default;
		friend class Device;
	private:
		FrameBuffer();

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
		~CommandBuffer() = default;
		friend class Device;

		bool Begin(bool disposable);

		void BeginRendering(FrameBuffer target);

		void BindGraphicsPipeline(const std::shared_ptr<GraphicsPipeline> &pipeline);

		void BindComputePipeline(const std::shared_ptr<ComputePipeline> &pipeline);

		void EndRendering();

		bool End();



		void WriteBundle(const std::shared_ptr<CommandBuffer> &bundle);

		void Reset();

		/**
		Changes the layout of an image

		\param[in] image The image to modify
		\param[in] aspectMask The aspect mask
		\param[in] oldLayout The image's prior layout
		\param[in] newLayout What layout to change the image to
		\param[in] oldQueueFamilyIndex Where to the image was previously viewable from
		\param[in] newQueueFamilyIndex Where to the image will be viewable from
		*/
		void SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t oldQueueFamilyIndex, uint32_t newQueueFamilyIndex);

		//Just a crutch until I finish all the other functions
		inline VkCommandBuffer Get() {
			return m_commandBuffer;
		}

	private:
		CommandBuffer();

		void Release(VkDevice device, VkCommandPool pool); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

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

	struct PresentableSurface
	{
		VkSurfaceCapabilitiesKHR caps;
		VkSurfaceKHR surface;
		VkFormat colorFormat;
		uint32_t presentQueueIndex, renderQueueIndex;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class Device
	{
	public:
		~Device() = default;
		friend class Instance;

		/**
		Creates a pipeline layout - reusable across multiple pipelines
		
		\param[in] bindings 
		\return If successful, a pointer to the resulting pipeline layout. If failed, `nullptr`.
		*/
		std::shared_ptr<PipelineLayout> CreatePipelineLayout(const std::vector<Descriptor> &bindings);
		
		/**
		Creates a shader module from SPIR-V bytecode
		
		\param[in] blobSize The size of the bytecode blob, in bytes
		\param[in] bytecode The bytecode to use
		\return If successful, a pointer to the resulting shader module. If failed, `nullptr`.
		*/
		std::shared_ptr<Shader> CreateShaderFromSPIRV(uint32_t blobSize, uint32_t *bytecode);
		/**
		Creates a shader module from GLSL source code
		
		\param[in] source The source to compile
		\param[in] stage What part of the pipeline this shader will be bound to
		\return If successful, a pointer to the resulting shader module. If failed, `nullptr`.
		
		\todo Validate stage to make sure we compile as a single stage
		*/
		std::shared_ptr<Shader> CreateShaderFromGLSL(const std::string &source, VkShaderStageFlagBits stage);
		
		/**
		Creates a graphics pipeline

		\param[in] state The state of the pipeline at each stage
		\param[in] frameBuffer The frame buffer this pipeline will alter
		\param[in] layout What information this pipeline expects to receive when used
		\param[in] shaders A list of shaders to bind to different stages of the pipeline
<<<<<<< HEAD
		\param[in] enableTesselation Enables tesselation. Note that if this is enabled, `state::tesselationState` must be filled out
		\return If successful, a pointer to the resulting graphics pipeline. If failed, `nullptr`.

		\todo Parameterize module entry point
=======
		\return If successful, a pointer to the resulting graphics pipeline. If failed, `nullptr`.
>>>>>>> 433ed478d12f6824d8a7d2954fc70e6d342e675f
		*/
		std::shared_ptr<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineState &state, const std::shared_ptr<FrameBuffer> &frameBuffer, const std::shared_ptr<PipelineLayout> &layout, const std::map<VkShaderStageFlagBits, Shader> &shaders, bool enableTesselation = false);

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

		void Release(); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		Instance *m_parent;
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

	/**
	\brief Wraps a Vulkan instance
	Can only be instantiated through `Vulkan::Initialize()`
	*/
	class Instance
	{
	public:
		~Instance() = default;
		friend std::shared_ptr<Instance> Initialize(const std::string &appName, uint32_t appVersion = 1);

		/**
		Counts and internally stores all connected GPUs

		\return The number of GPUs Vulkan could find
		*/
		uint32_t FindGpus();

		/**
		Fetches details about a given GPU

		\param[in] index 
		\return 
		*/
		const GpuProperties *GetGpuProperties(uint32_t index);
		
		/**
		Creates a Vulkan device on the specified GPU

		\param[in] gpuIndex Which GPU to target
		\return If successful, a pointer to the resulting device. If failed, `nullptr`.

		\todo Multithreaded command pools
		*/
		std::shared_ptr<Device> CreateDevice(uint32_t gpuIndex);

		/**
		Sets the presentation target to a Win32 window

		\param[in] gpuIndex Which GPU to use
		\param[in] hWnd Handle to the Win32 window
		\param[in] hInstance Handle to the Win32 instance
		\return `true` if successful. `false` if failed.
		*/
		bool HookWin32Window(uint32_t gpuIndex, HWND hWnd, HINSTANCE hInstance);

		/**
		Sets the presentation target to a monitor

		\param[in] monitorIndex Which monitor to start on
		\return `true` if successful. `false` if failed.
		*/
		bool HookMonitor(uint32_t monitorIndex);

		inline const PresentableSurface *GetPresentTarget() {
			return m_presentTarget;
		}

	private:
		Instance();

		void Release(); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		VkInstance m_instance;
		std::vector<GpuProperties> m_gpuProps;
		std::vector<VkPhysicalDevice> m_gpus;


		//Locations the device could potentially present to
		PresentableSurface *m_presentTarget;

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
	\param[in] appVersion The version of your application. Defaults to 1.
	\return The number of GPUs connected to this computer. 0 indicates failure.
	*/
	std::shared_ptr<Instance> Initialize(const std::string &appName, uint32_t appVersion); //appVersion was forward-declared to default to 1 as a friend to `Instance`
}

#endif
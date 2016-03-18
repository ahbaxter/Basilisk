/**
\file   backend.h
\author Andrew Baxter
\date   March 17, 2016

The virtual interface with the Vulkan API

\todo Verbose error reporting of VkResults
\todo Device::CreateBuffer(...)
\todo Graphics pipeline vertex input layout
\todo Can FrameBuffer image memory be contiguous?

*/

#ifndef BASILISK_BACKEND_H
#define BASILISK_BACKEND_H

#include <atomic>
#include "common.h"


namespace Vulkan
{
	constexpr uint32_t numQueues = 1; //Consolidated render + present queue
	constexpr uint32_t graphicsIndex = 0; //Index of graphics (render + present) queue

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
		uint32_t queueIndex;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct Descriptor
	{
		uint32_t bindPoint;
		VkDescriptorType type;
		VkShaderStageFlagBits visibility;
	};

	struct AttachmentDesc
	{
		VkFormat format;
		uint32_t width, height;
		VkAttachmentLoadOp load;
		VkAttachmentStoreOp store;
	};

	class SwapChain
	{
	public:
		~SwapChain() = default;
		friend class Device;
		friend class CommandBuffer;

		void NextBuffer();
		inline const uint32_t *GetBufferIndex() {
			return &m_currentImage;
		}

		inline const void SetAttachmentLoadOp(VkAttachmentLoadOp loadOp) {
			m_desc.load = loadOp;
		}
		inline const AttachmentDesc &GetAttachmentDesc() {
			return m_desc;
		}
	private:
		SwapChain();

		void Release(VkDevice device, PFN_vkDestroySwapchainKHR func); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		VkSwapchainKHR m_swapChain;

		//One per backbuffer
		std::vector<VkImage> m_images;

		std::function<void(uint32_t *bufferIndex)> pfnAcquireNextImage;
		uint32_t m_currentImage;

		AttachmentDesc m_desc;
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
	
	struct ShaderStage
	{
		std::shared_ptr<Shader> shader;
		VkShaderStageFlagBits stage;
		std::string entryPoint;
	};
	
	/**

	\todo Encapsulate binding descriptions?
	*/
	class PipelineLayout
	{
	public:
		~PipelineLayout() = default;
		friend class Device;

	private:
		PipelineLayout();

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used
		
		VkDescriptorSetLayout m_setLayout;
		VkPipelineLayout m_layout;
	};

	class GraphicsPipeline
	{
	public:
		~GraphicsPipeline() = default;
		friend class Device;
		friend class CommandBuffer;

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
		friend class CommandBuffer;

	private:
		ComputePipeline();

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used
	};


	class FrameBuffer
	{
	public:
		~FrameBuffer() = default;
		friend class Device;
		friend class CommandBuffer;

		inline uint32_t NumAttachments() {
			return static_cast<uint32_t>(m_images.size());
		}

		bool SetClearValues(std::vector<VkClearValue> clearValues);
	private:
		FrameBuffer();

		void Release(VkDevice device); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		void ResizeVectors(uint32_t size);
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_views;
		std::vector<VkFormat> m_formats;
		std::vector<VkDeviceMemory> m_memory;
		std::vector<VkClearValue> m_clearValues;

		VkFramebuffer m_frameBuffer;
		VkRenderPass m_renderPass;

		VkRect2D m_renderArea;
	};

	class CommandBuffer
	{
	public:
		~CommandBuffer() = default;
		friend class Device;

		bool Begin(bool reusable);

		void BeginRendering(const std::shared_ptr<FrameBuffer> &target, bool allowBundles);

		void BindGraphicsPipeline(const std::shared_ptr<GraphicsPipeline> &pipeline);

		void BindComputePipeline(const std::shared_ptr<ComputePipeline> &pipeline);

		void BindVertexBuffers(...);

		void BindIndexBuffer(...);

		void DrawIndexed(uint32_t count);

		void SetLineWidth(float width);

		void SetViewport(const VkViewport &viewport);

		void SetScissor(const VkRect2D &scissor);

		/**
		Blits the first image in the framebuffer to the current buffer of the swap chain
		*/
		void Blit(const std::shared_ptr<FrameBuffer> &src, const std::shared_ptr<SwapChain> &dst);

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
		*/
		void SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout);

	private:
		CommandBuffer();

		void Release(VkDevice device, VkCommandPool pool); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		VkCommandBuffer m_commandBuffer;
	};

	class Device
	{
	public:
		~Device() = default;
		friend class Instance;

		/**
		Waits for all queued operations to complete

		\todo Handle a lost device
		*/
		void Join();

		/**
		Creates a pipeline layout - reusable across multiple pipelines
		
		\param[in] bindings 
		\return If successful, a pointer to the resulting pipeline layout. If failed, `nullptr`.
		*/
		std::shared_ptr<PipelineLayout> CreatePipelineLayout(const std::vector<Descriptor> &bindings);
		
		/**
		Creates a shader module from SPIR-V bytecode
		
		\param[in] bytecode The bytecode to use
		\return If successful, a pointer to the resulting shader module. If failed, `nullptr`.
		*/
		std::shared_ptr<Shader> CreateShaderFromSPIRV(const std::vector<uint32_t> &bytecode);
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
		\param[in] stages A list of shaders to bind to different stages of the pipeline
		\param[in] patchCtrlPoints Tesselation patch control points. Defaults to 0 (not using tesselation)
		\return If successful, a pointer to the resulting graphics pipeline. If failed, `nullptr`.

		\todo Parameterize render subpass index
		*/
		std::shared_ptr<GraphicsPipeline> CreateGraphicsPipeline(const std::shared_ptr<FrameBuffer> &frameBuffer, const std::shared_ptr<PipelineLayout> &layout, const std::vector<ShaderStage> &stages, uint32_t patchCtrlPoints = 0);

		/**
		Creates a swap chain
		
		\param[in] resolution The resolution to use. Might be ignored, depending on GPU quirks.
		\param[in] numBuffers The number of buffers to use. Defaults to 2 (double-buffered). Might be clamped to fit the GPU's capabilities.
		\return If successful, a pointer to the resulting swap chain. If failed, `nullptr`.
		*/
		std::shared_ptr<SwapChain> CreateSwapChain(glm::uvec2 resolution, uint32_t numBuffers);
		
		/**
		Creates a command buffer

		\param[in] poolIndex Which pool to allocate from
		\param[in] bundle Is this command buffer a secondary command buffer? Defaults to false.
		\return If successful, a pointer to the resulting command buffer. If failed, `nullptr`.

		\todo Queue index parameter
		*/
		std::shared_ptr<CommandBuffer> CreateCommandBuffer(uint32_t poolIndex, bool bundle = false);

		/**
		Creates a frame buffer
		
		\param[in] swapChain The primary render target
		\param[in] enableDepth Create a depth buffer?
		\return If successful, a pointer to the resulting frame buffer. If failed, `nullptr`.
		*/
		std::shared_ptr<FrameBuffer> CreateFrameBuffer(const std::vector<AttachmentDesc> &images, bool enableDepth);
		
		/**
		Executes pre-recorded commands stored in a command bundle

		\param[in] commands A list of command buffers to execute
		\return If the command buffers were valid, `true`. If execution failed, `false`.
		*/
		bool ExecuteCommands(const std::vector<std::shared_ptr<CommandBuffer>> &commands);

		/**
		Prepares a backbuffer for presenting

		\param[in] swapChain The swap chain to prepare
		\return If successful, `true`. If failed, `false`.
		*/
		bool PrePresent(const std::shared_ptr<SwapChain> &swapChain);
		/**
		Presents the most recent backbuffer
		
		\param[in] swapChain The swap chain to present
		\return If successful, `true`. If failed, `false`.

		\todo Enable/disable vsync?
		*/
		bool Present(const std::shared_ptr<SwapChain> &swapChain);
		/**
		Prepares a backbuffer for rendering

		\param[in] swapChain The swap chain to prepare
		\return If successful, `true`. If failed, `false`.
		*/
		bool PostPresent(const std::shared_ptr<SwapChain> &swapChain);
	private:
		Device();

		void Release(); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used

		GpuProperties m_gpuProps;
		PresentableSurface m_targetSurface;

		VkDevice m_device;
		std::array<VkQueue, numQueues> m_queues;
		std::array<VkCommandPool, numQueues> m_commandPools;
		VkSemaphore m_presentComplete, m_renderComplete;
		VkCommandBuffer m_cmdPrePresent, m_cmdPostPresent;
		VkSubmitInfo m_submitInfo;

		//VK_KHR_swapchain function pointers
		PFN_vkCreateSwapchainKHR pfnCreateSwapchainKHR;
		PFN_vkDestroySwapchainKHR pfnDestroySwapchainKHR;
		PFN_vkGetSwapchainImagesKHR pfnGetSwapchainImagesKHR;
		PFN_vkAcquireNextImageKHR pfnAcquireNextImageKHR;
		PFN_vkQueuePresentKHR pfnQueuePresentKHR;
		/*VK_KHR_display function pointers
		PFN_vkGetPhysicalDeviceDisplayPropertiesKHR pfnGetPhysicalDeviceDisplayPropertiesKHR;
		PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR pfnGetPhysicalDeviceDisplayPlanePropertiesKHR;
		PFN_vkGetDisplayPlaneSupportedDisplaysKHR pfnGetDisplayPlaneSupportedDisplaysKHR;
		PFN_vkGetDisplayModePropertiesKHR pfnGetDisplayModePropertiesKHR;
		PFN_vkCreateDisplayModeKHR pfnCreateDisplayModeKHR;
		PFN_vkGetDisplayPlaneCapabilitiesKHR pfnGetDisplayPlaneCapabilitiesKHR;
		PFN_vkCreateDisplayPlaneSurfaceKHR pfnCreateDisplayPlaneSurfaceKHR;
		//VK_KHR_display_swapchain function pointers
		PFN_vkCreateSharedSwapchainsKHR pfnCreateSharedSwapchainsKHR;*/
		

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

		\return The number of GPUs Vulkan found
		*/
		uint32_t FindGpus();

		/**
		Fetches details about a given GPU

		\param[in] gpuIndex Which GPU to query
		\return If successful, details about the requested GPU. If failed, `nullptr`.
		*/
		const GpuProperties *GetGpuProperties(uint32_t gpuIndex);
		
		/**
		Creates a Vulkan device on the specified GPU, presenting to a given window

		\param[in] gpuIndex Which GPU to target
		\param[in] hWnd The Win32 window handle to present to
		\param[in] hInstance The Win32 instance handle to use
		\return If successful, a pointer to the resulting device. If failed, `nullptr`.

		\todo Multithreaded command pools
		*/
		std::shared_ptr<Device> CreateDeviceOnWindow(uint32_t gpuIndex, HWND hWnd, HINSTANCE hInstance);
	private:
		Instance();
		void Release(); //Custom deallocator for shared_ptr. Calls Vulkan's vkDestroy... functions to free the memory used



		VkInstance m_instance;
		std::vector<GpuProperties> m_gpuProps;
		std::vector<VkPhysicalDevice> m_gpus;

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
	std::shared_ptr<Instance> Initialize(const std::string &appName, uint32_t appVersion);
}

#endif
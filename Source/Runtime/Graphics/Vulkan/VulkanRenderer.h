#pragma once
#include <Runtime/Engine/RenderInterface.h>
#include <Runtime/Memory/ResourceManagerImp.h>
#include <Runtime/Memory/ResourceManager.h>
#include <Runtime/Memory/Vulkan/VulkanResourceManager.h>
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanTextureView.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"

struct VulkanInitializerList
{
	// all the features that should be enabled in vulkan(ex: tessellationShaders, multiviewporting, etc..)
	VkPhysicalDeviceFeatures EnabledFeatures;
	
	// all instance layers that should be enabled in vulkan(ex: renderdoc capture)
	const char** InstanceLayers;

	// the count of instance layers provided in 'inInstanceLayers'
	uint32 InstanceLayersCount;

	// all instance extensions that should be enabled on vulkan(ex: debug_utils extension)
	const char** InstanceExtensions;

	// the count of instance extensions provided in 'inInstanceExtensions'
	uint32 InstanceExtensionCount;

	// all device extensions that should be enabled on vulkan(ex: descriptor indexing, multiview, etc..)
	const char** DeviceExtensions;

	// the count of device extensions provided in 'inDeviceExtensions'
	uint32 DeviceExtensionCount;
};

/**
* A vulkan renderer uses the Vulkan Graphics API to render a world, debug objects primitives, etc...
*/
class VulkanRenderer : public RenderInterface
{
private:
	VkInstance VulkanInstance;
	VkPhysicalDevice PhysicalDevice;

	VulkanSurface* Surface;

	// Logical device handle -> wrapped into a class
	VulkanDevice* Device;
	VulkanCommandPool* CommandPool;

	std::vector<std::string> SupportedInstanceExtensions;
	std::vector<std::string> SupportedInstanceLayers;

	// Depth buffer format (selected during Vulkan initialization)
	VkFormat DepthFormat;

	/* Synchronization */
	VkSemaphore PresentationComplete; // Swap chain image presentation
	VkSemaphore RenderComplete; // Command buffer submission and execution

	/** Pipeline stages used to wait at for graphics queue submissions */
	VkPipelineStageFlags SubmitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	// swapchain
	VulkanSwapChain* Swapchain;

	/* Depth and Stencil buffering */
	VulkanTextureView* DepthStencilView;

	/* Renderpass */
	VulkanRenderLayout* RenderPassLayout;
	VulkanRenderPass* RenderPass;// VkRenderPass RenderPass;

	/* Pipeline cache */
	VkPipelineCache PipelineCache;

	// List of available frame buffers (same as number of swap chain images)
	std::vector<VulkanFrameBuffer*> FrameBuffers;

	// Active frame buffer index
	uint32 CurrentBuffer = 0;

	// Main memory heap for all vulkan allocation, (Index, Vertex, storage buffers, etc...)
	VulkanMemoryHeap* MainVulkanMemoryHeap;

	// Create the pipeline layout, since we have no push constants nor descriptor sets, we just want an empty layout
	VulkanPipelineLayout* PipelineLayout;

	// Create the graphics pipeline 
	VulkanGraphicsPipeline* GraphicsPipeline;

	/* Vulkan Resource Managements	*/
	IResourceManager* MainVulkanResourceManager;
	ResourceManager* GraphicsResourceManager;
	VulkanShaderFactory* ShaderFactory;

	// Current Render Viewport
	RenderViewportSize ViewportSize;

	VulkanVertexShader* VertShader;
	VulkanFragmentShader* PixelShader;

	/*---------------------------------------------------------------------
	* ImGui 
	* ----------------------------------------------------------------------
	*/
	VulkanTextureView* ImguiFontTextureView;
	VkSampler ImguiSampler;

	VulkanDescriptorPool* ImguiDescriptorPool;
	VulkanDescriptorSetsLayout* ImguiDescriptorSetsLayout;
	VkDescriptorSet ImguiDescriptorSet;

	VkPipelineCache ImguiPipelineCache;
	VulkanPipelineLayout* ImguiPipelineLayout;
	//VkPipelineLayout ImguiPipelineLayout;

	VulkanVertexShader* ImguiVertexShader;
	VulkanFragmentShader* ImguiPixelShader;

	//VkPipeline ImguiPipeline;
	VulkanGraphicsPipeline* ImguiPipeline;

	uint32 ImguiVertexCount;
	uint32 ImguiIndexCount;

	VkBuffer ImguiVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory ImguiVertexBufferData = nullptr;

	VkBuffer ImguiIndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory ImguiIndexBufferData = nullptr;

	void* ImguiVertexBufferMapped = nullptr;
	void* ImguiIndexBufferMapped = nullptr;

public:
	VulkanRenderer();
	~VulkanRenderer();

	/**
	* Initializes vulkan for basic rendering 
	*/
	virtual bool Init(const RendererInitializerList& inRenderInitializerList) override;

	/**
	* Begins a new render frame
	*/
	virtual void BeginRenderFrame() override;

	/**
	* Renders a game world
	*
	* @param inGameWorld - the world to render
	*/
	virtual void Render(GameWorld* inGameWorld) override;

	/**
	* End the render frame | MUST BE CALLED IF BEGIN RENDER FRAME WAS CALLED BEFORE CALLING ANOTHER BEGIN
	*/
	virtual void EndRenderFrame() override;

	/**
	* Begins a new render frame for editor GUI
	*/
	virtual void BeginEditorGuiRenderFrame() override;

	/**
	* End the render frame for editor GUI | MUST BE CALLED IF BEGIN RENDER FRAME WAS CALLED BEFORE CALLING ANOTHER BEGIN
	*/
	virtual void EndEditorGuiRenderFrame() override;
	
	/**
	* Called when the render viewport is resized 
	* 
	* @param inNewViewportSize - new viewport size of the render viewport 
	*/
	virtual void OnRenderViewportResized(RenderViewportSize& inNewViewportSize) override;

	/**
	* Shutdowns Vulkan renderer 
	*/
	virtual void Shutdown() override;

private:
	/**
	* Initliazes vulkan from ground up, creating a logical device, swapchain, picking a physical device, etc...	
	* @param inVulkanInitializerList - initialize list for initializiing vulkan 
	* @returns bool - true if vulkan was initialized successfully, false otherwise 
	*/
	bool InitVulkan(VulkanInitializerList& inVulkanInitializerList);

	/**
	* Creates a vulkan instance from inVulkanInitializerList
	* @returns bool - true if vulkan instance was created successfully, false otherwise 
	*/
	bool CreateVulkanInstance(VulkanInitializerList& inVulkanInitializerList);

	/**
	* Initializes imgui, binds it to vulkan api and gets it ready for rendering and usage 
	*/
	bool InitImGui();

	/** 
	* Update vertex and index buffer containing the imGui elements when required 
	*/
	void UpdateImguiBuffers();

	/**
	* Draws a imgui frame to the command buffer passed in
	* 
	* @param inCommandBuffer imgui frame will be drawn to this command buffer 
	*/
	void DrawImguiFrame(VulkanCommandBuffer* inCommandBuffer);
};

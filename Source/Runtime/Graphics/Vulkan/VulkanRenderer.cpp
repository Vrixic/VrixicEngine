/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#include "VulkanRenderer.h"
#include <Core/Application.h>
#include <External/imgui/Includes/imgui.h>
#include <Runtime/Graphics/Vulkan/VulkanUtils.h>
#include <Runtime/Core/Math/Vector2D.h>
#include <Runtime/Core/Math/VrixicMathHelper.h>

#include <External/glfw/Includes/GLFW/glfw3.h>

#define RENDER_DOC 0

VulkanRenderer* VulkanRenderer::InstanceHandle = nullptr;

// UI params are set via push constants
struct PushConstBlock {
	Vrixic::Math::Vector2D scale;
	Vrixic::Math::Vector2D translate;
} pushConstBlock;

VulkanRenderer::VulkanRenderer()
	: VulkanInstance(VK_NULL_HANDLE), PhysicalDevice(VK_NULL_HANDLE), Surface(nullptr), Device(nullptr), CommandPool(nullptr),
	DepthFormat(VK_FORMAT_UNDEFINED), PresentationComplete(VK_NULL_HANDLE), RenderComplete(VK_NULL_HANDLE), Swapchain(nullptr), DepthStencilView(nullptr),
	RenderPassLayout(nullptr), RenderPass(nullptr), PipelineCache(VK_NULL_HANDLE), MainVulkanMemoryHeap(nullptr),
	PipelineLayout(nullptr), GraphicsPipeline(nullptr), MainVulkanResourceManager(nullptr), GraphicsResourceManager(nullptr),
	ShaderFactory(nullptr),

#ifdef VULKAN_STANDALONE
	ImguiFontTextureView(nullptr), ImguiSampler(VK_NULL_HANDLE), ImguiDescriptorPool(nullptr), ImguiDescriptorSetsLayout(nullptr),
	ImguiDescriptorSet(VK_NULL_HANDLE), ImguiPipelineCache(VK_NULL_HANDLE), ImguiPipelineLayout(nullptr),
	ImguiVertexShader(nullptr), ImguiPixelShader(nullptr), ImguiPipeline(nullptr), ImguiVertexCount(0), ImguiIndexCount(0),
	ImguiVertexBuffer(nullptr), ImguiIndexBuffer(nullptr),
#endif // #ifdef VULKAN_STANDALONE

	VertShader(nullptr), PixelShader(nullptr)
{
	ViewportSize = { 0, 0 };

	VE_ASSERT(InstanceHandle == nullptr, "Only one Vulkan renderer can exists at a time!");
	InstanceHandle = this;
}

VulkanRenderer::~VulkanRenderer()
{
	Shutdown();
}

bool VulkanRenderer::Init(const RendererInitializerList& inRenderInitializerList)
{
#ifdef VULKAN_GLFW
	InitVulkanGLFW(inRenderInitializerList);
#else
	InitVulkanStandalone(inRenderInitializerList);
#endif // GLFW_VULKAN_WINDOW

	return true;
}

void VulkanRenderer::BeginRenderFrame()
{
	// Firstly complete that last command buffer draw commands 
	VulkanCommandBuffer* LastCommandBuffer = CommandPool->GetCommandBuffer(CurrentBuffer);

	// Use a fence to wait until the command buffer has finished execution before using it again
	// Start of frame we would want to wait until last frame has finished 
	LastCommandBuffer->SetWaitFence();

	// SRS - on other platforms use original bare code with local semaphores/fences for illustrative purposes
	// Get next image in the swap chain (back/front buffer)
	VkResult Acquire = Swapchain->AcquireNextImage(LastCommandBuffer, &CurrentBuffer);// VTemp->AcquireNextImage(VTemp->PresentationComplete, &VTemp->CurrentBuffer);
	if (!((Acquire == VK_SUCCESS) || (Acquire == VK_SUBOPTIMAL_KHR))) {
		VK_CHECK_RESULT(Acquire, "[EntryPoint]: Could not aquire next swapchain image!");
	}

	//// Get the new command buffer 
	//VulkanCommandBuffer* CurrentCommandBuffer = CommandPool->GetCommandBuffer(CurrentBuffer);
	//
	//// Begin the buffer to start queuing in draw command/requests
	//CurrentCommandBuffer->BeginCommandBuffer();
	//
	//// Start the first sub pass specified in our default render pass setup by the base class
	//// This will clear the color and depth attachment
	//CurrentCommandBuffer->BeginRenderPass(RenderPass, FrameBuffers[CurrentBuffer]);
}

void VulkanRenderer::BeginRecordingDrawCommands(int32 inCommandBufferIndex)
{
}

void VulkanRenderer::BeginCommandBuffer()
{
	VulkanCommandBuffer* CurrentCommandBuffer = CommandPool->GetCommandBuffer(CurrentBuffer);

	// Begin the buffer to start queuing in draw command/requests
	CurrentCommandBuffer->BeginCommandBuffer();
}

void VulkanRenderer::BeginRenderPass(VulkanRenderPass* inRenderPass)
{
	VulkanCommandBuffer* CurrentCommandBuffer = CommandPool->GetCommandBuffer(CurrentBuffer);

	// Start the first sub pass specified in our default render pass setup by the base class
	// This will clear the color and depth attachment
	CurrentCommandBuffer->BeginRenderPass(RenderPass, FrameBuffers[CurrentBuffer]);
}

void VulkanRenderer::BeginRenderPass(VkRenderPassBeginInfo& inInfo)
{
	VulkanCommandBuffer* CurrentCommandBuffer = CommandPool->GetCommandBuffer(CurrentBuffer);
	vkCmdBeginRenderPass(*CurrentCommandBuffer->GetCommandBufferHandle(), &inInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderer::Render(GameWorld* inGameWorld)
{
	VulkanCommandBuffer* CommandBuffer = CommandPool->GetCommandBuffer(CurrentBuffer);

	// Update dynamic viewport state
	VkViewport viewport = {};
	viewport.height = (float)ViewportSize.Height;
	viewport.width = (float)ViewportSize.Width;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(*CommandBuffer->GetCommandBufferHandle(), 0, 1, &viewport);

	// Update dynamic scissor state
	VkRect2D scissor = {};
	scissor.extent.width = ViewportSize.Width;
	scissor.extent.height = ViewportSize.Height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(*CommandBuffer->GetCommandBufferHandle(), 0, 1, &scissor);

	VkDeviceSize offsets[1] = { 0 };

	// Bind the rendering pipeline
	// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
	vkCmdBindPipeline(*CommandBuffer->GetCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, *GraphicsPipeline->GetPipelineHandle());

	// Bind Vertex Buffer
	//vkCmdBindVertexBuffers(*CommandBuffer->GetCommandBufferHandle(),
	//	0, 1, VertexBuffer->GetBufferHandle(), offsets);
	//
	//vkCmdBindIndexBuffer(*CommandBuffer->GetCommandBufferHandle(),
	//	*IndexBuffer->GetBufferHandle(), 0, VK_INDEX_TYPE_UINT32);
	//
	//vkCmdDrawIndexed(*CommandBuffer->GetCommandBufferHandle(),
	//	3, 1, 0, 0, 1);

	// Bind descriptor sets describing shader binding points
	//vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	// Bind the rendering pipeline
	// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
	//vkCmdBindPipeline(DrawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	// Bind triangle vertex buffer (contains position and colors)
	//vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, &vertices.buffer, offsets);

	// Bind triangle index buffer
	//vkCmdBindIndexBuffer(drawCmdBuffers[i], indices.buffer, 0, VK_INDEX_TYPE_UINT32);

	// Draw indexed triangle
	//vkCmdDrawIndexed(drawCmdBuffers[i], indices.count, 1, 0, 0, 1);
}

void VulkanRenderer::EndRecordingDrawCommands(int32 inCommandBufferIndex)
{
}

void VulkanRenderer::EndCommandBuffer()
{
	VulkanCommandBuffer* CurrentCommandBuffer = CommandPool->GetCommandBuffer(CurrentBuffer);
	CurrentCommandBuffer->EndCommandBuffer();
}

void VulkanRenderer::EndRenderPass()
{
	VulkanCommandBuffer* CurrentCommandBuffer = CommandPool->GetCommandBuffer(CurrentBuffer);
	vkCmdEndRenderPass(*CurrentCommandBuffer->GetCommandBufferHandle());
}

void VulkanRenderer::EndRenderFrame()
{
	VulkanCommandBuffer* CurrentCommandBuffer = CommandPool->GetCommandBuffer(CurrentBuffer);

	//// Firstly end the current command buffers submission for drawing commands
	//CurrentCommandBuffer->EndRenderPass();
	//CurrentCommandBuffer->EndCommandBuffer();

	/* after waiting reset the fence */
	CurrentCommandBuffer->ResetWaitFence();

	// Submit to the graphics queue passing a wait fence
	Device->GetGraphicsQueue()->SubmitQueue(CurrentCommandBuffer, &RenderComplete);

	// Present the current buffer to the swap chain
	// Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
	// This ensures that the image is not presented to the windowing system until all commands have been submitted
	VkResult present = Swapchain->QueuePresent(Device->GetPresentQueue(), &RenderComplete, CurrentBuffer);// VTemp->QueuePresent(VTemp->Device->GetPresentQueue()->GetQueueHandle(), VTemp->CurrentBuffer, VTemp->RenderComplete);
	if (!((present == VK_SUCCESS) || (present == VK_SUBOPTIMAL_KHR))) {
		VK_CHECK_RESULT(present, "[EntryPoint]: Failed to present an image!");
	}
}

void VulkanRenderer::OnRenderViewportResized(RenderViewportSize& inNewViewportSize)
{
	// Ensure all operations on the device have been finished before destroying resources
	Device->WaitUntilIdle();

	// Recreate swap chain
	ViewportSize = inNewViewportSize;

	Swapchain->Recreate(false, &ViewportSize.Width, &ViewportSize.Height);
	{
		delete DepthStencilView;

		VkImageCreateInfo ImageCreateInfo = VulkanUtils::Initializers::ImageCreateInfo();
		ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageCreateInfo.format = DepthFormat;
		ImageCreateInfo.extent = { ViewportSize.Width, ViewportSize.Height, 1 };
		ImageCreateInfo.mipLevels = 1;
		ImageCreateInfo.arrayLayers = 1;
		ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		DepthStencilView = new VulkanTextureView(Device, ImageCreateInfo);

		VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
		if (DepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
			AspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		DepthStencilView->CreateImageView(VK_IMAGE_VIEW_TYPE_2D, DepthFormat, 0, 1, 0, 1, AspectFlags);
	}

	/* Update Renderpasses render area to new area since window was resized */
	VkRect2D RenderArea = { 0,0, ViewportSize.Width, ViewportSize.Height };
	RenderPass->UpdateRenderArea(RenderArea);
	RenderPass->UpdateExtent2D(RenderArea.extent);

	{
		for (uint32 i = 0; i < FrameBuffers.size(); ++i)
		{
			FrameBuffers[i]->DestroyBuffer();
		}

		VkImageView Attachments[2];

		// Depth/Stencil attachment is the same for all frame buffers
		Attachments[1] = *DepthStencilView->GetImageViewHandle();

		VkExtent2D Extent = { ViewportSize.Width, ViewportSize.Height };

		FrameBuffers.resize(Swapchain->GetImageCount());
		for (uint32 i = 0; i < Swapchain->GetImageCount(); ++i)
		{
			Attachments[0] = Swapchain->GetSwapchainBuffer(i)->View;
			FrameBuffers[i]->AllocateBuffer(2, Attachments, &Extent);
		}
	}

	// Command buffers need to be recreated as they may store
	// references to the recreated frame buffer
	CommandPool->DestroyBuffers();

	// Create command buffers and fences
	{
		// Create one command buffer for each swap chain image and reuse for rendering
		for (uint32 i = 0; i < Swapchain->GetImageCount(); ++i)
		{
			VulkanCommandBuffer* CommandBuffer = CommandPool->CreateCommandBuffer(i);
			CommandBuffer->AllocateCommandBuffer();
		}

		for (uint32 i = 0; i < Swapchain->GetImageCount(); ++i)
		{
			CommandPool->GetCommandBuffer(i)->AddWaitSemaphore(&PresentationComplete);
		}
	}

	// Now recreate new command buffers
	//BuildCommandBuffers();
}

void VulkanRenderer::Shutdown()
{
	InstanceHandle = nullptr;

	Device->WaitUntilIdle();

	vkDestroyPipelineCache(*Device->GetDeviceHandle(), PipelineCache, nullptr);

	for (uint32 i = 0; i < FrameBuffers.size(); ++i)
	{
		delete FrameBuffers[i];
	}

	delete CommandPool;

	delete DepthStencilView;

	delete RenderPassLayout;
	delete RenderPass;
	delete Swapchain;

	vkDestroySemaphore(*Device->GetDeviceHandle(), PresentationComplete, nullptr);
	vkDestroySemaphore(*Device->GetDeviceHandle(), RenderComplete, nullptr);

#ifdef VULKAN_STANDALONE
	/* Imgui */

	delete ImguiFontTextureView;
	vkDestroySampler(*Device->GetDeviceHandle(), ImguiSampler, nullptr);

	delete ImguiDescriptorPool;
	delete ImguiDescriptorSetsLayout;

	vkDestroyPipelineCache(*Device->GetDeviceHandle(), ImguiPipelineCache, nullptr);
	delete ImguiPipelineLayout;

	delete ImguiVertexShader;
	delete ImguiPixelShader;

	delete ImguiPipeline;

	vkDestroyBuffer(*Device->GetDeviceHandle(), ImguiVertexBuffer, nullptr);
	vkFreeMemory(*Device->GetDeviceHandle(), ImguiVertexBufferData, nullptr);

	vkDestroyBuffer(*Device->GetDeviceHandle(), ImguiIndexBuffer, nullptr);
	vkFreeMemory(*Device->GetDeviceHandle(), ImguiIndexBufferData, nullptr);

	/* Imgui */
#endif

	delete MainVulkanMemoryHeap;

	delete PixelShader;
	delete VertShader;
	delete ShaderFactory;
	delete GraphicsResourceManager;
	delete MainVulkanResourceManager;

	delete PipelineLayout;
	delete GraphicsPipeline;

	delete Surface;
	delete Device;

	vkDestroyInstance(VulkanInstance, nullptr);
}

#ifdef VULKAN_STANDALONE
bool VulkanRenderer::InitVulkanStandalone(const RendererInitializerList& inRenderInitializerList)
{
	ViewportSize = inRenderInitializerList.ViewportSize;

	VkResult Result;

	// Firstly Initlaize Vulkan
	VulkanInitializerList VulkanInitList = { 0 };

	// Select all the feature to enable 
	VkPhysicalDeviceFeatures EnabledFeatures = { };
	EnabledFeatures.tessellationShader = VK_TRUE;
	EnabledFeatures.geometryShader = VK_TRUE;
	EnabledFeatures.fillModeNonSolid = VK_TRUE;
	EnabledFeatures.samplerAnisotropy = VK_TRUE; //MSAA
	EnabledFeatures.multiViewport = VK_TRUE;

	VulkanInitList.EnabledFeatures = EnabledFeatures;

	// All instance extensions to enable 
	const uint32 InstanceExtensionCount = 1;
	const char* InstanceExtensions[InstanceExtensionCount]
	{
		"VK_EXT_debug_utils"
	};

	VulkanInitList.InstanceExtensions = InstanceExtensions;
	VulkanInitList.InstanceExtensionCount = InstanceExtensionCount;

	// All instance layers to enable 
	const uint32 InstanceLayerCount = 1;
	const char* InstanceLayers[InstanceLayerCount]
	{

#if RENDER_DOC
		"VK_LAYER_RENDERDOC_Capture",
#else
		"VK_LAYER_KHRONOS_validation",
#endif
	};

	VulkanInitList.InstanceLayers = InstanceLayers;
	VulkanInitList.InstanceLayersCount = InstanceLayerCount;

	// All device extensions to enable 
	const uint32 DeviceExtensionsCount = 2;
	const char* DeviceExtensions[DeviceExtensionsCount]
	{
		"VK_EXT_descriptor_indexing",
		"VK_KHR_multiview"
	};

	VulkanInitList.DeviceExtensions = DeviceExtensions;
	VulkanInitList.DeviceExtensionCount = DeviceExtensionsCount;

	// Initialize Vulkan
	if (!CreateVulkanInstance(VulkanInitList))
	{
		ASSERT(false, "Vulkan initialization failed...");
		return false;
	}

	VE_CORE_LOG_INFO("Successfully created an Instance..");

	// Create Vulkan Surface and Device
	{
		// Physical Device
		uint32 PhysicalDevicesCount = 0;

		// Get Number of available physical devices
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(VulkanInstance, &PhysicalDevicesCount, nullptr), "[EntryPoint]: No physical devices (GPU) found!");
		if (PhysicalDevicesCount == 0)
		{
			VE_CORE_LOG_ERROR("No device with Vulkan support found");
			return false;
		}

		// Enumerate physical devices
		std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDevicesCount);
		Result = (vkEnumeratePhysicalDevices(VulkanInstance, &PhysicalDevicesCount, PhysicalDevices.data()));
		if (Result != VK_SUCCESS)
		{
			VE_CORE_LOG_ERROR("Could not enumerate physical devices");
			return false;
		}

		// GPU Selection
		VulkanUtils::Helpers::GetBestPhysicalDevice(PhysicalDevices.data(), PhysicalDevicesCount, PhysicalDevice);

		// Find a suitable depth format
		VkBool32 ValidDepthFormat = VulkanUtils::Helpers::GetSupportedDepthFormat(PhysicalDevice, &DepthFormat);
		assert(ValidDepthFormat);
		Device = new VulkanDevice(PhysicalDevice, VulkanInitList.EnabledFeatures, VulkanInitList.DeviceExtensionCount, VulkanInitList.DeviceExtensions);
		Surface = new VulkanSurface(
			Device, &VulkanInstance, static_cast<HINSTANCE*>(inRenderInitializerList.NativeWindowInstanceHandle),
			static_cast<HWND*>(inRenderInitializerList.NativeWindowHandle)
		);
		Device->CreateDevice(Surface);
	}

	// Create Swapchain
	Swapchain = new VulkanSwapChain(Device, Surface, inRenderInitializerList.ViewportSize.Width, inRenderInitializerList.ViewportSize.Height);

	// Create Command Buffer pool and command buffers
	{
		// Create a default command pool for graphics command buffers
		CommandPool = new VulkanCommandPool(Device);
		CommandPool->CreateCommandPool(Device->GetGraphicsQueue()->GetQueueIndex());

		// Create command buffers
		{
			// Create one command buffer for each swap chain image and reuse for rendering
			for (uint32 i = 0; i < Swapchain->GetImageCount(); ++i)
			{
				VulkanCommandBuffer* CommandBuffer = CommandPool->CreateCommandBuffer(i);
				CommandBuffer->AllocateCommandBuffer();
			}

			VE_CORE_LOG_INFO("Successfully created draw command buffers...");
		}
	}

	// Create synchronization objects (Semaphores)
	{
		// Semaphores (Used for correct command ordering)
		VkSemaphoreCreateInfo SemaphoreCreateInfo = VulkanUtils::Initializers::SemaphoreCreateInfo(nullptr);

		// Create a semaphore used to synchronize image presentation
		// Ensures that the image is displayed before we start submitting new commands to the queue
		VK_CHECK_RESULT(vkCreateSemaphore(*Device->GetDeviceHandle(), &SemaphoreCreateInfo, nullptr, &PresentationComplete), "[EntryPoint]: Failed to create a semaphore for image presentation!");

		for (uint32 i = 0; i < Swapchain->GetImageCount(); ++i)
		{
			CommandPool->GetCommandBuffer(i)->AddWaitSemaphore(&PresentationComplete);
		}

		// Create a semaphore used to synchronize command submission
		// Ensures that the image is not presented until all commands have been submitted and executed
		VK_CHECK_RESULT(vkCreateSemaphore(*Device->GetDeviceHandle(), &SemaphoreCreateInfo, nullptr, &RenderComplete), "[EntryPoint]: Failed to create a semaphore for render synchronization!");
	}

	// Setting up depth and stencil buffers
	{
		VkImageCreateInfo ImageCreateInfo = VulkanUtils::Initializers::ImageCreateInfo();
		ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageCreateInfo.format = DepthFormat;
		ImageCreateInfo.extent = { inRenderInitializerList.ViewportSize.Width, inRenderInitializerList.ViewportSize.Height, 1 };
		ImageCreateInfo.mipLevels = 1;
		ImageCreateInfo.arrayLayers = 1;
		ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		DepthStencilView = new VulkanTextureView(Device, ImageCreateInfo);

		VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
		if (DepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
			AspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		DepthStencilView->CreateImageView(VK_IMAGE_VIEW_TYPE_2D, DepthFormat, 0, 1, 0, 1, AspectFlags);
		VE_CORE_LOG_INFO("Successfully created depth stencil buffers...");
	}

	// Setting up render pass
	{
		std::vector<VkAttachmentDescription> Attachments = { };
		Attachments.resize(2);
		// Color attachment
		Attachments[0].format = *Surface->GetColorFormat();//ColorFormat;
		Attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		Attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		Attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		Attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		Attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		Attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// Depth attachment
		Attachments[1].format = DepthFormat;
		Attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		Attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		Attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		Attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		Attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		Attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference ColorReference = {};
		ColorReference.attachment = 0;
		ColorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference DepthReference = {};
		DepthReference.attachment = 1;
		DepthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkRect2D RenderArea = { 0,0, inRenderInitializerList.ViewportSize.Width, inRenderInitializerList.ViewportSize.Height };
		RenderPassLayout = new VulkanRenderLayout(Device, 1, RenderArea, &RenderArea.extent);
		RenderPassLayout->SetAttachments(Attachments);
		RenderPassLayout->SetColorReference(ColorReference);
		RenderPassLayout->SetDepthReference(DepthReference);

		// Set clear values for all framebuffer attachments with loadOp set to clear
		// We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
		std::vector<VkClearValue> ClearValues;
		ClearValues.resize(2);
		ClearValues[0].color = { { 0.0f, 0.0f, 0.2f, 1.0f } };
		ClearValues[1].depthStencil = { 1.0f, 0 };
		RenderPassLayout->SetClearValues(ClearValues);

		// Subpass dependencies for layout transitions
		std::vector<VkSubpassDependency> SubpassDependency;
		SubpassDependency.resize(2);

		SubpassDependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		SubpassDependency[0].dstSubpass = 0;
		SubpassDependency[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		SubpassDependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		SubpassDependency[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		SubpassDependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		SubpassDependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		SubpassDependency[1].srcSubpass = 0;
		SubpassDependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		SubpassDependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		SubpassDependency[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		SubpassDependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		SubpassDependency[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		SubpassDependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		RenderPass = new VulkanRenderPass(Device, *RenderPassLayout, SubpassDependency);
		VE_CORE_LOG_INFO("Successfully created renderpass...");
	}

	// Create Pipeline cache
	{
		VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {};
		PipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VK_CHECK_RESULT(vkCreatePipelineCache(*Device->GetDeviceHandle(), &PipelineCacheCreateInfo, nullptr, &PipelineCache), "[EntryPoint]: Failed to create a pipeline cache!");
		VE_CORE_LOG_INFO("Successfully created pipeline cache...");
	}

	// Create Frame buffers
	{
		VkImageView Attachments[2];

		// Depth/Stencil attachment is the same for all frame buffers
		Attachments[1] = *DepthStencilView->GetImageViewHandle();

		VkExtent2D Extent = { inRenderInitializerList.ViewportSize.Width, inRenderInitializerList.ViewportSize.Height };

		FrameBuffers.resize(Swapchain->GetImageCount());
		for (uint32 i = 0; i < Swapchain->GetImageCount(); ++i)
		{
			Attachments[0] = Swapchain->GetSwapchainBuffer(i)->View;
			FrameBuffers[i] = new VulkanFrameBuffer(Device, RenderPass);
			FrameBuffers[i]->AllocateBuffer(2, Attachments, &Extent);
		}

		VE_CORE_LOG_INFO("Successfully created framebuffers...");
	}

	// allocate 1 gibibytes of memory -> 1024 mebibytes = 1 gib
	MainVulkanMemoryHeap = new VulkanMemoryHeap(Device, 1);

	// Prepare the vulkan pipeline
	{
		// Create the pipeline layout, since we have no push constants nor descriptor sets, we just want an empty layout
		PipelineLayout = new VulkanPipelineLayout(Device);
		PipelineLayout->CreateEmpty();

		// Create the graphics pipeline 
		GraphicsPipeline = new VulkanGraphicsPipeline(Device);
		VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo = VulkanUtils::Initializers::GraphicsPipelineCreateInfo();

		MainVulkanResourceManager = new VulkanResourceManager(Device);
		GraphicsResourceManager = new ResourceManager(MainVulkanResourceManager);
		ShaderFactory = new VulkanShaderFactory(GraphicsResourceManager);

		const char VertexShaderStr[] = "float4 main(float3 inVertex : POSITION) : SV_POSITION { return float4(inVertex, 1.0f); }";
		const char PixelShaderStr[] = "float4 main(float4 inPosition : SV_POSITION) : SV_TARGET { return float4(1.0f, 0.0f, 0.0f, 1.0f); }";

		VertShader = ShaderFactory->CreateVertexShaderFromString(Device, VertexShaderStr, true);
		PixelShader = ShaderFactory->CreateFragmentShaderFromString(Device, PixelShaderStr, true);

		{
			VkPipelineShaderStageCreateInfo VertexStageCreateInfo = VulkanUtils::Initializers::PipelineShaderStageCreateInfo();
			VertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			VertexStageCreateInfo.module = *(VkShaderModule*)GraphicsResourceManager->GetShaderModule(VertShader->GetShaderKey());
			VertexStageCreateInfo.pName = "main";

			VkPipelineShaderStageCreateInfo PixelStageCreateInfo = VulkanUtils::Initializers::PipelineShaderStageCreateInfo();
			PixelStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			PixelStageCreateInfo.module = *(VkShaderModule*)GraphicsResourceManager->GetShaderModule(PixelShader->GetShaderKey());;
			PixelStageCreateInfo.pName = "main";

			VkPipelineShaderStageCreateInfo ShaderStages[2] = { VertexStageCreateInfo,PixelStageCreateInfo };

			GraphicsPipelineCreateInfo.stageCount = 2;
			GraphicsPipelineCreateInfo.pStages = ShaderStages;

			VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo = VulkanUtils::Initializers::PipelineInputAssemblyStateCreateInfo();
			InputAssemblyStateCreateInfo.primitiveRestartEnable = false;
			InputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

			GraphicsPipelineCreateInfo.pInputAssemblyState = &InputAssemblyStateCreateInfo;

			VulkanUtils::Descriptions::VertexBinding VertBinding = { };
			VertBinding.Binding = 0;
			VertBinding.Stride = sizeof(float) * 3;
			VertBinding.InputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			VulkanUtils::Descriptions::VertexAttribute VertAttribute = { };
			VertAttribute.Binding = 0;
			VertAttribute.Format = VK_FORMAT_R32G32B32_SFLOAT;
			VertAttribute.Location = 0;
			VertAttribute.Offset = 0;

			VkVertexInputBindingDescription VertexInputBindingDescription = { };
			VertBinding.WriteTo(VertexInputBindingDescription);

			VkVertexInputAttributeDescription VertexInputAttributeDescription = { };
			VertAttribute.WriteTo(VertexInputAttributeDescription);

			VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo = VulkanUtils::Initializers::PipelineVertexInputStateCreateInfo();
			VertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
			VertexInputStateCreateInfo.vertexAttributeDescriptionCount = 1;
			VertexInputStateCreateInfo.pVertexBindingDescriptions = &VertexInputBindingDescription;
			VertexInputStateCreateInfo.pVertexAttributeDescriptions = &VertexInputAttributeDescription;

			GraphicsPipelineCreateInfo.pVertexInputState = &VertexInputStateCreateInfo;

			VkViewport Viewport = { 0, 0, inRenderInitializerList.ViewportSize.Width, inRenderInitializerList.ViewportSize.Height, 0.0f, 1.0f };
			VkRect2D Scissor = { {0, 0}, { inRenderInitializerList.ViewportSize.Width, inRenderInitializerList.ViewportSize.Height} };

			VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo = VulkanUtils::Initializers::PipelineViewportStateCreateInfo();
			PipelineViewportStateCreateInfo.viewportCount = 1;
			PipelineViewportStateCreateInfo.scissorCount = 1;
			PipelineViewportStateCreateInfo.pViewports = &Viewport;
			PipelineViewportStateCreateInfo.pScissors = &Scissor;

			GraphicsPipelineCreateInfo.pViewportState = &PipelineViewportStateCreateInfo;

			VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo = VulkanUtils::Initializers::PipelineRasterizationStateCreateInfo();
			RasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
			RasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
			RasterizationStateCreateInfo.lineWidth = 1.0f;
			RasterizationStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
			RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			RasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
			RasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
			RasterizationStateCreateInfo.depthBiasClamp = 0.0f;
			RasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
			RasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

			GraphicsPipelineCreateInfo.pRasterizationState = &RasterizationStateCreateInfo;

			VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo = VulkanUtils::Initializers::PipelineMultisampleStateCreateInfo();
			MultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
			MultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			MultisampleStateCreateInfo.minSampleShading = 1.0f;
			MultisampleStateCreateInfo.pSampleMask = VK_NULL_HANDLE;
			MultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
			MultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

			GraphicsPipelineCreateInfo.pMultisampleState = &MultisampleStateCreateInfo;

			VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo = VulkanUtils::Initializers::PipelineDepthStencilStateCreateInfo();
			DepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
			DepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
			DepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
			DepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
			DepthStencilStateCreateInfo.minDepthBounds = 0.0f;
			DepthStencilStateCreateInfo.maxDepthBounds = 1.0f;
			DepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

			GraphicsPipelineCreateInfo.pDepthStencilState = &DepthStencilStateCreateInfo;

			VkPipelineColorBlendAttachmentState ColorBlendAttachmentState = { };
			ColorBlendAttachmentState.colorWriteMask = 0xF;
			ColorBlendAttachmentState.blendEnable = VK_FALSE;
			ColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
			ColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
			ColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
			ColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			ColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
			ColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

			VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo = VulkanUtils::Initializers::PipelineColorBlendStateCreateInfo();
			ColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
			ColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
			ColorBlendStateCreateInfo.attachmentCount = 1;
			ColorBlendStateCreateInfo.pAttachments = &ColorBlendAttachmentState;
			ColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
			ColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
			ColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
			ColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

			GraphicsPipelineCreateInfo.pColorBlendState = &ColorBlendStateCreateInfo;

			VkDynamicState DynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

			VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo = VulkanUtils::Initializers::PipelineDynamicStateCreateInfo();
			DynamicStateCreateInfo.dynamicStateCount = 2;
			DynamicStateCreateInfo.pDynamicStates = DynamicStates;

			GraphicsPipelineCreateInfo.pDynamicState = &DynamicStateCreateInfo;

			GraphicsPipelineCreateInfo.layout = *PipelineLayout->GetPipelineLayoutHandle();
			GraphicsPipelineCreateInfo.renderPass = *RenderPass->GetRenderPassHandle();
			GraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

			GraphicsPipeline->Create(GraphicsPipelineCreateInfo);
		}
	}

	if (!InitImGui())
	{
		ASSERT(false, "[VulkanRenderer]: Failed to initialize imgui...")
	}

	VE_CORE_LOG_INFO("Sucessfully initialized Imgui!");

	return true;
}
#endif // VULKAN_STANDALONE

#ifdef VULKAN_GLFW
bool VulkanRenderer::InitVulkanGLFW(const RendererInitializerList& inRenderInitializerList)
{
	ViewportSize = inRenderInitializerList.ViewportSize;

	VkResult Result;

	// Firstly Initlaize Vulkan
	VulkanInitializerList VulkanInitList = { 0 };

	// Select all the feature to enable 
	VkPhysicalDeviceFeatures EnabledFeatures = { };
	EnabledFeatures.tessellationShader = VK_TRUE;
	EnabledFeatures.geometryShader = VK_TRUE;
	EnabledFeatures.fillModeNonSolid = VK_TRUE;
	EnabledFeatures.samplerAnisotropy = VK_TRUE; //MSAA
	EnabledFeatures.multiViewport = VK_TRUE;

	VulkanInitList.EnabledFeatures = EnabledFeatures;

	// All instance extensions to enable 
	const uint32 InstanceExtensionCount = 1;
	const char* InstanceExtensions[InstanceExtensionCount]
	{
		"VK_EXT_debug_utils"
	};

	VulkanInitList.InstanceExtensions = InstanceExtensions;
	VulkanInitList.InstanceExtensionCount = InstanceExtensionCount;

	// All instance layers to enable 
	const uint32 InstanceLayerCount = 1;
	const char* InstanceLayers[InstanceLayerCount]
	{

#if RENDER_DOC
		"VK_LAYER_RENDERDOC_Capture",
#else
		"VK_LAYER_KHRONOS_validation",
#endif
	};

	VulkanInitList.InstanceLayers = InstanceLayers;
	VulkanInitList.InstanceLayersCount = InstanceLayerCount;

	// All device extensions to enable 
	const uint32 DeviceExtensionsCount = 2;
	const char* DeviceExtensions[DeviceExtensionsCount]
	{
		"VK_EXT_descriptor_indexing",
		"VK_KHR_multiview",
	};

	VulkanInitList.DeviceExtensions = DeviceExtensions;
	VulkanInitList.DeviceExtensionCount = DeviceExtensionsCount;

	// Initialize Vulkan
	if (!CreateVulkanInstance(VulkanInitList))
	{
		ASSERT(false, "Vulkan initialization failed...");
		return false;
	}

	VE_CORE_LOG_INFO("Successfully created an Instance..");

	// Create Vulkan Surface and Device
	{
		// Physical Device
		uint32 PhysicalDevicesCount = 0;

		// Get Number of available physical devices
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(VulkanInstance, &PhysicalDevicesCount, nullptr), "[VulkanRenderer]: No physical devices (GPU) found!");
		if (PhysicalDevicesCount == 0)
		{
			VE_CORE_LOG_ERROR("No device with Vulkan support found");
			return false;
		}

		// Enumerate physical devices
		std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDevicesCount);
		Result = (vkEnumeratePhysicalDevices(VulkanInstance, &PhysicalDevicesCount, PhysicalDevices.data()));
		if (Result != VK_SUCCESS)
		{
			VE_CORE_LOG_ERROR("Could not enumerate physical devices");
			return false;
		}

		// GPU Selection
		VulkanUtils::Helpers::GetBestPhysicalDevice(PhysicalDevices.data(), PhysicalDevicesCount, PhysicalDevice);

		// Find a suitable depth format
		VkBool32 ValidDepthFormat = VulkanUtils::Helpers::GetSupportedDepthFormat(PhysicalDevice, &DepthFormat);
		assert(ValidDepthFormat);
		Device = new VulkanDevice(PhysicalDevice, VulkanInitList.EnabledFeatures, VulkanInitList.DeviceExtensionCount, VulkanInitList.DeviceExtensions);

		// Let glfw create the surface for us 
		VkSurfaceKHR SurfaceHandle;
		VK_CHECK_RESULT(glfwCreateWindowSurface(VulkanInstance, (GLFWwindow*)Application::Get()->GetWindow().GetGLFWNativeHandle(), nullptr, &SurfaceHandle), "[VulkanRenderer]: glfw failed to create a window surface..");
		Surface = new VulkanSurface(Device, &VulkanInstance, SurfaceHandle);
		Device->CreateDevice(Surface);
	}

	// Create Swapchain
	Swapchain = new VulkanSwapChain(Device, Surface, inRenderInitializerList.ViewportSize.Width, inRenderInitializerList.ViewportSize.Height);

	// Create Command Buffer pool and command buffers
	{
		// Create a default command pool for graphics command buffers
		CommandPool = new VulkanCommandPool(Device);
		CommandPool->CreateCommandPool(Device->GetGraphicsQueue()->GetQueueIndex());

		// Create command buffers
		{
			// Create one command buffer for each swap chain image and reuse for rendering
			for (uint32 i = 0; i < Swapchain->GetImageCount(); ++i)
			{
				VulkanCommandBuffer* CommandBuffer = CommandPool->CreateCommandBuffer(i);
				CommandBuffer->AllocateCommandBuffer();
			}

			VE_CORE_LOG_INFO("Successfully created draw command buffers...");
		}
	}

	// Create synchronization objects (Semaphores)
	{
		// Semaphores (Used for correct command ordering)
		VkSemaphoreCreateInfo SemaphoreCreateInfo = VulkanUtils::Initializers::SemaphoreCreateInfo(nullptr);

		// Create a semaphore used to synchronize image presentation
		// Ensures that the image is displayed before we start submitting new commands to the queue
		VK_CHECK_RESULT(vkCreateSemaphore(*Device->GetDeviceHandle(), &SemaphoreCreateInfo, nullptr, &PresentationComplete), "[EntryPoint]: Failed to create a semaphore for image presentation!");

		for (uint32 i = 0; i < Swapchain->GetImageCount(); ++i)
		{
			CommandPool->GetCommandBuffer(i)->AddWaitSemaphore(&PresentationComplete);
		}

		// Create a semaphore used to synchronize command submission
		// Ensures that the image is not presented until all commands have been submitted and executed
		VK_CHECK_RESULT(vkCreateSemaphore(*Device->GetDeviceHandle(), &SemaphoreCreateInfo, nullptr, &RenderComplete), "[EntryPoint]: Failed to create a semaphore for render synchronization!");
	}

	// Setting up depth and stencil buffers
	{
		VkImageCreateInfo ImageCreateInfo = VulkanUtils::Initializers::ImageCreateInfo();
		ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageCreateInfo.format = DepthFormat;
		ImageCreateInfo.extent = { inRenderInitializerList.ViewportSize.Width, inRenderInitializerList.ViewportSize.Height, 1 };
		ImageCreateInfo.mipLevels = 1;
		ImageCreateInfo.arrayLayers = 1;
		ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		DepthStencilView = new VulkanTextureView(Device, ImageCreateInfo);

		VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
		if (DepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
			AspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		DepthStencilView->CreateImageView(VK_IMAGE_VIEW_TYPE_2D, DepthFormat, 0, 1, 0, 1, AspectFlags);
		VE_CORE_LOG_INFO("Successfully created depth stencil buffers...");
	}

	// Setting up render pass
	{
		std::vector<VkAttachmentDescription> Attachments = { };
		Attachments.resize(2);
		// Color attachment
		Attachments[0].format = *Surface->GetColorFormat();//ColorFormat;
		Attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		Attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		Attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		Attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		Attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		Attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// Depth attachment
		Attachments[1].format = DepthFormat;
		Attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		Attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		Attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		Attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		Attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		Attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference ColorReference = {};
		ColorReference.attachment = 0;
		ColorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference DepthReference = {};
		DepthReference.attachment = 1;
		DepthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkRect2D RenderArea = { 0,0, inRenderInitializerList.ViewportSize.Width, inRenderInitializerList.ViewportSize.Height };
		RenderPassLayout = new VulkanRenderLayout(Device, 1, RenderArea, &RenderArea.extent);
		RenderPassLayout->SetAttachments(Attachments);
		RenderPassLayout->SetColorReference(ColorReference);
		RenderPassLayout->SetDepthReference(DepthReference);

		// Set clear values for all framebuffer attachments with loadOp set to clear
		// We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
		std::vector<VkClearValue> ClearValues;
		ClearValues.resize(2);
		ClearValues[0].color = { { 0.0f, 0.0f, 0.2f, 1.0f } };
		ClearValues[1].depthStencil = { 1.0f, 0 };
		RenderPassLayout->SetClearValues(ClearValues);

		// Subpass dependencies for layout transitions
		std::vector<VkSubpassDependency> SubpassDependency;
		SubpassDependency.resize(2);

		SubpassDependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		SubpassDependency[0].dstSubpass = 0;
		SubpassDependency[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		SubpassDependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		SubpassDependency[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		SubpassDependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		SubpassDependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		SubpassDependency[1].srcSubpass = 0;
		SubpassDependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		SubpassDependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		SubpassDependency[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		SubpassDependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		SubpassDependency[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		SubpassDependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		RenderPass = new VulkanRenderPass(Device, *RenderPassLayout, SubpassDependency);
		VE_CORE_LOG_INFO("Successfully created renderpass...");
	}

	// Create Pipeline cache
	{
		VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {};
		PipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VK_CHECK_RESULT(vkCreatePipelineCache(*Device->GetDeviceHandle(), &PipelineCacheCreateInfo, nullptr, &PipelineCache), "[EntryPoint]: Failed to create a pipeline cache!");
		VE_CORE_LOG_INFO("Successfully created pipeline cache...");
	}

	// Create Frame buffers
	{
		VkImageView Attachments[2];

		// Depth/Stencil attachment is the same for all frame buffers
		Attachments[1] = *DepthStencilView->GetImageViewHandle();

		VkExtent2D Extent = { inRenderInitializerList.ViewportSize.Width, inRenderInitializerList.ViewportSize.Height };

		FrameBuffers.resize(Swapchain->GetImageCount());
		for (uint32 i = 0; i < Swapchain->GetImageCount(); ++i)
		{
			Attachments[0] = Swapchain->GetSwapchainBuffer(i)->View;
			FrameBuffers[i] = new VulkanFrameBuffer(Device, RenderPass);
			FrameBuffers[i]->AllocateBuffer(2, Attachments, &Extent);
		}

		VE_CORE_LOG_INFO("Successfully created framebuffers...");
	}

	// allocate 1 gibibytes of memory -> 1024 mebibytes = 1 gib
	MainVulkanMemoryHeap = new VulkanMemoryHeap(Device, 1);

	// Prepare the vulkan pipeline
	{
		// Create the pipeline layout, since we have no push constants nor descriptor sets, we just want an empty layout
		PipelineLayout = new VulkanPipelineLayout(Device);
		PipelineLayout->CreateEmpty();

		// Create the graphics pipeline 
		GraphicsPipeline = new VulkanGraphicsPipeline(Device);
		VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo = VulkanUtils::Initializers::GraphicsPipelineCreateInfo();

		MainVulkanResourceManager = new VulkanResourceManager(Device);
		GraphicsResourceManager = new ResourceManager(MainVulkanResourceManager);
		ShaderFactory = new VulkanShaderFactory(GraphicsResourceManager);

		const char VertexShaderStr[] = "float4 main(float3 inVertex : POSITION) : SV_POSITION { return float4(inVertex, 1.0f); }";
		const char PixelShaderStr[] = "float4 main(float4 inPosition : SV_POSITION) : SV_TARGET { return float4(1.0f, 0.0f, 0.0f, 1.0f); }";

		VertShader = ShaderFactory->CreateVertexShaderFromString(Device, VertexShaderStr, true);
		PixelShader = ShaderFactory->CreateFragmentShaderFromString(Device, PixelShaderStr, true);

		{
			VkPipelineShaderStageCreateInfo VertexStageCreateInfo = VulkanUtils::Initializers::PipelineShaderStageCreateInfo();
			VertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			VertexStageCreateInfo.module = *(VkShaderModule*)GraphicsResourceManager->GetShaderModule(VertShader->GetShaderKey());
			VertexStageCreateInfo.pName = "main";

			VkPipelineShaderStageCreateInfo PixelStageCreateInfo = VulkanUtils::Initializers::PipelineShaderStageCreateInfo();
			PixelStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			PixelStageCreateInfo.module = *(VkShaderModule*)GraphicsResourceManager->GetShaderModule(PixelShader->GetShaderKey());;
			PixelStageCreateInfo.pName = "main";

			VkPipelineShaderStageCreateInfo ShaderStages[2] = { VertexStageCreateInfo,PixelStageCreateInfo };

			GraphicsPipelineCreateInfo.stageCount = 2;
			GraphicsPipelineCreateInfo.pStages = ShaderStages;

			VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo = VulkanUtils::Initializers::PipelineInputAssemblyStateCreateInfo();
			InputAssemblyStateCreateInfo.primitiveRestartEnable = false;
			InputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

			GraphicsPipelineCreateInfo.pInputAssemblyState = &InputAssemblyStateCreateInfo;

			VulkanUtils::Descriptions::VertexBinding VertBinding = { };
			VertBinding.Binding = 0;
			VertBinding.Stride = sizeof(float) * 3;
			VertBinding.InputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			VulkanUtils::Descriptions::VertexAttribute VertAttribute = { };
			VertAttribute.Binding = 0;
			VertAttribute.Format = VK_FORMAT_R32G32B32_SFLOAT;
			VertAttribute.Location = 0;
			VertAttribute.Offset = 0;

			VkVertexInputBindingDescription VertexInputBindingDescription = { };
			VertBinding.WriteTo(VertexInputBindingDescription);

			VkVertexInputAttributeDescription VertexInputAttributeDescription = { };
			VertAttribute.WriteTo(VertexInputAttributeDescription);

			VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo = VulkanUtils::Initializers::PipelineVertexInputStateCreateInfo();
			VertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
			VertexInputStateCreateInfo.vertexAttributeDescriptionCount = 1;
			VertexInputStateCreateInfo.pVertexBindingDescriptions = &VertexInputBindingDescription;
			VertexInputStateCreateInfo.pVertexAttributeDescriptions = &VertexInputAttributeDescription;

			GraphicsPipelineCreateInfo.pVertexInputState = &VertexInputStateCreateInfo;

			VkViewport Viewport = { 0, 0, (float)inRenderInitializerList.ViewportSize.Width, (float)inRenderInitializerList.ViewportSize.Height, 0.0f, 1.0f };
			VkRect2D Scissor = { {0, 0}, { (float)inRenderInitializerList.ViewportSize.Width, (float)inRenderInitializerList.ViewportSize.Height} };

			VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo = VulkanUtils::Initializers::PipelineViewportStateCreateInfo();
			PipelineViewportStateCreateInfo.viewportCount = 1;
			PipelineViewportStateCreateInfo.scissorCount = 1;
			PipelineViewportStateCreateInfo.pViewports = &Viewport;
			PipelineViewportStateCreateInfo.pScissors = &Scissor;

			GraphicsPipelineCreateInfo.pViewportState = &PipelineViewportStateCreateInfo;

			VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo = VulkanUtils::Initializers::PipelineRasterizationStateCreateInfo();
			RasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
			RasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
			RasterizationStateCreateInfo.lineWidth = 1.0f;
			RasterizationStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
			RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			RasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
			RasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
			RasterizationStateCreateInfo.depthBiasClamp = 0.0f;
			RasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
			RasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

			GraphicsPipelineCreateInfo.pRasterizationState = &RasterizationStateCreateInfo;

			VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo = VulkanUtils::Initializers::PipelineMultisampleStateCreateInfo();
			MultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
			MultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			MultisampleStateCreateInfo.minSampleShading = 1.0f;
			MultisampleStateCreateInfo.pSampleMask = VK_NULL_HANDLE;
			MultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
			MultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

			GraphicsPipelineCreateInfo.pMultisampleState = &MultisampleStateCreateInfo;

			VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo = VulkanUtils::Initializers::PipelineDepthStencilStateCreateInfo();
			DepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
			DepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
			DepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
			DepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
			DepthStencilStateCreateInfo.minDepthBounds = 0.0f;
			DepthStencilStateCreateInfo.maxDepthBounds = 1.0f;
			DepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

			GraphicsPipelineCreateInfo.pDepthStencilState = &DepthStencilStateCreateInfo;

			VkPipelineColorBlendAttachmentState ColorBlendAttachmentState = { };
			ColorBlendAttachmentState.colorWriteMask = 0xF;
			ColorBlendAttachmentState.blendEnable = VK_FALSE;
			ColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
			ColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
			ColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
			ColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			ColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
			ColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

			VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo = VulkanUtils::Initializers::PipelineColorBlendStateCreateInfo();
			ColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
			ColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
			ColorBlendStateCreateInfo.attachmentCount = 1;
			ColorBlendStateCreateInfo.pAttachments = &ColorBlendAttachmentState;
			ColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
			ColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
			ColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
			ColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

			GraphicsPipelineCreateInfo.pColorBlendState = &ColorBlendStateCreateInfo;

			VkDynamicState DynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

			VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo = VulkanUtils::Initializers::PipelineDynamicStateCreateInfo();
			DynamicStateCreateInfo.dynamicStateCount = 2;
			DynamicStateCreateInfo.pDynamicStates = DynamicStates;

			GraphicsPipelineCreateInfo.pDynamicState = &DynamicStateCreateInfo;

			GraphicsPipelineCreateInfo.layout = *PipelineLayout->GetPipelineLayoutHandle();
			GraphicsPipelineCreateInfo.renderPass = *RenderPass->GetRenderPassHandle();
			GraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

			GraphicsPipeline->Create(GraphicsPipelineCreateInfo);
		}
	}

	return true;
}
#endif

bool VulkanRenderer::CreateVulkanInstance(VulkanInitializerList& inVulkanInitializerList)
{
	VkApplicationInfo ApplicationInfo = VulkanUtils::Initializers::ApplicationInfo();
	ApplicationInfo.pApplicationName = "Sandbox Project";
	ApplicationInfo.pEngineName = "Vrixic Engine";
	ApplicationInfo.apiVersion = VK_API_VERSION_1_3;

	std::vector<const char*> InstanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };
	std::vector<const char*> InstanceLayers = { };

	InstanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

	// Get extensions supported by the Instance and store for later use
	uint32 ExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr);
	if (ExtensionCount > 0)
	{
		std::vector<VkExtensionProperties> Extensions(ExtensionCount);
		if (vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, Extensions.data()) == VK_SUCCESS)
		{
			for (uint32 i = 0; i < ExtensionCount; ++i)
			{
				SupportedInstanceExtensions.push_back(Extensions[i].extensionName);
			}
		}
	}

	// Get layers supported by the Instance and store for later use
	uint32 LayerCount = 0;
	vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);
	if (LayerCount > 0)
	{
		std::vector<VkLayerProperties> Layers(LayerCount);
		if (vkEnumerateInstanceLayerProperties(&LayerCount, Layers.data()) == VK_SUCCESS)
		{
			for (uint32 i = 0; i < LayerCount; ++i)
			{
				SupportedInstanceLayers.push_back(Layers[i].layerName);
			}
		}
	}

	// Enabled requested Instance extensions
	if (inVulkanInitializerList.InstanceExtensionCount > 0)
	{
		for (uint32 i = 0; i < inVulkanInitializerList.InstanceExtensionCount; ++i)
		{
			// Output message if requested extension is not available
			if (std::find(SupportedInstanceExtensions.begin(), SupportedInstanceExtensions.end(), inVulkanInitializerList.InstanceExtensions[i]) == SupportedInstanceExtensions.end())
			{
				VE_CORE_LOG_FATAL("Enabled Instance extension \"{0}\" is not present at Instance level", inVulkanInitializerList.InstanceExtensions[i]);
			}

			InstanceExtensions.push_back(inVulkanInitializerList.InstanceExtensions[i]);
		}
	}

	// Enabled requested Instance layers
	if (inVulkanInitializerList.InstanceLayersCount > 0)
	{
		for (uint32 i = 0; i < inVulkanInitializerList.InstanceLayersCount; ++i)
		{
			// Output message if requested extension is not available
			if (std::find(SupportedInstanceLayers.begin(), SupportedInstanceLayers.end(), inVulkanInitializerList.InstanceLayers[i]) == SupportedInstanceLayers.end())
			{
				VE_CORE_LOG_FATAL("Enabled Instance layer \"{0}\" is not present at Instance level", inVulkanInitializerList.InstanceLayers[i]);
			}

			InstanceLayers.push_back(inVulkanInitializerList.InstanceLayers[i]);
		}
	}

	VkInstanceCreateInfo InstanceCreateInfo = VulkanUtils::Initializers::InstanceCreateInfo();
	InstanceCreateInfo.pNext = NULL;
	InstanceCreateInfo.pApplicationInfo = &ApplicationInfo;

	if (InstanceExtensions.size() > 0)
	{
		/* Debugging by default */
		InstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);	// SRS - Dependency when VK_EXT_DEBUG_MARKER is enabled
		InstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		InstanceCreateInfo.enabledExtensionCount = (uint32_t)InstanceExtensions.size();
		InstanceCreateInfo.ppEnabledExtensionNames = InstanceExtensions.data();
	}

	if (InstanceExtensions.size() > 0)
	{
		InstanceCreateInfo.enabledLayerCount = (uint32_t)InstanceLayers.size();
		InstanceCreateInfo.ppEnabledLayerNames = InstanceLayers.data();
	}

	// Debug Setup
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	VulkanUtils::DebugUtils::PopulateDebugMessengerCreateInfo(debugCreateInfo);

	InstanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

	return vkCreateInstance(&InstanceCreateInfo, nullptr, &VulkanInstance) == VK_SUCCESS;
}

#ifdef VULKAN_STANDALONE
bool VulkanRenderer::InitImguiStandalone()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	{
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Color scheme
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
		// Dimensions
		io.DisplaySize = ImVec2(ViewportSize.Width, ViewportSize.Height);
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	}

	// Create font texture
	unsigned char* fontData;
	int texWidth, texHeight;
	io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
	VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

	// Create target image for copy
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.extent.width = texWidth;
		imageInfo.extent.height = texHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		ImguiFontTextureView = new VulkanTextureView(Device, imageInfo);
	}

	// Image view
	{
		VkFormat FontImageViewFormat = VK_FORMAT_R8G8B8A8_UNORM;
		VkImageAspectFlags FontImageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

		ImguiFontTextureView->CreateImageView
		(
			VK_IMAGE_VIEW_TYPE_2D,
			FontImageViewFormat,
			0, 1, 0, 1, FontImageAspectFlags
		);
	}

	// Staging buffers for font data upload - Bind the memory as well
	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;
	void* StagingBufferMappedData;
	{
		VkBufferCreateInfo bufCreateInfo{};
		bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufCreateInfo.size = uploadSize;
		VK_CHECK_RESULT(vkCreateBuffer(*Device->GetDeviceHandle(), &bufCreateInfo, nullptr, &StagingBuffer), "[VulkanRenderer]: imgui creation, failed to create staging buffer!");

		// Create the memory backing up the buffer handle
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc{};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkGetBufferMemoryRequirements(*Device->GetDeviceHandle(), StagingBuffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		// Find a memory type index that fits the properties of the buffer
		memAlloc.memoryTypeIndex = Device->GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, nullptr);
		// If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
		VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
		if (VK_BUFFER_USAGE_TRANSFER_SRC_BIT & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
			allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
			allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
			memAlloc.pNext = &allocFlagsInfo;
		}

		VK_CHECK_RESULT(vkAllocateMemory(*Device->GetDeviceHandle(), &memAlloc, nullptr, &StagingBufferMemory), "[VulkanRenderer]: imgui creation, failed to allocate staging buffer!");
		VK_CHECK_RESULT(vkBindBufferMemory(*Device->GetDeviceHandle(), StagingBuffer, StagingBufferMemory, 0), "[VulkanRenderer]: imgui creation, failed to bind staging buffer!");


		VkDeviceSize Alignment = memReqs.alignment;
		VkDeviceSize Size = uploadSize;
		VkBufferUsageFlags UsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags MemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VK_CHECK_RESULT(vkMapMemory(*Device->GetDeviceHandle(), StagingBufferMemory, 0, Size, 0, &StagingBufferMappedData), "[VulkanRenderer]: imgui creation, failed to map staging buffer!");
		memcpy(StagingBufferMappedData, fontData, uploadSize);
		if (StagingBufferMappedData)
		{
			vkUnmapMemory(*Device->GetDeviceHandle(), StagingBufferMemory);
			StagingBufferMappedData = nullptr;
		}
	}

	VulkanCommandBuffer ImguiCommandBuffer(Device, CommandPool, 0);
	ImguiCommandBuffer.AllocateCommandBuffer();

	// If requested, also start recording for the new command buffer
	ImguiCommandBuffer.BeginCommandBuffer();

	// Prepare for transfer
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		// Create an image barrier object
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.image = *ImguiFontTextureView->GetImageHandle();
		imageMemoryBarrier.subresourceRange = subresourceRange;

		// Source layouts (old)
		// Source access mask controls actions that have to be finished on the old layout
		// before it will be transitioned to the new layout
		switch (VK_IMAGE_LAYOUT_UNDEFINED)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			imageMemoryBarrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
		}

		// Target layouts (new)
			// Destination access mask controls the dependency for the new image layout
		switch (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (imageMemoryBarrier.srcAccessMask == 0)
			{
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
		}

		// Put barrier inside setup command buffer
		vkCmdPipelineBarrier(
			*ImguiCommandBuffer.GetCommandBufferHandle(),
			VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}

	// Copy
	VkBufferImageCopy bufferCopyRegion = {};
	bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferCopyRegion.imageSubresource.layerCount = 1;
	bufferCopyRegion.imageExtent.width = texWidth;
	bufferCopyRegion.imageExtent.height = texHeight;
	bufferCopyRegion.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(
		*ImguiCommandBuffer.GetCommandBufferHandle(),
		StagingBuffer,
		*ImguiFontTextureView->GetImageHandle(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&bufferCopyRegion
	);

	// Prepare for shader read
	{
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		VkImageLayout oldImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		VkImageLayout newImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkImage image = *ImguiFontTextureView->GetImageHandle();

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = aspectMask;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		// Create an image barrier object
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		// Source layouts (old)
		// Source access mask controls actions that have to be finished on the old layout
		// before it will be transitioned to the new layout
		switch (oldImageLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			imageMemoryBarrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
		}

		// Target layouts (new)
		// Destination access mask controls the dependency for the new image layout
		switch (newImageLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (imageMemoryBarrier.srcAccessMask == 0)
			{
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
		}

		// Put barrier inside setup command buffer
		vkCmdPipelineBarrier(
			*ImguiCommandBuffer.GetCommandBufferHandle(),
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}

	{
		if (ImguiCommandBuffer.GetCommandBufferHandle() == VK_NULL_HANDLE)
		{
			ASSERT(false, "[VulkanRenderer]: imgui command buffer not created successfully!")
		}

		ImguiCommandBuffer.EndCommandBuffer();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = ImguiCommandBuffer.GetCommandBufferHandle();

		ImguiCommandBuffer.ResetWaitFence();
		Device->GetPresentQueue()->SubmitQueue(&ImguiCommandBuffer, submitInfo);
		ImguiCommandBuffer.SetWaitFence();

		//ImguiCommandBuffer.FreeCommandBuffer();
	}

	if (StagingBuffer)
	{
		vkDestroyBuffer(*Device->GetDeviceHandle(), StagingBuffer, nullptr);
	}
	if (StagingBufferMemory)
	{
		vkFreeMemory(*Device->GetDeviceHandle(), StagingBufferMemory, nullptr);
	}

	// Font texture Sampler
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VK_CHECK_RESULT(vkCreateSampler(*Device->GetDeviceHandle(), &samplerInfo, nullptr, &ImguiSampler), "[VulkanRenderer]: imgui creation - failed to create a font sampler!");
	}

	// Descriptor pool
	{
		VkDescriptorPoolSize descriptorPoolSize{};
		descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorPoolSize.descriptorCount = 1;
		std::vector<VkDescriptorPoolSize> poolSizes = { descriptorPoolSize };

		ImguiDescriptorSetsLayout = new VulkanDescriptorSetsLayout(Device);
		VulkanUtils::Descriptions::DescriptorSetLayoutCreateInfo DescSetLayoutCreateInfo = { 0 };
		VulkanUtils::Descriptions::DescriptorSetLayoutBinding DescSetLayoutBinding = { 0 };
		DescSetLayoutBinding.DescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		DescSetLayoutBinding.StageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		DescSetLayoutBinding.Binding = 0;
		DescSetLayoutBinding.DescriptorCount = 1;

		uint32 DescSetHandle = ImguiDescriptorSetsLayout->CreateDescriptorSetLayout(DescSetLayoutBinding, DescSetLayoutCreateInfo);
		ImguiDescriptorPool = new VulkanDescriptorPool(Device, *ImguiDescriptorSetsLayout, 2, poolSizes);
		ImguiDescriptorPool->AllocateDescriptorSets(1, &ImguiDescriptorSet, DescSetHandle);

		VulkanUtils::Descriptions::DescriptorImageInfo DescImageInfo = { 0 };
		DescImageInfo.Sampler = ImguiSampler;
		DescImageInfo.ImageView = *ImguiFontTextureView->GetImageViewHandle();
		DescImageInfo.ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VulkanUtils::Descriptions::WriteDescriptorSet WriteDescSet = { 0 };
		WriteDescSet.DstSet = ImguiDescriptorSet;
		WriteDescSet.DescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		WriteDescSet.DstBinding = 0;
		WriteDescSet.DescriptorCount = 1;
		// Same was updating the desc set 
		ImguiDescriptorPool->BindDescriptorSetToTexture(DescImageInfo, WriteDescSet);
	}

	{
		// Pipeline cache
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VK_CHECK_RESULT(vkCreatePipelineCache(*Device->GetDeviceHandle(), &pipelineCacheCreateInfo, nullptr, &ImguiPipelineCache), "[VulkanRenderer]: imgui creation - failed to create a pipeline cache!");

		// Pipeline layout
		// Push constants for UI rendering parameters
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstBlock);
		std::vector<VkPushConstantRange> PushConstantRanges = { pushConstantRange };

		ImguiPipelineLayout = new VulkanPipelineLayout(Device);
		ImguiPipelineLayout->Create(ImguiDescriptorSetsLayout, &PushConstantRanges);
	}

	{
		// Setup graphics pipeline for UI rendering
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.flags = 0;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizationState{};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.flags = 0;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;

		// Enable blending
		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.blendEnable = VK_TRUE;
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &blendAttachmentState;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		viewportState.flags = 0;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.flags = 0;

		std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		dynamicState.flags = 0;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = *ImguiPipelineLayout->GetPipelineLayoutHandle();
		pipelineCreateInfo.renderPass = *RenderPass->GetRenderPassHandle();
		pipelineCreateInfo.flags = 0;
		pipelineCreateInfo.basePipelineIndex = -1;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = 2;

		// Vertex bindings an attributes based on ImGui vertex definition
		VkVertexInputBindingDescription vertexInputBinding{};
		vertexInputBinding.binding = 0;
		vertexInputBinding.stride = sizeof(ImDrawVert);
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		std::vector<VkVertexInputBindingDescription> vertexInputBindings = { vertexInputBinding };

		VkVertexInputAttributeDescription vInputAttribDescription1{};
		vInputAttribDescription1.location = 0;
		vInputAttribDescription1.binding = 0;
		vInputAttribDescription1.format = VK_FORMAT_R32G32_SFLOAT;
		vInputAttribDescription1.offset = offsetof(ImDrawVert, pos); // Location 0: Position

		VkVertexInputAttributeDescription vInputAttribDescription2{};
		vInputAttribDescription2.location = 1;
		vInputAttribDescription2.binding = 0;
		vInputAttribDescription2.format = VK_FORMAT_R32G32_SFLOAT;
		vInputAttribDescription2.offset = offsetof(ImDrawVert, uv); // Location 1: UV

		VkVertexInputAttributeDescription vInputAttribDescription3{};
		vInputAttribDescription3.location = 2;
		vInputAttribDescription3.binding = 0;
		vInputAttribDescription3.format = VK_FORMAT_R8G8B8A8_UNORM;
		vInputAttribDescription3.offset = offsetof(ImDrawVert, col); // Location 2: Color

		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
			vInputAttribDescription1,
			vInputAttribDescription2,
			vInputAttribDescription3
		};

		VkPipelineVertexInputStateCreateInfo vertexInputState{};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
		vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
		vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

		pipelineCreateInfo.pVertexInputState = &vertexInputState;

		// Create shaders ../Assets/Shaders/imgui/ --  inside of sandbox
		ImguiVertexShader = ShaderFactory->CreateVertexShaderFromPath(Device, "../Assets/Shaders/imgui/ImguiVertex.hlsl", false);
		ImguiPixelShader = ShaderFactory->CreateFragmentShaderFromPath(Device, "../Assets/Shaders/imgui/ImguiPixel.hlsl", false);

		VkPipelineShaderStageCreateInfo shaderStageV = {};
		shaderStageV.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageV.stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStageV.module = *(VkShaderModule*)GraphicsResourceManager->GetShaderModule(ImguiVertexShader->GetShaderKey());
		shaderStageV.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStageP = {};
		shaderStageP.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageP.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStageP.module = *(VkShaderModule*)GraphicsResourceManager->GetShaderModule(ImguiPixelShader->GetShaderKey());
		shaderStageP.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[2] = { };
		shaderStages[0] = shaderStageV;
		shaderStages[1] = shaderStageP;

		pipelineCreateInfo.pStages = shaderStages;

		ImguiPipeline = new VulkanGraphicsPipeline(Device);
		ImguiPipeline->Create(pipelineCreateInfo);
	}

	return true;
}

void VulkanRenderer::UpdateImguiBuffers()
{
	ImDrawData* imDrawData = ImGui::GetDrawData();

	// Note: Alignment is done inside buffer creation
	VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
	VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

	if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
		return;
	}

	// Update buffers only if vertex or index count has been changed compared to current buffer size
	// Vertex buffer
	if ((ImguiVertexBuffer == VK_NULL_HANDLE) || (ImguiVertexCount != imDrawData->TotalVtxCount))
	{
		{
			if (ImguiVertexBufferMapped)
			{
				vkUnmapMemory(*Device->GetDeviceHandle(), ImguiVertexBufferData);
				ImguiVertexBufferMapped = nullptr;
			}

			if (ImguiVertexBuffer)
			{
				Device->WaitUntilIdle();
				vkDestroyBuffer(*Device->GetDeviceHandle(), ImguiVertexBuffer, nullptr);
				vkFreeMemory(*Device->GetDeviceHandle(), ImguiVertexBufferData, nullptr);
			}
		}

		{
			// Create the buffer handle
			VkBufferCreateInfo BufferCreateInfo = VulkanUtils::Initializers::BufferCreateInfo();
			BufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			BufferCreateInfo.size = vertexBufferSize;

			VK_CHECK_RESULT(vkCreateBuffer(*Device->GetDeviceHandle(), &BufferCreateInfo, nullptr, &ImguiVertexBuffer), "[VulkanRenderer]: Failed trying to create imgui vertex buffer");

			//Get the memory required to allocate the buffer
			VkMemoryRequirements memory_requirement;
			vkGetBufferMemoryRequirements(*Device->GetDeviceHandle(), ImguiVertexBuffer, &memory_requirement);

			VkMemoryAllocateInfo memory_allocate_info = {};
			memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memory_allocate_info.allocationSize = memory_requirement.size;
			memory_allocate_info.pNext = VK_NULL_HANDLE;

			memory_allocate_info.memoryTypeIndex = Device->GetMemoryTypeIndex(memory_requirement.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, nullptr);

			//Allocate and Bind the buffer
			VK_CHECK_RESULT(vkAllocateMemory(*Device->GetDeviceHandle(), &memory_allocate_info, nullptr, &ImguiVertexBufferData), "[VulkanRenderer]: Failed trying to allocate imgui vertex buffer memory");
			VK_CHECK_RESULT(vkBindBufferMemory(*Device->GetDeviceHandle(), ImguiVertexBuffer, ImguiVertexBufferData, 0), "[VulkanRenderer]: Failed trying to bind imgui vertex buffer memory");
		}

		ImguiVertexCount = imDrawData->TotalVtxCount;
		ImguiVertexBufferMapped = new ImDrawVert[ImguiVertexCount];
		vkMapMemory(*Device->GetDeviceHandle(), ImguiVertexBufferData, 0, VK_WHOLE_SIZE, 0, &ImguiVertexBufferMapped);
	}

	// Index buffer
	if ((ImguiIndexBuffer == VK_NULL_HANDLE) || (ImguiIndexCount < imDrawData->TotalIdxCount))
	{
		{
			if (ImguiIndexBufferMapped)
			{
				vkUnmapMemory(*Device->GetDeviceHandle(), ImguiIndexBufferData);
				ImguiIndexBufferMapped = nullptr;
			}

			if (ImguiIndexBuffer)
			{
				vkDestroyBuffer(*Device->GetDeviceHandle(), ImguiIndexBuffer, nullptr);
				vkFreeMemory(*Device->GetDeviceHandle(), ImguiIndexBufferData, nullptr);
			}
		}

		{
			// Create the buffer handle
			VkBufferCreateInfo BufferCreateInfo = VulkanUtils::Initializers::BufferCreateInfo();
			BufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			BufferCreateInfo.size = indexBufferSize;

			VK_CHECK_RESULT(vkCreateBuffer(*Device->GetDeviceHandle(), &BufferCreateInfo, nullptr, &ImguiIndexBuffer), "[VulkanRenderer]: Failed trying to create imgui index buffer");

			//Get the memory required to allocate the buffer
			VkMemoryRequirements memory_requirement;
			vkGetBufferMemoryRequirements(*Device->GetDeviceHandle(), ImguiIndexBuffer, &memory_requirement);

			VkMemoryAllocateInfo memory_allocate_info = {};
			memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memory_allocate_info.allocationSize = memory_requirement.size;
			memory_allocate_info.pNext = VK_NULL_HANDLE;

			memory_allocate_info.memoryTypeIndex = Device->GetMemoryTypeIndex(memory_requirement.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, nullptr);

			//Allocate and Bind the buffer
			VK_CHECK_RESULT(vkAllocateMemory(*Device->GetDeviceHandle(), &memory_allocate_info, nullptr, &ImguiIndexBufferData), "[VulkanRenderer]: Failed trying to allocate imgui index buffer memory");
			VK_CHECK_RESULT(vkBindBufferMemory(*Device->GetDeviceHandle(), ImguiIndexBuffer, ImguiIndexBufferData, 0), "[VulkanRenderer]: Failed trying to bind imgui index buffer memory");
		}

		ImguiIndexCount = imDrawData->TotalIdxCount;
		ImguiIndexBufferMapped = new ImDrawIdx[ImguiIndexCount];

		vkMapMemory(*Device->GetDeviceHandle(), ImguiIndexBufferData, 0, VK_WHOLE_SIZE, 0, &ImguiIndexBufferMapped);
	}

	// Upload data
	ImDrawVert* vtxDst = (ImDrawVert*)ImguiVertexBufferMapped;
	ImDrawIdx* idxDst = (ImDrawIdx*)ImguiIndexBufferMapped;

	for (int n = 0; n < imDrawData->CmdListsCount; n++) {
		const ImDrawList* cmd_list = imDrawData->CmdLists[n];
		memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtxDst += cmd_list->VtxBuffer.Size;
		idxDst += cmd_list->IdxBuffer.Size;
	}

	// Flush to make writes visible to GPU
	VkMappedMemoryRange mappedRangeV = {};
	mappedRangeV.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRangeV.memory = ImguiVertexBufferData;
	mappedRangeV.offset = 0;
	mappedRangeV.size = VK_WHOLE_SIZE;

	vkFlushMappedMemoryRanges(*Device->GetDeviceHandle(), 1, &mappedRangeV);

	VkMappedMemoryRange mappedRangeI = {};
	mappedRangeI.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRangeI.memory = ImguiIndexBufferData;
	mappedRangeI.offset = 0;
	mappedRangeI.size = VK_WHOLE_SIZE;
	vkFlushMappedMemoryRanges(*Device->GetDeviceHandle(), 1, &mappedRangeI);
}

void VulkanRenderer::DrawImguiFrame(VulkanCommandBuffer* inCommandBuffer)
{
	using namespace Vrixic::Math;

	VkCommandBuffer CommandBuffer = *inCommandBuffer->GetCommandBufferHandle();

	ImGuiIO& io = ImGui::GetIO();

	vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *ImguiPipelineLayout->GetPipelineLayoutHandle(),
		0, 1, &ImguiDescriptorSet, 0, nullptr);
	vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *ImguiPipeline->GetPipelineHandle());

	VkViewport viewport{};
	viewport.width = ImGui::GetIO().DisplaySize.x;
	viewport.height = ImGui::GetIO().DisplaySize.y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(CommandBuffer, 0, 1, &viewport);

	// UI scale and translate via push constants
	pushConstBlock.scale = Vector2D(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
	pushConstBlock.translate = Vector2D(-1.0f);
	vkCmdPushConstants(CommandBuffer, *ImguiPipelineLayout->GetPipelineLayoutHandle(),
		VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);

	// Render commands
	ImDrawData* imDrawData = ImGui::GetDrawData();
	int32_t vertexOffset = 0;
	int32_t indexOffset = 0;

	if (imDrawData->CmdListsCount > 0) {

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &ImguiVertexBuffer, offsets);
		vkCmdBindIndexBuffer(CommandBuffer, ImguiIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

		for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
		{
			const ImDrawList* cmd_list = imDrawData->CmdLists[i];
			for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
				VkRect2D scissorRect;
				scissorRect.offset.x = MathUtils::Max((int32_t)(pcmd->ClipRect.x), 0);
				scissorRect.offset.y = MathUtils::Max((int32_t)(pcmd->ClipRect.y), 0);
				scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
				scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
				vkCmdSetScissor(CommandBuffer, 0, 1, &scissorRect);
				vkCmdDrawIndexed(CommandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmd_list->VtxBuffer.Size;
		}
	}
}
#endif
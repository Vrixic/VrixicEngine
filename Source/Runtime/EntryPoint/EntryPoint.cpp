#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <cassert>

#include "Runtime/Graphics/Vulkan/VulkanDevice.h"
#include "Runtime/Graphics/Vulkan/VulkanCommandBuffer.h"
#include "Runtime/Graphics/Vulkan/VulkanRenderPass.h"
#include "Runtime/Graphics/Vulkan/VulkanFrameBuffer.h"
#include "Runtime/Graphics/Vulkan/VulkanTextureView.h"
#include "Runtime/Graphics/Vulkan/VulkanBuffer.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <Runtime/Graphics/Vulkan/VulkanBuffer.h>
#include <Runtime/Graphics/Vulkan/VulkanShader.h>
#include <Runtime/Graphics/Vulkan/VulkanPipeline.h>
#include <Runtime/Memory/Vulkan/VulkanResourceManager.h>

#define RENDER_DOC 1

typedef uint32_t uint32;

uint32 WindowWidth = 1280;
uint32 WindowHeight = 720;

HINSTANCE WindowInstance;
HWND Window;

std::string Name = "Vrixic";

class VulkanAPI;

VulkanAPI* VTemp;

const char VertexShaderStr[] = "float4 main(float3 inVertex : POSITION) : SV_POSITION { return float4(inVertex, 1.0f); }";
const char PixelShaderStr[] = "float4 main(float4 inPosition : SV_POSITION) : SV_TARGET { return float4(1.0f, 0.0f, 0.0f, 1.0f); }";

class VulkanAPI
{
public:
	uint32 Width;
	uint32 Height;

	uint32 DestWidth;
	uint32 DestHeight;

	float AspectRatio;

	bool Deallocated, NoDrawing;

	/* Vulkan specifics */
	VkInstance Instance;
	VkPhysicalDevice PhysicalDevice;

	VulkanSurface* Surface;
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

	/* Window Stuff Win32*/
	HINSTANCE WindowInstance;
	HWND Window;

	uint32 Indices[3] =
	{
		0, 1, 2
	};

	struct Vertex
	{
		float Position[3];
	};

	Vertex Vertices[3]
	{
		{0.0f, 0.75f, 0.0f}, {0.75f, -0.75f, 0.0f}, {-0.75f, -0.75f, 0.0f}
	};

	VulkanMemoryHeap* MainVulkanMemoryHeap;

	VulkanBuffer* IndexBuffer;
	VulkanBuffer* VertexBuffer;

	// Create the pipeline layout, since we have no push constants nor descriptor sets, we just want an empty layout
	VulkanPipelineLayout* PipelineLayout;

	// Create the graphics pipeline 
	VulkanGraphicsPipeline* GraphicsPipeline;

	IResourceManager* MainVulkanResourceManager;
	ResourceManager* GraphicsResourceManager;
	VulkanShaderFactory* ShaderFactory;

	VulkanVertexShader* VertShader;
	VulkanFragmentShader* PixelShader;

public:
	VulkanAPI(uint32_t width, uint32_t height, HINSTANCE& windowInstance, HWND& window)
	{
		Width = width;
		Height = height;

		AspectRatio = width / static_cast<float>(height);

		Deallocated = false;
		NoDrawing = false;

		Instance = { };
		//Surface->GetSurfaceHandle() = { };
		PhysicalDevice = { };
		//Device = { };
		CommandPool = { };
		//SwapchainKHR = { };

		WindowInstance = windowInstance;
		Window = window;
	}

	~VulkanAPI()
	{
		Device->WaitUntilIdle();

		delete CommandPool;

		vkDestroyPipelineCache(*Device->GetDeviceHandle(), PipelineCache, nullptr);

		for (uint32 i = 0; i < FrameBuffers.size(); ++i)
		{
			delete FrameBuffers[i];
		}

		delete DepthStencilView;

		delete RenderPassLayout;
		delete RenderPass;
		delete Swapchain;

		vkDestroySemaphore(*Device->GetDeviceHandle(), PresentationComplete, nullptr);
		vkDestroySemaphore(*Device->GetDeviceHandle(), RenderComplete, nullptr);

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
		vkDestroyInstance(Instance, nullptr);
	}

	bool InitVulkan(VkPhysicalDeviceFeatures enabledFeatures, const char** instanceLayers, uint32 layersCount, const char** instanceExtensions, uint32 instanceExtensionCount, const char** deviceExtensions, uint32 deviceExtensionCount)
	{
		VkResult Result;

		// Create Vulkan Instance
		Result = CreateInstance(instanceLayers, layersCount, instanceExtensions, instanceExtensionCount);
		if (Result != VK_SUCCESS) {
			std::cout << "Could not create Vulkan instance!";
			return false;
		}
		else
		{
			std::cout << "Successfully created an instance..\n";
		}

		// Physical Device
		uint32 PhysicalDevicesCount = 0;

		// Get Number of available physical devices
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(Instance, &PhysicalDevicesCount, nullptr));
		if (PhysicalDevicesCount == 0)
		{
			std::cout << "No device with Vulkan support found";
			return false;
		}

		// Enumerate physical devices
		std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDevicesCount);
		Result = (vkEnumeratePhysicalDevices(Instance, &PhysicalDevicesCount, PhysicalDevices.data()));
		if (Result != VK_SUCCESS)
		{
			std::cout << "Could not enumerate physical devices";
			return false;
		}

		// GPU Selection
		VulkanUtils::Helpers::GetBestPhysicalDevice(PhysicalDevices.data(), PhysicalDevicesCount, PhysicalDevice);

		// Find a suitable depth format
		VkBool32 ValidDepthFormat = VulkanUtils::Helpers::GetSupportedDepthFormat(PhysicalDevice, &DepthFormat);
		assert(ValidDepthFormat);
		Device = new VulkanDevice(PhysicalDevice, enabledFeatures, deviceExtensionCount, deviceExtensions);
		Surface = new VulkanSurface(Device, &Instance, &WindowInstance, &Window);
		Device->CreateDevice(Surface);

		// Create Swapchain
		Swapchain = new VulkanSwapChain(Device, Surface, Width, Height);

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

			std::cout << "successfully created draw command buffers...\n";
		}

		// Create synchronization objects (Semaphores)
		{
			// Semaphores (Used for correct command ordering)
			VkSemaphoreCreateInfo SemaphoreCreateInfo = VulkanUtils::Initializers::SemaphoreCreateInfo(nullptr);

			// Create a semaphore used to synchronize image presentation
			// Ensures that the image is displayed before we start submitting new commands to the queue
			VK_CHECK_RESULT(vkCreateSemaphore(*Device->GetDeviceHandle(), &SemaphoreCreateInfo, nullptr, &PresentationComplete));

			for (uint32 i = 0; i < Swapchain->GetImageCount(); ++i)
			{
				CommandPool->GetCommandBuffer(i)->AddWaitSemaphore(&PresentationComplete);
			}

			// Create a semaphore used to synchronize command submission
			// Ensures that the image is not presented until all commands have been submitted and executed
			VK_CHECK_RESULT(vkCreateSemaphore(*Device->GetDeviceHandle(), &SemaphoreCreateInfo, nullptr, &RenderComplete));
		}

		// Setting up depth and stencil buffers
		{
			VkImageCreateInfo ImageCreateInfo = VulkanUtils::Initializers::ImageCreateInfo();
			ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			ImageCreateInfo.format = DepthFormat;
			ImageCreateInfo.extent = { Width, Height, 1 };
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
			std::cout << "successfully created depth stencil buffers...\n";
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

			VkRect2D RenderArea = { 0,0, Width, Height };
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

			RenderPass = new VulkanRenderPass(Device, *RenderPassLayout);
			std::cout << "successfully created renderpass...\n";
		}

		// Create Pipeline cache
		{
			VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {};
			PipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
			VK_CHECK_RESULT(vkCreatePipelineCache(*Device->GetDeviceHandle(), &PipelineCacheCreateInfo, nullptr, &PipelineCache));
			std::cout << "successfully created pipeline cache...\n";
		}

		// Create Frame buffers
		{
			VkImageView Attachments[2];

			// Depth/Stencil attachment is the same for all frame buffers
			Attachments[1] = *DepthStencilView->GetImageViewHandle();

			VkExtent2D Extent = { Width, Height };

			FrameBuffers.resize(Swapchain->GetImageCount());
			for (uint32 i = 0; i < Swapchain->GetImageCount(); ++i)
			{
				Attachments[0] = Swapchain->GetSwapchainBuffer(i)->View;
				FrameBuffers[i] = new VulkanFrameBuffer(Device, RenderPass);
				FrameBuffers[i]->AllocateBuffer(2, Attachments, &Extent);
			}

			std::cout << "successfully created framebuffers...\n";
		}

		PrepareVulkanPipeline();

		// allocate 1 gibibytes of memory -> 1024 mebibytes = 1 gib
		MainVulkanMemoryHeap = new VulkanMemoryHeap(Device, 1024);

		VulkanUtils::Descriptions::VulkanBufferCreateInfo BufferCreateInfo = { };
		BufferCreateInfo.BufferUsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		BufferCreateInfo.DeviceSize = sizeof(Indices);
		BufferCreateInfo.MemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		IndexBuffer = MainVulkanMemoryHeap->AllocateBuffer(EBufferType::Index, BufferCreateInfo);
		
		BufferCreateInfo.BufferUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		BufferCreateInfo.DeviceSize = sizeof(Vertices);
		VertexBuffer = MainVulkanMemoryHeap->AllocateBuffer(EBufferType::Vertex, BufferCreateInfo);

		memcpy(IndexBuffer->GetMappedPointer(), Indices, sizeof(Indices));
		memcpy(VertexBuffer->GetMappedPointer(), Vertices, sizeof(Vertices));

		return true;
	}

	void PrepareVulkanPipeline()
	{
		// Create the pipeline layout, since we have no push constants nor descriptor sets, we just want an empty layout
		PipelineLayout = new VulkanPipelineLayout(VTemp->Device);
		PipelineLayout->CreateEmpty();

		// Create the graphics pipeline 
		GraphicsPipeline = new VulkanGraphicsPipeline(VTemp->Device);
		VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo = VulkanUtils::Initializers::GraphicsPipelineCreateInfo();

		MainVulkanResourceManager = new VulkanResourceManager(VTemp->Device);
		GraphicsResourceManager = new ResourceManager(MainVulkanResourceManager);
		ShaderFactory = new VulkanShaderFactory(GraphicsResourceManager);

		VertShader = ShaderFactory->CreateVertexShaderFromString(VTemp->Device, VertexShaderStr);
		PixelShader = ShaderFactory->CreateFragmentShaderFromString(VTemp->Device, PixelShaderStr);

		{
			VkPipelineShaderStageCreateInfo VertexStageCreateInfo = VulkanUtils::Initializers::PipelineShaderStageCreateInfo();
			VertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			VertexStageCreateInfo.module = *(VkShaderModule*)GraphicsResourceManager->GetShaderModule(VertShader->GetShaderKey());
			VertexStageCreateInfo.pName = "main";

			VkPipelineShaderStageCreateInfo PixelStageCreateInfo = VulkanUtils::Initializers::PipelineShaderStageCreateInfo();
			PixelStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			PixelStageCreateInfo.module = *(VkShaderModule*)GraphicsResourceManager->GetShaderModule(PixelShader->GetShaderKey());;
			PixelStageCreateInfo.pName = "main";

			VkPipelineShaderStageCreateInfo ShaderStages[2] = { VertexStageCreateInfo, PixelStageCreateInfo };

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

			VkViewport Viewport = { 0, 0, VTemp->Width, VTemp->Height, 0.0f, 1.0f };
			VkRect2D Scissor = { {0, 0}, { VTemp->Width, VTemp->Height} };

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
			GraphicsPipelineCreateInfo.renderPass = *VTemp->RenderPass->GetRenderPassHandle();
			GraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

			GraphicsPipeline->Create(GraphicsPipelineCreateInfo);
		}
	}

	void BuildCommandBuffers()
	{
		for (uint32 i = 0; i < Swapchain->GetImageCount(); ++i)
		{
			// Set target frame buffer
			VulkanCommandBuffer* CommandBuffer = CommandPool->GetCommandBuffer(i);

			CommandBuffer->BeginCommandBuffer();

			// Start the first sub pass specified in our default render pass setup by the base class
			// This will clear the color and depth attachment
			CommandBuffer->BeginRenderPass(RenderPass, FrameBuffers[i]);

			// Update dynamic viewport state
			VkViewport viewport = {};
			viewport.height = (float)Height;
			viewport.width = (float)Width;
			viewport.minDepth = (float)0.0f;
			viewport.maxDepth = (float)1.0f;
			vkCmdSetViewport(*CommandBuffer->GetCommandBufferHandle(), 0, 1, &viewport);

			// Update dynamic scissor state
			VkRect2D scissor = {};
			scissor.extent.width = Width;
			scissor.extent.height = Height;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetScissor(*CommandBuffer->GetCommandBufferHandle(), 0, 1, &scissor);

			VkDeviceSize offsets[1] = { 0 };

			// Bind the rendering pipeline
			// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
			vkCmdBindPipeline(*CommandBuffer->GetCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, *GraphicsPipeline->GetPipelineHandle());

			// Bind Vertex Buffer
			vkCmdBindVertexBuffers(*CommandBuffer->GetCommandBufferHandle(),
				0, 1, VertexBuffer->GetBufferHandle(), offsets);

			vkCmdBindIndexBuffer(*CommandBuffer->GetCommandBufferHandle(),
				*IndexBuffer->GetBufferHandle(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(*CommandBuffer->GetCommandBufferHandle(),
				3, 1, 0, 0, 1);

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

			CommandBuffer->EndRenderPass();// vkCmdEndRenderPass(DrawCommandBuffers[i]);

			// Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to
			// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system

			CommandBuffer->EndCommandBuffer();// VK_CHECK_RESULT(vkEndCommandBuffer(DrawCommandBuffers[i]));
		}
	}

	void WindowResized()
	{
		// Ensure all operations on the device have been finished before destroying resources
		Device->WaitUntilIdle();

		// Recreate swap chain
		Width = DestWidth;
		Height = DestHeight;

		if (Width == 0 || Height == 0)
		{
			std::cout << std::endl;
		}

		Swapchain->Recreate(false, &Width, &Height);

		{
			delete DepthStencilView;

			VkImageCreateInfo ImageCreateInfo = VulkanUtils::Initializers::ImageCreateInfo();
			ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			ImageCreateInfo.format = DepthFormat;
			ImageCreateInfo.extent = { Width, Height, 1 };
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
		VkRect2D RenderArea = { 0,0, Width, Height };
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

			VkExtent2D Extent = { Width, Height };

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

		VTemp->BuildCommandBuffers();
	}

	VkResult CreateInstance(const char** instanceLayers, uint32 layersCount, const char** instanceExtensions, uint32 extensionCount)
	{
		VkApplicationInfo ApplicationInfo = VulkanUtils::Initializers::ApplicationInfo();
		ApplicationInfo.pApplicationName = "App Name";
		ApplicationInfo.pEngineName = "Engine Name";
		ApplicationInfo.apiVersion = VK_API_VERSION_1_3;

		std::vector<const char*> InstanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };
		std::vector<const char*> InstanceLayers = { };

		InstanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

		// Get extensions supported by the instance and store for later use
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

		// Get layers supported by the instance and store for later use
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
		if (extensionCount > 0)
		{
			for (uint32 i = 0; i < extensionCount; ++i)
			{
				// Output message if requested extension is not available
				if (std::find(SupportedInstanceExtensions.begin(), SupportedInstanceExtensions.end(), instanceExtensions[i]) == SupportedInstanceExtensions.end())
				{
					std::cerr << "Enabled instance extension \"" << instanceExtensions[i] << "\" is not present at instance level\n";
				}

				InstanceExtensions.push_back(instanceExtensions[i]);
			}
		}

		// Enabled requested Instance layers
		if (layersCount > 0)
		{
			for (uint32 i = 0; i < layersCount; ++i)
			{
				// Output message if requested extension is not available
				if (std::find(SupportedInstanceLayers.begin(), SupportedInstanceLayers.end(), instanceLayers[i]) == SupportedInstanceLayers.end())
				{
					std::cerr << "Enabled instance layer \"" << instanceLayers[i] << "\" is not present at instance level\n";
				}

				InstanceLayers.push_back(instanceLayers[i]);
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

		// The VK_LAYER_KHRONOS_validation contains all current validation functionality.
		//const char* ValidationLayerName = "VK_LAYER_KHRONOS_validation";
		//
		//// Check if this layer is available at instance level
		//uint32_t InstanceLayerCount;
		//vkEnumerateInstanceLayerProperties(&InstanceLayerCount, nullptr);
		//std::vector<VkLayerProperties> InstanceLayerProperties(InstanceLayerCount);
		//vkEnumerateInstanceLayerProperties(&InstanceLayerCount, InstanceLayerProperties.data());
		//bool ValidationLayerPresent = false;
		//for (const VkLayerProperties& layer : InstanceLayerProperties) {
		//	if (strcmp(layer.layerName, ValidationLayerName) == 0) {
		//		ValidationLayerPresent = true;
		//		break;
		//	}
		//}
		//
		//if (ValidationLayerPresent) {
		//	InstanceCreateInfo.ppEnabledLayerNames = &ValidationLayerName;
		//	InstanceCreateInfo.enabledLayerCount = 1;
		//}
		//else {
		//	std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
		//}

		return vkCreateInstance(&InstanceCreateInfo, nullptr, &Instance);
	}

};

std::string GetWindowTitle()
{
	return "Poop";
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		/* Dont want window to close before we dont clean up out memory usage */
	case WM_CLOSE:
		PostQuitMessage(0);
		return true;
	case WM_PAINT:
		ValidateRect(Window, NULL);
		break;
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED && VTemp != nullptr)
		{
			if (((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)))
			{
				VTemp->DestWidth = LOWORD(lParam);
				VTemp->DestHeight = HIWORD(lParam);

				VTemp->WindowResized();
			}
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
	}

	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	WindowInstance = hInstance;
	{
		_CrtDumpMemoryLeaks();
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);

		/* Allocate a console */
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		FILE* Stream;
		freopen_s(&Stream, "CONIN$", "r", stdin);
		freopen_s(&Stream, "CONOUT$", "w+", stdout);
		freopen_s(&Stream, "CONOUT$", "w+", stderr);

		WNDCLASSEX WindowClass;
		WindowClass.cbSize = sizeof(WNDCLASSEX);
		WindowClass.style = CS_HREDRAW | CS_VREDRAW;
		WindowClass.lpfnWndProc = WndProc;
		WindowClass.cbClsExtra = 0;
		WindowClass.cbWndExtra = 0;
		WindowClass.hInstance = hInstance;
		WindowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		WindowClass.lpszMenuName = NULL;
		std::wstring W(Name.begin(), Name.end());
		WindowClass.lpszClassName = W.c_str();
		WindowClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

		if (!RegisterClassEx(&WindowClass))
		{
			std::cout << "Could not register window class!\n";
			fflush(stdout);
			exit(1);
		}

		int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
		int WindowX = ScreenWidth / 2 - WindowWidth / 2;
		int WindowY = ScreenHeight / 2 - WindowHeight / 2;

		DWORD DwExStyle;
		DWORD DwStyle;

		/* Make the style a popup and u have a border less wwindow*/

		DwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE; //WS_POPUP
		DwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN; //WS_POPUP

		RECT WindowRect;
		WindowRect.left = 0L;
		WindowRect.top = 0L;
		WindowRect.right = (long)WindowWidth;
		WindowRect.bottom = (long)WindowHeight;

		AdjustWindowRectEx(&WindowRect, DwStyle, FALSE, DwExStyle);

		std::string WindowTitle = GetWindowTitle();
		std::wstring WWinTitle(WindowTitle.begin(), WindowTitle.end());
		Window = CreateWindowEx(0,
			W.c_str(),
			WWinTitle.c_str(),
			DwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			WindowX,
			WindowY,
			WindowRect.right - WindowRect.left,
			WindowRect.bottom - WindowRect.top,
			NULL,
			NULL,
			hInstance,
			NULL);

		if (!Window)
		{
			printf("Could not create window!\n");
			fflush(stdout);
			return 0;
		}

		ShowWindow(Window, SW_SHOW);
		SetForegroundWindow(Window);
		SetFocus(Window);
	}

	/* Create Vulkan Stuff */
	VTemp = new VulkanAPI(WindowWidth, WindowHeight, WindowInstance, Window);

	const uint32 InstanceExtensionCount = 0;
	/*const char* InstanceExtensions[InstanceExtensionCount]
	{
		""
	};	*/

	
#if RENDER_DOC
	const uint32 InstanceLayerCount = 2;
#else
	const uint32 InstanceLayerCount = 1;
#endif
	const char* InstanceLayers[InstanceLayerCount]
	{
		"VK_LAYER_KHRONOS_validation",
#if RENDER_DOC
		"VK_LAYER_RENDERDOC_Capture"
#endif
	};

	const uint32 DeviceExtensionsCount = 2;
	const char* DeviceExtensions[DeviceExtensionsCount]
	{
		"VK_EXT_descriptor_indexing",
		"VK_KHR_multiview"
	};

	VkPhysicalDeviceFeatures EnabledFeatures = { };
	EnabledFeatures.tessellationShader = VK_TRUE;
	EnabledFeatures.geometryShader = VK_TRUE;
	EnabledFeatures.fillModeNonSolid = VK_TRUE;
	EnabledFeatures.samplerAnisotropy = VK_TRUE; //MSAA
	EnabledFeatures.multiViewport = VK_TRUE;

	if (!VTemp->InitVulkan(EnabledFeatures, InstanceLayers, InstanceLayerCount, nullptr, InstanceExtensionCount, DeviceExtensions, DeviceExtensionsCount))
	{
		std::cout << "Vulkan initialization failed...";
		std::cin.get();
		return 0;
	}

	std::wstring T = L"Debug Log";
	SetConsoleTitle(T.c_str());

	MSG Message;
	while (GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);

		switch (Message.message)
		{
		case WM_QUIT:
			std::cout << "Quit Message Posted";
			break;
		default:
			break;
		}

		DispatchMessage(&Message);

		// Draw
		{
			VulkanCommandBuffer* LastCommandBuffer = VTemp->CommandPool->GetCommandBuffer(VTemp->CurrentBuffer);

			// Use a fence to wait until the command buffer has finished execution before using it again
			// Start of frame we would want to wait until last frame has finished 
			LastCommandBuffer->SetWaitFence();

			VTemp->BuildCommandBuffers();

			// SRS - on other platforms use original bare code with local semaphores/fences for illustrative purposes
			// Get next image in the swap chain (back/front buffer)
			VkResult Acquire = VTemp->Swapchain->AcquireNextImage(LastCommandBuffer, &VTemp->CurrentBuffer);// VTemp->AcquireNextImage(VTemp->PresentationComplete, &VTemp->CurrentBuffer);
			if (!((Acquire == VK_SUCCESS) || (Acquire == VK_SUBOPTIMAL_KHR))) {
				VK_CHECK_RESULT(Acquire);
			}

			VulkanCommandBuffer* CurrentCommandBuffer = VTemp->CommandPool->GetCommandBuffer(VTemp->CurrentBuffer);

			/* after waiting reset the fence */
			CurrentCommandBuffer->ResetWaitFence();

			// Submit to the graphics queue passing a wait fence
			VTemp->Device->GetGraphicsQueue()->SubmitQueue(CurrentCommandBuffer, &VTemp->RenderComplete);

			// Present the current buffer to the swap chain
			// Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
			// This ensures that the image is not presented to the windowing system until all commands have been submitted
			VkResult present = VTemp->Swapchain->QueuePresent(VTemp->Device->GetPresentQueue(), &VTemp->RenderComplete, VTemp->CurrentBuffer);// VTemp->QueuePresent(VTemp->Device->GetPresentQueue()->GetQueueHandle(), VTemp->CurrentBuffer, VTemp->RenderComplete);
			if (!((present == VK_SUCCESS) || (present == VK_SUBOPTIMAL_KHR))) {
				VK_CHECK_RESULT(present);
			}
		}

	}

	delete VTemp;

	std::cout << "Press Enter to exit....\n\n";
	std::cin.get();

	return 0;
}

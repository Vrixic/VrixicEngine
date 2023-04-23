/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "VulkanRenderInterface.h"

#include <Runtime/Graphics/Vulkan/VulkanBuffer.h>
#include <Runtime/Graphics/Vulkan/VulkanCommandBuffer.h>
#include <Runtime/Graphics/Vulkan/VulkanFence.h>
#include <Runtime/Graphics/Vulkan/VulkanPipeline.h>
#include <Runtime/Graphics/Vulkan/VulkanRenderPass.h>
#include <Runtime/Graphics/Vulkan/VulkanSemaphore.h>
#include <Runtime/Graphics/Vulkan/VulkanTextureView.h>
//#include <Runtime/Memory/Vulkan/VulkanResourceManager.h>

VulkanRenderInterface::VulkanRenderInterface(const VulkanRendererConfig& inVulkanRendererConfig)
{
    VE_ASSERT(CreateVulkanInstance(inVulkanRendererConfig), "[VulkanRenderInterface]: failed to create a vulkan instance object..");

    VkResult Result;

    // Create Devices
    {
        // Create Physical Device, pick the best physical device, query some renderer information
        PhysicalDevice = new VulkanPhysicalDevice();
        PhysicalDevice->PickBestPhysicalDevice(VulkanInstance);

        PhysicalDevice->QueryDeviceProperties(RendererInformation);

        // information for device creation
        VkPhysicalDeviceFeatures EnabledFeatures = Convert(inVulkanRendererConfig.EnabledDeviceFeatures);
        VkPhysicalDevice PhysicalDeviceHandle = PhysicalDevice->GetPhysicalDeviceHandle();

        // Create Logical Device 
        Device = new VulkanDevice(PhysicalDeviceHandle, EnabledFeatures, (uint32)inVulkanRendererConfig.EnabledDeviceExtensionCount, (const char**)inVulkanRendererConfig.EnabledDeviceExtensions);
        //Device->CreateDevice((VulkanSurface*)inVulkanRendererConfig.SurfacePtr);
    }
}

VulkanRenderInterface::~VulkanRenderInterface()
{
    if (VulkanInstance != VK_NULL_HANDLE)
    {
        Shutdown();
    }
}

void VulkanRenderInterface::Initialize()
{
    // Create Resource Managements Resources
    {
        //MainVulkanResourceManager = new VulkanResourceManager(Device);
        //GraphicsResourceManager = new ResourceManager(MainVulkanResourceManager);

        ShaderFactoryMain = new VulkanShaderFactory(Device);
        ShaderPoolMain = new VulkanShaderPool(Device);

        // allocate 1 gibibytes of memory -> 1024 mebibytes = 1 gib
        VulkanMemoryHeapMain = new VulkanMemoryHeap(Device, 1);
    }

    // we could also create default resources for use???....
}

void VulkanRenderInterface::Shutdown()
{
    Device->WaitUntilIdle();

    //delete GraphicsResourceManager;
    //delete MainVulkanResourceManager;

    delete VulkanMemoryHeapMain;

    delete ShaderFactoryMain;
    delete ShaderPoolMain;

    delete Device;
    vkDestroyInstance(VulkanInstance, nullptr);

    delete PhysicalDevice;
}

SwapChain* VulkanRenderInterface::CreateSwapChain(const SwapChainConfig& inSwapChainConfig, Surface* inSurface)
{
    VulkanSurface* SurfacePtr = (VulkanSurface*)inSurface;
    VulkanSwapChain* SwapChain = new VulkanSwapChain(Device, SurfacePtr, inSwapChainConfig);

    return SwapChain;
}

ICommandBuffer* VulkanRenderInterface::CreateCommandBuffer(const CommandBufferConfig& inCmdBufferConfig)
{
    VulkanQueue* CmdBufferQueue = (VulkanQueue*)inCmdBufferConfig.CommandQueue;
    VulkanCommandBuffer* CommandBufferPtr = CmdBufferQueue->GetCommandPool()->CreateCommandBuffer(0);
    CommandBufferPtr->AllocateCommandBuffer(inCmdBufferConfig);

    return CommandBufferPtr;
}

void VulkanRenderInterface::Free(ICommandBuffer* inCommandBufferToFree)
{
    VulkanCommandBuffer* CmdBufferPtr = (VulkanCommandBuffer*)inCommandBufferToFree;
    CmdBufferPtr->FreeCommandBuffer();
}

Buffer* VulkanRenderInterface::CreateBuffer(const BufferConfig& inBufferConfig)
{
    Buffer* Buff = VulkanMemoryHeapMain->AllocateBuffer(inBufferConfig);
    return Buff;
}

void VulkanRenderInterface::WriteToBuffer(Buffer* inBuffer, uint64 inOffset, const void* inData, uint64 inDataSize)
{
    VulkanBuffer* Buff = (VulkanBuffer*)inBuffer;
    uint8* MappedPointer = (uint8*)Buff->GetMappedPointer();
    MappedPointer += inOffset;

    memcpy(MappedPointer, inData, inDataSize);
}

void VulkanRenderInterface::ReadFromBuffer(Buffer* inBuffer, uint64 inOffset, void* outData, uint64 inDataSize)
{
    // Since as of rightnow we always map our memory and never unmap, it is already available for 
    // read on CPU 
    // so this is a waste but, for now, it will be fine

    VulkanBuffer* Buff = (VulkanBuffer*)inBuffer;
    uint8* MappedPointer = (uint8*)Buff->GetMappedPointer();
    MappedPointer += inOffset;

    memcpy(outData, MappedPointer, inDataSize);
}

void VulkanRenderInterface::Free(Buffer* inBuffer)
{
    // this is already handled in the memory heap side, but we can still clean up here 
    // bad should start using MemoryManager
    delete inBuffer;
}

Texture* VulkanRenderInterface::CreateTexture(const TextureConfig& inTextureConfig)
{
    VulkanTextureView* Texture = new VulkanTextureView(Device, inTextureConfig);
    Texture->CreateDefaultImageView();
    return Texture;
}

void VulkanRenderInterface::Free(Texture* inTexture)
{
    // Just delete the texture
    delete inTexture;
}

IFrameBuffer* VulkanRenderInterface::CreateFrameBuffer(const FrameBufferConfig& inFrameBufferConfig)
{
    VulkanFrameBuffer* FrameBuffer = new VulkanFrameBuffer(Device);
    FrameBuffer->Create(inFrameBufferConfig);
    return FrameBuffer;
}

void VulkanRenderInterface::Free(IFrameBuffer* inFrameBuffer)
{
    // Just delete the framebuffer
    delete inFrameBuffer;
}

IRenderPass* VulkanRenderInterface::CreateRenderPass(const RenderPassConfig& inRenderPassConfig)
{
    // This is not a great usage of how the render pass system was created to be used,
    // for now this works but change later for better and original use 

    VkRect2D Rect = { };
    Rect.extent.width = inRenderPassConfig.RenderArea.Width;
    Rect.extent.height = inRenderPassConfig.RenderArea.Height;

    VulkanRenderLayout RenderLayout(Device, inRenderPassConfig.ColorAttachments.size(), Rect);
    IRenderPass* RenderPass = new VulkanRenderPass(Device, RenderLayout, inRenderPassConfig);

    return RenderPass;
}

void VulkanRenderInterface::Free(IRenderPass* inRenderPass)
{
    // Just delete the render pass 
    delete inRenderPass;
}

PipelineLayout* VulkanRenderInterface::CreatePipelineLayout(const PipelineLayoutConfig& inPipelineLayoutConfig)
{
    VulkanPipelineLayout* Layout = new VulkanPipelineLayout(Device, inPipelineLayoutConfig);
    Layout->Create(nullptr);

    return Layout;
}

void VulkanRenderInterface::Free(PipelineLayout* inPipelineLayout)
{
    // Just delete the pipeline layout
    delete inPipelineLayout;
}

IPipeline* VulkanRenderInterface::CreatePipeline(const GraphicsPipelineConfig& inGraphicsPipelineConfig)
{
    VulkanGraphicsPipeline* Pipeline = new VulkanGraphicsPipeline(Device);
    Pipeline->Create(inGraphicsPipelineConfig);
    return Pipeline;
}

void VulkanRenderInterface::Free(IPipeline* inPipeline)
{
    // Just delete the pipeline
    delete inPipeline;
}

ISemaphore* VulkanRenderInterface::CreateRenderSemaphore(const SemaphoreConfig& inSemaphoreConfig)
{
    VulkanSemaphore* Semaphore = new VulkanSemaphore(Device);
    Semaphore->Create(inSemaphoreConfig);
    return Semaphore;
}

void VulkanRenderInterface::Free(ISemaphore* inSemaphore)
{
    // Just delete the semaphore
    delete inSemaphore;
}

IFence* VulkanRenderInterface::CreateFence()
{
    VulkanFence* Fence = new VulkanFence(Device);
    return Fence;
}

void VulkanRenderInterface::Free(IFence* inFence)
{
    // Just delete the fence
    delete inFence;
}

Shader* VulkanRenderInterface::CreateShader(const ShaderConfig& inShaderConfig)
{
    VulkanShader* ShaderPtr = ShaderFactoryMain->CreateShader(ShaderPoolMain, inShaderConfig);
    return ShaderPtr;
}

void VulkanRenderInterface::Free(Shader* inShader)
{
    // Just delete the shader
    delete inShader;
}

Sampler* VulkanRenderInterface::CreateSampler(const SamplerConfig& inSamplerConfig)
{
    VE_ASSERT(false, "[VulkanRenderInterface]: Creation of samplers still need to be implemented...!");
    return nullptr;
}

void VulkanRenderInterface::Free(Sampler* inSampler)
{
    // Just delete the sampler
    delete inSampler;
}

bool VulkanRenderInterface::CreateVulkanInstance(const VulkanRendererConfig& inVulkanRendererConfig)
{
    VkApplicationInfo ApplicationInfo = VulkanUtils::Initializers::ApplicationInfo();
    ApplicationInfo.pApplicationName = inVulkanRendererConfig.AppInstanceInfo.ApplicationName.c_str();
    ApplicationInfo.applicationVersion = inVulkanRendererConfig.AppInstanceInfo.ApplicationVersion;
    ApplicationInfo.pEngineName = inVulkanRendererConfig.AppInstanceInfo.EngineName.c_str();
    ApplicationInfo.apiVersion = VK_API_VERSION_1_3;

    std::vector<const char*> InstanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
    std::vector<const char*> InstanceLayers = { };

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
    if (inVulkanRendererConfig.EnabledInstanceExtensions.size() > 0)
    {
        for (uint32 i = 0; i < inVulkanRendererConfig.EnabledInstanceExtensions.size(); ++i)
        {
            // Output message if requested extension is not available
            if (std::find(SupportedInstanceExtensions.begin(), SupportedInstanceExtensions.end(), inVulkanRendererConfig.EnabledInstanceExtensions[i]) == SupportedInstanceExtensions.end())
            {
                VE_CORE_LOG_FATAL("Enabled Instance extension \"{0}\" is not present at Instance level", inVulkanRendererConfig.EnabledInstanceExtensions[i]);
            }

            InstanceExtensions.push_back(inVulkanRendererConfig.EnabledInstanceExtensions[i].c_str());
        }
    }

    // Enabled requested Instance layers
    if (inVulkanRendererConfig.EnabledInstanceLayers.size() > 0)
    {
        for (uint32 i = 0; i < inVulkanRendererConfig.EnabledInstanceLayers.size(); ++i)
        {
            // Output message if requested extension is not available
            if (std::find(SupportedInstanceLayers.begin(), SupportedInstanceLayers.end(), inVulkanRendererConfig.EnabledInstanceLayers[i]) == SupportedInstanceLayers.end())
            {
                VE_CORE_LOG_FATAL("Enabled Instance layer \"{0}\" is not present at Instance level", inVulkanRendererConfig.EnabledInstanceLayers[i]);
            }

            InstanceLayers.push_back(inVulkanRendererConfig.EnabledInstanceLayers[i].c_str());
        }
    }

    VkInstanceCreateInfo InstanceCreateInfo = VulkanUtils::Initializers::InstanceCreateInfo();
    InstanceCreateInfo.pNext = NULL;
    InstanceCreateInfo.pApplicationInfo = &ApplicationInfo;

    if (InstanceExtensions.size() > 0)
    {
#if _DEBUG
        InstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);	// SRS - Dependency when VK_EXT_DEBUG_MARKER is enabled
        InstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // _DEBUG

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

VkPhysicalDeviceFeatures VulkanRenderInterface::Convert(const PhysicalDeviceFeatures& inFeatures)
{
#define BOOL(x) (VkBool32)(x)

    VkPhysicalDeviceFeatures Features = { };

    Features.fillModeNonSolid = BOOL(inFeatures.FillModeNonSolid);
    Features.geometryShader = BOOL(inFeatures.GeometryShader);
    Features.tessellationShader = BOOL(inFeatures.TessellationShader);
    Features.multiViewport = BOOL(inFeatures.MultiViewports);
    Features.samplerAnisotropy = BOOL(inFeatures.SamplerAnisotropy);

    return Features;
}

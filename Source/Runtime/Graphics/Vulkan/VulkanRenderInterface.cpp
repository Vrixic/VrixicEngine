/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "VulkanRenderInterface.h"

#include <Core/Application.h>
#include <Runtime/Graphics/Vulkan/VulkanBuffer.h>
#include <Runtime/Graphics/Vulkan/VulkanCommandBuffer.h>
#include <Runtime/Graphics/Vulkan/VulkanFence.h>
#include <Runtime/Graphics/Vulkan/VulkanPipeline.h>
#include <Runtime/Graphics/Vulkan/VulkanRenderPass.h>
#include <Runtime/Graphics/Vulkan/VulkanSampler.h>
#include <Runtime/Graphics/Vulkan/VulkanSemaphore.h>
#include <Runtime/Graphics/Vulkan/VulkanTextureView.h>
#include "VulkanCommandBufferManager.h"

#include <External/imgui/Includes/imgui.h>
#include <External/imgui/Includes/imgui_impl_glfw.h>

#include <External/glfw/Includes/GLFW/glfw3.h>

VulkanRenderInterface::HImGuiData VulkanRenderInterface::ImGuiData = { };

VulkanRenderInterface::VulkanRenderInterface(const FVulkanRendererConfig& inVulkanRendererConfig)
{
    VE_FUNC_ASSERT(CreateVulkanInstance(inVulkanRendererConfig), true, "[VulkanRenderInterface]: failed to create a vulkan instance object..");

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
    CommandBufferManager = new VulkanCommandBufferManager(Device);

    FCommandBufferManagerConfig CommandBufferManagerConfig = { CommandBufferManager, VGameEngine::Get()->GetTaskScheduler().GetNumTaskThreads() };
    CommandBufferManager::Get().Init(&CommandBufferManagerConfig);

    // Create Resource Managements Resources
    {
        ShaderFactoryMain = new VulkanShaderFactory(Device);
        ShaderPoolMain = new VulkanShaderPool(Device);

        // allocate 1 gibibytes of memory -> 1024 mebibytes = 1 gib
        VulkanMemoryHeapMain = new VulkanMemoryHeap(Device, 1024);
    }

    // Create Descriptor Pools
    {
        static const uint32 MAX_GLOBAL_POOL_ELEMENTS = 128;
        VkDescriptorPoolSize DescriptorPoolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, MAX_GLOBAL_POOL_ELEMENTS },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_GLOBAL_POOL_ELEMENTS },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_GLOBAL_POOL_ELEMENTS },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_GLOBAL_POOL_ELEMENTS },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, MAX_GLOBAL_POOL_ELEMENTS },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, MAX_GLOBAL_POOL_ELEMENTS },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_GLOBAL_POOL_ELEMENTS },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_GLOBAL_POOL_ELEMENTS },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_GLOBAL_POOL_ELEMENTS },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, MAX_GLOBAL_POOL_ELEMENTS },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, MAX_GLOBAL_POOL_ELEMENTS}
        };
        uint32 ArraySize = (sizeof(DescriptorPoolSizes) / sizeof((DescriptorPoolSizes)[0]));
        uint32 MaxSets = MAX_GLOBAL_POOL_ELEMENTS * ArraySize;

        GlobalDescriptorPool = new VulkanDescriptorPool(Device, MaxSets, DescriptorPoolSizes, ArraySize);
    }
    {
        BindlessDescriptorPool = nullptr;
        if (Device->SupportsBindlessTexturing())
        {
            VkDescriptorPoolSize BindlessPoolSizes[] =
            {
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, PipelineLayout::MAX_NUM_BINDLESS_RESOURCES },
                //{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_BINDLESS_RESOURCES },
            };

            uint32 ArraySize = (sizeof(BindlessPoolSizes) / sizeof((BindlessPoolSizes)[0]));
            uint32 MaxSets = PipelineLayout::MAX_NUM_BINDLESS_RESOURCES * ArraySize;

            BindlessDescriptorPool = new VulkanDescriptorPool(Device, MaxSets, BindlessPoolSizes, ArraySize);

            VkDescriptorSetLayoutBinding DescriptorSetLayoutBindings[4];
            // Actual descriptor set layout
            VkDescriptorSetLayoutBinding& ImageSamplerBinding = DescriptorSetLayoutBindings[0];
            ImageSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            ImageSamplerBinding.descriptorCount = PipelineLayout::MAX_NUM_BINDLESS_RESOURCES;
            ImageSamplerBinding.binding = PipelineLayout::BINDLESS_TEXTURE_BINDING_INDEX;
            ImageSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            ImageSamplerBinding.pImmutableSamplers = nullptr;

            //VkDescriptorSetLayoutBinding& StorageImageBinding = DescriptorSetLayoutBindings[1];
            //StorageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            //StorageImageBinding.descriptorCount = MAX_BINDLESS_RESOURCES;
            //StorageImageBinding.binding = BINDLESS_TEXTURE_BINDING_NUM + 1;
            //StorageImageBinding.stageFlags = VK_SHADER_STAGE_ALL;
            //StorageImageBinding.pImmutableSamplers = nullptr;

            VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
            DescriptorSetLayoutCreateInfo.bindingCount = ArraySize;
            DescriptorSetLayoutCreateInfo.pBindings = DescriptorSetLayoutBindings;
            DescriptorSetLayoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

            // TODO: reenable variable descriptor count
            // Binding flags
            VkDescriptorBindingFlags BindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | /*VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |*/ VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
            VkDescriptorBindingFlags BindingFlags[4];

            BindingFlags[0] = BindlessFlags;
            BindingFlags[1] = BindlessFlags;

            VkDescriptorSetLayoutBindingFlagsCreateInfoEXT ExtendedInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT, nullptr };
            ExtendedInfo.bindingCount = ArraySize;
            ExtendedInfo.pBindingFlags = BindingFlags;

            DescriptorSetLayoutCreateInfo.pNext = &ExtendedInfo;

            vkCreateDescriptorSetLayout(*Device->GetDeviceHandle(), &DescriptorSetLayoutCreateInfo, nullptr, &BindlessDescriptorSetLayout);
        }
    }

    // we could also create default resources for use???....
}

void VulkanRenderInterface::Shutdown()
{
    Device->WaitUntilIdle();

    delete CommandBufferManager;

    delete GlobalDescriptorPool;
    delete BindlessDescriptorPool;

    vkDestroyDescriptorSetLayout(*Device->GetDeviceHandle(), BindlessDescriptorSetLayout, nullptr);

    ShutdownImGui();

    delete VulkanMemoryHeapMain;

    delete ShaderFactoryMain;
    delete ShaderPoolMain;

    delete Device;
    vkDestroyInstance(VulkanInstance, nullptr);

    delete PhysicalDevice;
}

SwapChain* VulkanRenderInterface::CreateSwapChain(const FSwapChainConfig& inSwapChainConfig, Surface* inSurface)
{
    VulkanSurface* SurfacePtr = (VulkanSurface*)inSurface;
    VulkanSwapChain* SwapChain = new VulkanSwapChain(Device, SurfacePtr, inSwapChainConfig);

    return SwapChain;
}

ICommandBuffer* VulkanRenderInterface::CreateCommandBuffer(const FCommandBufferConfig& inCmdBufferConfig)
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

Buffer* VulkanRenderInterface::CreateBuffer(const FBufferConfig& inBufferConfig)
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
    //memcpy(MappedPointer, inData, inDataSize);
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

TextureResource* VulkanRenderInterface::CreateTexture(const FTextureConfig& inTextureConfig)
{
    VulkanTextureView* Texture = new VulkanTextureView(Device, inTextureConfig);
    Texture->CreateDefaultImageView();
    return Texture;
}

void VulkanRenderInterface::WriteToTexture(const TextureResource* inTexture, const FTextureWriteInfo& inTextureWriteInfo)
{
    VE_ASSERT(inTexture != nullptr, VE_TEXT("{VulkanRenderInterface]: cannot write to a invalid texture...its null..."));

    VkCommandBuffer CommandBufferHandle = Device->GetGraphicsQueue()->CreateSingleTimeCommandBuffer(true);

    VulkanTextureView* VulkanTexture = (VulkanTextureView*)inTexture;

    // Pre copy memory barrier to perform texture layout transition
    Device->AddImageBarrier
    (
        CommandBufferHandle, VulkanTexture, inTextureWriteInfo.Subresource,
        VulkanTexture->GetImageLayout(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        0, VK_ACCESS_TRANSFER_WRITE_BIT, ERenderQueueType::Graphics, ERenderQueueType::Graphics
    );

    // Copy buffer data into the texture/image
    {
        HCopyBufferTextureInfo CopyBufferToTextureInfo = { };
        CopyBufferToTextureInfo.CommandBufferHandle = CommandBufferHandle;
        CopyBufferToTextureInfo.TextureHandle = VulkanTexture;
        CopyBufferToTextureInfo.Subresource = &inTextureWriteInfo.Subresource;
        CopyBufferToTextureInfo.Offset.x = inTextureWriteInfo.Offset.Width;
        CopyBufferToTextureInfo.Offset.y = inTextureWriteInfo.Offset.Height;
        CopyBufferToTextureInfo.Offset.z = inTextureWriteInfo.Offset.Depth;

        VulkanBuffer* BufferHandle = (VulkanBuffer*)inTextureWriteInfo.BufferHandle;
        CopyBufferToTextureInfo.BufferHandle = BufferHandle;

        CopyBufferToTextureInfo.Extent.width = inTextureWriteInfo.Extent.Width;
        CopyBufferToTextureInfo.Extent.height = inTextureWriteInfo.Extent.Height;
        CopyBufferToTextureInfo.Extent.depth = inTextureWriteInfo.Extent.Depth;

        if (VulkanTexture->GetKtxTextureHandle() != nullptr)
        {
            Device->CopyBufferToTextureKtx(CopyBufferToTextureInfo);
        }
        else
        {
            Device->CopyBufferToTexture(CopyBufferToTextureInfo);
        }
    }

    // Post copy memory barrier 
    Device->AddImageBarrier
    (
        CommandBufferHandle, VulkanTexture, inTextureWriteInfo.Subresource,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 
        ERenderQueueType::Graphics, ERenderQueueType::Graphics
    );

    Device->GetGraphicsQueue()->FlushSingleTimeCommandBuffer(CommandBufferHandle, true);

    /** Change the image layout for the texture passed in */
    VulkanTexture->SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void VulkanRenderInterface::ReadFromTexture(const TextureResource* inTexture, const FTextureSection& inTextureSection, const ETextureLayout inFinalTextureLayout, FTextureReadInfo& outTextureReadInfo)
{
    VulkanTextureView* VulkanTexture = (VulkanTextureView*)inTexture;

    const FOffset3D Offset = CalculateTextureOffsetByType(VulkanTexture->GetType(), inTextureSection.Offset, 0);
    const FExtent3D Extent = CalculateTextureExtentByType(VulkanTexture->GetType(), inTextureSection.Extent, 0);
    const EPixelFormat Format = VulkanTypeConverter::Convert(VulkanTexture->GetImageFormat());

    uint32 ImageSize = Extent.Width * Extent.Height * Extent.Depth * inTextureSection.Subresource.NumArrayLayers;
    // Assume they are BGRA8 or RG16
    uint32 ImageDataSize = ImageSize * 4;

    FBufferConfig BufferConfig = { };
    BufferConfig.InitialData = nullptr;
    BufferConfig.Size = ImageDataSize;
    BufferConfig.UsageFlags = FResourceBindFlags::DstTransfer | FResourceBindFlags::StagingBuffer;
    BufferConfig.MemoryFlags = FMemoryFlags::HostVisible | FMemoryFlags::HostCoherent;

    VulkanBuffer* StagingBuffer = VulkanMemoryHeapMain->AllocateBuffer(BufferConfig);

    // Copy the newly created staging buffer into hardware texture and then transfer image
    // into a state where we can sample from
    VkCommandBuffer CommandBufferHandle = Device->GetGraphicsQueue()->CreateSingleTimeCommandBuffer(true);

    HTransitionTextureLayoutInfo LayoutInfo = { };
    LayoutInfo.CommandBufferHandle = CommandBufferHandle;
    LayoutInfo.TextureHandle = VulkanTexture;
    LayoutInfo.OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    LayoutInfo.NewLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    LayoutInfo.Subresource = &inTextureSection.Subresource;

    Device->TransitionTextureLayout(LayoutInfo);

    HCopyBufferTextureInfo CopyInfo = { };
    CopyInfo.CommandBufferHandle = CommandBufferHandle;
    CopyInfo.BufferHandle = StagingBuffer;
    CopyInfo.Subresource = &inTextureSection.Subresource;
    CopyInfo.Extent = { Extent.Width, Extent.Height, Extent.Depth };
    CopyInfo.Offset = { Offset.X, Offset.Y, Offset.Z };
    CopyInfo.TextureHandle = VulkanTexture;

    Device->CopyTextureToBuffer(CopyInfo);

    LayoutInfo.OldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    LayoutInfo.NewLayout = VulkanTypeConverter::ConvertTextureLayoutToVk(inFinalTextureLayout);// VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    Device->TransitionTextureLayout(LayoutInfo);

    Device->GetGraphicsQueue()->FlushSingleTimeCommandBuffer(CommandBufferHandle, true);

    // Map the staging buffer to a CPU memory space
    //StagingBuffer->Map(VK_WHOLE_SIZE, 0);
    TPointer<uint8> MemoryPtr = MemoryManager::Get().MallocAligned<uint8>(ImageDataSize);
    memcpy(MemoryPtr.Get(), StagingBuffer->GetMappedPointer(), ImageDataSize);
    //StagingBuffer->Unmap();

    outTextureReadInfo.Data = MemoryPtr.Get();
    outTextureReadInfo.SizeInByte = ImageDataSize;
    outTextureReadInfo.Format = Format;

    /** Change the image layout for the texture passed in */
    VulkanTexture->SetImageLayout(LayoutInfo.NewLayout);
}

void VulkanRenderInterface::SetTextureLayout(const TextureResource* inTexture, const ETextureLayout inNewTextureLayout)
{
    VulkanTextureView* Texture = (VulkanTextureView*)inTexture;
    Texture->SetImageLayout(VulkanTypeConverter::ConvertTextureLayoutToVk(inNewTextureLayout));
}

void VulkanRenderInterface::Free(TextureResource* inTexture)
{
    // Just delete the texture
    delete inTexture;
}

IFrameBuffer* VulkanRenderInterface::CreateFrameBuffer(const FFrameBufferConfig& inFrameBufferConfig)
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

IRenderPass* VulkanRenderInterface::CreateRenderPass(const FRenderPassConfig& inRenderPassConfig)
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

PipelineLayout* VulkanRenderInterface::CreatePipelineLayout(const FPipelineLayoutConfig& inPipelineLayoutConfig) const
{
    VulkanPipelineLayout* Layout = new VulkanPipelineLayout(Device, inPipelineLayoutConfig);

    // meaning we have bindless set 
    if (inPipelineLayoutConfig.NumSets > 1)
    {
        Layout->GetDescriptorSetsLayoutHandle()->DescriptorSetLayoutHandles.push_back(BindlessDescriptorSetLayout);
        Layout->Create(nullptr);
        Layout->GetDescriptorSetsLayoutHandle()->DescriptorSetLayoutHandles.pop_back();

        return Layout;
    }

    Layout->Create(nullptr);

    return Layout;
}

PipelineLayout* VulkanRenderInterface::CreatePipelineLayoutFromShaders(const Shader** inShaders, uint8 inNumShaders) const
{
    FPipelineLayoutConfig LayoutConfig = { };

    for (uint8 shaderIndex = 0; shaderIndex < inNumShaders; ++shaderIndex)
    {
        VulkanShader* VulkShader = (VulkanShader*)inShaders[shaderIndex];
        VulkShader->ParseSpirvCodeIntoPipelineLayoutConfig(LayoutConfig);
    }

    return CreatePipelineLayout(LayoutConfig);
}

void VulkanRenderInterface::Free(PipelineLayout* inPipelineLayout)
{
    // Just delete the pipeline layout
    delete inPipelineLayout;
}

IPipeline* VulkanRenderInterface::CreatePipeline(const FGraphicsPipelineConfig& inGraphicsPipelineConfig)
{
    VulkanGraphicsPipeline* Pipeline = new VulkanGraphicsPipeline(Device);
    Pipeline->Create(inGraphicsPipelineConfig);
    return Pipeline;
}

IPipeline* VulkanRenderInterface::CreatePipelineWithCache(const FGraphicsPipelineConfig& inGraphicsPipelineConfig, const std::string& inPipelineCachePath)
{
    VulkanGraphicsPipeline* Pipeline = new VulkanGraphicsPipeline(Device);

    VkPipelineCache PipelineCache = VK_NULL_HANDLE;
    VkPipelineCacheCreateInfo PipelineCacheCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };

    WIN32_FILE_ATTRIBUTE_DATA FileInformation;
    bool bShouldCreateNewCache = GetFileAttributesExA(inPipelineCachePath.c_str(), GetFileExInfoStandard, &FileInformation);
    if (bShouldCreateNewCache)
    {
        FILE* FileHandle = fopen(inPipelineCachePath.c_str(), "rb");

        uint8* Data = nullptr;
        uint64 DataSize = 0;
        if (FileHandle) {

            fseek(FileHandle, 0, SEEK_END);
            DataSize = ftell(FileHandle);
            fseek(FileHandle, 0, SEEK_SET);

            Data = (uint8*)new uint8[DataSize];
            fread(Data, DataSize, 1, FileHandle);

            fclose(FileHandle);
        }

        VkPipelineCacheHeaderVersionOne* PipelineCacheHeader = (VkPipelineCacheHeaderVersionOne*)Data;

        if (PipelineCacheHeader->deviceID == Device->GetPhysicalDeviceProperties()->deviceID &&
            PipelineCacheHeader->vendorID == Device->GetPhysicalDeviceProperties()->vendorID &&
            memcmp(PipelineCacheHeader->pipelineCacheUUID, Device->GetPhysicalDeviceProperties()->pipelineCacheUUID, VK_UUID_SIZE) == 0)
        {
            PipelineCacheCreateInfo.initialDataSize = DataSize;
            PipelineCacheCreateInfo.pInitialData = Data;

            bShouldCreateNewCache = false;
        }

        vkCreatePipelineCache(*Device->GetDeviceHandle(), &PipelineCacheCreateInfo, nullptr, &PipelineCache);

        delete[] Data;
    }
    else
    {
        vkCreatePipelineCache(*Device->GetDeviceHandle(), &PipelineCacheCreateInfo, nullptr, &PipelineCache);
        bShouldCreateNewCache = true;
    }

    Pipeline->Create(inGraphicsPipelineConfig, PipelineCache, bShouldCreateNewCache ? inPipelineCachePath.c_str() : nullptr);
    return Pipeline;
}

void VulkanRenderInterface::Free(IPipeline* inPipeline)
{
    // Just delete the pipeline
    delete inPipeline;
}

ISemaphore* VulkanRenderInterface::CreateRenderSemaphore(const FSemaphoreConfig& inSemaphoreConfig)
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

Shader* VulkanRenderInterface::CreateShader(const FShaderConfig& inShaderConfig)
{
    VulkanShader* ShaderPtr = ShaderFactoryMain->CreateShader(ShaderPoolMain, inShaderConfig);
    return ShaderPtr;
}

void VulkanRenderInterface::Free(Shader* inShader)
{
    // Just delete the shader
    delete inShader;
}

Sampler* VulkanRenderInterface::CreateSampler(const FSamplerConfig& inSamplerConfig)
{
    VulkanSampler* SamplerVk = new VulkanSampler(Device);
    SamplerVk->Create(inSamplerConfig);

    return SamplerVk;
}

void VulkanRenderInterface::Free(Sampler* inSampler)
{
    // Just delete the sampler
    delete inSampler;
}

IDescriptorSets* VulkanRenderInterface::CreateDescriptorSet(FDescriptorSetsConfig& inDescriptorSetConfig)
{
    VulkanDescriptorSets* DescriptorSet = new VulkanDescriptorSets(Device, inDescriptorSetConfig.NumSets);

    if (inDescriptorSetConfig.bIsBindlessSet)
    {
        VE_ASSERT(BindlessDescriptorPool->AllocateDescriptorSets(DescriptorSet, &BindlessDescriptorSetLayout), VE_TEXT("[VulkanRenderInterface]: Failed to allocate a descriptor set that is bindless..."));
        return DescriptorSet;
    }

    VulkanPipelineLayout* VPipelineLayout = (VulkanPipelineLayout*)inDescriptorSetConfig.PipelineLayoutPtr;
    VE_ASSERT(GlobalDescriptorPool->AllocateDescriptorSets(DescriptorSet, VPipelineLayout->GetDescriptorSetsLayoutHandle(), 0), VE_TEXT("[VulkanRenderInterface]: Failed to allocate a descriptor set that is bindless..."));

    return DescriptorSet;
}

void VulkanRenderInterface::Free(IDescriptorSets* inDescriptorSets)
{
    // Just delete the descriptor set(s)
    delete inDescriptorSets;
}

bool VulkanRenderInterface::SupportsBindlessTexturing() const
{
    return Device->SupportsBindlessTexturing();
}

void VulkanRenderInterface::InitImGui(SwapChain* inMainSwapChain, Surface* inSurface)
{
    VE_ASSERT(Device->GetDeviceHandle() != nullptr, "[VulkanRenderInterface]: Cannot init ImGui without a valid VulkanDevice.. Have you called VulkanDevice::Create()??");

    ImGuiData.Instance = VulkanInstance;
    ImGuiData.Device = *Device->GetDeviceHandle();

    VulkanSurface* VkSurface = (VulkanSurface*)inSurface;

    // Create Descriptor Pool
    {
        VkDescriptorPoolSize PoolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = {};
        DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        DescriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        DescriptorPoolCreateInfo.maxSets = 1000 * IM_ARRAYSIZE(PoolSizes);
        DescriptorPoolCreateInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(PoolSizes);
        DescriptorPoolCreateInfo.pPoolSizes = PoolSizes;

        VK_CHECK_RESULT(vkCreateDescriptorPool(ImGuiData.Device, &DescriptorPoolCreateInfo, ImGuiData.AllocatorCallback, &ImGuiData.DescriptorPool), "[VulkanRenderInterface]: Failed to create a descriptor pool for ImGui!!");
    }

    // Render Layout and Renderpass creation
    {
        VkRect2D RenderArea = { {0,0},
            { Application::Get()->GetWindow().GetWidth(), Application::Get()->GetWindow().GetHeight()} };
        ImGuiData.RenderLayout = new VulkanRenderLayout(Device, 1, RenderArea);

        // attachments 
        {
            std::vector<VkAttachmentDescription> Attachment;
            Attachment.resize(1);
            Attachment[0].format = VkSurface->GetSurfaceFormat()->format;
            Attachment[0].samples = VK_SAMPLE_COUNT_1_BIT;
            Attachment[0].loadOp = 0 ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            Attachment[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            Attachment[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            Attachment[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            Attachment[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            Attachment[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            ImGuiData.RenderLayout->SetAttachments(Attachment);

            VkAttachmentReference ColorAttachment;
            ColorAttachment.attachment = 0;
            ColorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            ImGuiData.RenderLayout->SetColorReference(ColorAttachment);
        }

        // Subpass dependency 
        std::vector<VkSubpassDependency> SubpassDependency;
        SubpassDependency.resize(1);
        SubpassDependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        SubpassDependency[0].dstSubpass = 0;
        SubpassDependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        SubpassDependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        SubpassDependency[0].srcAccessMask = 0;
        SubpassDependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        // Create renderpass from render layout and subpass dependency 
        ImGuiData.RenderPass = new VulkanRenderPass(Device, *ImGuiData.RenderLayout, SubpassDependency);
    }

    // Frame Buffers creation
    {
        VkImageView Attachment[1];
        VkExtent2D Extent = { Application::Get()->GetWindow().GetWidth(), Application::Get()->GetWindow().GetHeight() };

        ImGuiData.FrameBuffers.resize(inMainSwapChain->GetImageCount());
        for (uint32_t i = 0; i < inMainSwapChain->GetImageCount(); i++)
        {
            VulkanTextureView* TextureView = (VulkanTextureView*)inMainSwapChain->GetTextureAt(i);
            Attachment[0] = *TextureView->GetImageViewHandle();

            ImGuiData.FrameBuffers[i] = new VulkanFrameBuffer(Device);
            ImGuiData.FrameBuffers[i]->Create(1, Attachment, &Extent, ImGuiData.RenderPass);
        }
    }

    {
        IMGUI_CHECKVERSION();
        ImGui::SetCurrentContext(ImGui::CreateContext());
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)Application::Get()->GetWindow().GetGLFWNativeHandle(), true);
        ImGui_ImplVulkan_InitInfo ImguiVulkanInitInfo = { 0 };
        ImguiVulkanInitInfo.Instance = VulkanInstance;
        ImguiVulkanInitInfo.PhysicalDevice = *Device->GetPhysicalDeviceHandle();
        ImguiVulkanInitInfo.Device = *Device->GetDeviceHandle();
        ImguiVulkanInitInfo.QueueFamily = Device->GetGraphicsQueue()->GetFamilyIndex();
        ImguiVulkanInitInfo.Queue = Device->GetGraphicsQueue()->GetQueueHandle();
        ImguiVulkanInitInfo.PipelineCache = VK_NULL_HANDLE;
        ImguiVulkanInitInfo.DescriptorPool = ImGuiData.DescriptorPool;
        ImguiVulkanInitInfo.Subpass = 0;
        ImguiVulkanInitInfo.MinImageCount = 2;
        ImguiVulkanInitInfo.ImageCount = inMainSwapChain->GetImageCount();
        ImguiVulkanInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        ImguiVulkanInitInfo.Allocator = nullptr;
        ImguiVulkanInitInfo.CheckVkResultFn = ImguiCheckVkResultFunc;
        ImGui_ImplVulkan_Init(&ImguiVulkanInitInfo, *ImGuiData.RenderPass->GetRenderPassHandle());
    }

    VulkanQueue* VkCommandQueue = (VulkanQueue*)GetCommandQueue();

    // Upload Fonts
    {
        //// Load Fonts
        // // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
        // // - Read 'docs/FONTS.md' for more instructions and details.
        // // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        // //io.Fonts->AddFontDefault();
        // //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
        // //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        // //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        // //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        // //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
        // //IM_ASSERT(font != NULL);
        // 
        // Use any command queue
        //VkCommandPool CommandPool = VkCommandQueue->GetCommandPool()->GetCommandPoolHandle();
        VkCommandBuffer CommandBuffer = *CommandBufferManager->GetCommandBuffer(0, 0)->GetCommandBufferHandle(); //*VkCommandQueue->GetCommandPool()->GetCommandBuffer(0)->GetCommandBufferHandle();

        //VK_CHECK_RESULT(vkResetCommandPool(*Device->GetDeviceHandle(), CommandPool, 0), "[]");
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK_RESULT(vkBeginCommandBuffer(CommandBuffer, &begin_info), "");

        ImGui_ImplVulkan_CreateFontsTexture(CommandBuffer);

        VkSubmitInfo SubmitInfo = {};
        SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        SubmitInfo.commandBufferCount = 1;
        SubmitInfo.pCommandBuffers = &CommandBuffer;
        VK_CHECK_RESULT(vkEndCommandBuffer(CommandBuffer), "");
        VK_CHECK_RESULT(vkQueueSubmit(Device->GetGraphicsQueue()->GetQueueHandle(), 1, &SubmitInfo, VK_NULL_HANDLE), "");

        Device->WaitUntilIdle();
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

void VulkanRenderInterface::BeginImGuiFrame() const
{
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void VulkanRenderInterface::RenderImGui(const ICommandBuffer* inCommandBuffer, uint32 inCurrentImageIndex) const
{
    ImGuiIO& io = ImGui::GetIO();
    Application& app = *Application::Get();
    io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

    // Rendering
    ImGui::Render();
    ImDrawData* main_draw_data = ImGui::GetDrawData();
    const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
    if (!main_is_minimized)
    {
        //VulkanRenderer::Get()->BeginImguiRenderPass(&ImguiWindowData);

        FRenderPassBeginInfo RPBeginInfo = { };

        RPBeginInfo.ClearValues = nullptr;
        RPBeginInfo.NumClearValues = 0;

        RPBeginInfo.RenderPassPtr = ImGuiData.RenderPass;
        RPBeginInfo.FrameBuffer = ImGuiData.FrameBuffers[inCurrentImageIndex];

        inCommandBuffer->BeginRenderPass(RPBeginInfo);

        VulkanCommandBuffer* VkCurrentCommandBuffer = (VulkanCommandBuffer*)inCommandBuffer;

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(main_draw_data, *VkCurrentCommandBuffer->GetCommandBufferHandle());
        inCommandBuffer->EndRenderPass();
    }

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void VulkanRenderInterface::EndImGuiFrame() const
{
}

void VulkanRenderInterface::OnRenderViewportResized(SwapChain* inMainSwapchain, const FExtent2D& inNewRenderViewport)
{
    VulkanSwapChain* SwapchainVk = (VulkanSwapChain*)inMainSwapchain;
    ImGui_ImplVulkan_SetMinImageCount(SwapchainVk->GetMinImageCount());
    /**
    * Recreate dear imgui frame buffers and update render area for renderpass
    */
    {
        for (uint32 i = 0; i < ImGuiData.FrameBuffers.size(); i++)
        {
            delete ImGuiData.FrameBuffers[i];
        }

        // Frame Buffers creation
        VkImageView Attachment[1];
        VkExtent2D Extent = { Application::Get()->GetWindow().GetWidth(), Application::Get()->GetWindow().GetHeight() };

        ImGuiData.FrameBuffers.resize(inMainSwapchain->GetImageCount());
        for (uint32_t i = 0; i < inMainSwapchain->GetImageCount(); i++)
        {
            VulkanTextureView* TextureView = (VulkanTextureView*)inMainSwapchain->GetTextureAt(i);
            Attachment[0] = *TextureView->GetImageViewHandle();

            ImGuiData.FrameBuffers[i] = new VulkanFrameBuffer(Device);
            ImGuiData.FrameBuffers[i]->Create(1, Attachment, &Extent, ImGuiData.RenderPass);
        }

        FRect2D NewRenderArea = { };
        NewRenderArea.Width = inNewRenderViewport.Width;
        NewRenderArea.Height = inNewRenderViewport.Height;
        ImGuiData.RenderPass->UpdateRenderArea(NewRenderArea);
    }

    ImGuiIO& IO = ImGui::GetIO();
    IO.DisplaySize = { (float)inNewRenderViewport.Width, (float)inNewRenderViewport.Height };
}

void VulkanRenderInterface::ShutdownImGui()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete ImGuiData.RenderLayout;
    delete ImGuiData.RenderPass;

    for (uint32 i = 0; i < ImGuiData.FrameBuffers.size(); i++)
    {
        delete ImGuiData.FrameBuffers[i];
    }

    vkDestroyDescriptorPool(*Device->GetDeviceHandle(), ImGuiData.DescriptorPool, nullptr);
}

bool VulkanRenderInterface::CreateVulkanInstance(const FVulkanRendererConfig& inVulkanRendererConfig)
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

#if _DEBUG
    // Debug Setup
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    VulkanUtils::DebugUtils::PopulateDebugMessengerCreateInfo(debugCreateInfo);

    InstanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#endif

    return vkCreateInstance(&InstanceCreateInfo, nullptr, &VulkanInstance) == VK_SUCCESS;
}

VkPhysicalDeviceFeatures VulkanRenderInterface::Convert(const FPhysicalDeviceFeatures& inFeatures)
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

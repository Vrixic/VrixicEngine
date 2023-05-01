/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "VulkanCommandBuffer.h"
#include <Misc/Defines/VulkanProfilerDefines.h>
#include <Misc/Defines/StringDefines.h>

#include <Runtime/Graphics/Vulkan/VulkanFence.h>
#include <Runtime/Graphics/Vulkan/VulkanSemaphore.h>
#include <Runtime/Graphics/Vulkan/VulkanTypeConverter.h>
#include "VulkanTextureView.h"

/* ------------------------------------------------------------------------------- */
/* -----------------------             Device             ------------------------ */
/* ------------------------------------------------------------------------------- */

VulkanDevice::VulkanDevice(VkPhysicalDevice& gpu, VkPhysicalDeviceFeatures& enabledFeatures, uint32 deviceExtensionCount, const char** deviceExtensions)
    : PhysicalDeviceHandle(gpu), LogicalDeviceHandle(VK_NULL_HANDLE), GraphicsQueue(nullptr), ComputeQueue(nullptr), TransferQueue(nullptr)
{
    VE_PROFILE_VULKAN_FUNCTION();

    // Store Physical Device properties 
    vkGetPhysicalDeviceProperties(gpu, &PhysicalDeviceProperties);
    vkGetPhysicalDeviceFeatures(gpu, &PhysicalDeviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(gpu, &PhysicalDeviceMemProperties);

    // Queue family properties, used for setting up requested queues upon device creation
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDeviceHandle, &QueueFamilyCount, nullptr);
    ASSERT(QueueFamilyCount > 0, "[VulkanDevice]: No queue families found on physical device (GPU)!");
    QueueFamilyProperties.resize(QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDeviceHandle, &QueueFamilyCount, QueueFamilyProperties.data());

    PhysicalDeviceFeatures = enabledFeatures;

    for (uint32 i = 0; i < deviceExtensionCount; ++i)
    {
        PhysicalDeviceExtensions.push_back(std::string(deviceExtensions[i]));
    }
}

VulkanDevice::~VulkanDevice()
{
    VE_PROFILE_VULKAN_FUNCTION();

    if (LogicalDeviceHandle != VK_NULL_HANDLE)
    {
        delete TransferQueue;
        delete ComputeQueue;
        delete GraphicsQueue;

        vkDestroyDevice(LogicalDeviceHandle, nullptr);
        LogicalDeviceHandle = VK_NULL_HANDLE;
    }
}

/* Creates the logical device */
void VulkanDevice::CreateDevice(VulkanSurface* surface)
{
    VE_PROFILE_VULKAN_FUNCTION();

    // supported device extensions
    uint32 DeviceExtensionCount = 0;
    std::vector<std::string> SupportedDeviceExtensions;
    vkEnumerateDeviceExtensionProperties(PhysicalDeviceHandle, nullptr, &DeviceExtensionCount, nullptr);
    if (DeviceExtensionCount > 0)
    {
        std::vector<VkExtensionProperties> DeviceExtensions(DeviceExtensionCount);
        if (vkEnumerateDeviceExtensionProperties(PhysicalDeviceHandle, nullptr, &DeviceExtensionCount, &DeviceExtensions.front()) == VK_SUCCESS)
        {
            for (uint32 i = 0; i < DeviceExtensionCount; ++i)
            {
                SupportedDeviceExtensions.push_back(DeviceExtensions[i].extensionName);
            }
        }
    }

    bool UseSwapChain = true;
    VkQueueFlags RequestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

    // Desired queues need to be requested upon logical device creation
    // Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
    // requests different queue types

    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos = { };

    // Get queue family indices for the requested queue family types
    // Note that the indices may overlap depending on the implementation

    const float DefaultQueuePriority = 0.0f;

    /* Index to the family of the queue */
    uint32 GraphicsQueueFamilyIndex = -1;
    uint32 ComputeQueueFamilyIndex = -1;
    uint32 TransferQueueFamilyIndex = -1;

    // Graphics Queue
    if (RequestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
    {
        GraphicsQueueFamilyIndex = VulkanUtils::Helpers::GetQueueFamilyIndex(QueueFamilyProperties, VK_QUEUE_GRAPHICS_BIT);
        VkDeviceQueueCreateInfo QueueInfo = VulkanUtils::Initializers::DeviceQueueCreateInfo(GraphicsQueueFamilyIndex, 1, &DefaultQueuePriority);
        QueueCreateInfos.push_back(QueueInfo);
    }
    else
    {
        GraphicsQueueFamilyIndex = 0;
    }

    // Dedicated Compute queue
    if (RequestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
    {
        ComputeQueueFamilyIndex = VulkanUtils::Helpers::GetQueueFamilyIndex(QueueFamilyProperties, VK_QUEUE_COMPUTE_BIT);
        if (ComputeQueueFamilyIndex != GraphicsQueueFamilyIndex)
        {
            // If Compute family index differs, we need an additional queue create info for the Compute queue
            VkDeviceQueueCreateInfo QueueInfo = VulkanUtils::Initializers::DeviceQueueCreateInfo(ComputeQueueFamilyIndex, 1, &DefaultQueuePriority);
            QueueCreateInfos.push_back(QueueInfo);
        }
    }
    else
    {
        // Else we use the same queue
        ComputeQueueFamilyIndex = GraphicsQueueFamilyIndex;
    }

    // Dedicated Transfer queue
    if (RequestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
    {
        TransferQueueFamilyIndex = VulkanUtils::Helpers::GetQueueFamilyIndex(QueueFamilyProperties, VK_QUEUE_TRANSFER_BIT);
        if ((TransferQueueFamilyIndex != GraphicsQueueFamilyIndex) && (TransferQueueFamilyIndex != ComputeQueueFamilyIndex))
        {
            // If Transfer family index differs, we need an additional queue create info for the Transfer queue
            VkDeviceQueueCreateInfo QueueInfo = VulkanUtils::Initializers::DeviceQueueCreateInfo(TransferQueueFamilyIndex, 1, &DefaultQueuePriority);
            QueueCreateInfos.push_back(QueueInfo);
        }
    }
    else
    {
        // Else we use the same queue
        TransferQueueFamilyIndex = GraphicsQueueFamilyIndex;
    }

    // Create the logical device representation
    std::vector<const char*> DeviceExtensions = { };
    for (uint32 i = 0; i < PhysicalDeviceExtensions.size(); ++i)
    {
        DeviceExtensions.push_back(PhysicalDeviceExtensions[i].c_str());
    }

    if (UseSwapChain)
    {
        DeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    VkDeviceCreateInfo DeviceCreateInfo = VulkanUtils::Initializers::DeviceCreateInfo();
    DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());;
    DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
    DeviceCreateInfo.pEnabledFeatures = &PhysicalDeviceFeatures;

    if (DeviceExtensions.size() > 0)
    {
        for (uint32 i = 0; i < DeviceExtensions.size(); ++i)
        {
            if (!VulkanUtils::Helpers::ExtensionSupported(DeviceExtensions[i], SupportedDeviceExtensions))
            {
                std::cerr << "Enabled device extension \"" << DeviceExtensions[i] << "\" is not supported!";
            }
        }

        DeviceCreateInfo.enabledExtensionCount = (uint32)DeviceExtensions.size();
        DeviceCreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();
    }

    VK_CHECK_RESULT(vkCreateDevice(PhysicalDeviceHandle, &DeviceCreateInfo, nullptr, &LogicalDeviceHandle), "[VulkanDevice]: Failed to create a logical device!");

    // Iterate over each queue to learn whether it supports presenting:
    // Find a queue with present support
    // Will be used to present the swap chain images to the windowing system
    std::vector<VkBool32> SupportsPresent(QueueFamilyCount);
    for (uint32 i = 0; i < QueueFamilyCount; ++i)
    {
        surface->fpGetPhysicalDeviceSurfaceSupportKHR(PhysicalDeviceHandle, i, *surface->GetSurfaceHandle(), &SupportsPresent[i]);
    }

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    uint32_t GraphicsQueueNodeIndex = UINT32_MAX;
    uint32_t TransferQueueNodeIndex = UINT32_MAX;
    uint32_t ComputeQueueNodeIndex = UINT32_MAX;

    uint32_t PresentQueueNodeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < QueueFamilyCount; i++)
    {
        if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            if (GraphicsQueueNodeIndex == UINT32_MAX)
            {
                GraphicsQueueNodeIndex = i;
            }

            if (SupportsPresent[i] == VK_TRUE)
            {
                GraphicsQueueNodeIndex = i;
                PresentQueueNodeIndex = i;
                break;
            }
        }
    }

    if (PresentQueueNodeIndex == UINT32_MAX)
    {
        // If there's no queue that supports both present and graphics
        // try to find a separate present queue
        for (uint32_t i = 0; i < QueueFamilyCount; ++i)
        {
            if (SupportsPresent[i] == VK_TRUE)
            {
                PresentQueueNodeIndex = i;
                break;
            }
        }
    }

    // Exit if either a graphics or a presenting queue hasn't been found
    if (GraphicsQueueNodeIndex == UINT32_MAX || PresentQueueNodeIndex == UINT32_MAX)
    {
        //std::cout << "Could not find a graphics and/or presenting queue!";
        assert(false);
    }

    if (GraphicsQueueNodeIndex != PresentQueueNodeIndex)
    {
        //std::cout << ("Separate graphics and presenting queues are not supported yet!");
        assert(false);
    }

    if (ComputeQueueNodeIndex == UINT32_MAX)
    {
        ComputeQueueNodeIndex = GraphicsQueueNodeIndex;
    }

    if (TransferQueueNodeIndex == UINT32_MAX)
    {
        TransferQueueNodeIndex = GraphicsQueueNodeIndex;
    }

    /* Create the Queue */
    GraphicsQueue = new VulkanQueue(this, GraphicsQueueFamilyIndex, GraphicsQueueNodeIndex);
    ComputeQueue = new VulkanQueue(this, ComputeQueueFamilyIndex, ComputeQueueNodeIndex);
    TransferQueue = new VulkanQueue(this, TransferQueueFamilyIndex, TransferQueueNodeIndex);
}

/* For Destorying vulkan */
void VulkanDevice::WaitUntilIdle() const
{
    VE_PROFILE_VULKAN_FUNCTION();

    vkDeviceWaitIdle(LogicalDeviceHandle);
}

void VulkanDevice::TransitionTextureLayout(const HTransitionTextureLayoutInfo& inTransitionImageLayoutInfo)
{
    // Create a Image Memory Barrier 
    VkImageMemoryBarrier ImageMemoryBarrier = VulkanUtils::Initializers::ImageMemoryBarrier();
    ImageMemoryBarrier.oldLayout = inTransitionImageLayoutInfo.OldLayout;
    ImageMemoryBarrier.newLayout = inTransitionImageLayoutInfo.NewLayout;
    ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageMemoryBarrier.image = *inTransitionImageLayoutInfo.TextureHandle->GetImageHandle();
    ImageMemoryBarrier.subresourceRange.aspectMask = GetImageAspectFlags(inTransitionImageLayoutInfo.TextureHandle->GetImageFormat());
    ImageMemoryBarrier.subresourceRange.baseMipLevel = inTransitionImageLayoutInfo.Subresource->BaseMipLevel;
    ImageMemoryBarrier.subresourceRange.levelCount = inTransitionImageLayoutInfo.Subresource->NumMipLevels;
    ImageMemoryBarrier.subresourceRange.baseArrayLayer = inTransitionImageLayoutInfo.Subresource->BaseArrayLayer;
    ImageMemoryBarrier.subresourceRange.layerCount = inTransitionImageLayoutInfo.Subresource->NumArrayLayers;

    VkPipelineStageFlags SrcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags DstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    if (inTransitionImageLayoutInfo.OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        inTransitionImageLayoutInfo.NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        ImageMemoryBarrier.srcAccessMask = 0;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        SrcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (inTransitionImageLayoutInfo.OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        && inTransitionImageLayoutInfo.NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        SrcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        DstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        VE_ASSERT(false, VE_TEXT("[VulkanDevice]: Unsupported layout transition.... "));
    }

    /** Change the image layut for the texture passed in */
    inTransitionImageLayoutInfo.TextureHandle->SetImageLayout(inTransitionImageLayoutInfo.NewLayout);

    vkCmdPipelineBarrier(inTransitionImageLayoutInfo.CommandBufferHandle,
        SrcStageMask, DstStageMask, 0, 0, nullptr, 0,
        nullptr, 1, &ImageMemoryBarrier);
}

void VulkanDevice::CopyBufferToTexture(const HCopyBufferToTextureInfo& inCopyBufferToTexture)
{
    VkBufferImageCopy BufferImageCopy;
    BufferImageCopy.bufferOffset = 0;
    BufferImageCopy.bufferRowLength = 0;
    BufferImageCopy.bufferImageHeight = 0;
    BufferImageCopy.imageSubresource.aspectMask = GetImageAspectFlags(inCopyBufferToTexture.TextureHandle->GetImageFormat());
    BufferImageCopy.imageSubresource.mipLevel = inCopyBufferToTexture.Subresource->BaseMipLevel;
    BufferImageCopy.imageSubresource.baseArrayLayer = inCopyBufferToTexture.Subresource->BaseArrayLayer;
    BufferImageCopy.imageSubresource.layerCount = inCopyBufferToTexture.Subresource->NumArrayLayers;
    BufferImageCopy.imageOffset = inCopyBufferToTexture.Offset;
    BufferImageCopy.imageExtent = inCopyBufferToTexture.Extent;

    vkCmdCopyBufferToImage(inCopyBufferToTexture.CommandBufferHandle,
        inCopyBufferToTexture.BufferHandle, *inCopyBufferToTexture.TextureHandle->GetImageHandle(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferImageCopy);
}

uint32_t VulkanDevice::GetMemoryTypeIndex(uint32_t inTypeBits, VkMemoryPropertyFlags inProperties, VkBool32* outMemTypeFound) const
{
    VE_PROFILE_VULKAN_FUNCTION();

    for (uint32_t i = 0; i < PhysicalDeviceMemProperties.memoryTypeCount; i++)
    {
        if ((inTypeBits & 1) == 1)
        {
            if ((PhysicalDeviceMemProperties.memoryTypes[i].propertyFlags & inProperties) == inProperties)
            {
                if (outMemTypeFound)
                {
                    *outMemTypeFound = true;
                }
                return i;
            }
        }
        inTypeBits >>= 1;
    }

    if (outMemTypeFound)
    {
        *outMemTypeFound = false;
        return 0;
    }
    else
    {
        throw std::runtime_error("Could not find a matching memory type");
    }
}

VkImageAspectFlags VulkanDevice::GetImageAspectFlags(VkFormat inFormat)
{
    VkImageAspectFlags ImageAspectFlags = 0;

    if (inFormat & VK_IMAGE_ASPECT_COLOR_BIT)
    {
        ImageAspectFlags |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    else if (inFormat & VK_IMAGE_ASPECT_DEPTH_BIT)
    {
        if (inFormat & VK_IMAGE_ASPECT_STENCIL_BIT)
        {
            ImageAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        ImageAspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else if (inFormat & VK_IMAGE_ASPECT_STENCIL_BIT)
    {
        if (inFormat & VK_IMAGE_ASPECT_DEPTH_BIT)
        {
            ImageAspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        ImageAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    return ImageAspectFlags;
}

/* ------------------------------------------------------------------------------- */
/* -----------------------             Queue             ------------------------- */
/* ------------------------------------------------------------------------------- */

VulkanQueue::VulkanQueue(VulkanDevice* device, uint32 queueFamilyIndex, uint32 queueIndex)
    : Device(device), FamilyIndex(queueFamilyIndex), QueueIndex(queueIndex)
{
    VE_PROFILE_VULKAN_FUNCTION();

    // Get the queue
    vkGetDeviceQueue(*device->GetDeviceHandle(), queueFamilyIndex, queueIndex, &Queue);

    // Create the Command Pool associated with this queue
    CommandPool = new VulkanCommandPool(device);
    CommandPool->CreateCommandPool(FamilyIndex);
}

VulkanQueue::~VulkanQueue()
{
    if (CommandPool != nullptr)
    {
        delete CommandPool;
        CommandPool = nullptr;
    }
}

void VulkanQueue::Submit(ICommandBuffer* inCommandBuffer, uint32 inNumSignalSemaphores, ISemaphore* inSignalSemaphores)
{
    VulkanCommandBuffer* CommandBuff = (VulkanCommandBuffer*)inCommandBuffer;
    VulkanSemaphore* Semaphores = (VulkanSemaphore*)inSignalSemaphores;

    SubmitQueue(CommandBuff, inNumSignalSemaphores, Semaphores->GetSemaphoresHandle());
}

void VulkanQueue::SetWaitFence(IFence* inWaitFence, uint64 inTimeout) const
{
    VulkanFence* Fence = (VulkanFence*)inWaitFence;
    Fence->Wait(inTimeout);
}

void VulkanQueue::ResetWaitFence(IFence* inWaitFence) const
{
    VulkanFence* Fence = (VulkanFence*)inWaitFence;
    Fence->Reset();
}

void VulkanQueue::SetWaitIdle()
{
    vkQueueWaitIdle(GetQueueHandle());
}

void VulkanQueue::SubmitQueue(VulkanCommandBuffer* commandBuffer, uint32 numSignalSemaphores, VkSemaphore* signalSemaphore)
{
    VE_PROFILE_VULKAN_FUNCTION();

    // Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
    VkPipelineStageFlags WaitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // The submit info structure specifies a command buffer queue submission batch
    VkSubmitInfo SubmitInfo = VulkanUtils::Initializers::SubmitInfo();
    SubmitInfo.pWaitDstStageMask = &WaitStageMask;               // Pointer to the list of pipeline stages that the semaphore waits will occur at
    SubmitInfo.waitSemaphoreCount = commandBuffer->GetWaitSemaphoresCount();                           // One wait semaphore
    SubmitInfo.signalSemaphoreCount = numSignalSemaphores;                         // One signal semaphore
    SubmitInfo.pCommandBuffers = commandBuffer->GetCommandBufferHandle();//&VTemp.DrawCommandBuffers[VTemp.CurrentBuffer]; // Command buffers(s) to execute in this batch (submission)
    SubmitInfo.commandBufferCount = 1;                           // One command buffer

    // SRS - on other platforms use original bare code with local semaphores/fences for illustrative purposes
    SubmitInfo.pWaitSemaphores = commandBuffer->GetWaitSemaphores();      // Semaphore(s) to wait upon before the submitted command buffer starts executing
    SubmitInfo.pSignalSemaphores = signalSemaphore;     // Semaphore(s) to be signaled when command buffers have completed

    // get vulkan specific fence
    VulkanFence* WaitFence = (VulkanFence*)commandBuffer->GetWaitFence();

    // Submit to the graphics queue passing a wait fence
    VK_CHECK_RESULT(vkQueueSubmit(Queue, 1, &SubmitInfo, WaitFence->GetFenceHandle()), "[VulkanQueue]: Failed to submit a command buffer to graphics queue!");
}

void VulkanQueue::SubmitQueue(VulkanCommandBuffer* commandBuffer, VkSemaphore* signalSemaphore)
{
    VE_PROFILE_VULKAN_FUNCTION();

    SubmitQueue(commandBuffer, 1, signalSemaphore);
}

void VulkanQueue::SubmitQueue(VulkanCommandBuffer* commandBuffer, const VkSubmitInfo& inSubmitInfo) const
{
    VE_PROFILE_VULKAN_FUNCTION();

    // get vulkan specific fence
    VulkanFence* WaitFence = (VulkanFence*)commandBuffer->GetWaitFence();

    // Submit to the graphics queue passing a wait fence
    VK_CHECK_RESULT(vkQueueSubmit(Queue, 1, &inSubmitInfo, WaitFence->GetFenceHandle()), "[VulkanQueue]: Failed to submit a command buffer to graphics queue!");
}

VkCommandBuffer VulkanQueue::CreateSingleTimeCommandBuffer(bool inShouldBegin)
{
    VkCommandBuffer CommandBufferHandle = VK_NULL_HANDLE;

    // Allocate command buffer with default configs..
    VkCommandBufferAllocateInfo CommandBufferAllocateInfo = VulkanUtils::Initializers::CommandBufferAllocateInfo();
    CommandBufferAllocateInfo.commandPool = CommandPool->GetCommandPoolHandle();
    CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    CommandBufferAllocateInfo.commandBufferCount = 1;

    VK_CHECK_RESULT(vkAllocateCommandBuffers(*Device->GetDeviceHandle(), &CommandBufferAllocateInfo, &CommandBufferHandle), VE_TEXT("[VulkanQueue]: Failed to create a default command buffer...!"));

    // Only begin the newly created command buffer if user wanted to 
    if (inShouldBegin)
    {
        VkCommandBufferBeginInfo CommandBufferBeginInfo = VulkanUtils::Initializers::CommandBufferBeginInfo(nullptr);
        CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK_RESULT(vkBeginCommandBuffer(CommandBufferHandle, &CommandBufferBeginInfo), VE_TEXT("[VulkanQueue]: Failed to begin a command buffer...!"));
    }

    return CommandBufferHandle;
}

void VulkanQueue::FlushSingleTimeCommandBuffer(VkCommandBuffer inCommandBuffer, bool inShouldFree)
{
    // Firstly end the command buffer so its not in a recording state any more 
    VK_CHECK_RESULT(vkEndCommandBuffer(inCommandBuffer), VE_TEXT("[VulkanQueue]: Failed to end command buffer...!"));

    // Ensure that all the commands are processed in the command buffer
    VulkanFence Fence(Device);
    Fence.Reset();

    VkSubmitInfo SubmitInfo = VulkanUtils::Initializers::SubmitInfo();
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &inCommandBuffer;

    vkQueueSubmit(Queue, 1, &SubmitInfo, Fence.GetFenceHandle());

    // Wait for fence 
    Fence.Wait(UINT64_MAX);

    if (inShouldFree)
    {
        vkFreeCommandBuffers(*Device->GetDeviceHandle(), CommandPool->GetCommandPoolHandle(), 1, &inCommandBuffer);
    }
}

/* ------------------------------------------------------------------------------- */
/* -----------------------             Surface           ------------------------- */
/* ------------------------------------------------------------------------------- */
VulkanSurface::VulkanSurface(VulkanDevice* device, VkInstance instance, HINSTANCE* windowInstance, HWND* window)
    : Device(device), InstanceHandle(instance), ColorFormat(VK_FORMAT_UNDEFINED), ColorSpace((VkColorSpaceKHR)0)
{
    VE_PROFILE_VULKAN_FUNCTION();

    /* Get all required function pointers */
    fpGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR"));
    fpGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
    fpGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
    fpGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));

    // Init surface
    VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = VulkanUtils::Initializers::Win32SurfaceCreateInfoKHR(windowInstance, window);
    VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(instance, &SurfaceCreateInfo, nullptr, &SurfaceHandle), "[VulkanSurface]: Failed to create a Win32 Surface!!");

    // Get list of supported surface formats
    uint32_t FormatCount;
    VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceFormatsKHR(*device->GetPhysicalDeviceHandle(), SurfaceHandle, &FormatCount, NULL), "[VulkanSurface]: Failed to retreive physical device (GPU) surface formats count!");
    ASSERT(FormatCount > 0, "[VulkanSurface]: Failed to retreive physical device (GPU) surface formats; Format count < 0");

    std::vector<VkSurfaceFormatKHR> SurfaceFormats(FormatCount);
    VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceFormatsKHR(*device->GetPhysicalDeviceHandle(), SurfaceHandle, &FormatCount, SurfaceFormats.data()), "[VulkanSurface]: Failed to retreive physical device (GPU) surface formats!");

    // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
        // there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
    if ((FormatCount == 1) && (SurfaceFormats[0].format == VK_FORMAT_UNDEFINED))
    {
        ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
        ColorSpace = SurfaceFormats[0].colorSpace;
    }
    else
    {
        // iterate over the list of available surface format and
        // check for the presence of VK_FORMAT_B8G8R8A8_UNORM
        bool Found_B8G8R8A8_UNORM = false;
        for (auto&& surfaceFormat : SurfaceFormats)
        {
            if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
            {
                ColorFormat = surfaceFormat.format;
                ColorSpace = surfaceFormat.colorSpace;
                Found_B8G8R8A8_UNORM = true;

                break;
            }
        }

        // in case VK_FORMAT_B8G8R8A8_UNORM is not available
        // select the first available color format
        if (!Found_B8G8R8A8_UNORM)
        {
            ColorFormat = SurfaceFormats[0].format;
            ColorSpace = SurfaceFormats[0].colorSpace;
        }
    }

    SurfaceFormat.colorSpace = ColorSpace;
    SurfaceFormat.format = ColorFormat;
}

VulkanSurface::VulkanSurface(VulkanDevice* inDevice, VkInstance inInstance, VkSurfaceKHR inSurfaceHandle)
{
    Device = inDevice;
    InstanceHandle = inInstance;
    SurfaceHandle = inSurfaceHandle;

    /* Get all required function pointers */
    fpGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(vkGetInstanceProcAddr(inInstance, "vkGetPhysicalDeviceSurfaceSupportKHR"));
    fpGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(vkGetInstanceProcAddr(inInstance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
    fpGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(vkGetInstanceProcAddr(inInstance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
    fpGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(vkGetInstanceProcAddr(inInstance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));

    // Get list of supported surface formats
    uint32_t FormatCount;
    VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceFormatsKHR(*Device->GetPhysicalDeviceHandle(), SurfaceHandle, &FormatCount, NULL), "[VulkanSurface]: Failed to retreive physical device (GPU) surface formats count!");
    ASSERT(FormatCount > 0, "[VulkanSurface]: Failed to retreive physical device (GPU) surface formats; Format count < 0");

    std::vector<VkSurfaceFormatKHR> SurfaceFormats(FormatCount);
    VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceFormatsKHR(*Device->GetPhysicalDeviceHandle(), SurfaceHandle, &FormatCount, SurfaceFormats.data()), "[VulkanSurface]: Failed to retreive physical device (GPU) surface formats!");

    // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
        // there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
    if ((FormatCount == 1) && (SurfaceFormats[0].format == VK_FORMAT_UNDEFINED))
    {
        ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
        ColorSpace = SurfaceFormats[0].colorSpace;
    }
    else
    {
        // iterate over the list of available surface format and
        // check for the presence of VK_FORMAT_B8G8R8A8_UNORM
        bool Found_B8G8R8A8_UNORM = false;
        for (auto&& surfaceFormat : SurfaceFormats)
        {
            if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
            {
                ColorFormat = surfaceFormat.format;
                ColorSpace = surfaceFormat.colorSpace;
                Found_B8G8R8A8_UNORM = true;

                break;
            }
        }

        // in case VK_FORMAT_B8G8R8A8_UNORM is not available
        // select the first available color format
        if (!Found_B8G8R8A8_UNORM)
        {
            ColorFormat = SurfaceFormats[0].format;
            ColorSpace = SurfaceFormats[0].colorSpace;
        }
    }

    SurfaceFormat.colorSpace = ColorSpace;
    SurfaceFormat.format = ColorFormat;
}

VulkanSurface::~VulkanSurface()
{
    VE_PROFILE_VULKAN_FUNCTION();

    Device->WaitUntilIdle();

    vkDestroySurfaceKHR(InstanceHandle, SurfaceHandle, nullptr);
}

EPixelFormat VulkanSurface::GetColorFormat() const
{
    return VulkanTypeConverter::Convert(ColorFormat);
}

/* ------------------------------------------------------------------------------- */
/* -----------------------            Swapchain          ------------------------- */
/* ------------------------------------------------------------------------------- */

VulkanSwapChain::VulkanSwapChain(VulkanDevice* inDevice, VulkanSurface* inSurface, const FSwapChainConfig& inConfig)
    : Device(inDevice), SurfacePtr(inSurface), SwapChainHandle(VK_NULL_HANDLE), ImageCount(0)
{
    /* Get the function pointers */
    fpCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(vkGetDeviceProcAddr(*Device->GetDeviceHandle(), "vkCreateSwapchainKHR"));
    fpDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(vkGetDeviceProcAddr(*Device->GetDeviceHandle(), "vkDestroySwapchainKHR"));
    fpGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(vkGetDeviceProcAddr(*Device->GetDeviceHandle(), "vkGetSwapchainImagesKHR"));
    fpAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(*Device->GetDeviceHandle(), "vkAcquireNextImageKHR"));
    fpQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(vkGetDeviceProcAddr(*Device->GetDeviceHandle(), "vkQueuePresentKHR"));

    ImageWidth = 0;
    ImageHeight = 0;

    Create(inConfig, VK_NULL_HANDLE);
}

VulkanSwapChain::~VulkanSwapChain()
{
    Device->WaitUntilIdle();

    /* No Need to destroy the swap chain images ourselfs as the when deleting the swap chain the images gets deleted as well*/
    vkDestroySwapchainKHR(*Device->GetDeviceHandle(), SwapChainHandle, nullptr);

    for (uint32 i = 0; i < ImageViews.size(); ++i)
    {
        ImageViews[i]->ImageHandle = VK_NULL_HANDLE;
        ImageViews[i]->Device = Device;
        delete ImageViews[i];
    }
}

void VulkanSwapChain::Present(ICommandQueue* inSubmissionQueue, ISemaphore* inWaitSemaphore, uint32 inImageIndex)
{
    VulkanQueue* QueuePtr = (VulkanQueue*)inSubmissionQueue;
    VulkanSemaphore* SemaphorePtr = (VulkanSemaphore*)inWaitSemaphore;

    QueuePresent(QueuePtr, SemaphorePtr->GetSemaphoresHandle(), inImageIndex);
}

bool VulkanSwapChain::ResizeSwapChain(const FExtent2D& inNewResolution)
{
    // Firstly check if there is no change 
    if (ImageWidth == inNewResolution.Width && ImageHeight == inNewResolution.Height)
    {
        return false;
    }

    // Recreate a new swapchain
    CreateSwapChain(SwapChainHandle, inNewResolution, Configuration.bEnableVSync);

    return true;
}

bool VulkanSwapChain::SetVSyncInterval(uint32 inVSyncInterval)
{
    return false;
}

//VkResult VulkanSwapChain::AcquireNextImage(VulkanCommandBuffer* lastCommandBuffer, uint32* imageIndex)
//{
//    // By setting timeout to UINT64_MAX we will always wait until the next image has been acquired or an actual error is thrown
//    // With that we don't have to handle VK_NOT_READY		
//    return fpAcquireNextImageKHR(*Device->GetDeviceHandle(), SwapChainHandle, UINT64_MAX, *lastCommandBuffer->GetWaitSemaphores(), (VkFence)nullptr, imageIndex);
//}

VkResult VulkanSwapChain::QueuePresent(VulkanQueue* queue, VkSemaphore* waitSemaphore, uint32 imageIndex)
{
    VkPresentInfoKHR PresentInfo = VulkanUtils::Initializers::PresentInfoKHR();
    PresentInfo.pNext = NULL;
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = &SwapChainHandle;
    PresentInfo.pImageIndices = &imageIndex;
    // Check if a wait semaphore has been specified to wait for before presenting the image
    if (waitSemaphore != VK_NULL_HANDLE)
    {
        PresentInfo.pWaitSemaphores = waitSemaphore;
        PresentInfo.waitSemaphoreCount = 1;
    }

    return fpQueuePresentKHR(queue->GetQueueHandle(), &PresentInfo);
}

void VulkanSwapChain::Create(const FSwapChainConfig& inConfig, VkSwapchainKHR inOldSwapChain)
{
    // Get physical device surface properties and formats
    VkSurfaceCapabilitiesKHR SurfaceCapabilities;
    VK_CHECK_RESULT(SurfacePtr->fpGetPhysicalDeviceSurfaceCapabilitiesKHR(*Device->GetPhysicalDeviceHandle(), *SurfacePtr->GetSurfaceHandle(), &SurfaceCapabilities), "[VulkanSwapChain]: Failed to retreive physical device (GPU) surface capabilities!");

    // Select the swapchain present mode
    VkPresentModeKHR SwapchainPresentMode = SelectSwapChainPresentMode(inConfig.bEnableVSync);

    // Determine the number of images
    MinImageCount = SurfaceCapabilities.minImageCount;
    ImageCount = SelectSwapChainImageCount(inConfig.NumSwapBuffers);

    CreateSwapChain(inOldSwapChain, inConfig.ScreenResolution, inConfig.bEnableVSync);
}

void VulkanSwapChain::CreateSwapChain(VkSwapchainKHR inOldSwapChain, const FExtent2D inResolution, bool bEnableVSync)
{
    VE_ASSERT(SurfacePtr != nullptr, VE_TEXT("[VulkanSwapChain]: cannot select resolution if surface is not valid!!"));

    // Get physical device surface properties and formats
    VkSurfaceCapabilitiesKHR SurfaceCapabilities;
    VK_CHECK_RESULT(SurfacePtr->fpGetPhysicalDeviceSurfaceCapabilitiesKHR(*Device->GetPhysicalDeviceHandle(), *SurfacePtr->GetSurfaceHandle(), &SurfaceCapabilities), "[VulkanSwapChain]: Failed to retreive physical device (GPU) surface capabilities!");

    // Select the swapchain resolution based on the device capabilities 
    ImageWidth = inResolution.Width;
    ImageHeight = inResolution.Height;
    SelectSwapChainResolution(&ImageWidth, &ImageHeight);

    // Select the swapchain present mode
    VkPresentModeKHR SwapchainPresentMode = SelectSwapChainPresentMode(bEnableVSync);

    // Find the transformation of the surface
    VkSurfaceTransformFlagsKHR PreTransform;
    if (SurfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        // We prefer a non-rotated transform
        PreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        PreTransform = SurfaceCapabilities.currentTransform;
    }

    // Find a supported composite alpha format(not all devices support alpha opaque)
    VkCompositeAlphaFlagBitsKHR CompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // Simply select the first composite alpha format available
    std::vector<VkCompositeAlphaFlagBitsKHR> CompositeAlphaFlags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };

    for (auto& CompositeAlphaFlag : CompositeAlphaFlags) {
        if (SurfaceCapabilities.supportedCompositeAlpha & CompositeAlphaFlag) {
            CompositeAlpha = CompositeAlphaFlag;
            break;
        };
    }

    VkSwapchainCreateInfoKHR SwapchainCreateInfo = VulkanUtils::Initializers::SwapchainCreateInfoKHR();
    SwapchainCreateInfo.surface = *SurfacePtr->GetSurfaceHandle();
    SwapchainCreateInfo.minImageCount = ImageCount;
    SwapchainCreateInfo.imageFormat = *SurfacePtr->GetVkColorFormat();
    SwapchainCreateInfo.imageColorSpace = *SurfacePtr->GetColorSpace();
    SwapchainCreateInfo.imageExtent = { ImageWidth, ImageHeight };
    SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    SwapchainCreateInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)PreTransform;
    SwapchainCreateInfo.imageArrayLayers = 1;
    SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    SwapchainCreateInfo.queueFamilyIndexCount = 0;
    SwapchainCreateInfo.presentMode = SwapchainPresentMode;
    // Setting oldSwapChain to the saved handle of the previous swapchain aids in resource reuse and makes sure that we can still present already acquired images
    SwapchainCreateInfo.oldSwapchain = inOldSwapChain;
    // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
    SwapchainCreateInfo.clipped = VK_TRUE;
    SwapchainCreateInfo.compositeAlpha = CompositeAlpha;

    // Enable transfer source on swap chain images if supported
    if (SurfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        SwapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    // Enable transfer destination on swap chain images if supported
    if (SurfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        SwapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    // Create the swapchain
    /**
    * Can fail unexpectedly due to Renderdoc and Vulkan Validation Layers (Only one can be selected at a time)
    */
    VK_CHECK_RESULT(fpCreateSwapchainKHR(*Device->GetDeviceHandle(), &SwapchainCreateInfo, nullptr, &SwapChainHandle), "[VulkanSwapChain]: Failed to create a swapchain!");

    // If an existing swap chain is re-created, destroy the old swap chain
    // This also cleans up all the presentable images
    if (inOldSwapChain != VK_NULL_HANDLE)
    {
        fpDestroySwapchainKHR(*Device->GetDeviceHandle(), inOldSwapChain, nullptr);
    }

    Images.clear();

    // Get the swapchain image count
    VK_CHECK_RESULT(fpGetSwapchainImagesKHR(*Device->GetDeviceHandle(), SwapChainHandle, &ImageCount, NULL), "[VulkanSwapChain]: Failed to retreive a swapchain images count!");

    // Get the swap chain images
    Images.resize(ImageCount);
    VK_CHECK_RESULT(fpGetSwapchainImagesKHR(*Device->GetDeviceHandle(), SwapChainHandle, &ImageCount, Images.data()), "[VulkanSwapChain]: Failed to retreive a swapchain images!");

    CreateSwapChainImageViews();
}

void VulkanSwapChain::CreateSwapChainImageViews()
{
    // Clear old information
    for (uint32 i = 0; i < ImageViews.size(); ++i)
    {
        ImageViews[i]->ImageHandle = VK_NULL_HANDLE;
        ImageViews[i]->Device = Device;
        delete ImageViews[i];
    }

    ImageViews.clear();

    // Get the swap chain buffers containing the image and imageview
    ImageViews.resize(ImageCount);

    VkImageViewCreateInfo ColorAttachmentView = VulkanUtils::Initializers::ImageViewCreateInfo();
    ColorAttachmentView.format = *SurfacePtr->GetVkColorFormat();//ColorFormat;
    ColorAttachmentView.components = {
        VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A
    };
    ColorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ColorAttachmentView.subresourceRange.baseMipLevel = 0;
    ColorAttachmentView.subresourceRange.levelCount = 1;
    ColorAttachmentView.subresourceRange.baseArrayLayer = 0;
    ColorAttachmentView.subresourceRange.layerCount = 1;
    ColorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;

    for (uint32 i = 0; i < ImageCount; ++i)
    {
        ImageViews[i] = new VulkanTextureView();

        ImageViews[i]->ImageHandle = Images[i];
        ImageViews[i]->ImageFormat = *SurfacePtr->GetVkColorFormat();
        ImageViews[i]->NumMipLevels = 1;
        ImageViews[i]->NumArrayLayers = 1;

        ColorAttachmentView.image = ImageViews[i]->ImageHandle;
        VK_CHECK_RESULT(vkCreateImageView(*Device->GetDeviceHandle(), &ColorAttachmentView, nullptr, &ImageViews[i]->ViewHandle), "[VulkanSwapChain]: Failed to create an image view!");
    }
}

void VulkanSwapChain::SelectSwapChainResolution(uint32* outWidth, uint32* outHeight)
{
    VE_ASSERT(SurfacePtr != nullptr, VE_TEXT("[VulkanSwapChain]: cannot select resolution if surface is not valid!!"));

    // Get physical device surface properties and formats
    VkSurfaceCapabilitiesKHR SurfaceCapabilities;
    VK_CHECK_RESULT(SurfacePtr->fpGetPhysicalDeviceSurfaceCapabilitiesKHR(*Device->GetPhysicalDeviceHandle(), *SurfacePtr->GetSurfaceHandle(), &SurfaceCapabilities), "[VulkanSwapChain]: Failed to retreive physical device (GPU) surface capabilities!");

    VkExtent2D SwapchainExtent = {};
    // If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
    if (SurfaceCapabilities.currentExtent.width == (uint32_t)-1)
    {
        // If the surface size is undefined, the size is set to
        // the size of the images requested.
        SwapchainExtent.width = *outWidth;
        SwapchainExtent.height = *outHeight;
    }
    else
    {
        // If the surface size is defined, the swap chain size must match
        SwapchainExtent = SurfaceCapabilities.currentExtent;

        *outWidth = SurfaceCapabilities.currentExtent.width;
        *outHeight = SurfaceCapabilities.currentExtent.height;
    }
}

uint32 VulkanSwapChain::SelectSwapChainImageCount(uint32 inNumDesiredImages)
{
    VE_ASSERT(SurfacePtr != nullptr, VE_TEXT("[VulkanSwapChain]: cannot select resolution if surface is not valid!!"));

    // Get physical device surface properties and formats
    VkSurfaceCapabilitiesKHR SurfaceCapabilities;
    VK_CHECK_RESULT(SurfacePtr->fpGetPhysicalDeviceSurfaceCapabilitiesKHR(*Device->GetPhysicalDeviceHandle(), *SurfacePtr->GetSurfaceHandle(), &SurfaceCapabilities), "[VulkanSwapChain]: Failed to retreive physical device (GPU) surface capabilities!");

    uint32_t DesiredNumberOfSwapchainImages = inNumDesiredImages + 1;
    if ((SurfaceCapabilities.maxImageCount > 0) && (DesiredNumberOfSwapchainImages > SurfaceCapabilities.maxImageCount))
    {
        DesiredNumberOfSwapchainImages = SurfaceCapabilities.maxImageCount;
    }

    return DesiredNumberOfSwapchainImages;
}

VkPresentModeKHR VulkanSwapChain::SelectSwapChainPresentMode(bool inEnableVSync)
{
    VE_ASSERT(SurfacePtr != nullptr, VE_TEXT("[VulkanSwapChain]: cannot select resolution if surface is not valid!!"));

    // Get available present modes
    uint32_t PresentModeCount = -1;
    VK_CHECK_RESULT(SurfacePtr->fpGetPhysicalDeviceSurfacePresentModesKHR(*Device->GetPhysicalDeviceHandle(), *SurfacePtr->GetSurfaceHandle(), &PresentModeCount, NULL), "[VulkanSwapChain]: Failed to retreive physical device (GPU) surface present modes count!");
    ASSERT(PresentModeCount > 0, "[VulkanSwapChain]: Failed to retreive physical device (GPU) surface present modes count; Surface Capabilites count equals 0");

    // Select a present mode for the swapchain
    std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
    VK_CHECK_RESULT(SurfacePtr->fpGetPhysicalDeviceSurfacePresentModesKHR(*Device->GetPhysicalDeviceHandle(), *SurfacePtr->GetSurfaceHandle(), &PresentModeCount, PresentModes.data()), "[VulkanSwapChain]: Failed to retreive physical device (GPU) surface present modes!");

    // The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
    // This mode waits for the vertical blank ("v-sync")
    VkPresentModeKHR SwapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    // If v-sync is not requested, try to find a mailbox mode
    // It's the lowest latency non-tearing present mode available
    if (inEnableVSync)
    {
        for (size_t i = 0; i < PresentModeCount; i++)
        {
            if (PresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                SwapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
            if (PresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                SwapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    return SwapchainPresentMode;
}

void VulkanSwapChain::AcquireNextImageIndex(ISemaphore* inLastCommandBuffer, uint32* outIndex) const
{
    // By setting timeout to UINT64_MAX we will always wait until the next image has been acquired or an actual error is thrown
    // With that we don't have to handle VK_NOT_READY		

    VulkanSemaphore* Semaphore = (VulkanSemaphore*)inLastCommandBuffer;
    VK_CHECK_RESULT(fpAcquireNextImageKHR(*Device->GetDeviceHandle(), SwapChainHandle, UINT64_MAX, *Semaphore->GetSemaphoresHandle(), (VkFence)nullptr, outIndex), "[VulkanSwapchain]: Failed to acquire the next swap chain image index...!");
}

Texture* VulkanSwapChain::GetTextureAt(uint32 inTextureIndex) const
{
    return (Texture*)ImageViews[inTextureIndex];
}

EPixelFormat VulkanSwapChain::GetColorFormat() const
{
    return SurfacePtr->GetColorFormat();
}

EPixelFormat VulkanSwapChain::GetDepthStencilFormat() const
{
    return EPixelFormat();
}


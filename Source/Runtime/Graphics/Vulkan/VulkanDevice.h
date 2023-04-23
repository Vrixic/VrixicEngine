/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"
#pragma comment(lib, "vulkan-1.lib")

#include <Core/Core.h>
#include "VulkanUtils.h"
#include "Misc/Defines/GenericDefines.h"
#include <Runtime/Graphics/Surface.h>
#include <Runtime/Graphics/SwapChain.h>

#include <vector>
#include <string>

class VulkanShaderFactory;
class VulkanSurface;
class VulkanSwapChain;
class VulkanQueue;
class VulkanCommandBuffer;
class VulkanCommandPool;
class VulkanTextureView;

/**
* Representation of vulkan device
*/
class VRIXIC_API VulkanDevice
{
private:
    /* Representation of GPU */
    VkDevice LogicalDeviceHandle;

    /* All Enabled Validation layers */
    std::vector<std::string> ValidationLayers;

    /* GPU */
    VkPhysicalDevice PhysicalDeviceHandle;

    /* All Enabled Device Extensions */
    std::vector<std::string> PhysicalDeviceExtensions;

    VkPhysicalDeviceProperties PhysicalDeviceProperties;
    VkPhysicalDeviceFeatures PhysicalDeviceFeatures;
    VkPhysicalDeviceMemoryProperties PhysicalDeviceMemProperties;

    /*
    * All Queue family properties
    */
    uint32 QueueFamilyCount;
    std::vector<VkQueueFamilyProperties> QueueFamilyProperties;

    /*
    * Graphics queue used to submit graphics primitive/info
    * Compute queue used to submut compute info
    * Transfer queue used for transferring data
    */
    VulkanQueue* GraphicsQueue;
    VulkanQueue* ComputeQueue;
    VulkanQueue* TransferQueue;

public:
    /**
    * @param inGPU - The GPU to be used for device creation
    * @param inEnabledFeatures - The features that will be enabled if available on the GPU
    * @param inDeviceExtensionCount - count of device extentions
    * @param inDeviceExtensions - the extensions to be enabled on this device if the device supports them
    *
    * @remarks Does all the setup for Device creation
    */
    VulkanDevice(VkPhysicalDevice& inGPU, VkPhysicalDeviceFeatures& inEnabledFeatures, uint32 inDeviceExtensionCount, const char** inDeviceExtensions);

    ~VulkanDevice();

    VulkanDevice(const VulkanDevice& other) = delete;
    VulkanDevice operator=(const VulkanDevice& other) = delete;

    /**
    * Creates the logical device
    *
    * @param inSurface - The surface to be used for creating the device
    */
    void CreateDevice(VulkanSurface* inSurface);

    /**
    * Waits until the device is idle... not executing any commands..
    */
    void WaitUntilIdle() const;

public:
    /* Returns the logical device */
    inline const VkDevice* GetDeviceHandle() const
    {
        return &LogicalDeviceHandle;
    }

    inline const VkPhysicalDevice* GetPhysicalDeviceHandle() const
    {
        return &PhysicalDeviceHandle;
    }

    inline VulkanQueue* GetGraphicsQueue() const
    {
        return GraphicsQueue;
    }

    inline VulkanQueue* GetComputeQueue() const
    {
        return ComputeQueue;
    }

    inline VulkanQueue* GetTransferQueue() const
    {
        return TransferQueue;
    }

    inline VulkanQueue* GetPresentQueue() const
    {
        return GraphicsQueue;
    }

    inline const VkPhysicalDeviceProperties* GetPhysicalDeviceProperties() const
    {
        return &PhysicalDeviceProperties;
    }

    inline const VkPhysicalDeviceMemoryProperties* GetPhysicalDeviceMemoryProperties() const
    {
        return &PhysicalDeviceMemProperties;
    }

    inline const VkPhysicalDeviceFeatures* GetPhysicalDeviceFeatures() const
    {
        return &PhysicalDeviceFeatures;
    }

    inline const std::vector<VkQueueFamilyProperties>* GetQueueFamilyProperties() const
    {
        return &QueueFamilyProperties;
    }

    /**
    * Get the index of a memory type that has all the requested property bits set
    *
    * @param typeBits Bit mask with bits set for each memory type supported by the resource to request for (from VkMemoryRequirements)
    * @param properties Bit mask of properties for the memory type to request
    * @param (Optional) memTypeFound Pointer to a bool that is set to true if a matching memory type has been found
    *
    * @return Index of the requested memory type
    *
    * @throw Throws an exception if memTypeFound is null and no memory type could be found that supports the requested properties
    */
    uint32_t GetMemoryTypeIndex(uint32_t inTypeBits, VkMemoryPropertyFlags inProperties, VkBool32* outMemTypeFound) const;
};

/**
* A representation of a Vulkan Queue, derieve from command queue and has the ability to submit command buffers
* @note A queue has a command pool which should be used to allocate all other command buffers for that respective queue
* @example GraphicsQueue -> Command Pool ( A command buffer used for graphics will use the Graphics Queue for its creation...)
*          ComputeQueue  -> Command Pool 
*/
class VRIXIC_API VulkanQueue : public ICommandQueue
{
private:
    /* The Device this queue belongs to */
    VulkanDevice* Device;
    VkQueue Queue;

    /* The queue index into the family of queue queues of this device */
    uint32 QueueIndex;

    /* The family index into the family of queues */
    uint32 FamilyIndex;

    /* The command pool associated with this queue */
    VulkanCommandPool* CommandPool;

public:
    /**
    * @param inQueueFamilyIndex - The queue family index this queue belongs to
    * @param inQueueIndex - The queue index this queue represents
    *
    * @remarks Sets up queue submission
    */
    VulkanQueue(VulkanDevice* inDevice, uint32 inQueueFamilyIndex, uint32 inQueueIndex);

    ~VulkanQueue();

    VulkanQueue(const VulkanQueue& other) = delete;
    VulkanQueue operator=(const VulkanQueue& other) = delete;

    /* ICommandQueue Interface Begin */

    /**
    * Submits the specified command buffer to a queue (Can be any type of queue, ex: graphics, compute),
    *  uses the command buffers fence to submit the buffer
    *
    * @param inCommandBuffer the command buffer that will get submitted
    */
    virtual void Submit(ICommandBuffer* inCommandBuffer, uint32 inNumSignalSemaphores, ISemaphore* inSignalSemaphores) override;

    /**
    * Sets a wait fence that will block the CPU execution until the fence has been signaled
    *
    * @param inWaitFence the fence the CPU will wait to be signaled
    * @param inTimeout this is the waiting timeout in nanoseconds
    */
    virtual void SetWaitFence(IFence* inWaitFence, uint64 inTimeout) override;

    /**
    * Blocks the CPU execution until all submitted command/fences have been completed in other words signaled,
    * (Vulkan Description) -> this is the equivalent to having submitted a valid fence to every previously executed queue submission command
    * that accepts a fence, then waiting for all of those fences to signal using vkWaitForFences with an infinite timeout
    */
    virtual void SetWaitIdle() override;

    /* ICommandQueue Interface End */

    /**
    * Submits a command buffer to this queue using multiple signals
    *
    * @param commandBuffer The command buffer to be submitted to this queue, the waitSemaphores of the command buffer will be used
    * @param numSignalSemaphores number of signal semaphores
    * @param signalSemaphores the signal semaphore(s)
    */
    void SubmitQueue(VulkanCommandBuffer* inCommandBuffer, uint32 inNumSignalSemaphores, VkSemaphore* inSignalSemaphores);

    /**
    * Submits command buffer to this queue using only one signal semaphore
    *
    * @param commandBuffer The command buffer to be submitted to this queue, the waitSemaphores of the command buffer will be used
    * @param signalSemaphores the signal semaphore
    */
    void SubmitQueue(VulkanCommandBuffer* inCommandBuffer, VkSemaphore* inSignalSemaphore);

    /**
    * Submits a submit info to the queue
    *
    * @param commandBuffer The command buffer to be submitted to this queue, the waitSemaphores of the command buffer will be used
    * @param inSubmitInfo - the queue submission info
    */
    void SubmitQueue(VulkanCommandBuffer* inCommandBuffer, const VkSubmitInfo& inSubmitInfo) const;

public:
    inline VkQueue GetQueueHandle() const { return Queue; }

    inline uint32 GetQueueIndex() const { return QueueIndex; }

    inline uint32 GetFamilyIndex() const { return FamilyIndex; }

    inline VulkanCommandPool* GetCommandPool() const { return CommandPool; }
};

/**
* Representation of vulkan surface
*/
class VRIXIC_API VulkanSurface : public Surface
{
private:
    VkInstance InstanceHandle;
    VulkanDevice* Device;

    VkSurfaceKHR SurfaceHandle;

    VkFormat ColorFormat;
    VkColorSpaceKHR ColorSpace;

    VkSurfaceFormatKHR SurfaceFormat;

    //uint32 GraphicsQueueNodeIndex;

    //std::vector<VkQueueFamilyProperties> QueueFamilyProperties;

public:
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;

public:
    /**
    * @param inInstance - The vulkan instance this surfance will use
    * @param inSurfaceHandle - the surface to the window that is already created by another object
    *
    * @remarks Creates the Surface
    */
    VulkanSurface(VulkanDevice* inDevice, VkInstance inInstance, VkSurfaceKHR inSurfaceHandle);

    /**
    * Creates a vulkan surface KHR for the window passed in (-- Need to make it windows independent )
    * @param inInstance - The vulkan instance this surfance will use
    * @param inWindowInstance - the window instance this surface will use
    * @param inWindow - this window this surface will use
    *
    * @remarks Creates the Surface
    */
    VulkanSurface(VulkanDevice* inDevice, VkInstance inInstance, HINSTANCE* inWindowInstance, HWND* inWindow);

    ~VulkanSurface();

    VulkanSurface(const VulkanSurface& other) = delete;
    VulkanSurface operator=(const VulkanSurface& other) = delete;

public:
    inline const VkSurfaceKHR* GetSurfaceHandle() const
    {
        return &SurfaceHandle;
    }

    inline const VkFormat* GetVkColorFormat() const
    {
        return &ColorFormat;
    }

    inline const VkColorSpaceKHR* GetColorSpace() const
    {
        return &ColorSpace;
    }

    inline const VkSurfaceFormatKHR* GetSurfaceFormat() const
    {
        return &SurfaceFormat;
    }

    /** Surface Interface Begin */

    /**
    * @returns EFormat the color format of this surface
    */
    virtual EPixelFormat GetColorFormat() const override;

    /** Surface Interface End */

    //inline const std::vector<VkQueueFamilyProperties>* GetQueueFamilyProperties() const
    //{
        //return &QueueFamilyProperties;
    //}
};

class VRIXIC_API VulkanSwapChain : public SwapChain
{
private:
    VulkanDevice* Device;
    VulkanSurface* SurfacePtr;
    VkSwapchainKHR SwapChainHandle;

    uint32 ImageWidth;
    uint32 ImageHeight;

    uint32 MinImageCount;
    uint32 ImageCount;

    /* Swapchain Images */
    std::vector<VkImage> Images;
    std::vector<VulkanTextureView*> ImageViews;

    //struct SwapChainBuffer
    //{
    //    VkImage Image;
    //    VkImageView View;
    //};
    ///* Swap chain buffers */
    //std::vector<SwapChainBuffer> Buffers;

    VkPresentModeKHR SwapchainPresentMode;

public:
    PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
    PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
    PFN_vkQueuePresentKHR fpQueuePresentKHR;

public:
    /**
    * Creates the swapchain
    *
    * @param inSurface - the surface that will be used to create the swapchain
    * @param inConfig - the configuration information containing how the swapchain should be made
    */
    VulkanSwapChain(VulkanDevice* inDevice, VulkanSurface* inSurface, const SwapChainConfig& inConfig);

    ~VulkanSwapChain();

    VulkanSwapChain(const VulkanSwapChain& other) = delete;
    VulkanSwapChain operator=(const VulkanSwapChain& other) = delete;

public:
    /** SwapChain Interface Begin */

    /**
    * Presents the current buffer to the screen
    *
    * @param inSubmissionQueue the presentation queue used for presenting an image
    * @param inWaitSemaphore (Optional) Semaphore that is waited on before the image gets presented (only used if its != to nullptr
    * @param inImageIndex the index of the swapchain image to queue for presentation
    */
    virtual void Present(ICommandQueue* inSubmissionQueue, ISemaphore* inWaitSemaphore, uint32 inImageIndex) override;

    /**
    * Resizes all buffers/images within the swapchain, essentially recreating the swapchain
    * @returns bool true if it resized, false otherwise
    * @remarks use cases is on window resize or what ever render target its rendering to resized
    */
    virtual bool ResizeSwapChain(const Extent2D& inNewResolution) override;

    /**
    * Sets the vsync interval for this swapchain (vertical synchronization), 0 to disable, and 1 or more halfs the refresh rate
    *  | As of now this function DOES NOT DO ANYTHING
    * @returns bool true if successfully set to new vsync interval, false otherwise making it invalid
    */
    virtual bool SetVSyncInterval(uint32 inVSyncInterval) override;

    /** SwapChain Interface End */

    /**
    * Acquires the next image in the swap chain
    *
    * @param presentCompleteSemaphore (Optional) Semaphore that is signaled when the image is ready for use
    * @param imageIndex Pointer to the image index that will be increased if the next image could be acquired
    *
    * @remarks The function will always wait until the next image has been acquired by setting timeout to UINT64_MAX
    *
    * @return VkResult of the image acquisition
    */
    VkResult AcquireNextImage(VulkanCommandBuffer* inLastCommandBuffer, uint32* inImageIndex);

private:
    /**
    * Queue an image for presentation
    *
    * @param queue Presentation queue for presenting the image
    * @param imageIndex Index of the swapchain image to queue for presentation
    * @param waitSemaphore (Optional) Semaphore that is waited on before the image is presented (only used if != VK_NULL_HANDLE)
    *
    * @return VkResult of the queue presentation
    */
    VkResult QueuePresent(VulkanQueue* inQueue, VkSemaphore* inWaitSemaphore, uint32 inImageIndex);

    /**
    * Creates a new swapchain
    *
    * @param inConfig - the configuration information containing how the swapchain should be made
    * @param inOldSwapChain - old swapchain that was in use..
    *
    * @remarks if 'inOldSwapChain' is not VK_NULL_HANDLE, then that swapchain will be used to recreate the new one being created
    */
    void Create(const SwapChainConfig& inConfig, VkSwapchainKHR inOldSwapChain);

    /**
    * Creates a swapchain
    *
    * @param inResolution the size of the swapchain images
    * @param bEnableVSync true if vsync is enabled, false otherwise
    */
    void CreateSwapChain(VkSwapchainKHR inOldSwapChain, const Extent2D inResolution, bool bEnableVSync);

    /**
    * Creates all of the image views for the swapchain images
    */
    void CreateSwapChainImageViews();

    /**
    * Selects or picks the swapchain resolution
    *
    * @param outWidth desired width of the images of the swapchain, can get changed depending on device capabilities
    * @param outHeight desired height of the images of the swapchain, can get changed depending on device capabilities
    */
    void SelectSwapChainResolution(uint32* outWidth, uint32* outHeight);

    /**
    * Selects or picks the number of images to get from the surface to be used by the swapchain
    *
    * @param inNumDesiredImages number of images (buffers) desired to get from surface for use by the swapchain
    */
    uint32 SelectSwapChainImageCount(uint32 inNumDesiredImages);

    /**
    * Selects or picks the swapchain present mode
    *
    * @param inEnableVSync true to enable vsync, false otherwise
    */
    VkPresentModeKHR SelectSwapChainPresentMode(bool inEnableVSync);

public:
    inline const VkSwapchainKHR* GetSwapChainHandle() const
    {
        return &SwapChainHandle;
    }

    /**
    * Returns the texture that is requested from the texture this swapchain is using
    *
    * @returns Texture* the texture at the index specified
    */
    virtual Texture* GetTextureAt(uint32 inTextureIndex) const override;

    inline uint32 GetImageCount() const override
    {
        return ImageCount;
    }

    inline uint32 GetMinImageCount() const
    {
        return MinImageCount;
    }

    //inline const SwapChainBuffer* GetSwapchainBuffer(uint32 index) const
    //{
    //    return &Buffers[index];
    //}

    inline const VkPresentModeKHR* GetSwapchainPresentMode() const
    {
        return &SwapchainPresentMode;
    }

    /** SwapChain Interface Begin */

    /**
    * @returns EFormat the color format of this swapchain
    */
    virtual EPixelFormat GetColorFormat() const override;

    /**
    * @returns EFormat the depth stencil format of this swapchain
    */
    virtual EPixelFormat GetDepthStencilFormat() const override;

    /** SwapChain Interface End */
};


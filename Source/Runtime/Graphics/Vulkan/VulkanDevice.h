#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"
#pragma comment(lib, "vulkan-1.lib")

#include <Core/Core.h>
#include "VulkanUtils.h"
#include "Misc/Defines/GenericDefines.h"

#include <vector>
#include <string>

class VulkanShaderFactory;
class VulkanSurface;
class VulkanSwapChain;
class VulkanQueue;
class VulkanCommandBuffer;

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

class VRIXIC_API VulkanQueue
{
private:
	/* The Device this queue belongs to */
	VulkanDevice* Device;
	VkQueue Queue;

	/* The queue index into the family of queue queues of this device */
	uint32 QueueIndex;

	/* The family index into the family of queues */
	uint32 FamilyIndex;

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

public:
	inline VkQueue GetQueueHandle() const { return Queue; }

	inline uint32 GetQueueIndex() const { return QueueIndex; }

	inline uint32 GetFamilyIndex() const { return FamilyIndex; }
};

/**
* Representation of vulkan surface 
*/
class VRIXIC_API VulkanSurface
{
private:
	VkInstance* InstanceHandle;
	VulkanDevice* Device;

	VkSurfaceKHR SurfaceHandle;

	VkFormat ColorFormat;
	VkColorSpaceKHR ColorSpace;

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
	* @param inWindowInstance - the window instance this surface will use
	* @param inWindow - this window this surface will use
	* 
	* @remarks Creates the Surface 
	*/
	VulkanSurface(VulkanDevice* inDevice, VkInstance* inInstance, HINSTANCE* inWindowInstance, HWND* inWindow);

	~VulkanSurface();

	VulkanSurface(const VulkanSurface& other) = delete;
	VulkanSurface operator=(const VulkanSurface& other) = delete;

public:
	inline const VkSurfaceKHR* GetSurfaceHandle() const
	{
		return &SurfaceHandle;
	}

	inline const VkFormat* GetColorFormat() const
	{
		return &ColorFormat;
	}

	inline const VkColorSpaceKHR* GetColorSpace() const
	{
		return &ColorSpace;
	}

	//inline const std::vector<VkQueueFamilyProperties>* GetQueueFamilyProperties() const
	//{
		//return &QueueFamilyProperties;
	//}
};

class VRIXIC_API VulkanSwapChain
{
private:
	VulkanDevice* Device;
	VulkanSurface* Surface;
	VkSwapchainKHR SwapChainHandle;

	uint32 ImageWidth;
	uint32 ImageHeight;

	uint32 ImageCount;

	/* Swapchain Images */
	std::vector<VkImage> Images;
	struct SwapChainBuffer
	{
		VkImage Image;
		VkImageView View;
	};
	/* Swap chain buffers */
	std::vector<SwapChainBuffer> Buffers;

public:
	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR fpQueuePresentKHR;

public:
	/**
	* @param inSurface - the surface that will be used to create the swapchain
	* @param inRequestImageWidth - the requested image width of the swapchain images
	* @param inRequestImageHeight - the requested image height of the swapchain images
	* 
	* @remarks Creates the SpawnChain, also depending on the device capabilities the 'requestImageWidth' and 'requestImageHeight' not be approved 
	*/
	VulkanSwapChain(VulkanDevice* inDevice, VulkanSurface* inSurface, uint32 inRequestImageWidth, uint32 inRequestImageHeight);

	~VulkanSwapChain();

	VulkanSwapChain(const VulkanSwapChain& other) = delete;
	VulkanSwapChain operator=(const VulkanSwapChain& other) = delete;

public:
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
	VkResult AcquireNextImage(VulkanCommandBuffer* inLastCommandBuffer, uint32_t* inImageIndex);

	/**
	* Queue an image for presentation
	*
	* @param queue Presentation queue for presenting the image
	* @param imageIndex Index of the swapchain image to queue for presentation
	* @param waitSemaphore (Optional) Semaphore that is waited on before the image is presented (only used if != VK_NULL_HANDLE)
	*
	* @return VkResult of the queue presentation
	*/
	VkResult QueuePresent(VulkanQueue* inQueue, VkSemaphore* inWaitSemaphore, uint32_t inImageIndex);

	
	/**
	* If swapchain gets old, recreate a new on
	* 
	* @param inVSync - enable or disable vsync
	* @param outImageWidth - requested image width, but if swapchain disapproves it, the new image width will be stored instead
	* @param outImageHeight - requested image height, but if swapchain disapproves it, the new image height will be stored instead
	* 
	* @remarks Current swapchain will be used to recreate new one 
	*/
	void Recreate(bool inVSync, uint32* outImageWidth, uint32* outImageHeight);

private:

	/**
	* Creates a new swapchain
	*
	* @param inVSync - enable or disable vsync
	* @param outImageWidth - requested image width, but if swapchain disapproves it, the new image width will be stored instead
	* @param outImageHeight - requested image height, but if swapchain disapproves it, the new image height will be stored instead
	* @param inOldSwapChain - old swapchain that was in use.. 
	* 
	* @remarks if 'inOldSwapChain' is not VK_NULL_HANDLE, then that swapchain will be used to recreate the new one being created 
	*/
	void Create(bool inVSync, uint32* outImageWidth, uint32* outImageHeight, VkSwapchainKHR inOldSwapChain);

public:
	inline const VkSwapchainKHR* GetSwapChainHandle() const
	{
		return &SwapChainHandle;
	}

	inline uint32 GetImageCount() const
	{
		return ImageCount;
	}

	inline const SwapChainBuffer* GetSwapchainBuffer(uint32 index) const
	{
		return &Buffers[index];
	}
};


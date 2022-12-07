#include "VulkanCommandBuffer.h"

/* ------------------------------------------------------------------------------- */
/* -----------------------             Device             ------------------------ */
/* ------------------------------------------------------------------------------- */

VulkanDevice::VulkanDevice(VkPhysicalDevice& gpu, VkPhysicalDeviceFeatures& enabledFeatures, uint32 deviceExtensionCount, const char** deviceExtensions)
	: PhysicalDeviceHandle(gpu), LogicalDeviceHandle(VK_NULL_HANDLE), GraphicsQueue(nullptr), ComputeQueue(nullptr), TransferQueue(nullptr)
{
	// Store Physical Device properties 
	vkGetPhysicalDeviceProperties(gpu, &PhysicalDeviceProperties);
	vkGetPhysicalDeviceFeatures(gpu, &PhysicalDeviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(gpu, &PhysicalDeviceMemProperties);

	// Queue family properties, used for setting up requested queues upon device creation
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDeviceHandle, &QueueFamilyCount, nullptr);
	assert(QueueFamilyCount > 0);
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

	VK_CHECK_RESULT(vkCreateDevice(PhysicalDeviceHandle, &DeviceCreateInfo, nullptr, &LogicalDeviceHandle));

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
	vkDeviceWaitIdle(LogicalDeviceHandle);
}

uint32_t VulkanDevice::GetMemoryTypeIndex(uint32_t inTypeBits, VkMemoryPropertyFlags inProperties, VkBool32* outMemTypeFound) const
{
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

/* ------------------------------------------------------------------------------- */
/* -----------------------             Queue             ------------------------- */
/* ------------------------------------------------------------------------------- */

VulkanQueue::VulkanQueue(VulkanDevice* device, uint32 queueFamilyIndex, uint32 queueIndex)
	: Device(device), FamilyIndex(queueFamilyIndex), QueueIndex(queueIndex)
{
	vkGetDeviceQueue(*device->GetDeviceHandle(), queueFamilyIndex, queueIndex, &Queue);
}

VulkanQueue::~VulkanQueue() { }

void VulkanQueue::SubmitQueue(VulkanCommandBuffer* commandBuffer, uint32 numSignalSemaphores, VkSemaphore* signalSemaphore)
{
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

	// Submit to the graphics queue passing a wait fence
	VK_CHECK_RESULT(vkQueueSubmit(Queue, 1, &SubmitInfo, *commandBuffer->GetWaitFenceHandle()));
}

void VulkanQueue::SubmitQueue(VulkanCommandBuffer* commandBuffer, VkSemaphore* signalSemaphore)
{
	SubmitQueue(commandBuffer, 1, signalSemaphore);
}

/* ------------------------------------------------------------------------------- */
/* -----------------------             Surface           ------------------------- */
/* ------------------------------------------------------------------------------- */
VulkanSurface::VulkanSurface(VulkanDevice* device, VkInstance* instance, HINSTANCE* windowInstance, HWND* window)
	: Device(device), InstanceHandle(instance), ColorFormat(VK_FORMAT_UNDEFINED), ColorSpace((VkColorSpaceKHR)0)
{
	/* Get all required function pointers */
	fpGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(vkGetInstanceProcAddr(*instance, "vkGetPhysicalDeviceSurfaceSupportKHR"));
	fpGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(vkGetInstanceProcAddr(*instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
	fpGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(vkGetInstanceProcAddr(*instance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
	fpGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(vkGetInstanceProcAddr(*instance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));

	// Init surface
	VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = VulkanUtils::Initializers::Win32SurfaceCreateInfoKHR(windowInstance, window);
	VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(*instance, &SurfaceCreateInfo, nullptr, &SurfaceHandle));

	// Get list of supported surface formats
	uint32_t FormatCount;
	VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceFormatsKHR(*device->GetPhysicalDeviceHandle(), SurfaceHandle, &FormatCount, NULL));
	assert(FormatCount > 0);

	std::vector<VkSurfaceFormatKHR> SurfaceFormats(FormatCount);
	VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceFormatsKHR(*device->GetPhysicalDeviceHandle(), SurfaceHandle, &FormatCount, SurfaceFormats.data()));

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
}

VulkanSurface::~VulkanSurface()
{
	Device->WaitUntilIdle();

	vkDestroySurfaceKHR(*InstanceHandle, SurfaceHandle, nullptr);
}

/* ------------------------------------------------------------------------------- */
/* -----------------------            Swapchain          ------------------------- */
/* ------------------------------------------------------------------------------- */

VulkanSwapChain::VulkanSwapChain(VulkanDevice* device, VulkanSurface* surface, uint32 requestImageWidth, uint32 requestImageHeight)
	: Device(device), Surface(surface), ImageWidth(requestImageWidth), ImageHeight(requestImageHeight), SwapChainHandle(VK_NULL_HANDLE)
{
	/* Get the function pointers */
	fpCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(vkGetDeviceProcAddr(*Device->GetDeviceHandle(), "vkCreateSwapchainKHR"));
	fpDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(vkGetDeviceProcAddr(*Device->GetDeviceHandle(), "vkDestroySwapchainKHR"));
	fpGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(vkGetDeviceProcAddr(*Device->GetDeviceHandle(), "vkGetSwapchainImagesKHR"));
	fpAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(*Device->GetDeviceHandle(), "vkAcquireNextImageKHR"));
	fpQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(vkGetDeviceProcAddr(*Device->GetDeviceHandle(), "vkQueuePresentKHR"));

	Create(false, &requestImageWidth, &requestImageHeight, VK_NULL_HANDLE);
}

VulkanSwapChain::~VulkanSwapChain()
{
	Device->WaitUntilIdle();

	/* No Need to destroy the swap chain images ourselfs as the when deleting the swap chain the images gets deleted as well*/
	vkDestroySwapchainKHR(*Device->GetDeviceHandle(), SwapChainHandle, nullptr);

	for (uint32 i = 0; i < Buffers.size(); ++i)
	{
		vkDestroyImageView(*Device->GetDeviceHandle(), Buffers[i].View, nullptr);
	}
}

VkResult VulkanSwapChain::AcquireNextImage(VulkanCommandBuffer* lastCommandBuffer, uint32_t* imageIndex)
{
	// By setting timeout to UINT64_MAX we will always wait until the next image has been acquired or an actual error is thrown
	// With that we don't have to handle VK_NOT_READY		
	return fpAcquireNextImageKHR(*Device->GetDeviceHandle(), SwapChainHandle, UINT64_MAX, *lastCommandBuffer->GetWaitSemaphores(), (VkFence)nullptr, imageIndex);
}

VkResult VulkanSwapChain::QueuePresent(VulkanQueue* queue, VkSemaphore* waitSemaphore, uint32_t imageIndex)
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

void VulkanSwapChain::Create(bool vSync, uint32* imageWidth, uint32* imageHeight, VkSwapchainKHR oldSwapChain)
{
	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR SurfaceCapabilities;
	VK_CHECK_RESULT(Surface->fpGetPhysicalDeviceSurfaceCapabilitiesKHR(*Device->GetPhysicalDeviceHandle(), *Surface->GetSurfaceHandle(), &SurfaceCapabilities));

	// Get available present modes
	uint32_t PresentModeCount = -1;
	VK_CHECK_RESULT(Surface->fpGetPhysicalDeviceSurfacePresentModesKHR(*Device->GetPhysicalDeviceHandle(), *Surface->GetSurfaceHandle(), &PresentModeCount, NULL));
	assert(PresentModeCount > 0);

	std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
	VK_CHECK_RESULT(Surface->fpGetPhysicalDeviceSurfacePresentModesKHR(*Device->GetPhysicalDeviceHandle(), *Surface->GetSurfaceHandle(), &PresentModeCount, PresentModes.data()));

	VkExtent2D SwapchainExtent = {};
	// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
	if (SurfaceCapabilities.currentExtent.width == (uint32_t)-1)
	{
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		SwapchainExtent.width = *imageWidth;
		SwapchainExtent.height = *imageHeight;
	}
	else
	{
		// If the surface size is defined, the swap chain size must match
		SwapchainExtent = SurfaceCapabilities.currentExtent;
		*imageWidth = SurfaceCapabilities.currentExtent.width;
		*imageHeight = SurfaceCapabilities.currentExtent.height;
	}

	/* Set for later use */
	ImageWidth = *imageWidth;
	ImageHeight = *imageHeight;

	// Select a present mode for the swapchain

	// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
	// This mode waits for the vertical blank ("v-sync")
	VkPresentModeKHR SwapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	// If v-sync is not requested, try to find a mailbox mode
	// It's the lowest latency non-tearing present mode available
	if (!vSync)
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

	// Determine the number of images
	uint32_t DesiredNumberOfSwapchainImages = SurfaceCapabilities.minImageCount + 1;
	if ((SurfaceCapabilities.maxImageCount > 0) && (DesiredNumberOfSwapchainImages > SurfaceCapabilities.maxImageCount))
	{
		DesiredNumberOfSwapchainImages = SurfaceCapabilities.maxImageCount;
	}

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
	SwapchainCreateInfo.surface = *Surface->GetSurfaceHandle();
	SwapchainCreateInfo.minImageCount = DesiredNumberOfSwapchainImages;
	SwapchainCreateInfo.imageFormat = *Surface->GetColorFormat();
	SwapchainCreateInfo.imageColorSpace = *Surface->GetColorSpace();
	SwapchainCreateInfo.imageExtent = { SwapchainExtent.width, SwapchainExtent.height };
	SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	SwapchainCreateInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)PreTransform;
	SwapchainCreateInfo.imageArrayLayers = 1;
	SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	SwapchainCreateInfo.queueFamilyIndexCount = 0;
	SwapchainCreateInfo.presentMode = SwapchainPresentMode;
	// Setting oldSwapChain to the saved handle of the previous swapchain aids in resource reuse and makes sure that we can still present already acquired images
	SwapchainCreateInfo.oldSwapchain = oldSwapChain;
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
	VK_CHECK_RESULT(fpCreateSwapchainKHR(*Device->GetDeviceHandle(), &SwapchainCreateInfo, nullptr, &SwapChainHandle));

	// If an existing swap chain is re-created, destroy the old swap chain
	// This also cleans up all the presentable images
	if (oldSwapChain != VK_NULL_HANDLE)
	{
		for (uint32 i = 0; i < Buffers.size(); ++i)
		{
			vkDestroyImageView(*Device->GetDeviceHandle(), Buffers[i].View, nullptr);
		}
		fpDestroySwapchainKHR(*Device->GetDeviceHandle(), oldSwapChain, nullptr);
	}

	Images.clear();
	Buffers.clear();

	// Get the swapchain image count
	VK_CHECK_RESULT(fpGetSwapchainImagesKHR(*Device->GetDeviceHandle(), SwapChainHandle, &ImageCount, NULL));

	// Get the swap chain images
	Images.resize(ImageCount);
	VK_CHECK_RESULT(fpGetSwapchainImagesKHR(*Device->GetDeviceHandle(), SwapChainHandle, &ImageCount, Images.data()));

	// Get the swap chain buffers containing the image and imageview
	Buffers.resize(ImageCount);
	for (uint32 i = 0; i < ImageCount; ++i)
	{
		VkImageViewCreateInfo ColorAttachmentView = { };
		ColorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ColorAttachmentView.pNext = NULL;
		ColorAttachmentView.format = *Surface->GetColorFormat();//ColorFormat;
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
		ColorAttachmentView.flags = 0;

		Buffers[i].Image = Images[i];

		ColorAttachmentView.image = Buffers[i].Image;

		VK_CHECK_RESULT(vkCreateImageView(*Device->GetDeviceHandle(), &ColorAttachmentView, nullptr, &Buffers[i].View));
	}
}

void VulkanSwapChain::Recreate(bool vSync, uint32* imageWidth, uint32* imageHeight)
{
	// Use the current swap chain as the old one 
	Create(vSync, imageWidth, imageHeight, SwapChainHandle);
}


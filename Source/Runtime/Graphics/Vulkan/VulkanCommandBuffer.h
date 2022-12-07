#pragma once
#include "VulkanFrameBuffer.h"

class VulkanCommandPool;

/* 
*  @TODO: State Checking maybe?.... -> Should add some skind of states check, like if its currently in command buffer begins state or in render pass state
*/

/**
* Representation of vulkan Command Buffer
*/
class VulkanCommandBuffer
{
private:
	VulkanDevice* Device;
	VulkanCommandPool* CommandPool;
	VkCommandBuffer CommandBufferHandle;

	std::vector<VkSemaphore> WaitSemaphores;
	VkFence	WaitFence;

	uint32 ImageIndex;

public:
	/**
	* @param inCommandPool - The command pool used to create this command buffer
	* @param inImageIndex - The image index this command buffer will write to/ use
	* 
	* @remarks Create a wait fence on creation for this command buffer
	*/
	VulkanCommandBuffer(VulkanDevice* inDevice, VulkanCommandPool* inCommandPool, uint32 inImageIndex);

	~VulkanCommandBuffer();

	/**
	* Not Copyable 
	*/
	VulkanCommandBuffer(const VulkanCommandBuffer& other) = delete;
	VulkanCommandBuffer operator=(const VulkanCommandBuffer& other) = delete;

public:
	/**
	* Allocated the command buffer
	*/
	void AllocateCommandBuffer();

	/**
	* Frees the command buffer
	*/
	void FreeCommandBuffer();

	/**
	* Begins writing to the command buffer
	*/
	void BeginCommandBuffer();

	/**
	* End writing to the command buffer
	*/
	void EndCommandBuffer();

	/**
	* Build the render pass info, and begin the render pass
	*/
	void BeginRenderPass(const VulkanRenderPass* inRenderPass, VulkanFrameBuffer* inFrameBuffer);

	/**
	*  End the render pass 
	*/
	void EndRenderPass();

	/**
	* Add a wait semaphore
	*/
	void AddWaitSemaphore(VkSemaphore* inSemaphore);

	/**
	* Sets the wait fence
	*/
	void SetWaitFence() const;

	/**
	* Resets the wait fence
	*/
	void ResetWaitFence() const;

public:
	inline uint32 GetWaitSemaphoresCount() const
	{
		return (uint32)WaitSemaphores.size();
	}

	inline const VkSemaphore* GetWaitSemaphores() const
	{
		return WaitSemaphores.data();
	}

	inline const VkFence* GetWaitFenceHandle() const
	{
		return &WaitFence;
	}

	inline uint32 GetImageIndex() const
	{
		return ImageIndex;
	}

	inline const VkCommandBuffer* GetCommandBufferHandle() const
	{
		return &CommandBufferHandle;
	}

	inline VulkanCommandPool* GetCommandPool() const
	{
		return CommandPool;
	}

};

/**
* Representation of vulkan Command Pool
*/
class VulkanCommandPool
{
private:
	VulkanDevice* Device;
	VkCommandPool CommandPoolHandle;

	/* All of the Command buffers associated with this pool */
	std::vector<VulkanCommandBuffer*> CommandBuffers;

public:
	VulkanCommandPool(VulkanDevice* inDevice);
	~VulkanCommandPool();

	VulkanCommandPool(const VulkanCommandPool& other) = delete;
	VulkanCommandPool operator=(const VulkanCommandPool& other) = delete;

public:
	/**
	* Destroyes all command buffers
	*/
	void DestroyBuffers();

public:
	/**
	* Creates a command buffer using this pool
	* 
	* @param inImageIndex - the image index this command buffer will write to/ use
	* 
	* @returns the command buffer created
	*/
	VulkanCommandBuffer* CreateCommandBuffer(uint32 inImageIndex);

	/**
	* Creates a command pool
	* 
	* @param inQueueFamilyIndex - an index into the queue family used to create the command pool
	*/
	void CreateCommandPool(uint32 inQueueFamilyIndex);

public:

	inline VkCommandPool GetCommandPoolHandle() const
	{
		return CommandPoolHandle;
	}

	inline VulkanCommandBuffer* GetCommandBuffer(uint32 bufferIndex) const
	{
		return CommandBuffers[bufferIndex];
	}
};


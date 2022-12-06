#pragma once
#include "VulkanDevice.h"

/* ------------------------------------------------------------------------------- */
/**
* @TODO: Need to create a way to manage buffer and device memory without any memory leaks, maybe VulkanMemoryAllocator????....
*			A way to handle device memory without any leaks
*			What if we allocate a bunch of memory on the GPU heap and just share that memory???....
*			Create abstractions to implement that functionality^
*			What if we have memory heaps for different memory types... since they would be operated upon differently...
* 
* @TODO: Create a way to create VulkanBuffers without memory leaks
* @TODO: Establish a way for VulkanDeviceMemory to communiate with Vulkan Buffers 
* @TODO: Create ways to create different types of buffers easily: Storage, Uniform, Staging, etc...
*/
/* ------------------------------------------------------------------------------- */

/**
* Representation of a vulkan device memory
*/
class VulkanDeviceMemory
{
private:
	friend class VulkanDeviceMemoryAllocater;

	const VulkanDevice* Device;
	VkDeviceMemory MemoryHandle;
	VkDeviceSize Size;

	/* Used to check what memory type this device memory was made from */
	uint32 MemoryTypeIndex;

	void* MappedDataPtr;

	VulkanDeviceMemory(const VulkanDevice* inDevice)
		: Device(inDevice), MemoryHandle(VK_NULL_HANDLE), Size(0), MappedDataPtr(nullptr), MemoryTypeIndex(0) { }

	~VulkanDeviceMemory()
	{
		Device->WaitUntilIdle();
		vkFreeMemory(*Device->GetDeviceHandle(), MemoryHandle, nullptr);
	}

	VulkanDeviceMemory(const VulkanDeviceMemory& other) = delete;
	VulkanDeviceMemory operator=(const VulkanDeviceMemory& other) = delete;

public:
	/**
	* Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
	* 
	* @Param inSize - The size of the memory range to map.Pass VK_WHOLE_SIZE to complete buffer range.
	* @Param inOffset - Byte offset from beginning
	* 
	* @Return void* returns the pointer to the mapped memory location
	*/
	void* Map(VkDeviceSize inSize, VkDeviceSize inOffset)
	{
#if _DEBUG
		VK_CHECK_RESULT(vkMapMemory(*Device->GetDeviceHandle(), MemoryHandle, inOffset, inSize, 0, &MappedDataPtr));
#else
		vkMapMemory(*Device->GetDeviceHandle(), MemoryHandle, inOffset, inSize, 0, &MappedDataPtr);
#endif		
		return MappedDataPtr;
	}

	/**
	* Unmap a mapped memory range
	*
	* @Note Does not return a result as vkUnmapMemory can't fail
	*/
	void Unmap()
	{
		if (MappedDataPtr)
		{
			vkUnmapMemory(*Device->GetDeviceHandle(), MemoryHandle);
			MappedDataPtr = nullptr;
		}
	}

	/**
	* Flush a memory range of the buffer to make it visible to the device
	*
	* @Note Only required for non-coherent memory
	*
	* @Param inSize - Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the complete buffer range.
	* @Param inOffset - Byte offset from beginning
	*
	* @return bool if it was successfully flushed
	*/
	bool FlushMappedMemory(VkDeviceSize inSize, VkDeviceSize inOffset)
	{
		VkMappedMemoryRange MappedMemoryRange = { };
		MappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		MappedMemoryRange.memory = MemoryHandle;
		MappedMemoryRange.offset = inOffset;
		MappedMemoryRange.size = inSize;

		return vkFlushMappedMemoryRanges(*Device->GetDeviceHandle(), 1, &MappedMemoryRange);
	}

	/**
	* Invalidate a memory range of the buffer to make it visible to the host
	*
	* @Note Only required for non-coherent memory
	*
	* @Param inSize - Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate the complete buffer range.
	* @Param inOffset - Byte offset from beginning
	*
	* @Return bool if it was successfully invalidated
	*/
	bool Invalidate(VkDeviceSize inSize, VkDeviceSize inOffset)
	{
		VkMappedMemoryRange MappedMemoryRange = { };
		MappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		MappedMemoryRange.memory = MemoryHandle;
		MappedMemoryRange.offset = inOffset;
		MappedMemoryRange.size = inSize;

		return vkInvalidateMappedMemoryRanges(*Device->GetDeviceHandle(), 1, &MappedMemoryRange);
	}

public:
	inline const VkDeviceMemory* GetMemoryHandle() const
	{
		return &MemoryHandle;
	}

	inline VkDeviceSize GetMemorySize() const
	{
		return Size;
	}

};

/**
* Representation of a vulkan buffer (VkBuffer)
*/
class VulkanBuffer
{
private:
	const VulkanDevice* Device;
	VkBuffer BufferHandle;

public:
	VulkanBuffer(const VulkanDevice* inDevice)
		: Device(inDevice), BufferHandle(VK_NULL_HANDLE) { }

	~VulkanBuffer()
	{
		Device->WaitUntilIdle();
		vkDestroyBuffer(*Device->GetDeviceHandle(), BufferHandle, nullptr);
	}

	VulkanBuffer(const VulkanBuffer& other) = delete;
	VulkanBuffer operator=(const VulkanBuffer& other) = delete;

public:
	bool AllocateBufferMemory(VulkanUtils::Descriptions::VulkanBufferCreateInfo& inBufferCreateInfo)
	{
		//// Create the buffer handle
		//VkBufferCreateInfo BufferCreateInfo = VulkanUtils::Initializers::BufferCreateInfo();
		//BufferCreateInfo.usage = inBufferCreateInfo.BufferUsageFlags;
		//BufferCreateInfo.size = inBufferCreateInfo.DeviceSize;
		//
		//VK_CHECK_RESULT(vkCreateBuffer(*Device->GetDeviceHandle(), &BufferCreateInfo, nullptr, &BufferHandle));
		//
		//// Create the memory backing up the buffer handle
		//VkMemoryRequirements MemoryRequirements;
		//VkMemoryAllocateInfo MemoryAllocateInfo = VulkanUtils::Initializers::MemoryAllocateInfo();
		//vkGetBufferMemoryRequirements(*Device->GetDeviceHandle(), BufferHandle, &MemoryRequirements);
		//
		//MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		//MemoryAllocateInfo.memoryTypeIndex = VulkanUtils::Helpers::GetMemoryType(Device->GetPhysicalDeviceMemoryProperties(), MemoryRequirements.memoryTypeBits, 
		//	inBufferCreateInfo.MemoryPropertyFlags, nullptr);
		//
		//// If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
		//VkMemoryAllocateFlagsInfoKHR MemoryAllocateFlagsInfoKHR = { };
		//if (inBufferCreateInfo.BufferUsageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
		//	MemoryAllocateFlagsInfoKHR.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
		//	MemoryAllocateFlagsInfoKHR.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		//	MemoryAllocateInfo.pNext = &MemoryAllocateFlagsInfoKHR;
		//}
		//
		//VK_CHECK_RESULT(vkAllocateMemory(*Device->GetDeviceHandle(), &MemoryAllocateInfo, nullptr, &MemoryHandle));
		//
		//Alignment = MemoryRequirements.alignment;
		//Size = inBufferCreateInfo.DeviceSize;
		//BufferUsageFlags = inBufferCreateInfo.BufferUsageFlags;
		//MemoryPropertyFlags = inBufferCreateInfo.MemoryPropertyFlags;

		//if (inBufferCreateInfo.Data != nullptr)
		//{
		//	VK_CHECK_RESULT(buffer->map());
		//	memcpy(Data, inBufferCreateInfo.Data, Size);
		//	if ((MemoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
		//		buffer->flush();
		//
		//	buffer->unmap();
		//}
		//
		//// Initialize a default descriptor that covers the whole buffer size
		//buffer->setupDescriptor();

		// Attach the memory to the buffer object
		//return buffer->bind();

		return true;
	}

public:
	const VkBuffer* GetBufferHandle() const
	{
		return &BufferHandle;
	}
};

/**
* An allocater specifically for allocating vulkan device memory, all allocations for vulkan device should happen through this class
*	Keeps device memory leaks from happening
*/
class VulkanDeviceMemoryAllocater
{
private:
	VulkanDevice* Device;

	std::vector<VulkanDeviceMemory*> MemoryAllocations;

public:
	VulkanDeviceMemoryAllocater(VulkanDevice* inDevice)
		: Device(inDevice) { }

	~VulkanDeviceMemoryAllocater()
	{
		Device->WaitUntilIdle();
		for (uint32 i = 0; i < MemoryAllocations.size(); ++i)
		{
			if (MemoryAllocations[i] != nullptr)
			{
				delete MemoryAllocations[i];
			}
		}
	}

	VulkanDeviceMemoryAllocater(const VulkanDeviceMemoryAllocater& other) = delete;
	VulkanDeviceMemoryAllocater operator=(const VulkanDeviceMemoryAllocater& other) = delete;

public:
	/**
	* Allocated memory on the GPU/VulkanDevice
	* 
	* @Param inAllocationSize - Size of the memory to be allocated
	* @Param inMemoryTypeIndex - The memory type to be used, its index 
	* 
	* @Return uint32 - The ID of the Device Memory
	*/
	uint32 AllocateMemory(VkDeviceSize inAllocationSize, uint32 inMemoryTypeIndex)
	{
		VkMemoryAllocateInfo MemoryAllocateInfo = VulkanUtils::Initializers::MemoryAllocateInfo();
		MemoryAllocateInfo.allocationSize = inAllocationSize;
		MemoryAllocateInfo.memoryTypeIndex = inMemoryTypeIndex;

		VkDeviceMemory MemoryHandle = VK_NULL_HANDLE;
		VK_CHECK_RESULT(vkAllocateMemory(*Device->GetDeviceHandle(), &MemoryAllocateInfo, nullptr, &MemoryHandle));

		VulkanDeviceMemory* DeviceMemory = new VulkanDeviceMemory(Device);
		DeviceMemory->MemoryHandle = MemoryHandle;
		DeviceMemory->Size = inAllocationSize;
		DeviceMemory->MemoryTypeIndex = inMemoryTypeIndex;

		MemoryAllocations.push_back(DeviceMemory);

		return MemoryAllocations.size() - 1;
	}

	/**
	* Frees device memory by ID
	*
	* @Param inId - The ID of the Device Memory to be Freed
	*/
	void FreeMemory(uint32 inId)
	{
		delete MemoryAllocations[inId];
		MemoryAllocations[inId] = nullptr;
	}

public:
	/**
	* Gets the Device Memory by ID
	* 
	* @Param inId - the id of the device memory 
	*/
	inline const VulkanDeviceMemory* GetDeviceMemory(uint32 inId) const
	{
		return MemoryAllocations[inId];
	}

};

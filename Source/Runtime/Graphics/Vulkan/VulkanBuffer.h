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

/* ------------------------------------------------------------------------------- */
/**
* @TASK: Create a memory heap manager
*	-> Should manager a large block of GPU memory
*	-> When create a device memory object, it should assign certain blocks of memory to it
* 
*		-> Memory Heap - Allocates a large amount of memory up front
*/
/* ------------------------------------------------------------------------------- */

/**
* Representation of a vulkan device memory
* Allocates device memory for use, DO NOT CREATE THIS OBJECT YOURSELF
*/
class VulkanDeviceMemory
{
private:
	friend class VulkanDeviceMemoryAllocater;
	friend class VulkanBuffer;

	const VulkanDevice* Device;
	VkDeviceMemory MemoryHandle;
	VkDeviceSize Size;

	/* Used to check what memory type this device memory was made from */
	uint32 MemoryTypeIndex;

	void* MappedDataPtr;

	VulkanDeviceMemory(const VulkanDevice* inDevice)
		: Device(inDevice), MemoryHandle(VK_NULL_HANDLE), Size(0), MappedDataPtr(nullptr), MemoryTypeIndex(0) { }

	/**
	* Clean up vulkan device memory upon destruction
	*/
	~VulkanDeviceMemory()
	{
		if (MemoryHandle != VK_NULL_HANDLE)
		{
			Device->WaitUntilIdle();
			vkFreeMemory(*Device->GetDeviceHandle(), MemoryHandle, nullptr);
		}
	}

	VulkanDeviceMemory(const VulkanDeviceMemory& other) = delete;
	VulkanDeviceMemory operator=(const VulkanDeviceMemory& other) = delete;

private:
	/**
	* Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
	*
	* @param inSize - The size of the memory range to map.Pass VK_WHOLE_SIZE to complete buffer range.
	* @param inOffset - Byte offset from beginning
	*
	* @return void* returns the pointer to the mapped memory location
	*/
	void* Map(VkDeviceSize inSize, VkDeviceSize inOffset)
	{
#if _DEBUG | _EDITOR
		VK_CHECK_RESULT(vkMapMemory(*Device->GetDeviceHandle(), MemoryHandle, inOffset, inSize, 0, &MappedDataPtr));
#else
		vkMapMemory(*Device->GetDeviceHandle(), MemoryHandle, inOffset, inSize, 0, &MappedDataPtr);
#endif		
		return MappedDataPtr;
	}

	/**
	* Unmap a mapped memory range
	*
	* @remarks Does not return a result as vkUnmapMemory can't fail
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
	* @remarks Only required for non-coherent memory
	*
	* @param inSize - Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the complete buffer range.
	* @param inOffset - Byte offset from beginning
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
	* @remarks Only required for non-coherent memory
	*
	* @param inSize - Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate the complete buffer range.
	* @param inOffset - Byte offset from beginning
	*
	* @return bool if it was successfully invalidated
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
* An allocater specifically for allocating vulkan device memory, all allocations for vulkan device should happen through this class
*	Keeps device memory leaks from happening, will be used by VulkanMemoryHeap ONLY, DO NOT CREATE
*	THIS OBJECT YOURSELF
*/
class VulkanDeviceMemoryAllocater
{
private:
	friend class VulkanMemoryHeap;

	VulkanDevice* Device;

	std::vector<VulkanDeviceMemory*> MemoryAllocations;

public:
	VulkanDeviceMemoryAllocater(VulkanDevice* inDevice)
		: Device(inDevice) { }

	/**
	* Delete all memory allocations that were made while allocater was active
	*/
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

private:

	/// <summary>
	/// Allocated memory on the GPU/VulkanDevice
	/// </summary>
	/// <param name="inAllocationSize"> Size of the memory to be allocated </param>
	/// <param name="inMemoryTypeIndex"> The memory type to be used, its index </param>
	/// <remarks> Poopy head </remarks>
	/// <returns> The ID of the Device Memory </returns>
	uint32 AllocateMemory(VkDeviceSize inAllocationSize, uint32 inMemoryTypeIndex)
	{
		VkMemoryAllocateInfo MemoryAllocateInfo = VulkanUtils::Initializers::MemoryAllocateInfo();
		MemoryAllocateInfo.allocationSize = inAllocationSize;
		MemoryAllocateInfo.memoryTypeIndex = inMemoryTypeIndex;

		return AllocateMemory(MemoryAllocateInfo);
	}

	/**
	* Allocated memory on the GPU/VulkanDevice with info
	*
	* @param inMemoryAllocateInfo - Info on how to allocate the memory
	*
	* @returns uint32 - The ID of the Device Memory
	*
	* @remarks This can be used if you want to add additional flag information...
	*/
	uint32 AllocateMemory(const VkMemoryAllocateInfo& inMemoryAllocateInfo)
	{
		VkDeviceMemory MemoryHandle = VK_NULL_HANDLE;
#if _DEBUG | _EDITOR
		VK_CHECK_RESULT(vkAllocateMemory(*Device->GetDeviceHandle(), &inMemoryAllocateInfo, nullptr, &MemoryHandle));
#else
		vkAllocateMemory(*Device->GetDeviceHandle(), &inMemoryAllocateInfo, nullptr, &MemoryHandle)
#endif // _DEBUG

		VulkanDeviceMemory* DeviceMemory = new VulkanDeviceMemory(Device);
		DeviceMemory->MemoryHandle = MemoryHandle;
		DeviceMemory->Size = inMemoryAllocateInfo.allocationSize;
		DeviceMemory->MemoryTypeIndex = inMemoryAllocateInfo.memoryTypeIndex;

		MemoryAllocations.push_back(DeviceMemory);

		return MemoryAllocations.size() - 1;
	}

	/**
	* @summary Frees device memory by ID
	*
	* @param inId - The ID of the Device Memory to be Freed
	*/
	void FreeMemory(uint32 inId)
	{
		delete MemoryAllocations[inId];
		MemoryAllocations[inId] = nullptr;
	}

private:
	/**
	* Gets the Device Memory by ID
	*
	* @param inId - the id of the device memory
	*/
	inline const VulkanDeviceMemory* GetDeviceMemory(uint32 inId) const
	{
		return MemoryAllocations[inId];
	}
};

/**
* Representation of a vulkan buffer (VkBuffer)
* Memory visible to GPU, A view into the memory
*/
class VulkanBuffer
{
private:
	friend class VulkanMemoryHeap;

	const VulkanDevice* Device;
	VkBuffer BufferHandle;

	uint32 DeviceMemoryID;

	VkDeviceSize Size;
	VkDeviceSize Alignment;

public:
	VulkanBuffer(const VulkanDevice* inDevice, uint32 inDeviceMemoryID)
		: Device(inDevice), BufferHandle(VK_NULL_HANDLE), DeviceMemoryID(inDeviceMemoryID), Size(0), Alignment(0) { }

	/**
	* Destroy vulkan buffer upon destruction
	*/
	~VulkanBuffer()
	{
		if (BufferHandle != VK_NULL_HANDLE)
		{
			Device->WaitUntilIdle();
			vkDestroyBuffer(*Device->GetDeviceHandle(), BufferHandle, nullptr);
		}
	}

	VulkanBuffer(const VulkanBuffer& other) = delete;
	VulkanBuffer operator=(const VulkanBuffer& other) = delete;

public:
	/// <summary>
	/// Create Buffer handle using the create info (does extra things if BufferUsageFlags requires it), also creates
	/// its own Device Memory 
	/// </summary>
	/// <param name="inAllocater"> The allocater to use to allocate the device memory </param>
	/// <param name="inBufferCreateInfo"> Buffer Handle creation info </param>
	/// <param name="inMemoryTypeIndex"> </param>
	/// <returns> Buffer handle creation is successful </returns>
	bool AllocateBuffer(VulkanDeviceMemoryAllocater* const inAllocater, VulkanUtils::Descriptions::VulkanBufferCreateInfo& inBufferCreateInfo)
	{
		// Create the Buffer Handle 
		AllocateBuffer(inBufferCreateInfo);

		//// Create the memory backing up the buffer handle
		//VkMemoryRequirements MemoryRequirements;
		//VkMemoryAllocateInfo MemoryAllocateInfo = VulkanUtils::Initializers::MemoryAllocateInfo();
		//vkGetBufferMemoryRequirements(*Device->GetDeviceHandle(), BufferHandle, &MemoryRequirements);
		//
		//MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		//MemoryAllocateInfo.memoryTypeIndex = Device->GetMemoryTypeIndex(MemoryRequirements.memoryTypeBits, inBufferCreateInfo.MemoryPropertyFlags, nullptr);
		//
		//// If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
		//VkMemoryAllocateFlagsInfoKHR MemoryAllocateFlagsInfoKHR = { };
		//if (inBufferCreateInfo.BufferUsageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
		//	MemoryAllocateFlagsInfoKHR.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
		//	MemoryAllocateFlagsInfoKHR.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		//	MemoryAllocateInfo.pNext = &MemoryAllocateFlagsInfoKHR;
		//}
		//
		//Alignment = MemoryRequirements.alignment;
		//DeviceMemoryID = inAllocater->AllocateMemory(MemoryAllocateInfo);

		return true;
	}

	/**
	* Link/Bind the allocated memory block (Device Memory) block to buffer
	* 
	* @param offset (Optional) Byte offset (from the beginning) for the memory region to bind
	* 
	* @return bool if it was successfully bound
	*/
	/*bool Bind(VkDeviceSize inOffset)
	{
#if _DEBUG
		VK_CHECK_RESULT(vkBindBufferMemory(*Device->GetDeviceHandle(), BufferHandle, memory, inOffset));
#endif
	}*/

private:
	/// <summary>
	/// Creates a buffer handle using the create info 
	/// </summary>
	/// <param name="inBufferCreateInfo"> Buffer handle creation info </param>
	void AllocateBuffer(VulkanUtils::Descriptions::VulkanBufferCreateInfo& inBufferCreateInfo)
	{
		// Create the buffer handle
		VkBufferCreateInfo BufferCreateInfo = VulkanUtils::Initializers::BufferCreateInfo();
		BufferCreateInfo.usage = inBufferCreateInfo.BufferUsageFlags;
		BufferCreateInfo.size = inBufferCreateInfo.DeviceSize;

		Size = inBufferCreateInfo.DeviceSize;
#if _DEBUG | _EDITOR
		VK_CHECK_RESULT(vkCreateBuffer(*Device->GetDeviceHandle(), &BufferCreateInfo, nullptr, &BufferHandle));
#else
		vkCreateBuffer(*Device->GetDeviceHandle(), &BufferCreateInfo, nullptr, &BufferHandle)
#endif
	}

public:
	const VkBuffer* GetBufferHandle() const
	{
		return &BufferHandle;
	}
};

/**
* The type of buffer, used for creating/allocating memory of use for a certain type of buffer,
* each type of buffer will have it own offset into the memory heap
*/
enum class EBufferType
{
	Index = 0,
	Vertex,
	Storage,
	Uniform,
	Staging // Faster memory access 
};

/**
* Allocates a bunch of memory up front, Memory is split into buffer -> types: Index, Vertex, Storage
*/
class VulkanMemoryHeap
{
private:
	const VulkanDevice* Device;

public:
	VulkanMemoryHeap(VulkanDevice* inDevice)
		: Device(inDevice) { }

	~VulkanMemoryHeap() { }

	VulkanMemoryHeap(const VulkanMemoryHeap& other) = delete;
	VulkanMemoryHeap operator=(const VulkanMemoryHeap& other) = delete;

public:
	/**
	* Allocates a buffer to be used by clients
	* 
	* @param inBufferType The type of buffer to be created, refer to EBufferType...
	* @param inBufferCreateInfo Information of how the buffer should be created 
	* 
	* @return VulkanBuffer* A pointer to the buffer that was created
	*/
	VulkanBuffer* AllocateBuffer(EBufferType inBufferType, VulkanUtils::Descriptions::VulkanBufferCreateInfo& inBufferCreateInfo)
	{
		return nullptr;
	}
};

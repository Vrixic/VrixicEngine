/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "VulkanDevice.h"
#include <Misc/Defines/VulkanProfilerDefines.h>
#include <Runtime/Graphics/Buffer.h>
#include <Runtime/Memory/Core/MemoryManager.h>
#include "VulkanTypeConverter.h"

/**
* Buffer allocation diagram
*	MemoryHeap
*		|
*		Allocate Buffer : usable buffer for anything: index, vertex, storage, etc..
*
* A device should have its own memory heap - 1:1 ratio
* Do NOT create a VulkanBuffer/VulkanDeviceMemory/VulkanDeviceMemoryAllocater yourself
*	use VulkanMemoryHeap for those creations...
*/

/**
* Representation of a vulkan device memory
* Allocates device memory for use, DO NOT CREATE THIS OBJECT YOURSELF
*/
class VRIXIC_API VulkanDeviceMemory
{
public:
	VulkanDeviceMemory(const VulkanDevice* inDevice)
		: Device(inDevice), MemoryHandle(VK_NULL_HANDLE), Size(0), MappedDataPtr(nullptr), MemoryTypeIndex(0) { }

	/**
	* Clean up vulkan device memory upon destruction
	*/
	~VulkanDeviceMemory()
	{
		VE_PROFILE_VULKAN_FUNCTION();

		if (MappedDataPtr != nullptr)
		{
			Unmap();
		}

		if (MemoryHandle != VK_NULL_HANDLE)
		{
			Device->WaitUntilIdle();
			vkFreeMemory(*Device->GetDeviceHandle(), MemoryHandle, nullptr);
		}
	}

	VulkanDeviceMemory(const VulkanDeviceMemory& other) = delete;
	VulkanDeviceMemory operator=(const VulkanDeviceMemory& other) = delete;

public:
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
		VE_PROFILE_VULKAN_FUNCTION();

#if _DEBUG | _DEBUG_EDITOR
		VK_CHECK_RESULT(vkMapMemory(*Device->GetDeviceHandle(), MemoryHandle, inOffset, inSize, 0, &MappedDataPtr), "[VulkanBuffer]: Failed trying to map buffer memory");
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
		VE_PROFILE_VULKAN_FUNCTION();

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
		VE_PROFILE_VULKAN_FUNCTION();

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
		VE_PROFILE_VULKAN_FUNCTION();

		VkMappedMemoryRange MappedMemoryRange = { };
		MappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		MappedMemoryRange.memory = MemoryHandle;
		MappedMemoryRange.offset = inOffset;
		MappedMemoryRange.size = inSize;

		return vkInvalidateMappedMemoryRanges(*Device->GetDeviceHandle(), 1, &MappedMemoryRange);
	}

public:
	inline void* GetMappedPointer() const
	{
		return MappedDataPtr;
	}

	inline const VkDeviceMemory* GetMemoryHandle() const
	{
		return &MemoryHandle;
	}

	inline VkDeviceSize GetMemorySize() const
	{
		return Size;
	}

private:
    friend class VulkanDeviceMemoryAllocater;

    const VulkanDevice* Device;
    VkDeviceMemory MemoryHandle;
    VkDeviceSize Size;

    /** Used to check what memory type this device memory was made from */
    uint32 MemoryTypeIndex;

    void* MappedDataPtr;
};

/**
* An allocater specifically for allocating vulkan device memory, all allocations for vulkan device should happen through this class
*	Keeps device memory leaks from happening, will be used by VulkanMemoryHeap ONLY, DO NOT CREATE
*	THIS OBJECT YOURSELF
*/
class VRIXIC_API VulkanDeviceMemoryAllocater
{
public:
	VulkanDeviceMemoryAllocater(VulkanDevice* inDevice)
		: Device(inDevice) { }

	/**
	* Delete all memory allocations that were made while allocater was active
	*/
	~VulkanDeviceMemoryAllocater()
	{
		VE_PROFILE_VULKAN_FUNCTION();

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
	* @param inAllocationSize Size of the memory to be allocated
	* @param inMemoryTypeIndex The memory type to be used, its index
	*
	* @return uint32 The ID of the Device Memory
	*/
	uint32 AllocateMemory(VkDeviceSize inAllocationSize, uint32 inMemoryTypeIndex)
	{
		VE_PROFILE_VULKAN_FUNCTION();

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
		VE_PROFILE_VULKAN_FUNCTION();

		VkDeviceMemory MemoryHandle = VK_NULL_HANDLE;
#if _DEBUG | _DEBUG_EDITOR
		VK_CHECK_RESULT(vkAllocateMemory(*Device->GetDeviceHandle(), &inMemoryAllocateInfo, nullptr, &MemoryHandle), "[VulkanBuffer]: Failed trying to allocate buffer memory");
#else
		vkAllocateMemory(*Device->GetDeviceHandle(), &inMemoryAllocateInfo, nullptr, &MemoryHandle);
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
		VE_PROFILE_VULKAN_FUNCTION();

		delete MemoryAllocations[inId];
		MemoryAllocations[inId] = nullptr;
	}

private:
	/**
	* Gets the Device Memory by ID
	*
	* @param inId - the id of the device memory
	*/
	inline VulkanDeviceMemory* GetDeviceMemory(uint32 inId) const
	{
		return MemoryAllocations[inId];
	}

private:
    friend class VulkanMemoryHeap;

    VulkanDevice* Device;

    std::vector<VulkanDeviceMemory*> MemoryAllocations;
};

/**
* Representation of a vulkan buffer (VkBuffer)
* Memory visible to GPU, A view into the memory
* 
* -- Memory is already mapped to meaning Pointer to Memory in CPU side already exits, make sure to 64 byte align memory ranges when flushing 
*/
class VRIXIC_API VulkanBuffer : public Buffer
{
public:
	VulkanBuffer(const VulkanDevice* inDevice, const FBufferConfig& inBufferConfiguration, int32 inDeviceMemoryID, uint64 inOffset)
		: Device(inDevice), BufferHandle(VK_NULL_HANDLE), DeviceMemoryID(inDeviceMemoryID),
		Alignment(0), Offset(inOffset), DeviceMemory(nullptr) 
    { 
        BufferConfiguration = inBufferConfiguration;
    }

	/**
	* Destroy vulkan buffer upon destruction
	*/
	~VulkanBuffer()
	{
		if (BufferHandle != VK_NULL_HANDLE)
		{
			Device->WaitUntilIdle();
			vkDestroyBuffer(*Device->GetDeviceHandle(), BufferHandle, nullptr);
            BufferHandle = VK_NULL_HANDLE;
		}
	}

	VulkanBuffer(const VulkanBuffer& other) = delete;
	VulkanBuffer operator=(const VulkanBuffer& other) = delete;

public:
	/**
	* Invalidate a memory range to make it visible to the host CPU
	* refer to VulkanDeviceMemory::Invalidate() for in depth overview
	*/
	bool Invalidate(VkDeviceSize inSize, VkDeviceSize inOffset)
	{
		return DeviceMemory->Invalidate(inSize, Offset + inOffset);
	}

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
		inOffset += Offset;
		if (inSize == VK_WHOLE_SIZE)
		{
			inSize = BufferConfiguration.Size;
		}

		return DeviceMemory->Map(inSize, inOffset);
	}

	/**
	* Unmap a mapped memory range
	*
	* @remarks Does not return a result as vkUnmapMemory can't fail
	*/
	void Unmap()
	{
		DeviceMemory->Unmap();
	}

	/**
	* Flushes a mapped memory range to make it visible to the device,
	* refer to VulkanDeviceMemory::FlushMappedMemory() for in depth overview
	*/
	bool FlushMappedMemory(VkDeviceSize inSize, VkDeviceSize inOffset)
	{
		inOffset += Offset;
		if (inSize == VK_WHOLE_SIZE)
		{
			inSize = BufferConfiguration.Size;
		}

		return DeviceMemory->FlushMappedMemory(inSize, inOffset);
	}

private:
    /**
    * Create Buffer handle using the create info (does extra things if BufferUsageFlags requires it), also creates
    *
    * @param inAllocater The allocater to use to allocate the device memory
    * @param inBufferCreateInfo Buffer Handle creation info
    *
    * @return bool Buffer handle creation is successful
    */
    bool AllocateBuffer(VulkanDeviceMemoryAllocater* const inAllocater, VulkanUtils::Descriptions::FVulkanBufferCreateInfo& inBufferCreateInfo)
    {
        // Create the Buffer Handle 
        AllocateBuffer(inBufferCreateInfo);

        // Create the memory backing up the buffer handle
        VkMemoryRequirements MemoryRequirements;
        VkMemoryAllocateInfo MemoryAllocateInfo = VulkanUtils::Initializers::MemoryAllocateInfo();
        vkGetBufferMemoryRequirements(*Device->GetDeviceHandle(), BufferHandle, &MemoryRequirements);

        MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
        MemoryAllocateInfo.memoryTypeIndex = Device->GetMemoryTypeIndex(MemoryRequirements.memoryTypeBits, inBufferCreateInfo.MemoryPropertyFlags, nullptr);

        // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
        VkMemoryAllocateFlagsInfoKHR MemoryAllocateFlagsInfoKHR = { };
        if (inBufferCreateInfo.BufferUsageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            MemoryAllocateFlagsInfoKHR.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
            MemoryAllocateFlagsInfoKHR.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
            MemoryAllocateInfo.pNext = &MemoryAllocateFlagsInfoKHR;
        }

        Alignment = MemoryRequirements.alignment;
        DeviceMemoryID = inAllocater->AllocateMemory(MemoryAllocateInfo);

        return true;
    }

	/**
	* Creates a buffer handle using the create info
	*
	* @param inBufferCreateInfo Buffer handle creation info
	*/
	void AllocateBuffer(VulkanUtils::Descriptions::FVulkanBufferCreateInfo& inBufferCreateInfo)
	{
		// Create the buffer handle
		VkBufferCreateInfo BufferCreateInfo = VulkanUtils::Initializers::BufferCreateInfo();
		BufferCreateInfo.usage = inBufferCreateInfo.BufferUsageFlags;
		BufferCreateInfo.size = inBufferCreateInfo.DeviceSize;

		VK_CHECK_RESULT(vkCreateBuffer(*Device->GetDeviceHandle(), &BufferCreateInfo, nullptr, &BufferHandle), "[VulkanBuffer]: Failed trying to create buffer");
	}

	/**
	* Link/Bind the allocated memory block (Device Memory) block to buffer.., Bind device memory to a buffer
	*
	* @param offset (Optional) Byte offset (from the beginning) for the memory region to bind
	*
	* @return bool if it was successfully bound
	*/
	bool Bind(VkDeviceSize inOffset)
	{
#if _DEBUG
		VkResult Result;
		Result = vkBindBufferMemory(*Device->GetDeviceHandle(), BufferHandle, *DeviceMemory->GetMemoryHandle(), Offset + inOffset);
		VK_CHECK_RESULT(Result, "[VulkanBuffer]: Failed buffer bind/linkage");

		return Result == VK_SUCCESS;
#else
		return vkBindBufferMemory(*Device->GetDeviceHandle(), BufferHandle, *DeviceMemory->GetMemoryHandle(), Offset + inOffset) == VK_SUCCESS;
#endif
	}
public:
	uint64 GetBufferSize() const
	{
		return BufferConfiguration.Size;
	}

    /**
    * @returns uint64 the offset of the buffer from start of device memory 
    */
    uint64 GetBufferOffset() const
    {
        return Offset;
    }

	const VkBuffer* GetBufferHandle() const
	{
		return &BufferHandle;
	}

	/**
	* Returns the mapped pointer to the data 
	* The actual pointer gets offseted to reach buffer data 
	*/
	void* GetMappedPointer() const
	{
		uint8* PointerOffseted = static_cast<uint8*>(DeviceMemory->GetMappedPointer());
		PointerOffseted += Offset;

		return PointerOffseted;
	}

	///*
	//* @return VulkanDeviceMemory* the handle to the device memory occupied by the buffer 
	//*/
	//VulkanDeviceMemory* GetDeviceMemoryHandle() const
	//{
	//	return DeviceMemory;
	//}

private:
	uint32 GetDeviceMemoryID() const
	{
		return DeviceMemoryID;
	}

private:
    friend class VulkanMemoryHeap;

    const VulkanDevice* Device;
    VkBuffer BufferHandle;

    /** A ID for the device memory */
    int32 DeviceMemoryID;

    /** The device memory that the ID refers to if valid */
    VulkanDeviceMemory* DeviceMemory;

    uint64 Offset;

    VkDeviceSize Alignment;
};

/**
* The type of buffer, used for creating/allocating memory of use for a certain type of buffer,
* each type of buffer will have it own offset into the memory heap
*/
enum class EBufferType : uint32
{
	Index = 0,
	Vertex,
	Storage,
	Uniform,
	Staging // Faster memory access 
};

/**
* Allocates a bunch of memory up front, Memory is split into buffer -> types: Index, Vertex, Storage
* For any kind of buffer creation, clients have to use the heap interface
*/
class VRIXIC_API VulkanMemoryHeap
{
public:
	/**
	* Creates the buffer/heap
	*	1048576 bytes is one MiB(mebibytes)
	*
	* @param inHeapSizeInMebibytes Size of the heap in mebibtyes
	*/
	VulkanMemoryHeap(VulkanDevice* inDevice, uint32 inHeapSizeInMebibytes)
		: Device(inDevice), HeapSizeInMebibytes(inHeapSizeInMebibytes)
	{
		VE_PROFILE_VULKAN_FUNCTION();

		DeviceMemoryAllocater = new VulkanDeviceMemoryAllocater(inDevice);
		
		uint64 HeapSize = MEBIBYTES_TO_BYTES(inHeapSizeInMebibytes);

		VulkanUtils::Descriptions::FVulkanBufferCreateInfo BufferCreateInfo = { };
		BufferCreateInfo.DeviceSize = (VkDeviceSize)HeapSize;
		BufferCreateInfo.BufferUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		BufferCreateInfo.MemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

        FBufferConfig BufferConfg;
        BufferConfg.Size = HeapSize;

		Buffer = new VulkanBuffer(Device, BufferConfg, -1, 0);
		Buffer->AllocateBuffer(DeviceMemoryAllocater, BufferCreateInfo);
		MemoryID = Buffer->GetDeviceMemoryID();

		DeviceMemoryAllocater->GetDeviceMemory(MemoryID)->Map(VK_WHOLE_SIZE, 0);

		MemoryUsed = 0;
	}

	~VulkanMemoryHeap()
	{
		VE_PROFILE_VULKAN_FUNCTION();

		delete Buffer;
		delete DeviceMemoryAllocater;

		for (uint32 i = 0; i < AllocatedBuffers.size(); ++i)
		{
			if (AllocatedBuffers[i] != nullptr)
			{
				delete AllocatedBuffers[i];
			}
		}
	}

	VulkanMemoryHeap(const VulkanMemoryHeap& other) = delete;
	VulkanMemoryHeap operator=(const VulkanMemoryHeap& other) = delete;

public:
	/**
	* Allocates a buffer to be used by clients
	*
	* @param inBufferType The type of buffer to be created, refer to EBufferType...
	* @param inBufferConfig Information of how the buffer should be created
	*
	* @return VulkanBuffer* A pointer to the buffer that was created
	*/
	VulkanBuffer* AllocateBuffer(/*EBufferType inBufferType,*/ const FBufferConfig& inBufferConfig)
	{
		VE_PROFILE_VULKAN_FUNCTION();

		VulkanBuffer* Buffer = nullptr;

        if (inBufferConfig.UsageFlags & FResourceBindFlags::IndexBuffer)
        {
            Buffer = AllocateIndexBuffer(inBufferConfig);
        }
        else if (inBufferConfig.UsageFlags & FResourceBindFlags::VertexBuffer)
        {
            Buffer = AllocateVertexBuffer(inBufferConfig);
        }
        else if ((inBufferConfig.UsageFlags & FResourceBindFlags::UniformBuffer) || (inBufferConfig.UsageFlags & FResourceBindFlags::ConstantBuffer))
        {
            Buffer = AllocateUniformBuffer(inBufferConfig);
        }
        else if (inBufferConfig.UsageFlags & FResourceBindFlags::StorageBuffer)
        {
            Buffer = AllocateStorageBuffer(inBufferConfig);
        }
        else if (inBufferConfig.UsageFlags & FResourceBindFlags::StagingBuffer)
        {
            Buffer = AllocateStagingBuffer(inBufferConfig);
        }

        VE_ASSERT(Buffer != nullptr, VE_TEXT("[VulkanBuffer]: Failed to allocate buffer..."));

		return Buffer;
	}

private:
	/**
	* Allocated a index buffer
	*/
	VulkanBuffer* AllocateIndexBuffer(const FBufferConfig& inBufferConfig)
	{
		return AllocateBufferInternal(inBufferConfig);
	}

	/**
	* Allocated a vertex buffer
	*/
	VulkanBuffer* AllocateVertexBuffer(const FBufferConfig& inBufferConfig)
    {
        return AllocateBufferInternal(inBufferConfig);
    }

	/**
	* Allocated a storage buffer
	*/
	VulkanBuffer* AllocateStorageBuffer(const FBufferConfig& inBufferConfig)
    {
        return AllocateBufferInternal(inBufferConfig);
    }
	
	/**
	* Allocated a uniform buffer
	*/
	VulkanBuffer* AllocateUniformBuffer(const FBufferConfig& inBufferConfig)
    {
        return AllocateBufferInternal(inBufferConfig);
    }

    /**
    * Allocated a staging buffer
    */
    VulkanBuffer* AllocateStagingBuffer(const FBufferConfig& inBufferConfig)
    {
        return AllocateBufferInternal(inBufferConfig);
    }

	/**
	* Allocates a buffer, Alignment of memory offset is done for you
	*
	* @param inBufferCreateInfo information on how to create the buffer
	*
	* @return VulkanBuffer* The buffer that was created
	*/
	VulkanBuffer* AllocateBufferInternal(const FBufferConfig& inBufferConfig)
	{
		VE_PROFILE_VULKAN_FUNCTION();

        VulkanUtils::Descriptions::FVulkanBufferCreateInfo BufferCreateInfo = { 0 };

        BufferCreateInfo.BufferUsageFlags = VulkanTypeConverter::ConvertBufferUsageFlagsToVk(inBufferConfig.UsageFlags);
        BufferCreateInfo.DeviceSize = inBufferConfig.Size;
        BufferCreateInfo.MemoryPropertyFlags = VulkanTypeConverter::ConvertMemoryFlagsToVk(inBufferConfig.MemoryFlags);

		VulkanBuffer* AllocBuffer = new VulkanBuffer(Device, inBufferConfig , -1, MemoryUsed);
		AllocBuffer->AllocateBuffer(BufferCreateInfo);

        // Get teh memory requirements for its alignment 
        VkMemoryRequirements MemoryRequirements;
        VkMemoryAllocateInfo MemoryAllocateInfo = VulkanUtils::Initializers::MemoryAllocateInfo();
        vkGetBufferMemoryRequirements(*Device->GetDeviceHandle(), AllocBuffer->BufferHandle, &MemoryRequirements);

        /* Align the memory to alignment so next buffer will be aligned correctly */
        float AlignmentCheck = static_cast<float>(MemoryUsed) / static_cast<float>(MemoryRequirements.alignment);
        AlignmentCheck = ceilf(AlignmentCheck);

        MemoryUsed = static_cast<uint64>(AlignmentCheck * MemoryRequirements.alignment);

        // Set the new buffer variables 
		AllocBuffer->DeviceMemory = DeviceMemoryAllocater->GetDeviceMemory(MemoryID);
		AllocBuffer->DeviceMemoryID = MemoryID;
		AllocBuffer->Alignment = MemoryRequirements.alignment;
		AllocBuffer->Offset = MemoryUsed;

        // bind the new buffer 
		AllocBuffer->Bind(0);

        if (inBufferConfig.InitialData != nullptr)
        {
            memcpy(AllocBuffer->GetMappedPointer(), inBufferConfig.InitialData, inBufferConfig.Size);
        }

        MemoryUsed += inBufferConfig.Size;
        AllocatedBuffers.push_back(AllocBuffer);

		return AllocBuffer;
	}

private:
    friend class VulkanBuffer;

    const VulkanDevice* Device;

    /** Size of the heap in mebibytes */
    uint32 HeapSizeInMebibytes;

    /** where the memory is stored in VulkanDeviceMemoryAllocater */
    uint32 MemoryID;

    /** Heap buffer -> the view of the heap memory */
    VulkanBuffer* Buffer;
    VulkanDeviceMemoryAllocater* DeviceMemoryAllocater;

    /** Avoid memory leaking from buffers */ 
    std::vector<VulkanBuffer*> AllocatedBuffers;

    uint64 MemoryUsed;
};

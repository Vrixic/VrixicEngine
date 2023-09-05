/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "ResourcePool.h"
#include <Runtime/Memory/Core/MemoryManager.h>

void ResourcePool::Init(uint32 inPoolSize, uint32 inResourceSize)
{
    PoolSize = inPoolSize;
    ResourceSize = inResourceSize;

    // Add uint32 size as we use it for free indices MemBlock-> [Resources | Indices]
    MemoryHandle = TPointer<uint8>(MemoryManager::Get().MallocAligned<uint8>(inPoolSize * (inResourceSize + sizeof(uint32))));

    // Allocate Free Indices and Assign Default Values
    FreeIndices = (uint32*)(MemoryHandle.Get() + (inPoolSize * inResourceSize));

    FreeAll();
}

void ResourcePool::Shutdown()
{
    if (FreeIndicesHead != 0)
    {
        VE_CORE_LOG_INFO(VE_TEXT("[ResourcePool]: Has unfreed resources..."));
    }

    MemoryHandle.Free();
}

uint32 ResourcePool::Allocate()
{
    if (FreeIndicesHead < PoolSize)
    {
        const uint32 FreeIndex = FreeIndices[FreeIndicesHead++];
        ++UsedIndices;

        return FreeIndex;
    }

    VE_ASSERT(false, VE_TEXT("[ResourcePool]: No more resources left to allocate...!"));
    return UINT32_MAX;
}

void ResourcePool::Free(uint32 inResourceHandle)
{
    FreeIndices[--FreeIndicesHead] = inResourceHandle;
    --UsedIndices;
}

void ResourcePool::FreeAll()
{
    FreeIndicesHead = 0;
    UsedIndices = 0;

    for (uint32 i = 0; i < PoolSize; ++i)
    {
        FreeIndices[i] = i;
    }
}

void* ResourcePool::Get(uint32 inResourceHandle)
{
    if (inResourceHandle != UINT32_MAX)
    {
        return &MemoryHandle.Get()[inResourceHandle * ResourceSize];
    }
    return nullptr;
}

const void* ResourcePool::Get(uint32 inResourceHandle) const
{
    if (inResourceHandle != UINT32_MAX)
    {
        return &MemoryHandle.Get()[inResourceHandle * ResourceSize];
    }
    return nullptr;
}

/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "TextureResourcePool.h"
#include <Runtime/Memory/Core/MemoryManager.h>

void TextureResourcePool::Init(uint32 inPoolSize, uint32 inResourceSize)
{
    PoolSize = inPoolSize;
    ResourceSize = inResourceSize;

    // Add uint32 size as we use it for free indices MemBlock-> [Resources | Indices]
    MemoryHandle = TPointer<uint8>(MemoryManager::Get().MallocAligned<uint8>(inPoolSize * (inResourceSize + sizeof(uint32))));

    // Allocate Free Indices and Assign Default Values
    FreeIndices = (uint32*)(MemoryHandle.Get() + (inPoolSize * inResourceSize));

    FreeAll();
}

void TextureResourcePool::Shutdown()
{
    if (FreeIndicesHead != 0)
    {
        VE_CORE_LOG_INFO(VE_TEXT("[TextureResourcePool]: Has unfreed resources..."));
    }

    MemoryHandle.Free();
}

uint32 TextureResourcePool::Allocate()
{
    if (FreeIndicesHead < PoolSize)
    {
        const uint32 FreeIndex = FreeIndices[FreeIndicesHead++];
        ++UsedIndices;

        return FreeIndex;
    }

    VE_ASSERT(false, VE_TEXT("[TextureResourcePool]: No more resources left to allocate...!"));
    return UINT32_MAX;
}

void TextureResourcePool::Free(uint32 inResourceHandle)
{
    FreeIndices[--FreeIndicesHead] = inResourceHandle;
    --UsedIndices;
}

void TextureResourcePool::FreeAll()
{
    FreeIndicesHead = 0;
    UsedIndices = 0;

    for (uint32 i = 0; i < PoolSize; ++i)
    {
        FreeIndices[i] = i;
    }
}

void* TextureResourcePool::Get(uint32 inResourceHandle)
{
    if (inResourceHandle != UINT32_MAX)
    {
        return &MemoryHandle.Get()[inResourceHandle * ResourceSize];
    }
    return nullptr;
}

const void* TextureResourcePool::Get(uint32 inResourceHandle) const
{
    if (inResourceHandle != UINT32_MAX)
    {
        return &MemoryHandle.Get()[inResourceHandle * ResourceSize];
    }
    return nullptr;
}

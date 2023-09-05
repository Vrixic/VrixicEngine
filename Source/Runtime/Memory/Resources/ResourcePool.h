/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Misc/Defines/GenericDefines.h>

/**
* A base resource pool class
*
* @note all objects that can be used with a resource pool have to be child classes of RenderResource
*/
class ResourcePool
{
public:
    virtual void Init(uint32 inPoolSize, uint32 inResourceSize);
    virtual void Shutdown();

    uint32 Allocate();
    void Free(uint32 inResourceHandle);
    void FreeAll();

    void* Get(uint32 inResourceHandle);
    const void* Get(uint32 inResourceHandle) const;

protected:
    TPointer<uint8> MemoryHandle;

    uint32 PoolSize;
    uint32 ResourceSize;

    /** Used to keep track of freed/available resource */
    uint32* FreeIndices;
    uint32 FreeIndicesHead;

    /** For Resource Tracking */
    uint32 UsedIndices;
};

/**
* A generic resource pool that allows for easier resource pool creations
*/
template<class T>
class TResourcePool : public ResourcePool
{
public:
    virtual void Init(uint32 inPoolSize, uint32 inResourceSize);
    virtual void Shutdown();

    T* Allocate();
    void Free(T* inResource);

    T* Get(uint32 inResourceHandle);
    const T* Get(uint32 inResourceHandle) const;
};

template<class T>
inline void TResourcePool<T>::Init(uint32 inPoolSize, uint32 inResourceSize)
{
    ResourcePool::Init(inPoolSize, inResourceSize);
}

template<class T>
inline void TResourcePool<T>::Shutdown()
{
    ResourcePool::Shutdown();
}

template<class T>
inline T* TResourcePool<T>::Allocate()
{
    if (FreeIndicesHead < PoolSize)
    {
        const uint32 FreeIndex = FreeIndices[FreeIndicesHead++];
        ++UsedIndices;

        T* Resource = Get(FreeIndex);
        Resource->ResourcePoolIndex = FreeIndex;
        return Resource;
    }

    VE_ASSERT(false, VE_TEXT("[ResourcePool]: No more resources left to allocate...!"));
    return nullptr;
}

template<class T>
inline void TResourcePool<T>::Free(T* inResource)
{
    ResourcePool::Free(inResource->ResourcePoolIndex)
}

template<class T>
inline T* TResourcePool<T>::Get(uint32 inResourceHandle)
{
    return (T*)ResourcePool::Get(inResourceHandle);
}

template<class T>
inline const T* TResourcePool<T>::Get(uint32 inResourceHandle) const
{
    return (T*)ResourcePool::Get(inResourceHandle);
}

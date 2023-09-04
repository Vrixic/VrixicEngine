/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Misc/Defines/GenericDefines.h>

/**
* A resource pool for textures | Render Interface will be able to initialize this and create a texture resource pool for their specific render interface type 
*/
class TextureResourcePool
{
public:
    void Init(uint32 inPoolSize, uint32 inResourceSize);
    void Shutdown();

    uint32 Allocate();
    void Free(uint32 inResourceHandle);
    void FreeAll();

    void* Get(uint32 inResourceHandle);
    const void* Get(uint32 inResourceHandle) const;

private:
    TPointer<uint8> MemoryHandle;

    uint32 PoolSize;
    uint32 ResourceSize;

    /** Used to keep track of freed/available resource */
    uint32* FreeIndices;
    uint32 FreeIndicesHead;

    /** For Resource Tracking */
    uint32 UsedIndices;
};


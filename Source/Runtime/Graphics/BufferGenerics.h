/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>

/**
* Buffer usage flags indicate how the buffer will be used
*/
enum class EBufferUsageFlags
{
    // used as a vertex buffer
    Vertex,

    // used as an index buffer
    Index,

    // used as a storage buffer
    Storage
};

/**
* Buffer memory flags that are used to for allocating device memory 
*/
struct VRIXIC_API MemoryFlags
{
#define BIT(x) (1 << x)

    enum
    {
        // most efficient for device access memory 
        DeviceLocal = BIT(0),

        // specifies that memory can be mapped for host access 
        HostVisible = BIT(1),

        // specifies that host cache management commands (flush, invalidate) is not needed to flush host write
        // or make device writes visible to the host 
        HostCoherent = BIT(2),

        // specifies that memory is cached on the host. 'uncached memory is always coherent (slower than cached)'
        HostCached = BIT(3),
    };
};
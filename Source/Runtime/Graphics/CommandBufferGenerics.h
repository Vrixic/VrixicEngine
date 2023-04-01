/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include <Misc/Defines/GenericDefines.h>

/**
* Where the pipeline can be bound to 
*/
enum class EPipelineBindPoint
{
    Graphics,
    Compute
};

struct VRIXIC_API CommandBufferFlags
{
    enum
    {
        // Can Execute secondary command buffers and are submitted to queues
        Primary     = (1 << 0),

        // Can execute primary command buffers but are not directly submitted to queues
        Secondary   = (1 << 1)
    };
};

struct VRIXIC_API CommandBufferConfig
{
public:
    // Flags for command buffer creation
    uint32 Flags;

    // Number of command buffers to create/allocate
    uint32 NumBuffersToAllocate;

public:
    CommandBufferConfig() = default;
    CommandBufferConfig(const CommandBufferConfig&) = default;
    CommandBufferConfig& operator = (const CommandBufferConfig&) = default;
};
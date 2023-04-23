/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include "CommandQueue.h"
#include <Misc/Defines/GenericDefines.h>

/**
* Flags used to specifing the level of the command buffer 
*/
struct VRIXIC_API CommandBufferLevelFlags
{
    enum
    {
        // Can Execute secondary command buffers and are submitted to queues
        Primary     = (1 << 0),

        // Can execute primary command buffers but are not directly submitted to queues
        Secondary   = (1 << 1)
    };
};

/**
* Contains information used to configure how a command buffer is created 
*/
struct VRIXIC_API CommandBufferConfig
{
public:
    // Flags for command buffer creation
    uint32 Flags;

    // Number of command buffers to create/allocate
    uint32 NumBuffersToAllocate;

    // The queue this command buffer will get submitted to 
    ICommandQueue* CommandQueue;

public:
    CommandBufferConfig() = default;
    CommandBufferConfig(const CommandBufferConfig&) = default;
    CommandBufferConfig& operator = (const CommandBufferConfig&) = default;
};
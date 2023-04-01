/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Misc/Interface.h>
#include "CommandBuffer.h"
#include "Fence.h"

class VRIXIC_API ICommandQueue : public Interface
{
public:
    /**
    * Submits the specified command buffer to a queue (Can be any type of queue, ex: graphics, compute),
    *  uses the command buffers fence to submit the buffer
    * 
    * @param inCommandBuffer the command buffer that will get submitted
    */
    virtual void Submit(ICommandBuffer* inCommandBuffer) = 0;

    /**
    * Sets a wait fence that will block the CPU execution until the fence has been signaled
    * 
    * @param inWaitFence the fence the CPU will wait to be signaled
    * @param inTimeout this is the waiting timeout in nanoseconds 
    */
    virtual void SetWaitFence(IFence* inWaitFence, uint64 inTimeout) = 0;

    /**
    * Blocks the CPU execution until all submitted command/fences have been completed in other words signaled,
    * (Vulkan Description) -> this is the equivalent to having submitted a valid fence to every previously executed queue submission command 
    * that accepts a fence, then waiting for all of those fences to signal using vkWaitForFences with an infinite timeout
    */
    virtual void SetWaitIdle() = 0;
};
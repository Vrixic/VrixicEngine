/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Misc/Interface.h>
#include "CommandBuffer.h"
#include "Fence.h"
#include "Semaphore.h"


enum class ERenderQueueType
{
    Graphics,
    Compute,
    Transfer
};

class VRIXIC_API ICommandQueue : public Interface
{
public:
    ICommandQueue(ERenderQueueType inQueueType) : QueueType(inQueueType) { }

    /**
    * Submits the specified command buffer to a queue (Can be any type of queue, ex: graphics, compute),
    *  uses the command buffers fence to submit the buffer
    *
    * @param inCommandBuffer the command buffer that will get submitted
    * @param inNumWaitSemaphores number of wait semaphores
    * @param inWaitSemaphores the wait semaphore(s)
    * @param numSignalSemaphores number of signal semaphores
    * @param inSignalSemaphores the signal semaphore(s)
    * @param inWaitFence the wait fence 
    */
    virtual void Submit(ICommandBuffer* inCommandBuffer, uint32 inNumWaitSemaphores, ISemaphore* inWaitSemaphores, uint32 inNumSignalSemaphores, ISemaphore* inSignalSemaphores, IFence* inWaitFence) = 0;

    /**
    * Submits the specified command buffer to a queue (Can be any type of queue, ex: graphics, compute),
    *  uses the command buffers fence to submit the buffer
    *
    * @param inCommandBuffer the command buffer that will get submitted
    * @param numSignalSemaphores number of signal semaphores
    * @param inSignalSemaphores the signal semaphore(s)
    */
    virtual void Submit(ICommandBuffer* inCommandBuffer, uint32 inNumSignalSemaphores, ISemaphore* inSignalSemaphores) = 0;

    /**
    * Submits the specified command buffer to a queue (Can be any type of queue, ex: graphics, compute),
    *  uses the command buffers fence to submit the buffer
    *
    * @param inCommandBuffer the command buffer that will get submitted
    * @param inWaitFence the wait fence 
    */
    virtual void Submit(ICommandBuffer* inCommandBuffer, IFence* inWaitFence) = 0;

    /**
    * Sets a wait fence that will block the CPU execution until the fence has been signaled
    *
    * @param inWaitFence the fence the CPU will wait to be signaled
    * @param inTimeout this is the waiting timeout in nanoseconds
    */
    virtual void SetWaitFence(IFence* inWaitFence, uint64 inTimeout) const = 0;

    /**
    * Checks the wait fence passed in to see if it has been signaled
    *
    * @returns bool - true if the fence has already been signaled, false otherwise
    */
    virtual bool GetWaitFenceStatus(IFence* inWaitFence) const = 0;

    /**
    * Resets a wait fence
    *
    * @param inWaitFence the fence to reset
    */
    virtual void ResetWaitFence(IFence* inWaitFence) const = 0;

    /**
    * Blocks the CPU execution until all submitted command/fences have been completed in other words signaled,
    * (Vulkan Description) -> this is the equivalent to having submitted a valid fence to every previously executed queue submission command
    * that accepts a fence, then waiting for all of those fences to signal using vkWaitForFences with an infinite timeout
    */
    virtual void SetWaitIdle() = 0;

private:
    ERenderQueueType QueueType;
};
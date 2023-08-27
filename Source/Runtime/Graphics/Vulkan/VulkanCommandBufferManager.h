
#pragma once
#include <Runtime/Graphics/Renderer.h>
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"

#include <Runtime/Graphics/ICommandBufferManager.h>

#include <Misc/Defines/StringDefines.h>
    
/**
* Manages command buffers for the main queue family (The one that supports both graphics and presentation)
*/

class VulkanCommandBufferManager : public ICommandBufferManager
{
private:
    friend class VulkanRenderInterface;
    friend class CommandBufferManager;

    VulkanCommandBufferManager(VulkanDevice* inDevice)
    {
        Device = inDevice;
    }

    ~VulkanCommandBufferManager()
    {
        if (VulkanCommandPools.size())
        {
            Shutdown();
        }
    }

    void Init(uint32 inNumThreads) override 
    {
        NumPoolsPerFrame = inNumThreads;
        NumCommandBuffersPerThread = Renderer::Get().GetSwapchain()->GetImageCount();

        // Create the command pools: number of frames * number of threads 
        const uint32 NumPools = NumPoolsPerFrame * NumCommandBuffersPerThread;
        VulkanCommandPools.resize(NumPools);

        // 
        UsedCommandBuffers.resize(NumPools);
        UsedSecondaryCommandBuffers.resize(NumPools);

        VkCommandPoolCreateInfo CommandPoolCreateInfo = VulkanUtils::Initializers::CommandPoolCreateInfo
        (
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, Device->GetPresentQueue()->GetFamilyIndex(),
            nullptr
        );

        for (uint32 i = 0; i < NumPools; ++i)
        {
            VulkanCommandPool*& CommandPool = VulkanCommandPools[i];
            CommandPool = new VulkanCommandPool(Device);
            CommandPool->CreateCommandPool(Device->GetPresentQueue()->GetFamilyIndex());

            UsedCommandBuffers[i] = 0;
            UsedSecondaryCommandBuffers[i] = 0;
        }

        // Create Command Buffers: number of command pools * number of command buffers per pool
        const uint32 NumBuffers = NumPools * NumCommandBuffersPerThread;
        CommandBuffers.resize(NumBuffers);

        const uint32 NumSecondaryBuffers = NumPools * NumSecondaryCommandBuffersPerThread;
        SecondaryCommandBuffers.resize(NumBuffers);

        for (uint32 i = 0; i < NumBuffers; ++i)
        {
            const uint32 FrameIndex = i / (NumCommandBuffersPerThread * NumPoolsPerFrame);
            const uint32 ThreadIndex = (i / NumCommandBuffersPerThread) % NumPoolsPerFrame;
            const uint32 PoolIndex = (FrameIndex * NumPoolsPerFrame) + ThreadIndex;

            VulkanCommandBuffer*& CurrentCommandBuffer = CommandBuffers[i];
            VulkanCommandPool* CommandPool = VulkanCommandPools[PoolIndex];

            CurrentCommandBuffer = CommandPool->CreateCommandBuffer(FrameIndex);
            CurrentCommandBuffer->AllocateCommandBuffer();
        }

        FCommandBufferConfig CommandBufferConfig = { };
        CommandBufferConfig.CommandQueue = Device->GetPresentQueue();
        CommandBufferConfig.Flags = FCommandBufferLevelFlags::Secondary;
        CommandBufferConfig.NumBuffersToAllocate = 1;

        for (uint32 poolIndex = 0; poolIndex < NumPools; ++poolIndex)
        {
            VulkanCommandPool* CommandPool = VulkanCommandPools[poolIndex];
            for (uint32 scbIndex = 0; scbIndex < NumCommandBuffersPerThread; ++scbIndex)
            {
                VulkanCommandBuffer*& CommandBuffer = SecondaryCommandBuffers[(poolIndex * NumCommandBuffersPerThread) + scbIndex];
                CommandBuffer = CommandPool->CreateCommandBuffer(0);
                CommandBuffer->AllocateCommandBuffer(CommandBufferConfig);
            }
        }
    }

    void Shutdown() override
    {
        for (uint32 i = 0; i < VulkanCommandPools.size(); ++i)
        {
            delete VulkanCommandPools[i];
        }

        VulkanCommandPools.clear();
    }

    void ResetCommandPools(uint32 inFrameIndex) override
    {
        for (uint32 i = 0; i < NumPoolsPerFrame; ++i)
        {
            const uint32 PoolIndex = CalcPoolIndex(inFrameIndex, i);
            VulkanCommandPools[PoolIndex]->Reset();

            UsedCommandBuffers[PoolIndex] = 0;
            UsedSecondaryCommandBuffers[PoolIndex] = 0;
        }
    }

    VulkanCommandBuffer* GetCommandBuffer(uint32 inFrameIndex, uint32 inThreadIndex) override
    {
        const uint32 PoolIndex = CalcPoolIndex(inFrameIndex, inThreadIndex);
        uint32 CurrentUsedBuffer = UsedCommandBuffers[PoolIndex];
        //UsedCommandBuffers[PoolIndex]++;
        VE_ASSERT(CurrentUsedBuffer < NumCommandBuffersPerThread, VE_TEXT("[VulkanCommandBufferManager]: Thread {0} is trying to use more than {1} command buffers, which is not allowed..."), inThreadIndex, NumCommandBuffersPerThread);

        VulkanCommandBuffer* CommandBuffer = VulkanCommandPools[PoolIndex]->GetCommandBuffer(CurrentUsedBuffer);
        return CommandBuffer;
    }

    VulkanCommandBuffer* GetSecondaryCommandBuffer(uint32 inFrameIndex, uint32 inThreadIndex) override
    {
        const uint32 PoolIndex = CalcPoolIndex(inFrameIndex, inThreadIndex);
        uint32 CurrentUsedBuffer = UsedSecondaryCommandBuffers[PoolIndex];
        UsedSecondaryCommandBuffers[PoolIndex]++;
        VE_ASSERT(CurrentUsedBuffer < NumSecondaryCommandBuffersPerThread, VE_TEXT("[VulkanCommandBufferManager]: Thread {0} is trying to use more than {1} secondary command buffers, which is not allowed..."), inThreadIndex, NumCommandBuffersPerThread);

        VulkanCommandBuffer* CommandBuffer = VulkanCommandPools[PoolIndex]->GetCommandBuffer(NumCommandBuffersPerThread + CurrentUsedBuffer);
        return CommandBuffer;
    }

    uint32 CalcPoolIndex(uint32 inFrameIndex, uint32 inThreadIndex) const
    {
        return (inFrameIndex * NumPoolsPerFrame) + inThreadIndex;
    }

private:
    VulkanDevice* Device;

    uint16 NumPoolsPerFrame;
    const uint8 NumSecondaryCommandBuffersPerThread = 2;
    uint8 NumCommandBuffersPerThread = 3;

    std::vector<VulkanCommandPool*> VulkanCommandPools;
    std::vector<VulkanCommandBuffer*> CommandBuffers;
    std::vector<VulkanCommandBuffer*> SecondaryCommandBuffers;

    std::vector<uint8> UsedCommandBuffers; // per-frame used command buffers per thread 
    std::vector<uint8> UsedSecondaryCommandBuffers; // per-frame used command buffers per thread 
};
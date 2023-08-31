#pragma once
#include "CommandBuffer.h"
#include <Core/Misc/IManager.h>
#include <Misc/Assert.h>
#include <Misc/Defines/StringDefines.h>

/**
* Interface for CommandBufferManager which manages all of the command buffer to be used by the renderer
* 
* @Note the usage of this is optional, but it is a safe multi-threaded command buffer usage 
*/
class ICommandBufferManager : public Interface
{
public:
    virtual void Init(uint32 inNumThreads) = 0;
    virtual void Shutdown() = 0;

    virtual void ResetCommandPools(uint32 inFrameIndex) = 0;
    virtual ICommandBuffer* GetCommandBuffer(uint32 inFrameIndex, uint32 inThreadIndex) = 0;
    virtual ICommandBuffer* GetSecondaryCommandBuffer(uint32 inFrameIndex, uint32 inThreadIndex) = 0;
};

/**
* Configuration used to initialize 'CommandBufferManager'
*/
struct FCommandBufferManagerConfig
{
public:
    ICommandBufferManager* Manager = nullptr;
    uint32 NumThreads = 0;
};

class CommandBufferManager : public IManager
{
private:
    friend class Renderer;
    friend class VulkanRenderInterface;

    VRIXIC_STATIC_MANAGER(CommandBufferManager)

    virtual void Init(void* inConfig) override
    {
        VE_ASSERT(inConfig != nullptr, VE_TEXT("[CommandBufferManager]: Cannot initialize Command Buffer Manager without a valid configuration..."));
        VE_ASSERT(Manager == nullptr, VE_TEXT("[CommandBufferManager]: Cannot initialize Command Buffer Manager twice..."));

        FCommandBufferManagerConfig& Config = *((FCommandBufferManagerConfig*)(inConfig));
        Manager = Config.Manager;
        Manager->Init(Config.NumThreads);
    }

    void Shutdown()
    {
        Manager->Shutdown();
    }

    void ResetCommandPools(uint32 inFrameIndex)
    {
        Manager->ResetCommandPools(inFrameIndex);
    }

    ICommandBuffer* GetCommandBuffer(uint32 inFrameIndex, uint32 inThreadIndex)
    {
        return Manager->GetCommandBuffer(inFrameIndex, inThreadIndex);
    }

    ICommandBuffer* GetSecondaryCommandBuffer(uint32 inFrameIndex, uint32 inThreadIndex)
    {
        return Manager->GetSecondaryCommandBuffer(inFrameIndex, inThreadIndex);
    }

private:
    ICommandBufferManager* Manager = nullptr;
};

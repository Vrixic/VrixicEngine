#pragma once
#include "CommandBuffer.h"
#include <Core/Misc/Interface.h>
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

class CommandBufferManager
{
private:
    friend class Renderer;
    friend class VulkanRenderInterface;

    /**
    * Returns the one and only Instance to CommandBufferManager
    */
    static CommandBufferManager& Get()
    {
        static CommandBufferManager Instance;
        return Instance;
    }

    void Init(ICommandBufferManager* inManager, uint32 inNumThreads)
    {
        VE_ASSERT(Manager == nullptr, VE_TEXT("[CommandBufferManager]: Cannot initialize the command buffer manager twice..."));
        Manager = inManager;
        Manager->Init(inNumThreads);
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

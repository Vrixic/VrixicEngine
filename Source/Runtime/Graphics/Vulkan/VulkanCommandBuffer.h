/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include <Runtime/Graphics/CommandBuffer.h>
#include <Runtime/Graphics/CommandBufferGenerics.h>
#include "VulkanFrameBuffer.h"

class VulkanCommandPool;

/*
*  @TODO: State Checking maybe?.... -> Should add some skind of states check, like if its currently in command buffer begins state or in render pass state
*/

/**
* Representation of vulkan Command Buffer
*/
class VRIXIC_API VulkanCommandBuffer : public ICommandBuffer
{
private:
    VulkanDevice* Device;
    VulkanCommandPool* CommandPool;
    VkCommandBuffer CommandBufferHandle;

    std::vector<VkSemaphore> WaitSemaphores;
    VkFence	WaitFence;

    uint32 ImageIndex;

    uint32 AllocatedBufferCount;

public:
    /**
    * @param inCommandPool - The command pool used to create this command buffer
    * @param inImageIndex - The image index this command buffer will write to/ use
    *
    * @remarks Create a wait fence on creation for this command buffer
    */
    VulkanCommandBuffer(VulkanDevice* inDevice, VulkanCommandPool* inCommandPool, uint32 inImageIndex);

    ~VulkanCommandBuffer();

    /**
    * Not Copyable
    */
    VulkanCommandBuffer(const VulkanCommandBuffer& other) = delete;
    VulkanCommandBuffer operator=(const VulkanCommandBuffer& other) = delete;

public:
    /*-- ICommandBuffer Interface --*/

    /* ------------------------------------------------------------------------------- */
    /* -------------             Command Buffer Recording          ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Begins the recording process which enables the renderer to start listing GPU commands
    * @remarks resets all previously recorded commands
    */
    virtual void Begin() override;

    /**
    * Ends the recording process
    * @remarks The command buffer can now be submitted to a CommandQueue for presentation
    */
    virtual void End() override;

    /* ------------------------------------------------------------------------------- */
    /* ---------------            Viewports and Scissors           ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Sets viewports for the command buffer, at least 1 viewport has to be set or it'll be undefined behaviour
    *
    * @param inRenderViewport this viewport information will be set for the command buffer
    * @param inNumRenderViewport the number of (inRenderViewport) viewports being submitted
    */
    virtual void SetRenderViewports(const RenderViewport* inRenderViewports, uint32 inNumRenderViewports) override;

    /**
    * Sets scissors ( Rectangles ) for the command buffers
    *
    * @param inRenderScissors render scissors that will be set to this command buffer
    * @param inNumRenderScissors the number of (inRenderScissors) scissors being submitted
    */
    virtual void SetRenderScissors(const RenderScissor* inRenderScissors, uint32 inNumRenderScissors) override;

    /* ------------------------------------------------------------------------------- */
    /* ---------------               Input Assembly                ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Sets the specified vertex buffer passed in to be used for drawing
    *
    * @param inVertexBuffer the vertex buffer to set
    */
    virtual void SetVertexBuffer(Buffer& inVertexBuffer) override;

    /**
    * Sets the specified index buffer passed in to be used for drawing
    *
    * @param inIndexBuffer the index buffer to set
    * @remarks - fixed index buffer type hard set to VK_INDEX_TYPE_UINT32, need to abstract that to to configs 
    */
    virtual void SetIndexBuffer(Buffer& inIndexBuffer) override;

    /* ------------------------------------------------------------------------------- */
    /* ---------------                 RenderPass                  ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Begins a render pass enabling pipeline to render
    *
    * @param inRenderPassBeginInfo the information used to begin the render pass
    * @info render pass are just steps (subpasses) drawing commands are divided into for allowing things like attachments, etc.. to happen (also keeps relationships between the commands)
    */
    virtual void BeginRenderPass(const RenderPassBeginInfo& inRenderPassBeginInfo) override;

    /**
    * Ends the current render pass
    */
    virtual void EndRenderPass() override;

    /* ------------------------------------------------------------------------------- */
    /* ---------------                Pipeline Stuff               ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Binds a pipeline which is then used for drawing operations
    *
    * @param inPipeline the pipeline state to bind
    */
    virtual void BindPipeline(const IPipeline* inPipeline) override;

    /* ------------------------------------------------------------------------------- */
    /* ---------------                Drawing Stuff                ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Draws to the command buffer using the specified vertices from the currently bound vertex buffer
    *
    * @param inNumVertices number of verticies to draw from the first vertex index passed in
    * @param inFirstVertexIndex (default = 0) the first vertexs index (Offset from 0)
    */
    virtual void Draw(uint32 inNumVertices, uint32 inFirstVertexIndex = 0) override;

    /**
    * Draw to the command buffer using the specified indices from the currently bound vertex buffer
    *
    * @param inNumIndices the number of indices used to draw
    * @param inFirstIndex (default = 0)  an offset from 0 specifying the first index to start from
    * @param inVertexOffset (default = 0) specifies the vertex offset that is added all each and every index from the index buffer (offset can be postive or negative)
    */
    virtual void DrawIndexed(uint32 inNumIndices, uint32 inFirstIndex = 0, int32 inVertexOffset = 0) override;

    /**
    * Draws specified amount of instances to the command buffer which uses the currently bound vertex buffer
    *
    * @param inNumVertices the number of vertices to draw
    * @param inNumInstances number of instances to draw
    * @param inFirstVertexIndex (default = 0) the first vertexs index (Offset from 0)
    * @param inFirstInstanceIndex (default = 0) offset from 0 indicating the first instance, AKA instanceId
    */
    virtual void DrawInstanced(uint32 inNumVertices, uint32 inNumInstances, uint32 inFirstVertexIndex = 0, uint32 inFirstInstanceIndex = 0) override;

    /**
    * Draws specified amount of instances to the command buffer which uses the currently bound vertex and index buffers
    *
    * @param inNumIndices the number of indices used to draw
    * @param inNumInstances number of instances to draw
    * @param inFirstIndex (default = 0)  an offset from 0 specifying the first index to start from
    * @param inVertexOffset (default = 0) specifies the vertex offset that is added all each and every index from the index buffer (offset can be postive or negative)
    * @param inFirstInstanceIndex (default = 0) offset from 0 indicating the first instance, AKA instanceId
    */
    virtual void DrawIndexedInstanced(uint32 inNumIndices, uint32 inNumInstances, uint32 inFirstIndex = 0, uint32 inVertexOffset = 0, uint32 inFirstInstanceIndex = 0) override;

    /*-- EnmdICommandBuffer Interface --*/

    /**
    * Creates a wait fence for this command buffer 
    */
    void CreateWaitFence();

    /**
    * Allocates the command buffer
    */
    void AllocateCommandBuffer();

    /**
    * Allocates the command buffer
    * 
    * @param inConfig the configuration used to allocate this command buffer 
    */
    void AllocateCommandBuffer(const CommandBufferConfig& inConfig);

    /**
    * Frees the command buffer | This command buffer cannot be usable again 
    */
    void FreeCommandBuffer();

    /**
    * Begins writing to the command buffer
    */
    void BeginCommandBuffer();

    /**
    * End writing to the command buffer
    */
    void EndCommandBuffer();

    ///**
    //* Build the render pass info, and begin the render pass
    //*/
    //void BeginRenderPass(const VulkanRenderPass* inRenderPass, VulkanFrameBuffer* inFrameBuffer);

    ///**
    //*  End the render pass
    //*/
    //void EndRenderPass();

    /**
    * Add a wait semaphore
    */
    void AddWaitSemaphore(VkSemaphore* inSemaphore);

    /**
    * Sets the wait fence
    */
    void SetWaitFence() const;

    /**
    * Resets the wait fence
    */
    void ResetWaitFence() const;

public:
    inline uint32 GetWaitSemaphoresCount() const
    {
        return (uint32)WaitSemaphores.size();
    }

    inline const VkSemaphore* GetWaitSemaphores() const
    {
        return WaitSemaphores.data();
    }

    inline const VkFence* GetWaitFenceHandle() const
    {
        return &WaitFence;
    }

    inline uint32 GetImageIndex() const
    {
        return ImageIndex;
    }

    inline const VkCommandBuffer* GetCommandBufferHandle() const
    {
        return &CommandBufferHandle;
    }

    inline VulkanCommandPool* GetCommandPool() const
    {
        return CommandPool;
    }

    inline uint32 GetAllocatedBufferCount() const
    {
        return AllocatedBufferCount;
    }

};

/**
* Representation of vulkan Command Pool
*/
class VRIXIC_API VulkanCommandPool
{
private:
    VulkanDevice* Device;
    VkCommandPool CommandPoolHandle;

    /* All of the Command buffers associated with this pool */
    std::vector<VulkanCommandBuffer*> CommandBuffers;

public:
    VulkanCommandPool(VulkanDevice* inDevice);
    ~VulkanCommandPool();

    VulkanCommandPool(const VulkanCommandPool& other) = delete;
    VulkanCommandPool operator=(const VulkanCommandPool& other) = delete;

public:
    /**
    * Destroyes all command buffers
    */
    void DestroyBuffers();

public:
    /**
    * Creates a command buffer using this pool
    *
    * @param inImageIndex - the image index this command buffer will write to/ use
    *
    * @returns the command buffer created
    */
    VulkanCommandBuffer* CreateCommandBuffer(uint32 inImageIndex);

    /**
    * Creates a command pool
    *
    * @param inQueueFamilyIndex - an index into the queue family used to create the command pool
    */
    void CreateCommandPool(uint32 inQueueFamilyIndex);

    /**
    * Erases the command buffer 
    * 
    * @param inCmdBuffer the command buffer to be erased 
    */
    void EraseCommandBuffer(VulkanCommandBuffer* inCmdBuffer);

public:

    inline VkCommandPool GetCommandPoolHandle() const
    {
        return CommandPoolHandle;
    }

    inline VulkanCommandBuffer* GetCommandBuffer(uint32 bufferIndex) const
    {
        return CommandBuffers[bufferIndex];
    }
};


/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "Buffer.h"
#include "DescriptorSet.h"
#include "Pipeline.h"
#include "PipelineGenerics.h"
#include "RenderPass.h"

class IFence;

/**
* A graphics api independent command buffer that is used for storing/encoding GPU commands, which will get executed later in submission process
* @remarks just like a normal specific Command Buffer, you have to begin the encoding process and end it as well
*/
class VRIXIC_API ICommandBuffer : Interface
{
public:
    /* ------------------------------------------------------------------------------- */
    /* -------------             Command Buffer Recording          ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Begins the recording process which enables the renderer to start listing GPU commands
    * @remarks resets all previously recorded commands
    */
    virtual void Begin() const = 0;

    /**
    * Ends the recording process
    * @remarks The command buffer can now be submitted to a CommandQueue for presentation
    */
    virtual void End() const = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                 Synchronization               ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Adds a wait semaphore, that will be used for command buffer synchronization
    */
    //virtual void AddWaitSemaphore(ISemaphore* inWaitSemaphore) = 0;

    /* ------------------------------------------------------------------------------- */
    /* ---------------            Viewports and Scissors           ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Sets viewports for the command buffer, at least 1 viewport has to be set or it'll be undefined behaviour
    *
    * @param inRenderViewport this viewport information will be set for the command buffer
    * @param inNumRenderViewport the number of (inRenderViewport) viewports being submitted
    */
    virtual void SetRenderViewports(const FRenderViewport* inRenderViewport, uint32 inNumRenderViewports) = 0;

    /**
    * Sets scissors ( Rectangles ) for the command buffers
    *
    * @param inRenderScissors render scissors that will be set to this command buffer
    * @param inNumRenderScissors the number of (inRenderScissors) scissors being submitted
    */
    virtual void SetRenderScissors(const FRenderScissor* inRenderScissors, uint32 inNumRenderScissors) = 0;

    /* ------------------------------------------------------------------------------- */
    /* ---------------               Input Assembly                ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Sets the specified vertex buffer passed in to be used for drawing
    *
    * @param inVertexBuffer the vertex buffer to set
    */
    virtual void SetVertexBuffer(Buffer& inVertexBuffer) = 0;

    /**
    * Sets the specified index buffer passed in to be used for drawing
    *
    * @param inIndexBuffer the index buffer to set
    */
    virtual void SetIndexBuffer(Buffer& inIndexBuffer) = 0;

    /* ------------------------------------------------------------------------------- */
    /* ---------------                 RenderPass                  ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Begins a render pass enabling pipeline to render
    *
    * @param inRenderPassBeginInfo the information used to begin the render pass
    * @info render pass are just steps (subpasses) drawing commands are divided into for allowing things like attachments, etc.. to happen (also keeps relationships between the commands)
    */
    virtual void BeginRenderPass(const FRenderPassBeginInfo& inRenderPassBeginInfo) const = 0;

    /**
    * Ends the current render pass
    */
    virtual void EndRenderPass() const = 0;

    /* ------------------------------------------------------------------------------- */
    /* ---------------                Pipeline Stuff               ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Binds a pipeline which is then used for drawing operations
    *
    * @param inPipeline the pipeline state to bind
    */
    virtual void BindPipeline(const IPipeline* inPipeline) = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                 Descriptor Sets               ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Binds a descriptor set to the command buffer 
    * 
    * @param inDescriptorSetBindInfo the binding information used to bind the descriptor sets 
    */
    virtual void BindDescriptorSets(const FDescriptorSetsBindInfo& inDescriptorSetBindInfo) = 0;

    /* ------------------------------------------------------------------------------- */
    /* ---------------                Drawing Stuff                ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Draws to the command buffer using the specified vertices from the currently bound vertex buffer
    *
    * @param inNumVertices number of verticies to draw from the first vertex index passed in
    * @param inFirstVertexIndex (default = 0) the first vertexs index (Offset from 0)
    */
    virtual void Draw(uint32 inNumVertices, uint32 inFirstVertexIndex = 0) = 0;

    /**
    * Draw to the command buffer using the specified indices from the currently bound vertex buffer
    *
    * @param inNumIndices the number of indices used to draw
    * @param inFirstIndex (default = 0)  an offset from 0 specifying the first index to start from
    * @param inVertexOffset (default = 0) specifies the vertex offset that is added all each and every index from the index buffer (offset can be postive or negative)
    */
    virtual void DrawIndexed(uint32 inNumIndices, uint32 inFirstIndex = 0, int32 inVertexOffset = 0) = 0;

    /**
    * Draws specified amount of instances to the command buffer which uses the currently bound vertex buffer
    *
    * @param inNumVertices the number of vertices to draw
    * @param inNumInstances number of instances to draw
    * @param inFirstVertexIndex (default = 0) the first vertexs index (Offset from 0)
    * @param inFirstInstanceIndex (default = 0) offset from 0 indicating the first instance, AKA instanceId
    */
    virtual void DrawInstanced(uint32 inNumVertices, uint32 inNumInstances, uint32 inFirstVertexIndex = 0, uint32 inFirstInstanceIndex = 0) = 0;

    /**
    * Draws specified amount of instances to the command buffer which uses the currently bound vertex and index buffers
    *
    * @param inNumIndices the number of indices used to draw
    * @param inNumInstances number of instances to draw
    * @param inFirstIndex (default = 0)  an offset from 0 specifying the first index to start from
    * @param inVertexOffset (default = 0) specifies the vertex offset that is added all each and every index from the index buffer (offset can be postive or negative)
    * @param inFirstInstanceIndex (default = 0) offset from 0 indicating the first instance, AKA instanceId
    */
    virtual void DrawIndexedInstanced(uint32 inNumIndices, uint32 inNumInstances, uint32 inFirstIndex = 0, uint32 inVertexOffset = 0, uint32 inFirstInstanceIndex = 0) = 0;

public:
    /* ------------------------------------------------------------------------------- */
    /* -------------                 Synchronization               ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * @returns IFence* the wait fence in use by this command buffer
    */
    virtual IFence* GetWaitFence() const = 0;
};

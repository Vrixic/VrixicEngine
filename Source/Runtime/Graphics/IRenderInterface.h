/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "Buffer.h"
#include "BufferGenerics.h"
#include "CommandBufferGenerics.h"
#include "CommandPool.h"
#include "CommandPoolGenerics.h"
#include "CommandQueue.h"
#include <Core/Misc/Interface.h>
#include "DescriptorSet.h"
#include "FrameBuffer.h"
#include "Pipeline.h"
#include "PipelineGenerics.h"
#include "PipelineLayout.h"
#include "RenderInterfaceGenerics.h"
#include "RenderPass.h"
#include "RenderPassGenerics.h"
#include "Sampler.h"
#include "SamplerGenerics.h"
#include "Shader.h"
#include "SwapChain.h"
#include "Texture.h"

#include <vector>

/**
* IMGUI SECTION INFO
* Vrixic Engines editor will be made using ImGui, and so to make it simple to inject and use ImGui
* The implementation details of ImGui will be left for the specific Render Interface to implement
*/

/**
* All supported graphics interface, if a graphics interface is supported, it must include a renderer for itself, its resource specific management deriving from IResourceManager;
*/
static std::vector<ERenderInterfaceType> SupportedGraphicInterfaces = { ERenderInterfaceType::Vulkan };

class VRIXIC_API IRenderInterface : Interface
{
public:
    /**
    * Initializes the render interface
    */
    virtual void Initialize() = 0;

    /**
    * Shuts down this interface making it unuseable
    */
    virtual void Shutdown() = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                   Swap chains                 ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a new swapchain that is renders onto the surface that is used to create the swap chain
    * -> Do not pass in a null surface, it should already be created as windows are already created before renderer initialization
    *
    * @param inSwapChainConfig the configuration used to create a swapchain
    * @param inSurface the surface that the swapchain uses and render to
    *
    * @remarks multi-swapchains not supported yet
    */
    virtual SwapChain* CreateSwapChain(const FSwapChainConfig& inSwapChainConfig, Surface* inSurface) = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                 Command Buffers               ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates command buffer (s) with the specified settings as configured in the configuration | (For vulkan: The command pool is created by the queue in use; meaning a CommandQueue will always have its respective CommandPool)
    *
    * @param inCmdBufferConfig specifies the configuration of the command buffers (if empty, it'll create one command buffer that is level Primary)
    */
    virtual ICommandBuffer* CreateCommandBuffer(const FCommandBufferConfig& inCmdBufferConfig) = 0;

    /**
    * Releases/Destroys the command buffer passed in
    *
    * @param inCommandBufferToFree the command buffer to free
    */
    virtual void Free(ICommandBuffer* inCommandBufferToFree) = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                     Buffers                   ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a buffer with the specified buffer configuration
    *
    * @param inBufferConfig info used to create the buffer
    */
    virtual Buffer* CreateBuffer(const FBufferConfig& inBufferConfig) = 0;

    /**
    * Writes/Update data to the specified buffer (if data already exist, this will update)
    *
    * @param inBuffer the buffer to write to or update
    * @param inOffset a byte offset from the start of the buffer
    * @param inData pointer to the data that will be set to the buffer
    * @param inDataSize size in bytes that will be updated
    */
    virtual void WriteToBuffer(Buffer* inBuffer, uint64 inOffset, const void* inData, uint64 inDataSize) = 0;

    /**
    * Reads data from the buffer specified
    *
    * @param inBuffer the buffer to read the data from
    * @param inOffset a byte offset from the start of the buffer (specifys the start of the read)
    * @param outData a pointer to the data that will be set after buffer has been read
    * @param inDataSize amount of bytes to read from the buffer
    */
    virtual void ReadFromBuffer(Buffer* inBuffer, uint64 inOffset, void* outData, uint64 inDataSize) = 0;

    /**
    * Releases/Destroys the buffer passed in
    *
    * @param inBuffer the buffer to free
    */
    virtual void Free(Buffer* inBuffer) = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                    Textures                   ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a new texture
    *
    * @param inTextureConfig  info used to create the texture
    */
    virtual Texture* CreateTexture(const FTextureConfig& inTextureConfig) = 0;

    /**
    * Copies the data from the buffer provided and puts it into the texture
    *
    * @param inTexture the texture that will contain the data after copy
    * @param inTextureWriteInfo contains information used to write to the texture
    */
    virtual void WriteToTexture(const Texture* inTexture, const FTextureWriteInfo& inTextureWriteInfo) = 0;

    /**
    * Releases/Destroys the texture passed in
    *
    * @param inTexture the texture to free
    */
    virtual void Free(Texture* inTexture) = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                  Frame Buffers                ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a new frame buffer
    *
    * @param inFrameBufferConfig  info used to create the frame buffer
    */
    virtual IFrameBuffer* CreateFrameBuffer(const FFrameBufferConfig& inFrameBufferConfig) = 0;

    /**
    * Releases/Destroys the frame buffer passed in
    *
    * @param inFrameBuffer the frame buffer to free
    */
    virtual void Free(IFrameBuffer* inFrameBuffer) = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                   Render pass                 ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a new render pass
    *
    * @param inRenderPassConfig  info used to create the render pass
    */
    virtual IRenderPass* CreateRenderPass(const FRenderPassConfig& inRenderPassConfig) = 0;

    /**
    * Releases/Destroys the renderpass passed in
    *
    * @param inRenderPass the render pass to free
    */
    virtual void Free(IRenderPass* inRenderPass) = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                 Pipeline Layout               ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a pipeline layout
    *
    * @param inPipelineLayoutConfig info used to create the render pass
    */
    virtual PipelineLayout* CreatePipelineLayout(const FPipelineLayoutConfig& inPipelineLayoutConfig) = 0;

    /**
    * Releases/Destroys the pipeline layout passed in
    *
    * @param inPipelineLayout the pipeline layout to free
    */
    virtual void Free(PipelineLayout* inPipelineLayout) = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                    Pipeline                   ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a new graphics pipeline with the specified configurations
    *
    * @param inGraphicsPipelineConfig info used to create the graphics pipeline
    */
    virtual IPipeline* CreatePipeline(const FGraphicsPipelineConfig& inGraphicsPipelineConfig) = 0;

    /**
    * Releases/Destroys the pipeline passed in
    *
    * @param inPipeline the pipeline to free
    */
    virtual void Free(IPipeline* inPipeline) = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                   Semaphores                  ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a new semaphore object
    *
    * @param inSemaphoreConfig contains info on how to create the semaphore
    */
    virtual ISemaphore* CreateRenderSemaphore(const FSemaphoreConfig& inSemaphoreConfig) = 0;

    /**
    * Releases/Destroys the semaphore passed in
    *
    * @param inSemaphore the semaphore to free
    */
    virtual void Free(ISemaphore* inSemaphore) = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                     Fences                    ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a new Fence object
    */
    virtual IFence* CreateFence() = 0;

    /**
    * Releases/Destroys the fence passed in
    *
    * @param inFence the fence to free
    */
    virtual void Free(IFence* inFence) = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                     Shaders                   ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
  * Creates a new shader with the specified configurations
  *
  * @param inShaderConfig info used to create the shader
  */
    virtual Shader* CreateShader(const FShaderConfig& inShaderConfig) = 0;

    /**
    * Releases/Destroys the shader passed in
    *
    * @param inShader the shader to free
    */
    virtual void Free(Shader* inShader) = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                     Samplers                  ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
     * Creates a new sampler with the specified configurations
    *
     * @param inSamplerConfig info used to create the sampler
    */
    virtual Sampler* CreateSampler(const FSamplerConfig& inSamplerConfig) = 0;

    /**
    * Releases/Destroys the sampler passed in
    *
    * @param inSampler the sampler to free
    */
    virtual void Free(Sampler* inSampler) = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                 Descriptor Sets               ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
     * Creates a new descriptor set with the specified configurations
    *
     * @param inDescriptorSetConfig info used to create the descriptor set 
    */
    virtual IDescriptorSets* CreateDescriptorSet(FDescriptorSetsConfig& inDescriptorSetConfig) = 0;

    /**
    * Releases/Destroys the descriptor set(s) passed in
    *
    * @param inDescriptorSets the descriptor set(s) to free
    */
    virtual void Free(IDescriptorSets* inDescriptorSets) = 0;

    /* ------------------------------------------------------------------------------- */
    /* -------------                     ImGui                     ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Initializes ImGui using GLFW by default
    */
    virtual void InitImGui(SwapChain* inMainSwapChain, Surface* inSurface) = 0;

    /**
    * Starts a new frame for imgui
    */
    virtual void BeginImGuiFrame() const = 0;

    /**
    * Renders ImGui objects (Upload index/vertex data if need be)
    *
    * @param inCommandBuffer the command buffer to encode/draw to
    */
    virtual void RenderImGui(const ICommandBuffer* inCommandBuffer, uint32 inCurrentImageIndex) const = 0;

    /**
    * Starts a new frame for imgui
    */
    virtual void EndImGuiFrame() const = 0;

    /**
    * Called when window resizes but, this function is only here temporarily as all the imgui code...
    */
    virtual void OnRenderViewportResized(SwapChain* inMainSwapchain, const FExtent2D& inNewRenderViewport) = 0;

public:
    /**
    * @returns ERenderInterface the graphics API in use by this renderer
    */
    virtual ERenderInterfaceType GetRenderInterface() const = 0;

    /**
    * @returns RendererInfo& information about the renderer in use and what its using
    */
    virtual const FRendererInfo& GetRendererInfo() const = 0;

    /**
    * @returns ICommandQueue* the queue used for submission
    */
    virtual ICommandQueue* GetCommandQueue() = 0;
};
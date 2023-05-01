/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include "IRenderInterface.h"

struct FRendererConfig
{
    ERenderInterfaceType RenderInterfaceType;
    bool bEnableRenderDoc;
};

/**
* The main renderer used to renderer everything 
*/
class VRIXIC_API Renderer
{
public:
    /**
    * Returns the one and only Instance to the Renderer
    */
    static Renderer& Get()
    {
        static Renderer Instance;
        return Instance;
    }

    void Init(const FRendererConfig& inRendererConfig);

    void Shutdown();

public:

    void Render();

    /**
    * Creates a texture from the string specified 
    * @param outTextureBuffer the buffer that was created to copy data from and upload to image on gpu
    * @returns Texture* the texture that was created from string 
    */
    Texture* CreateTexture2D(const char* inTexturePath, Buffer* outTextureBuffer);

    /**
    * Should be called when the window resizes, creates new swapchain and frame buffers 
    * 
    * @param inNewRenderViewport the new window/viewport size 
    */
    bool OnRenderViewportResized(const FExtent2D& inNewRenderViewport);

private:
    void BeginFrame();
    void Present();

    void DrawEditorTools();

    void CreateVulkanRenderInterface(bool inEnableRenderDoc);

    bool OnRenderViewportResized_Vulkan(const FExtent2D& inNewRenderViewport);

private:
    /** The render interface is the object that connects us to a graphics API.. such as.. Vulkan, DirectX, etc.. */
    TPointer<IRenderInterface> RenderInterface;

    /** A pointer to the surface in use by the renderer (OS Specific) */
    Surface* SurfacePtr;

    /** The swapchain that is in use by the renderer*/
    SwapChain* SwapChainMain;

    /** The main commandbuffers that get presented to the queue that are associated with the images given from the swapchain */
    std::vector<ICommandBuffer*> CommandBuffers;

    /** Specifies the current swapchain image that command buffers will encode */
    uint32 CurrentImageIndex;

    /** Swap chain image presentation */
    ISemaphore* PresentationCompleteSemaphore;

    /** Command buffer submission and execution */
    ISemaphore* RenderCompleteSemaphore;

    /** Depth and Stencil buffering */
    Texture* DepthStencilView;

    IRenderPass* RenderPass;

    /** List of available frame buffers (same as number of swap chain images) */
    std::vector<IFrameBuffer*> FrameBuffers;

    /** */
    PipelineLayout* GraphicsPipelineLayout;

    /** Create the graphics pipeline  */
    IPipeline* GraphicsPipeline;

    Shader* VertexShader;
    Shader* FragmentShader;

    Buffer* VertexBuffer;
    Buffer* IndexBuffer;

    /** The main render viewport */
    FRenderViewport MainRenderViewport;
    FRenderScissor MainRenderScissor;

    /** Rendering Others */
    IDescriptorSets* TextureSet;

    Sampler* SamplerHandle;
    Buffer* CP2077BufferHandle;
    Texture* CP2077TextureHandle;

    Buffer* VELogoBufferHandle;
    Texture* VELogoTextureHandle;
    
    std::vector<Buffer*> TextureBuffers;
    std::vector<Texture*> Textures;
    std::vector<Sampler*> Samplers;
    std::vector<Buffer*> Buffers;
    std::vector<uint8*> BufferDatas;

    PipelineLayout* PBRPipelineLayout;
    IPipeline* PBRPipeline;
    Shader* PBRVertexShader;
    Shader* PBRFragmentShader;
};

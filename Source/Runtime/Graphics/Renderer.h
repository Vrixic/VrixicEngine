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

    void Render();

private:
    void BeginFrame();
    void Present();

    void DrawEditorTools();

    void CreateVulkanRenderInterface(bool inEnableRenderDoc);

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
};

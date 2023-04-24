/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "GameEngine.h"

#include <Runtime/Graphics/Vulkan/VulkanRenderInterface.h>

#include <Core/Application.h>
#include <Core/Platform/Windows/GLFWWindowsWindow.h>
#include <Runtime/Memory/Core/MemoryManager.h>

#include <External/imgui/Includes/imgui.h>

#define RENDER_DOC 0

VGameEngine::VGameEngine()
    : World(nullptr)
{
    // Configure the render interface
    VulkanRendererConfig RendererConfig = {};
    RendererConfig.AppInstanceInfo.ApplicationName = "Sandbox Project";
    RendererConfig.AppInstanceInfo.EngineName = "Vrixic Engine";

    RendererConfig.EnabledInstanceExtensions.push_back("VK_EXT_debug_utils");
#if RENDER_DOC
    RendererConfig.EnabledInstanceLayers.push_back("VK_LAYER_RENDERDOC_Capture");
#else
    RendererConfig.EnabledInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    // All device extensions to enable 
    const uint32 DeviceExtensionsCount = 2;
    const char* DeviceExtensions[DeviceExtensionsCount]
    {
        "VK_EXT_descriptor_indexing",
        "VK_KHR_multiview",
    };

    RendererConfig.EnabledDeviceExtensions = DeviceExtensions;
    RendererConfig.EnabledDeviceExtensionCount = DeviceExtensionsCount;

    // Select all the feature to enable 
    PhysicalDeviceFeatures EnabledFeatures = { };
    EnabledFeatures.TessellationShader = true;
    EnabledFeatures.GeometryShader = true;
    EnabledFeatures.FillModeNonSolid = true;
    EnabledFeatures.SamplerAnisotropy = true; //MSAA
    EnabledFeatures.MultiViewports = true;

    RendererConfig.EnabledDeviceFeatures = EnabledFeatures;

    RenderInterface = TPointer<IRenderInterface>((IRenderInterface**)MemoryManager::Get().MallocConstructAligned<VulkanRenderInterface>(sizeof(VulkanRenderInterface), 8, RendererConfig));

    // Create Render Resources

    // Get Vulkan Specific stuff
    VulkanRenderInterface* VkRenderInterface = (VulkanRenderInterface*)RenderInterface.Get();
    VkInstance VulkanInstance = VkRenderInterface->GetVulkanInstance();
    VulkanDevice* Device = VkRenderInterface->GetVulkanDevice();

    // Let glfw create the surface for us 
    VkSurfaceKHR SurfaceHandle;
    VK_CHECK_RESULT(glfwCreateWindowSurface(VulkanInstance, (GLFWwindow*)Application::Get()->GetWindow().GetGLFWNativeHandle(), nullptr, &SurfaceHandle), "[VulkanRenderer]: glfw failed to create a window surface..");
    SurfacePtr = new VulkanSurface(Device, VulkanInstance, SurfaceHandle);

    // Create the device 
    Device->CreateDevice((VulkanSurface*)SurfacePtr);

    // Initliaze the interface 
    RenderInterface.Get()->Initialize();

    // Create the swapchain
    SwapChainConfig SwapChainConfiguration = SwapChainConfig::CreateDefaultConfig();
    SwapChainMain = RenderInterface.Get()->CreateSwapChain(SwapChainConfiguration, SurfacePtr);

    // Create command buffers
    {
        CommandBufferConfig Config = { };
        Config.CommandQueue = RenderInterface.Get()->GetCommandQueue();
        Config.Flags = CommandBufferLevelFlags::Primary;
        Config.NumBuffersToAllocate = 1;

        CommandBuffers.resize(SwapChainMain->GetImageCount());
        for (uint32 i = 0; i < SwapChainMain->GetImageCount(); i++)
        {
            CommandBuffers[i] = RenderInterface.Get()->CreateCommandBuffer(Config);
        }

        VE_CORE_LOG_INFO("Successfully created draw command buffers...");
    }

    // Create synchronization objects (Semaphores)
    {
        SemaphoreConfig Config = { 1 };
        PresentationCompleteSemaphore = RenderInterface.Get()->CreateRenderSemaphore(Config);
        RenderCompleteSemaphore = RenderInterface.Get()->CreateRenderSemaphore(Config);
    }

    // Setting up depth and stencil buffers
    {
        TextureConfig Config = { };
        Config.Type = ETextureType::Texture2D;
        Config.Format = EPixelFormat::D32FloatS8X24UInt;
        Config.Extent = { SwapChainConfiguration.ScreenResolution.Width, SwapChainConfiguration.ScreenResolution.Height, 1 };
        Config.MipLevels = 1;
        Config.NumArrayLayers = 1;
        Config.NumSamples = 1;
        Config.BindFlags |= ResourceBindFlags::DepthStencilAttachment;

        DepthStencilView = RenderInterface.Get()->CreateTexture(Config);
        VE_CORE_LOG_INFO("Successfully created depth stencil buffers...");
    }

    // Setting up render pass
    {
        RenderPassConfig Config = { };
        Config.RenderArea = SwapChainConfiguration.ScreenResolution;
        Config.NumSamples = 1;

        // Depth Stencil Attachment
        AttachmentDescription DepthStencil = { };
        DepthStencil.Format = EPixelFormat::D32FloatS8X24UInt;
        DepthStencil.LoadOp = EAttachmentLoadOp::Clear;
        DepthStencil.StoreOp = EAttachmentStoreOp::Store;
        DepthStencil.InitialLayout = ETextureLayout::Undefined;
        DepthStencil.FinalLayout = ETextureLayout::DepthStencilAttachment;

        Config.DepthStencilAttachment = DepthStencil;

        // Color Attachment
        AttachmentDescription Color = { };
        Color.Format = SurfacePtr->GetColorFormat();
        Color.LoadOp = EAttachmentLoadOp::Clear;
        Color.StoreOp = EAttachmentStoreOp::Store;
        Color.InitialLayout = ETextureLayout::Undefined;
        Color.FinalLayout = ETextureLayout::PresentSrc;

        Config.ColorAttachments.push_back(Color);

        RenderPass = RenderInterface.Get()->CreateRenderPass(Config);
        VE_CORE_LOG_INFO("Successfully created renderpass...");
    }

    // Create Frame Buffers
    {
        FrameBuffers.resize(SwapChainMain->GetImageCount());

        FrameBufferAttachment DepthStencilAttachment = { };
        DepthStencilAttachment.Attachment = (Texture*)DepthStencilView;

        FrameBufferAttachment ColorAttachment = { };

        FrameBufferConfig Config = { };
        Config.RenderPass = RenderPass;
        Config.Resolution = SwapChainConfiguration.ScreenResolution;
        Config.Attachments.resize(2);
        Config.Attachments[1] = DepthStencilAttachment;

        for (uint32 i = 0; i < SwapChainMain->GetImageCount(); i++)
        {
            ColorAttachment.Attachment = SwapChainMain->GetTextureAt(i);

            Config.Attachments[0] = ColorAttachment;
            FrameBuffers[i] = RenderInterface.Get()->CreateFrameBuffer(Config);
        }

        VE_CORE_LOG_INFO("Successfully created framebuffers...");
    }

    // Prepare the vulkan pipeline
    {
        // Create a generic vertex shader as it is mandatory to have a vertex shader 
        ShaderConfig VertexSConfig = { };
        VertexSConfig.CompileFlags |= ShaderCompileFlags::InvertY;
        VertexSConfig.EntryPoint = "main";
        VertexSConfig.SourceCode = std::string("float4 main(float3 inVertex : POSITION) : SV_POSITION { return float4(inVertex, 1.0f); }");
        VertexSConfig.SourceType = EShaderSourceType::String;
        VertexSConfig.Type = EShaderType::Vertex;

        // Add a vertex binding
        VertexSConfig.VertexBindings.BindingNum = 0;
        VertexSConfig.VertexBindings.Stride = sizeof(float) * 3;
        VertexSConfig.VertexBindings.InputRate = EInputRate::Vertex;

        // Add a vertex attribute to the vertex binding 
        VertexInputAttribute Attribute = { };
        Attribute.BindingNum = 0;
        Attribute.Format = EPixelFormat::RGB32Float;
        Attribute.Location = 0;
        Attribute.Offset = 0;

        VertexSConfig.VertexBindings.AddVertexAttribute(Attribute);

        VertexShader = RenderInterface.Get()->CreateShader(VertexSConfig);

        // Create a fragment shader as well
        ShaderConfig FragmentSConfig = { };
        FragmentSConfig.CompileFlags |= ShaderCompileFlags::InvertY;
        FragmentSConfig.EntryPoint = "main";
        FragmentSConfig.SourceCode = std::string("float4 main(float4 inPosition : SV_POSITION) : SV_TARGET { return float4(1.0f, 0.0f, 0.0f, 1.0f); }");
        FragmentSConfig.SourceType = EShaderSourceType::String;
        FragmentSConfig.Type = EShaderType::Fragment;

        FragmentShader = RenderInterface.Get()->CreateShader(FragmentSConfig);

        // Empty Config, we dont not want to bind any push constants not, descriptors
        PipelineLayoutConfig PLConfig = { };
        GraphicsPipelineLayout = RenderInterface.Get()->CreatePipelineLayout(PLConfig);

        // Configuate the graphics pipeline 
        GraphicsPipelineConfig GPConfig = { };

        GPConfig.RenderPassPtr = RenderPass;
        GPConfig.PipelineLayoutPtr = GraphicsPipelineLayout;
        GPConfig.FragmentShader = FragmentShader;
        GPConfig.VertexShader = VertexShader;
        GPConfig.PrimitiveTopology = EPrimitiveTopology::TriangleList;

        // Viewports and scissors 
        RenderViewport Viewport = { };
        Viewport.X = 0.0f;
        Viewport.Y = 0.0f;

        Viewport.MinDepth = 0.0f;
        Viewport.MinDepth = 1.0f;

        Viewport.Width = (float)SwapChainConfiguration.ScreenResolution.Width;
        Viewport.Height = (float)SwapChainConfiguration.ScreenResolution.Height;

        GPConfig.Viewports.push_back(Viewport);

        RenderScissor Scissor = {};
        Scissor.OffsetX = 0;
        Scissor.OffsetY = 0;
        Scissor.Width = SwapChainConfiguration.ScreenResolution.Width;
        Scissor.Height = SwapChainConfiguration.ScreenResolution.Height;

        GPConfig.Scissors.push_back(Scissor);

        // Rasterization
        GPConfig.RasterizerState.bRasterizerDiscardEnabled = false;
        GPConfig.RasterizerState.PolygonMode = EPolygonMode::Fill;
        GPConfig.RasterizerState.LineWidth = 1.0f;
        GPConfig.RasterizerState.CullMode = ECullMode::None;
        GPConfig.RasterizerState.FrontFace = EFrontFace::CounterClockwise;
        GPConfig.RasterizerState.bDepthClampEnabled = false;
        GPConfig.RasterizerState.bDepthBiasEnabled = false;
        GPConfig.RasterizerState.DepthBias.Clamp = 0.0f;
        GPConfig.RasterizerState.DepthBias.ConstantFactor = 0.0f;
        GPConfig.RasterizerState.DepthBias.SlopeFactor = 0.0f;

        // Depth stencil state
        GPConfig.DepthState.bIsTestingEnabled = true;
        GPConfig.DepthState.bIsWritingEnabled = true;
        GPConfig.DepthState.CompareOp = ECompareOp::Less;

        GPConfig.StencilState.bIsTestingEnabled = false;

        // Color blend state
        GPConfig.BlendState.LogicOp = ELogicOp::Disabled;

        BlendOpConfig BOConfig = { };
        BOConfig.ColorWriteMask = 0xF;
        BOConfig.bIsBlendEnabled = false;
        BOConfig.SrcColorBlendFactor = EBlendFactor::SrcColor;
        BOConfig.DstColorBlendFactor = EBlendFactor::DstColor;
        BOConfig.ColorBlendOp = EBlendOp::Add;
        BOConfig.SrcAlphaBlendFactor = EBlendFactor::SrcAlpha;
        BOConfig.DstAlphaBlendFactor = EBlendFactor::DstAlpha;
        BOConfig.AlphaBlendOp = EBlendOp::Add;

        GPConfig.BlendState.BlendOpConfigs.push_back(BOConfig);

        GraphicsPipeline = RenderInterface.Get()->CreatePipeline(GPConfig);

        VE_CORE_LOG_INFO("Successfully created a graphics pipeline...");
    }

    // Create Vertex Buffer
    {
        BufferConfig Config = { };
        Config.UsageFlags = EBufferUsageFlags::Vertex;
        Config.MemoryFlags |= MemoryFlags::HostCached;

        uint32 Indices[3] =
        {
            0, 1, 2
        };

        struct Vertex
        {
            float Position[3];
        };

        Vertex Vertices[3]
        {
            {0.0f, 0.75f, 0.1f}, {0.75f, -0.75f, 0.1f}, {-0.75f, -0.75f, 0.1f}
        };

        Config.InitialData = Vertices;
        Config.Size = sizeof(Vertex) * 3;
        VertexBuffer = RenderInterface.Get()->CreateBuffer(Config);

        Config.InitialData = Indices;
        Config.Size = sizeof(uint32) * 3;
        Config.UsageFlags = EBufferUsageFlags::Index;
        IndexBuffer = RenderInterface.Get()->CreateBuffer(Config);
    }

    CurrentImageIndex = 0;
}

VGameEngine::~VGameEngine()
{
    Shutdown();
}

void VGameEngine::Init()
{
    // Initialize the renderer 

}

void VGameEngine::Tick()
{
    // Firstly complete that last command buffer draw commands 
    ICommandBuffer* LastCommandBuffer = CommandBuffers[CurrentImageIndex];

    // Use a fence to wait until the command buffer has finished execution before using it again
    // Start of frame we would want to wait until last frame has finished 
    RenderInterface.Get()->GetCommandQueue()->SetWaitFence(LastCommandBuffer->GetWaitFence(), UINT64_MAX);

    // Get the new image index
    SwapChainMain->AcquireNextImageIndex(PresentationCompleteSemaphore, &CurrentImageIndex);

    // The the newest command buffer we will draw to 
    ICommandBuffer* CurrentCommandBuffer = CommandBuffers[CurrentImageIndex];

    // Being encoding command to this command buffer
    CurrentCommandBuffer->Begin();

    // Begin Render pass
    RenderPassBeginInfo RPBeginInfo = { };
    RenderClearValues ClearValues[2];
    ClearValues[0].Color = { 0.0f, 0.0f, 0.2f,  1.0f };
    ClearValues[0].Depth = { 1.0f };
    ClearValues[0].Stencil = {0};

    RPBeginInfo.ClearValues = ClearValues;
    RPBeginInfo.NumClearValues = 2;

    RPBeginInfo.RenderPassPtr = RenderPass;
    RPBeginInfo.FrameBuffer = FrameBuffers[CurrentImageIndex];

    CurrentCommandBuffer->BeginRenderPass(RPBeginInfo);

    // Bind our graphics pipeline to the current command buffer 
    CurrentCommandBuffer->BindPipeline(GraphicsPipeline);

    // Bind vertex & index buffer
    CurrentCommandBuffer->SetVertexBuffer(*VertexBuffer);
    CurrentCommandBuffer->SetIndexBuffer(*IndexBuffer);

    // Draw 3 verts = triangle
    CurrentCommandBuffer->DrawIndexed(3);

    // End the render pas
    CurrentCommandBuffer->EndRenderPass();

    // Stop encoding commands to the command buffer
    CurrentCommandBuffer->End();

    // Reset our current command buffer wait fence 
    // so that when it is used next it will already be resetted 
    RenderInterface.Get()->GetCommandQueue()->ResetWaitFence(CurrentCommandBuffer->GetWaitFence());

    // Submit to the graphics queue passing a wait semaphore
    RenderInterface.Get()->GetCommandQueue()->Submit(CurrentCommandBuffer, 1, RenderCompleteSemaphore);

    // Present the current buffer to the swap chain
    // Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
    // This ensures that the image is not presented to the windowing system until all commands have been submitted
    SwapChainMain->Present(RenderInterface.Get()->GetCommandQueue(), RenderCompleteSemaphore, CurrentImageIndex);
}

void VGameEngine::DrawEditorTools()
{
    ImGui::ShowDemoWindow();
}

void VGameEngine::Shutdown()
{
    if (RenderInterface.IsValid())
    {
        // Clean up render resources
        for (uint32 i = 0; i < CommandBuffers.size(); i++)
        {
            RenderInterface.Get()->Free(CommandBuffers[i]);
        }

        for (uint32 i = 0; i < FrameBuffers.size(); i++)
        {
            RenderInterface.Get()->Free(FrameBuffers[i]);
        }

        RenderInterface.Get()->Free(DepthStencilView);

        RenderInterface.Get()->Free(VertexShader);
        RenderInterface.Get()->Free(FragmentShader);

        RenderInterface.Get()->Free(GraphicsPipelineLayout);
        RenderInterface.Get()->Free(GraphicsPipeline);

        RenderInterface.Get()->Free(RenderPass);
        delete SwapChainMain;
        delete SurfacePtr;

        RenderInterface.Get()->Free(PresentationCompleteSemaphore);
        RenderInterface.Get()->Free(RenderCompleteSemaphore);

        RenderInterface.Get()->Shutdown();
        MemoryManager::Get().Free((void**)RenderInterface.GetRaw());
        RenderInterface.Free();
    }
}

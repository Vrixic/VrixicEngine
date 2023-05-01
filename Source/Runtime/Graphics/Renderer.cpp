/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "Renderer.h"
#include <Misc/Assert.h>
#include <Misc/Defines/StringDefines.h>
#include <Runtime/Memory/Core/MemoryManager.h>

#include <Runtime/Graphics/Vulkan/VulkanRenderInterface.h>

#include <Core/Application.h>
#include <Core/Platform/Windows/GLFWWindowsWindow.h>
#include <Runtime/Memory/Core/MemoryManager.h>
#include <Runtime/Memory/ResourceManager.h>

#include <External/glfw/Includes/GLFW/glfw3.h>

void Renderer::Init(const FRendererConfig& inRendererConfig)
{
    ResourceManager::Get().Init();
    // Create the RenderInterface
    switch (inRendererConfig.RenderInterfaceType)
    {
    case ERenderInterfaceType::Direct3D12:
        VE_ASSERT(false, VE_TEXT("[Renderer]: Render Interface Type - Direct3D12 is not supported.. "));
        break;
    case ERenderInterfaceType::Vulkan:
        CreateVulkanRenderInterface(inRendererConfig.bEnableRenderDoc);
        break;
    default:
        VE_ASSERT(false, VE_TEXT("[Renderer]: Render Interface Type is not supported.. "));
        break;
    }
}

void Renderer::Shutdown()
{
    if (RenderInterface.IsValid())
    {
        RenderInterface.Get()->Free(TextureSet);
        RenderInterface.Get()->Free(SamplerHandle);
        RenderInterface.Get()->Free(CP2077TextureHandle);
        RenderInterface.Get()->Free(VELogoTextureHandle);
        //RenderInterface.Get()->Free(CP2077BufferHandle);

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

    ResourceManager::Get().Shutdown();
}

void Renderer::Render()
{
    BeginFrame();

    {
        // The the newest command buffer we will draw to 
        ICommandBuffer* CurrentCommandBuffer = CommandBuffers[CurrentImageIndex];

        // Being encoding command to this command buffer
        CurrentCommandBuffer->Begin();

        // Set the main render viewport
        CurrentCommandBuffer->SetRenderViewports(&MainRenderViewport, 1);
        CurrentCommandBuffer->SetRenderScissors(&MainRenderScissor, 1);

        // Begin Render pass
        FRenderPassBeginInfo RPBeginInfo = { };
        FRenderClearValues ClearValues[2];
        ClearValues[0].Color = { 0.0f, 0.0f, 0.2f,  1.0f };
        ClearValues[0].Depth = { 1.0f };
        ClearValues[0].Stencil = { 0 };

        RPBeginInfo.ClearValues = ClearValues;
        RPBeginInfo.NumClearValues = 2;

        RPBeginInfo.RenderPassPtr = RenderPass;
        RPBeginInfo.FrameBuffer = FrameBuffers[CurrentImageIndex];

        // Bind the descriptor set which contains the texture with combined sampler 
        FDescriptorSetsBindInfo BindInfo = { };
        BindInfo.DescriptorSets = TextureSet;
        BindInfo.NumSets = 1;
        BindInfo.PipelineBindPoint = EPipelineBindPoint::Graphics;
        BindInfo.PipelineLayoutPtr = GraphicsPipelineLayout;
        CurrentCommandBuffer->BindDescriptorSets(BindInfo);

        CurrentCommandBuffer->BeginRenderPass(RPBeginInfo);

        // Bind our graphics pipeline to the current command buffer 
        CurrentCommandBuffer->BindPipeline(GraphicsPipeline);

        // Bind vertex & index buffer
        CurrentCommandBuffer->SetVertexBuffer(*VertexBuffer);
        CurrentCommandBuffer->SetIndexBuffer(*IndexBuffer);

        // Draw 3 verts = triangle
        CurrentCommandBuffer->DrawIndexed(6);

        // End the render pas
        CurrentCommandBuffer->EndRenderPass();

        // IMGUI
        static bool bShowDemoWindow = true;
        RenderInterface.Get()->BeginImGuiFrame();

        DrawEditorTools();
        //ImGui::ShowDemoWindow(&bShowDemoWindow);

        RenderInterface.Get()->EndImGuiFrame();

        RenderInterface.Get()->RenderImGui(CurrentCommandBuffer, CurrentImageIndex);

        // Stop encoding commands to the command buffer
        CurrentCommandBuffer->End();
    }

    Present();
}

Texture* Renderer::CreateTexture2D(const char* inTexturePath, Buffer* outTextureBuffer)
{
    FTextureConfig Config = { };
    Config.BindFlags |= FResourceBindFlags::Sampled | FResourceBindFlags::DstTransfer;
    Config.Extent.Depth = 1;
    Config.MipLevels = 1;
    Config.NumArrayLayers = 1;
    Config.NumSamples = 1;
    Config.Type = ETextureType::Texture2D;
    //Config.Layout = ETextureLayout::Undefined;

    std::string TexturePath(inTexturePath);
    TextureHandle& TexHandle = ResourceManager::Get().LoadTexture(TexturePath);

    Config.Extent.Width = TexHandle.Width;
    Config.Extent.Height = TexHandle.Height;

    Config.Format = EPixelFormat::RGBA8UNorm;

    Texture* NewTextureHandle = RenderInterface.Get()->CreateTexture(Config);
    NewTextureHandle->SetPath(TexturePath);

    // Create Buffer for Image Memory
    FBufferConfig BufferConfig = { };
    BufferConfig.InitialData = TexHandle.GetMemoryHandle();
    BufferConfig.MemoryFlags |= FMemoryFlags::HostCoherent | FMemoryFlags::HostVisible;
    BufferConfig.Size = TexHandle.SizeInBytes;
    BufferConfig.UsageFlags |= FResourceBindFlags::UniformBuffer | FResourceBindFlags::SrcTransfer;;

    outTextureBuffer = RenderInterface.Get()->CreateBuffer(BufferConfig);

    // Copy Buffer Memory Into Image 
    FTextureWriteInfo TextureWriteInfo = { };
    TextureWriteInfo.BufferHandle = outTextureBuffer;
    TextureWriteInfo.Subresource.BaseArrayLayer = 0;
    TextureWriteInfo.Subresource.NumArrayLayers = 1;
    TextureWriteInfo.Subresource.BaseMipLevel = 0;
    TextureWriteInfo.Subresource.NumMipLevels = 1;

    TextureWriteInfo.Extent = { (uint32)TexHandle.Width, (uint32)TexHandle.Height, 1u };

    RenderInterface.Get()->WriteToTexture(NewTextureHandle, TextureWriteInfo);

    return NewTextureHandle;
}

bool Renderer::OnRenderViewportResized(const FExtent2D& inNewRenderViewport)
{
    switch (RenderInterface.Get()->GetRenderInterface())
    {
    case ERenderInterfaceType::Vulkan:
        return OnRenderViewportResized_Vulkan(inNewRenderViewport);
    case ERenderInterfaceType::Direct3D12:
        VE_ASSERT(false, VE_TEXT("[Renderer]: Something very wrong is happening, render interface should not be D3D12.. as its not supported.... wtf..."));
        break;
    default:
        break;
    }

    return false;
}

void Renderer::BeginFrame()
{
    // Firstly complete that last command buffer draw commands 
    ICommandBuffer* LastCommandBuffer = CommandBuffers[CurrentImageIndex];

    // Use a fence to wait until the command buffer has finished execution before using it again
    // Start of frame we would want to wait until last frame has finished 
    RenderInterface.Get()->GetCommandQueue()->SetWaitFence(LastCommandBuffer->GetWaitFence(), UINT64_MAX);

    // Get the new image index
    SwapChainMain->AcquireNextImageIndex(PresentationCompleteSemaphore, &CurrentImageIndex);
}

void Renderer::Present()
{
    ICommandBuffer* CurrentCommandBuffer = CommandBuffers[CurrentImageIndex];

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

void Renderer::DrawEditorTools()
{
    ImGui::ShowDemoWindow();
}

void Renderer::CreateVulkanRenderInterface(bool inEnableRenderDoc)
{
    // Configure Vulkan Render Interface
    FVulkanRendererConfig RendererConfig = {};
    RendererConfig.AppInstanceInfo.ApplicationName = "Sandbox Project";
    RendererConfig.AppInstanceInfo.EngineName = "Vrixic Engine";

    RendererConfig.EnabledInstanceExtensions.push_back("VK_EXT_debug_utils");

    if (inEnableRenderDoc)
    {
        RendererConfig.EnabledInstanceLayers.push_back("VK_LAYER_RENDERDOC_Capture");
    }
    else
    {
        RendererConfig.EnabledInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
    }

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
    FPhysicalDeviceFeatures EnabledFeatures = { };
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
    FSwapChainConfig SwapChainConfiguration = FSwapChainConfig::CreateDefaultConfig();
    SwapChainMain = RenderInterface.Get()->CreateSwapChain(SwapChainConfiguration, SurfacePtr);

    // Create command buffers
    {
        FCommandBufferConfig Config = { };
        Config.CommandQueue = RenderInterface.Get()->GetCommandQueue();
        Config.Flags = FCommandBufferLevelFlags::Primary;
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
        FSemaphoreConfig Config = { 1 };
        PresentationCompleteSemaphore = RenderInterface.Get()->CreateRenderSemaphore(Config);
        RenderCompleteSemaphore = RenderInterface.Get()->CreateRenderSemaphore(Config);
    }

    // Setting up depth and stencil buffers
    {
        FTextureConfig Config = { };
        Config.Type = ETextureType::Texture2D;
        Config.Format = EPixelFormat::D32FloatS8X24UInt;
        Config.Extent = { SwapChainConfiguration.ScreenResolution.Width, SwapChainConfiguration.ScreenResolution.Height, 1 };
        Config.MipLevels = 1;
        Config.NumArrayLayers = 1;
        Config.NumSamples = 1;
        Config.BindFlags |= FResourceBindFlags::DepthStencilAttachment;
        //Config.Layout = ETextureLayout::DepthStencilAttachment;

        DepthStencilView = RenderInterface.Get()->CreateTexture(Config);
        VE_CORE_LOG_INFO("Successfully created depth stencil buffers...");
    }

    // Setting up render pass
    {
        FRenderPassConfig Config = { };
        Config.RenderArea = SwapChainConfiguration.ScreenResolution;
        Config.NumSamples = 1;

        // Depth Stencil Attachment
        FAttachmentDescription DepthStencil = { };
        DepthStencil.Format = EPixelFormat::D32FloatS8X24UInt;
        DepthStencil.LoadOp = EAttachmentLoadOp::Clear;
        DepthStencil.StoreOp = EAttachmentStoreOp::Store;
        DepthStencil.InitialLayout = ETextureLayout::Undefined;
        DepthStencil.FinalLayout = ETextureLayout::DepthStencilAttachment;

        Config.DepthStencilAttachment = DepthStencil;

        // Color Attachment
        FAttachmentDescription Color = { };
        Color.Format = SurfacePtr->GetColorFormat();
        Color.LoadOp = EAttachmentLoadOp::Clear;
        Color.StoreOp = EAttachmentStoreOp::Store;
        Color.InitialLayout = ETextureLayout::Undefined;
        Color.FinalLayout = ETextureLayout::PresentSrc;

        Config.ColorAttachments.push_back(Color);

        FSubpassDependencyDescription SBDesc = { };
        SBDesc.SrcAccessMaskFlags = 0;
        SBDesc.DstAccessMaskFlags = FSubpassAssessFlags::ColorAttachmentRead | FSubpassAssessFlags::ColorAttachmentWrite;

        Config.SubpassDependencies.push_back(SBDesc);

        RenderPass = RenderInterface.Get()->CreateRenderPass(Config);
        VE_CORE_LOG_INFO("Successfully created renderpass...");
    }

    // Create Frame Buffers
    {
        FrameBuffers.resize(SwapChainMain->GetImageCount());

        FFrameBufferAttachment DepthStencilAttachment = { };
        DepthStencilAttachment.Attachment = (Texture*)DepthStencilView;

        FFrameBufferAttachment ColorAttachment = { };

        FFrameBufferConfig Config = { };
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
        FShaderConfig VertexSConfig = { };
        VertexSConfig.CompileFlags |= FShaderCompileFlags::InvertY | FShaderCompileFlags::GLSL;
        VertexSConfig.EntryPoint = "main";
        VertexSConfig.SourceCode = std::string("#version 450 \n layout(location = 0) in vec3 InVertex; layout(location = 1) in vec2 InTexCoord; layout (location = 0) out vec2 OutTexCoord; void main() { gl_Position = vec4(InVertex, 1.0f); OutTexCoord = InTexCoord; }");

        VertexSConfig.SourceType = EShaderSourceType::String;
        VertexSConfig.Type = EShaderType::Vertex;

        // Add a vertex binding
        VertexSConfig.VertexBindings.BindingNum = 0;
        VertexSConfig.VertexBindings.Stride = sizeof(float) * 5;
        VertexSConfig.VertexBindings.InputRate = EInputRate::Vertex;

        // Add a vertex attribute to the vertex binding 
        FVertexInputAttribute Attribute = { };
        Attribute.BindingNum = 0;

        // Vertex Position
        Attribute.Format = EPixelFormat::RGB32Float;
        Attribute.Location = 0;
        Attribute.Offset = 0;

        VertexSConfig.VertexBindings.AddVertexAttribute(Attribute);

        // Texture Coord
        Attribute.Format = EPixelFormat::RG32Float;
        Attribute.Location = 1;
        Attribute.Offset = sizeof(float) * 3;

        VertexSConfig.VertexBindings.AddVertexAttribute(Attribute);

        VertexShader = RenderInterface.Get()->CreateShader(VertexSConfig);

        // Create a fragment shader as well
        FShaderConfig FragmentSConfig = { };
        FragmentSConfig.CompileFlags |= FShaderCompileFlags::InvertY | FShaderCompileFlags::GLSL;
        FragmentSConfig.EntryPoint = "main";
        //FragmentSConfig.SourceCode = std::string("#version 450 \n layout(binding = 0) uniform sampler2D TextureSampler; layout(location = 0) in vec2 InTexCoord; layout(location = 0) out vec4 OutFragColor; void main() { OutFragColor = texture(TextureSampler, InTexCoord); } ");
        FragmentSConfig.SourceCode = std::string("#version 450 \n layout(binding = 0) uniform sampler2D TexSampler[2]; layout(location = 0) in vec2 InTexCoord; layout(location = 0) out vec4 OutFragColor; void main() { OutFragColor = texture(TexSampler[0], InTexCoord); } ");
        FragmentSConfig.SourceType = EShaderSourceType::String;
        FragmentSConfig.Type = EShaderType::Fragment;

        FragmentShader = RenderInterface.Get()->CreateShader(FragmentSConfig);

        // Empty Config, we dont not want to bind any push constants not, descriptors
        FPipelineLayoutConfig PLConfig = { };
        FPipelineBindingDescriptor TextureBinding = { };

        FPipelineBindingSlot Slot = { };
        Slot.Index = 0;
        Slot.SetIndex = 0;

        TextureBinding.BindingSlot = Slot;
        TextureBinding.NumResources = 2;
        TextureBinding.ResourceType = EResourceType::Texture;
        TextureBinding.BindFlags |= FResourceBindFlags::Sampled;
        TextureBinding.StageFlags = FShaderStageFlags::FragmentStage;

        PLConfig.Bindings.push_back(TextureBinding);
        GraphicsPipelineLayout = RenderInterface.Get()->CreatePipelineLayout(PLConfig);

        // Configuate the graphics pipeline 
        FGraphicsPipelineConfig GPConfig = { };

        GPConfig.RenderPassPtr = RenderPass;
        GPConfig.PipelineLayoutPtr = GraphicsPipelineLayout;
        GPConfig.FragmentShader = FragmentShader;
        GPConfig.VertexShader = VertexShader;
        GPConfig.PrimitiveTopology = EPrimitiveTopology::TriangleList;

        // Viewports and scissors 
        MainRenderViewport = { };
        MainRenderViewport.X = 0.0f;

        // Since we flipped the viewport height, we now have to move up to the screen or else
        // we will be seeing a screen that is not being renderered 
        MainRenderViewport.Y = (float)SwapChainConfiguration.ScreenResolution.Height;

        MainRenderViewport.MinDepth = 0.0f;
        MainRenderViewport.MaxDepth = 1.0f;

        MainRenderViewport.Width = (float)SwapChainConfiguration.ScreenResolution.Width;
        // Since vulkan has a coordinate system where Y points down we have to flip the frame or viewports around the center
        MainRenderViewport.Height = -(float)SwapChainConfiguration.ScreenResolution.Height;

        // Dynamic Viewports 
        //GPConfig.Viewports.push_back(MainRenderViewport);

        //FRenderScissor Scissor = {};
        MainRenderScissor.OffsetX = 0;
        MainRenderScissor.OffsetY = 0;
        MainRenderScissor.Width = SwapChainConfiguration.ScreenResolution.Width;
        MainRenderScissor.Height = SwapChainConfiguration.ScreenResolution.Height;

        //GPConfig.Scissors.push_back(Scissor);

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

        FBlendOpConfig BOConfig = { };
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
        FBufferConfig Config = { };
        Config.UsageFlags |= FResourceBindFlags::VertexBuffer;
        Config.MemoryFlags |= FMemoryFlags::HostCached;

        /*uint32 Indices[3] =
        {
            0, 1, 2
        };*/

        uint32 Indices[6] =
        {
            0, 1, 2,
            0, 2, 3
        };

        struct Vertex
        {
            float Position[3];
            float UV[2];
        };

       /* Vertex Vertices[3]
        {
            {   { 0.0f,   0.75f, 0.1f},       {0.5f, 0.0f}},
            {   { 0.75f, -0.75f, 0.1f},       {1.0f, 1.0f}},
            {   {-0.75f, -0.75f, 0.1f},       {0.0f, 1.0f}},
        };*/

        Vertex Vertices[4]
        {
            {{-1.f, -1.f, 0.01f}, {0.0f, 1.0f}},
            {{-1.f,  1.f, 0.01f}, {0.0f, 0.0f}},
            {{ 1.f,  1.f, 0.01f}, {1.0f, 0.0f}},
            {{ 1.f, -1.f, 0.01f}, {1.0f, 1.0f}}
        };

        Config.InitialData = Vertices;
        Config.Size = sizeof(Vertex) * 4;
        VertexBuffer = RenderInterface.Get()->CreateBuffer(Config);

        Config.InitialData = Indices;
        Config.Size = sizeof(uint32) * 6;
        Config.UsageFlags |= FResourceBindFlags::IndexBuffer;
        IndexBuffer = RenderInterface.Get()->CreateBuffer(Config);
    }

    // Sampler And Textures
    {
        FSamplerConfig SamplerConfig = { };
        SamplerConfig.SetDefault();
        SamplerHandle = RenderInterface.Get()->CreateSampler(SamplerConfig);

        CP2077TextureHandle = CreateTexture2D("../Assets/Textures/Cybepunk2077.jpg", CP2077BufferHandle);
        VELogoTextureHandle = CreateTexture2D("../Assets/Textures/VrixicEngineLogo.png", VELogoBufferHandle);
    }

    // Descriptor Sets
    {
        FDescriptorSetsConfig Config = { };
        Config.NumSets = 1;
        Config.PipelineLayoutPtr = GraphicsPipelineLayout;
        TextureSet = RenderInterface.Get()->CreateDescriptorSet(Config);

        // Link to textue
        FDescriptorSetsLinkInfo LinkInfo = { };
        LinkInfo.ArrayElementStart = 0;
        LinkInfo.BindingStart = 0;
        LinkInfo.DescriptorCount = 1;
        LinkInfo.TextureSampler = SamplerHandle;
        LinkInfo.ResourceHandle.TextureHandle = CP2077TextureHandle;

        TextureSet->LinkToTexture(0, LinkInfo);

        LinkInfo.ArrayElementStart = 1;
        LinkInfo.ResourceHandle.TextureHandle = VELogoTextureHandle;
        TextureSet->LinkToTexture(0, LinkInfo);
    }

    CurrentImageIndex = 0;

    // Init Imgui 
    RenderInterface.Get()->InitImGui(SwapChainMain, SurfacePtr);
}

bool Renderer::OnRenderViewportResized_Vulkan(const FExtent2D& inNewRenderViewport)
{
    // Recreate Swapchain and Framebuffers
    bool bSuccessfullyResizedSwapchain = SwapChainMain->ResizeSwapChain(inNewRenderViewport);

    // Exit early as the swap chain was recreated no need to recreate everythign else 
    if (!bSuccessfullyResizedSwapchain)
    {
        return false;
    }

    // recreate the framebuffers
    {
        for (uint32 i = 0; i < FrameBuffers.size(); i++)
        {
            RenderInterface.Get()->Free(FrameBuffers[i]);
        }
        FrameBuffers.resize(SwapChainMain->GetImageCount());

        // resetup up depth and stencil buffers
        {
            RenderInterface.Get()->Free(DepthStencilView);

            FTextureConfig Config = { };
            Config.Type = ETextureType::Texture2D;
            Config.Format = EPixelFormat::D32FloatS8X24UInt;
            Config.Extent = { SwapChainMain->GetScreenWidth(), SwapChainMain->GetScreenHeight(), 1 };
            Config.MipLevels = 1;
            Config.NumArrayLayers = 1;
            Config.NumSamples = 1;
            Config.BindFlags |= FResourceBindFlags::DepthStencilAttachment;
            //Config.Layout = ETextureLayout::DepthStencilAttachment;

            DepthStencilView = RenderInterface.Get()->CreateTexture(Config);
        }

        FFrameBufferAttachment DepthStencilAttachment = { };
        DepthStencilAttachment.Attachment = (Texture*)DepthStencilView;

        FFrameBufferAttachment ColorAttachment = { };

        FFrameBufferConfig Config = { };
        Config.RenderPass = RenderPass;
        Config.Resolution.Width = SwapChainMain->GetScreenWidth();
        Config.Resolution.Height = SwapChainMain->GetScreenHeight();
        Config.Attachments.resize(2);
        Config.Attachments[1] = DepthStencilAttachment;

        for (uint32 i = 0; i < SwapChainMain->GetImageCount(); i++)
        {
            ColorAttachment.Attachment = SwapChainMain->GetTextureAt(i);

            Config.Attachments[0] = ColorAttachment;
            FrameBuffers[i] = RenderInterface.Get()->CreateFrameBuffer(Config);
        }
    }

    // Update render area
    FRect2D NewRenderArea = { };
    NewRenderArea.Width = inNewRenderViewport.Width;
    NewRenderArea.Height = inNewRenderViewport.Height;
    RenderPass->UpdateRenderArea(NewRenderArea);

    // Since we flipped the viewport height, we now have to move up to the screen or else
    // we will be seeing a screen that is not being renderered 
    MainRenderViewport.Y = (float)NewRenderArea.Height;

    MainRenderViewport.Width = (float)NewRenderArea.Width;
    // Since vulkan has a coordinate system where Y points down we have to flip the frame or viewports around the center
    MainRenderViewport.Height = -(float)NewRenderArea.Height;

    MainRenderScissor.Width = NewRenderArea.Width;
    MainRenderScissor.Height = NewRenderArea.Height;

    // Command buffers need to be recreated as they may store references to the recreated frame buffer
    {
        for (uint32 i = 0; i < CommandBuffers.size(); i++)
        {
            RenderInterface.Get()->Free(CommandBuffers[i]);
        }

        FCommandBufferConfig Config = { };
        Config.CommandQueue = RenderInterface.Get()->GetCommandQueue();
        Config.Flags = FCommandBufferLevelFlags::Primary;
        Config.NumBuffersToAllocate = 1;

        CommandBuffers.resize(SwapChainMain->GetImageCount());
        for (uint32 i = 0; i < SwapChainMain->GetImageCount(); i++)
        {
            CommandBuffers[i] = RenderInterface.Get()->CreateCommandBuffer(Config);
        }
    }

    // Tell the render interface that window just resized 
    RenderInterface.Get()->OnRenderViewportResized(SwapChainMain, inNewRenderViewport);

    return true;
}

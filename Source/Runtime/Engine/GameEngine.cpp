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
    VkInstance VulkanInstance =  VkRenderInterface->GetVulkanInstance();
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
        Config.Format = EPixelFormat::D24UNormS8UInt;
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
        DepthStencil.Format = EPixelFormat::D24UNormS8UInt;
        DepthStencil.LoadOp = EAttachmentLoadOp::Clear;
        DepthStencil.StoreOp = EAttachmentStoreOp::Store;

        Config.DepthStencilAttachment = DepthStencil;

        // Color Attachment
        AttachmentDescription Color = { };
        Color.Format = SurfacePtr->GetColorFormat();
        Color.LoadOp = EAttachmentLoadOp::Clear;
        Color.StoreOp = EAttachmentStoreOp::Store;

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
    // Start a new render frame 
    //Renderer.Get()->BeginRenderFrame();

    // Render the world first 
    //Renderer.Get()->Render(World);

    // Next start rendering the editor GUI
    //Renderer.Get()->BeginEditorGuiRenderFrame();

    // Draw the game editor GUI/tools
    //DrawEditorTools();

    // Stop the editor gui frame 
    //Renderer.Get()->EndEditorGuiRenderFrame();

    // End of the current render frame and submit the frame 
    //Renderer.Get()->EndRenderFrame();
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

/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Runtime/Graphics/IRenderInterface.h>
#include <Runtime/Graphics/Vulkan/VulkanPhysicalDevice.h>
#include <Runtime/Graphics/Vulkan/VulkanShader.h>
#include <Runtime/Memory/ResourceManager.h>

#include <imgui_impl_vulkan.h>

class VulkanFrameBuffer;
class VulkanMemoryHeap;
class VulkanRenderLayout;
class VulkanRenderPass;

/**
* Vulkans implementation of the render interface
*/
class VRIXIC_API VulkanRenderInterface final : public IRenderInterface
{
public:
    /**
    * Creates the vulkan interface, initializes it by creation the vulkan instance and picking the best physical device, then used to create the logical device
    *
    * @param inVulkanRendererConfig the configuration used to create and setup the renderer
    */
    VulkanRenderInterface(const FVulkanRendererConfig& inVulkanRendererConfig);

    ~VulkanRenderInterface();

    /** --  IRenderInterface Start -- */

    /**
    * Initializes the render interface
    */
    virtual void Initialize() override;

    /**
    * Shuts down this interface making it unuseable
    */
    virtual void Shutdown() override;

    /* ------------------------------------------------------------------------------- */
    /* -------------                   Swap chains                 ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a new swapchain that is renders onto the surface that is used to create the swap chain
    * -> Do not pass in a null surface, it should already be created as windows are already created before renderer initialization
    *
    * @param inSwapChainConfig the Configription used to create a swapchain
    * @param inSurface the surface that the swapchain uses and render to
    *
    * @remarks multi-swapchains not supported yet
    */
    virtual SwapChain* CreateSwapChain(const FSwapChainConfig& inSwapChainConfig, Surface* inSurface) override;

    /* ------------------------------------------------------------------------------- */
    /* -------------                 Command Buffers               ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates command buffer (s) with the specified settings as Configribed in the Configriptor
    *
    * @param inCmdBufferConfig specifies the Configription of the command buffers (if empty, it'll create one command buffer that is level Primary)
    */
    virtual ICommandBuffer* CreateCommandBuffer(const FCommandBufferConfig& inCmdBufferConfig) override;

    /**
    * Releases/Destroys the command buffer passed in
    *
    * @param inCommandBufferToFree the command buffer to free
    */
    virtual void Free(ICommandBuffer* inCommandBufferToFree) override;

    /* ------------------------------------------------------------------------------- */
    /* -------------                     Buffers                   ------------------- */
    /* ------------------------------------------------------------------------------- */

    /* ---------------------------------@TODO---------------------------------------- */
    /*
    * Create staging buffer for peak efficiency, as of now, all memory get mapped then never unmapped
    * but what if we have memory that won't change during the lifetime of application, slow for no reason.
    *
    * Affected: ReadFromBuffer, WriteToBuffer
    */
    /* ---------------------------------@TODO---------------------------------------- */

    /**
    * Creates a buffer with the specified buffer Configriptor
    *
    * @param inBufferConfig info used to create the buffer
    */
    virtual Buffer* CreateBuffer(const FBufferConfig& inBufferConfig) override;

    /**
    * Writes/Update data to the specified buffer (if data already exist, this will update)
    *
    * @param inBuffer the buffer to write to or update
    * @param inOffset a byte offset from the start of the buffer
    * @param inData pointer to the data that will be set to the buffer
    * @param inDataSize size in bytes that will be updated
    */
    virtual void WriteToBuffer(Buffer* inBuffer, uint64 inOffset, const void* inData, uint64 inDataSize) override;

    /**
    * Reads data from the buffer specified
    *
    * @param inBuffer the buffer to read the data from
    * @param inOffset a byte offset from the start of the buffer (specifys the start of the read)
    * @param outData a pointer to the data that will be set after buffer has been read
    * @param inDataSize amount of bytes to read from the buffer
    */
    virtual void ReadFromBuffer(Buffer* inBuffer, uint64 inOffset, void* outData, uint64 inDataSize) override;

    /**
    * Releases/Destroys the buffer passed in
    *
    * @param inBuffer the buffer to free
    */
    virtual void Free(Buffer* inBuffer) override;

    /* ------------------------------------------------------------------------------- */
    /* -------------                    Textures                   ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a new texture
    *
    * @param inTextureConfig  info used to create the texture
    */
    virtual Texture* CreateTexture(const FTextureConfig& inTextureConfig) override;

    /**
    * Copies the data from the buffer provided and puts it into the texture
    *
    * @param inTexture the texture that will contain the data after copy
    * @param inTextureWriteInfo contains information used to write to the texture
    */
    virtual void WriteToTexture(const Texture* inTexture, const FTextureWriteInfo& inTextureWriteInfo) override;

    /**
    * Reads image data from the specified texture
    *
    * @param inTexture the texture that will contain the data after copy
    * @param inTextureSection the section to be read from the texture
    * @param outTextureReadInfo contains the read information from the texture
    */
    virtual void ReadFromTexture(const Texture* inTexture, const FTextureSection& inTextureSection, const ETextureLayout inFinalTextureLayout, FTextureReadInfo& outTextureReadInfo) override;

    /**
    * Releases/Destroys the texture passed in
    *
    * @param inTexture the texture to free
    */
    virtual void Free(Texture* inTexture) override;

    /* ------------------------------------------------------------------------------- */
    /* -------------                  Frame Buffers                ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a new frame buffer
    *
    * @param inFrameBufferConfig  info used to create the frame buffer
    */
    virtual IFrameBuffer* CreateFrameBuffer(const FFrameBufferConfig& inFrameBufferConfig) override;

    /**
    * Releases/Destroys the frame buffer passed in
    *
    * @param inFrameBuffer the frame buffer to free
    */
    virtual void Free(IFrameBuffer* inFrameBuffer) override;

    /* ------------------------------------------------------------------------------- */
    /* -------------                   Render pass                 ------------------- */
    /* ------------------------------------------------------------------------------- */

    /* ---------------------------------@TODO---------------------------------------- */
    /*
    * Render layouting is completely 'ignored' when creating renderpasses, but can be highly useful,
    * when created many renderpasses quickly in turn needs a better usage in CreateRenderPass
    */
    /* ---------------------------------@TODO---------------------------------------- */

    /**
    * Creates a new render pass
    *
    * @param inRenderPassConfig  info used to create the render pass
    */
    virtual IRenderPass* CreateRenderPass(const FRenderPassConfig& inRenderPassConfig) override;

    /**
    * Releases/Destroys the renderpass passed in
    *
    * @param inRenderPass the render pass to free
    */
    virtual void Free(IRenderPass* inRenderPass) override;

    /* ------------------------------------------------------------------------------- */
    /* -------------                 Pipeline Layout               ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a pipeline layout
    *
    * @param inPipelineLayoutConfig info used to create the render pass
    */
    virtual PipelineLayout* CreatePipelineLayout(const FPipelineLayoutConfig& inPipelineLayoutConfig) const override;

    /**
    * Creates a pipeline layout from all the shaders passed in
    * @note: the shaders being passed in should be all the shaders the pipeline layout might be using
    *
    * @param inShaders the shaders that will be used to create the pipeline layout
    * @param inNumShaders the number of shaders being passed in 'inShaders'
    */
    virtual PipelineLayout* CreatePipelineLayoutFromShaders(const Shader** inShaders, uint8 inNumShaders) const override;

    /**
    * Releases/Destroys the pipeline layout passed in
    *
    * @param inPipelineLayout the pipeline layout to free
    */
    virtual void Free(PipelineLayout* inPipelineLayout) override;

    /* -------------                    Pipeline                   ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a new graphics pipeline with the specified configurations
    *
    * @param inGraphicsPipelineConfig info used to create the graphics pipeline
    */
    virtual IPipeline* CreatePipeline(const FGraphicsPipelineConfig& inGraphicsPipelineConfig) override;

    /**
    * Creates a new graphics pipeline with the specified configurations and with a pipeline cache
    *
    * @param inGraphicsPipelineConfig info used to create the graphics pipeline
    * @param inPipelineCachePath the path to the pipeline cache
    */
    virtual IPipeline* CreatePipelineWithCache(const FGraphicsPipelineConfig& inGraphicsPipelineConfig, const std::string& inPipelineCachePath) override;

    /**
    * Releases/Destroys the pipeline passed in
    *
    * @param inPipeline the pipeline to free
    */
    virtual void Free(IPipeline* inPipeline) override;

    /* ------------------------------------------------------------------------------- */
    /* -------------                   Semaphores                  ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a new semaphore object
    *
    * @param inSemaphoreConfig contains info on how to create the semaphore
    */
    virtual ISemaphore* CreateRenderSemaphore(const FSemaphoreConfig& inSemaphoreConfig) override;

    /**
    * Releases/Destroys the semaphore passed in
    *
    * @param inSemaphore the semaphore to free
    */
    virtual void Free(ISemaphore* inSemaphore) override;

    /* ------------------------------------------------------------------------------- */
    /* -------------                     Fences                    ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
    * Creates a new Fence object
    */
    virtual IFence* CreateFence() override;

    /**
    * Releases/Destroys the fence passed in
    *
    * @param inFence the fence to free
    */
    virtual void Free(IFence* inFence) override;

    /* ------------------------------------------------------------------------------- */
    /* -------------                     Shaders                   ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
  * Creates a new shader with the specified configurations
  *
  * @param inShaderConfig info used to create the shader
  */
    virtual Shader* CreateShader(const FShaderConfig& inShaderConfig) override;

    /**
    * Releases/Destroys the shader passed in
    *
    * @param inShader the shader to free
    */
    virtual void Free(Shader* inShader) override;

    /* ------------------------------------------------------------------------------- */
    /* -------------                     Samplers                  ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
     * Creates a new sampler with the specified configurations
    *
     * @param inSamplerConfig info used to create the sampler
    */
    virtual Sampler* CreateSampler(const FSamplerConfig& inSamplerConfig) override;

    /**
    * Releases/Destroys the sampler passed in
    *
    * @param inSampler the sampler to free
    */
    virtual void Free(Sampler* inSampler) override;

    /* ------------------------------------------------------------------------------- */
    /* -------------                 Descriptor Sets               ------------------- */
    /* ------------------------------------------------------------------------------- */

    /**
     * Creates a new descriptor set with the specified configurations
    *
     * @param inDescriptorSetConfig info used to create the descriptor set
    */
    virtual IDescriptorSets* CreateDescriptorSet(FDescriptorSetsConfig& inDescriptorSetConfig) override;

    /**
    * Releases/Destroys the descriptor set(s) passed in
    *
    * @param inDescriptorSets the descriptor set(s) to free
    */
    virtual void Free(IDescriptorSets* inDescriptorSets) override;

    /**
    * @returns bool true if the graphics card supports bindless texturing, false otherwise
    */
    virtual bool SupportsBindlessTexturing() const override;

    /* ------------------------------------------------------------------------------- */
    /* -------------                     ImGui                     ------------------- */
    /* ------------------------------------------------------------------------------- */

    static void ImguiCheckVkResultFunc(VkResult err)
    {
        VK_CHECK_RESULT(err, "[ImguiVulkanImpInit]: failed");
    }

    /**
    * Initializes ImGui using GLFW by default
    */
    virtual void InitImGui(SwapChain* inMainSwapChain, Surface* inSurface) override;

    /**
    * Starts a new frame for imgui
    */
    virtual void BeginImGuiFrame() const override;

    /**
    * Renders ImGui objects (Upload index/vertex data if need be)
    *
    * @param inCommandBuffer the command buffer to encode/draw to
    */
    virtual void RenderImGui(const ICommandBuffer* inCommandBuffer, uint32 inCurrentImageIndex) const override;

    /**
    * Starts a new frame for imgui
    */
    virtual void EndImGuiFrame() const override;

    /**
    * Called when window resizes but, this function is only here temporarily as all the imgui code...
    */
    virtual void OnRenderViewportResized(SwapChain* inMainSwapchain, const FExtent2D& inNewRenderViewport) override;

    /**
    * Shuts down ImGui and cleans up all its resources
    */
    void ShutdownImGui();

public:
    /**
    * @returns ERenderInterface the graphics API in use by this renderer
    */
    virtual ERenderInterfaceType GetRenderInterface() const override final
    {
        return ERenderInterfaceType::Vulkan;
    }

    /**
    * @returns RendererInfo& information about the renderer in use and what its using
    */
    virtual const FRendererInfo& GetRendererInfo() const override
    {
        return RendererInformation;
    }

    /**
    * @returns ICommandQueue* the queue used for submission
    */
    virtual ICommandQueue* GetCommandQueue()
    {
        return Device->GetPresentQueue();
    }

    /** --  IRenderInterface End -- */

    VkInstance GetVulkanInstance() const
    {
        return VulkanInstance;
    }

    VulkanDevice* GetVulkanDevice() const
    {
        return Device;
    }

private:
    /**
    * Creates the vulkan instance: VkInstance
    *
    * @param inVulkanRendererConfig uses some of the configurations for instance creation to create the instance
    * @returns bool true if the creation of vulkan instance was successfull, false otherwise
    */
    bool CreateVulkanInstance(const FVulkanRendererConfig& inVulkanRendererConfig);

    /**
    * Converts the passed in enabled features struct to the vulkan specific physical device features struct
    *
    * @param inFeatures the features to convert
    * @returns VkPhysicalDeviceFeatures the converted features
    */
    VkPhysicalDeviceFeatures Convert(const FPhysicalDeviceFeatures& inFeatures);

private:
    /** The vulkan instance */
    VkInstance VulkanInstance;

    /** The physical device the renderer will use (GPU) */
    VulkanPhysicalDevice* PhysicalDevice;

    /** The logical and physical device will be contained int here */
    VulkanDevice* Device;

    /** All of the supported instance extensions */
    std::vector<std::string> SupportedInstanceExtensions;

    /** All of the supported instance layers */
    std::vector<std::string> SupportedInstanceLayers;

    /** contains information about vulkan renderer */
    FRendererInfo RendererInformation;

    /** Bool to check if the graphics card supports bindless texturing */
    bool bSupportsBindlessTexturing;

    /* ---------------------------------@TODO---------------------------------------- */
    /*
    * Maybe not create the resource management inside here instead allow the client to create them and use them
    * how ever they like, and use the one the client might provide
    */
    /* ---------------------------------@TODO---------------------------------------- */

    /** Used to create shaders */
    VulkanShaderFactory* ShaderFactoryMain;

    /** Used by shader factories to allocate shader modules */
    VulkanShaderPool* ShaderPoolMain;

    /** Main memory heap for all vulkan allocation, (Index, Vertex, storage buffers, etc...) */
    VulkanMemoryHeap* VulkanMemoryHeapMain;

    /** Used when bindless is available for texture bindings */
    VkDescriptorSetLayout BindlessDescriptorSetLayout;
    class VulkanDescriptorPool* BindlessDescriptorPool;
    class VulkanDescriptorPool* GlobalDescriptorPool;

    /** - ImGui - **/

    /**
    * Helper struct that consists of all objects in use by the imgui render system
    */
    struct HImGuiData
    {
        VkAllocationCallbacks* AllocatorCallback = nullptr;
        VkInstance               Instance = VK_NULL_HANDLE;
        VkPhysicalDevice         PhysicalDevice = VK_NULL_HANDLE;
        VkDevice                 Device = VK_NULL_HANDLE;
        uint32_t                 QueueFamily = (uint32_t)-1;
        VkQueue                  Queue = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT DebugReport = VK_NULL_HANDLE;
        VkPipelineCache          PipelineCache = VK_NULL_HANDLE;
        VkDescriptorPool         DescriptorPool = VK_NULL_HANDLE;

        VulkanRenderLayout* RenderLayout = nullptr;
        VulkanRenderPass* RenderPass = nullptr;

        std::vector<VulkanFrameBuffer*> FrameBuffers;

        ImGui_ImplVulkanH_Window MainWindowData;
        int                      MinImageCount = 2;
        bool                     SwapChainRebuild = false;
    };
    static HImGuiData ImGuiData;

    /** - ImGui - **/
};

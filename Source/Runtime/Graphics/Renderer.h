/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include "IRenderInterface.h"

#include <Runtime/Core/Math/Matrix4D.h>
#include <Core/Events/MouseEvents.h>
#include <Core/Events/KeyEvent.h>

#include "ICommandBufferManager.h"

#include <Containers/Map.h>

#include <mutex>

static const uint32 UINT32InvalidIndex = 0xffffffff;
typedef uint32 TextureHandle;
static const uint32 InvalidTextureHandle = UINT32InvalidIndex;

struct FRendererConfig
{
    ERenderInterfaceType RenderInterfaceType;
    bool bEnableRenderDoc;
};

enum DebugFlags {
    DebugFlags_DisableSRGBConversion = 1 << 0,
    DebugFlags_OnlyDiffuseContribution = 1 << 1,
    DebugFlags_OnlyDiffuseLightContribution = 1 << 2,
    DebugFlags_OnlySpecularContribution = 1 << 3,
    DebugFlags_OnlySpecularLightContribution = 1 << 4,
    DebugFlags_OnlyLightContribution = 1 << 5
};

struct UniformBufferLocalConstants
{
    Matrix4D Matrix;
    Matrix4D ViewProjection;
    Vector4D Eye;
    Vector3D Light;
    uint32 DebugFlags;

    Vector3D LightPositions[4];
    Vector3D LightColors[4];

    static uint32 GetStaticSize()
    {
        return (2 * sizeof(Matrix4D)) + (sizeof(Vector4D) * 2) + (sizeof(Vector3D) * 8u);
    }
};

struct HDRBufferData
{
    int32 Face;
};

struct IBLData
{
    float Roughness;
};

enum MaterialFeatures {
    MaterialFeatures_ColorTexture = 1 << 0,
    MaterialFeatures_NormalTexture = 1 << 1,
    MaterialFeatures_RoughnessTexture = 1 << 2,
    MaterialFeatures_OcclusionTexture = 1 << 3,
    MaterialFeatures_EmissiveTexture = 1 << 4,

    MaterialFeatures_TangentVertexAttribute = 1 << 5,
    MaterialFeatures_TexcoordVertexAttribute = 1 << 6,
};

class CStaticMesh;

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

    const TPointer<IRenderInterface>& GetRenderInterface() const
    {
        return RenderInterface;
    }

public:
    void RenderStaticMesh(ICommandBuffer* CurrentCommandBuffer, CStaticMesh* inStaticMesh);
    void Render();

    /**
    * Creates a texture from the string specified
    * @param outTextureBuffer the buffer that was created to copy data from and upload to image on gpu
    * @returns Texture* the texture that was created from string
    */
    TextureHandle CreateTexture2D(const std::string& inTexturePath, Buffer*& outTextureBuffer, EPixelFormat inFormat = EPixelFormat::RGBA8UNorm);
    TextureHandle CreateTexture2DKtx(const std::string& inTexturePath, Buffer*& outTextureBuffer);

    TextureHandle CreateTextureCubemap(const std::string& inTexturePath, Buffer*& outTextureBuffer, EPixelFormat inFormat = EPixelFormat::RGBA8UNorm);
    TextureHandle CreateTextureCubemapKtx(const std::string& inTexturePath, Buffer*& outTextureBuffer);

    /**
    * Loads a shader to be used by pipeline
    */
    Shader* LoadShader(FShaderConfig& inShaderConfig);

    /**
    * Should be called when the window resizes, creates new swapchain and frame buffers
    *
    * @param inNewRenderViewport the new window/viewport size
    */
    bool OnRenderViewportResized(const FExtent2D& inNewRenderViewport);

    bool OnMouseButtonPressed(MouseButtonPressedEvent& inMouseEvent);
    bool OnMouseButtonReleased(MouseButtonReleasedEvent& inMouseEvent);
    bool OnMouseMoved(MouseMovedEvent& inMouseEvent);
    bool OnMouseScrolled(MouseScrolledEvent& inMouseEvent);

    bool OnKeyPressed(KeyPressedEvent& inKeyPressedEvent);
    bool OnKeyReleased(KeyReleasedEvent& inKeyReleasedEvent);

    /** Helper Functions */
    void CreatePBRPipeline();

    void CreateSkyboxPipeline();

    void LoadModels();

    /**
    *
    */
    void CreateSphereMeshData(float inRadius, uint32 inNumStacks, uint32 inNumSectors, std::vector<float>& outVerts, std::vector<float>& outNormals, std::vector<uint32>& outIndices, std::vector<float>& outTexCoords);
    void CreateSphereModels();

    void CreateHighDynamicImagePipeline(const char* inFilePath);
    uint32 ConvertBGRA_To_RGBA(uint32 inBGRA);
    void CreateCubemap(const char* inCubeMapName, const IPipeline* inPipeline, PipelineLayout* inPipelineLayout, IDescriptorSets* inDescriptorSet, float inViewportSize);

    void CreateIrradianceMap(class CSkybox* inSkyboxAsset, float inViewportSize);
    void CreateIrradianceMap(const char* inCubeMapName, float inViewportSize);

    void CreateCubemapFromHighDynamicImage(const char* inCubeMapName, float inViewportSize);

    void CreatePrefilterEnvMap(class CSkybox* inSkyboxAsset, float inViewportSize);
    void CreatePrefilterEnvMap(const char* inCubeMapName, float inViewportSize);

    void CreateBrdfIntegration(class CSkybox* inSkyboxAsset, float inViewportSize);
    void CreateBrdfIntegration(const char* inCubeMapName, float inViewportSize);

    PipelineLayout* CreatePipelineLayoutFromShaders(Shader* inVertexShader, Shader* inFragmentShader);

    void AddTextureToUpdate(const TextureHandle& inTextureHandle);

    void AddTextureUpdateCommands(uint32 inThreadIndex);

    TextureResource*& GetTextureResource(const TextureHandle inHandle)
    {
        return TexturesArray[inHandle];
    }

    static std::string MakePathToResource(const std::string& inResourceName, char inResourceType);

    inline SwapChain* GetSwapchain() const
    {
        return SwapChainMain;
    }

    inline uint32 GetCurrentFrame() const
    {
        return CurrentImageIndex;
    }

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
    /** Command Buffers will be created by the render interface of the CommandBufferManager for multi-threaded safety */
    //std::vector<ICommandBuffer*> CommandBuffers;

    /** Specifies the current swapchain image that command buffers will encode */
    uint32 CurrentImageIndex;

    /** Swap chain image presentation */
    ISemaphore* PresentationCompleteSemaphore;

    /** Command buffer submission and execution */
    ISemaphore* RenderCompleteSemaphore;

    /** Depth and Stencil buffering */
    TextureResource* DepthStencilView;

    IRenderPass* RenderPass;
    IRenderPass* BRDFIntegrationRenderPass;

    /** List of available frame buffers (same as number of swap chain images) */
    std::vector<IFrameBuffer*> FrameBuffers;

    /** The main render viewport */
    FRenderViewport MainRenderViewport;
    FRenderScissor MainRenderScissor;

    /** Rendering Others */
    IDescriptorSets* TextureSet;

    Sampler* SamplerHandle;
    Sampler* BRDFSamplerHandle;
    Sampler* LODSamplerHandle;
    Buffer* CP2077BufferHandle;
    TextureHandle CP2077TextureHandle;

    Buffer* VELogoBufferHandle;
    TextureHandle VELogoTextureHandle;

    std::vector<Buffer*> TextureBuffers;
    std::vector<TextureResource*> TexturesArray;
    //std::vector<Texture*> Textures;
    TMap<std::string, TextureHandle> TextureMap;

    std::vector<Sampler*> Samplers;
    std::vector<IDescriptorSets*> DescriptorSets;
    std::vector<Buffer*> Buffers;
    std::vector<uint8*> BufferDatas;

    // Includes texture mapping: normals, tangent, ....
    PipelineLayout* PBRTexturePipelineLayout;
    IPipeline* PBRTexturePipeline;
    IPipeline* PBRTexturePipelineStencil;
    IPipeline* PBRTexturePipelineOutline;
    Shader* PBRVertexShader;
    Shader* PBRTextureFragmentShader;
    Shader* PBRVertexShaderOutline;
    Shader* PBRTextureFragmentShaderOutline;

    Buffer* CubeVertexBuffer;
    Buffer* CubeVertexTexcoordBuffer;
    Buffer* CubeIndexBuffer;

    Buffer* QuadVertexBuffer;
    Buffer* QuadVertexTexcoordBuffer;

    class CSkybox* SkyboxAsset;

    PipelineLayout* HDRPipelineLayout;
    IPipeline* HDRPipeline;
    Shader* HDRVertexShader;
    Shader* HDRFragmentShader;
    Buffer* HDRTextureBuffer;
    IDescriptorSets* HDRDescSet;

    IPipeline* IrridiancePipeline;
    PipelineLayout* IrradiancePipelineLayout;
    Shader* IrridianceVertexShader;
    Shader* IrridianceFragmentShader;
    IDescriptorSets* IrridianceDescSet;
    TextureHandle IrridianceTexture;

    Buffer* SkyboxRefractBuffer;

    IPipeline* PrefilterEnvMapPipeline;
    Shader* PrefilterEnvMapVertexShader;
    Shader* PrefilterEnvMapFragmentShader;
    IDescriptorSets* PrefilterEnvMapDescSet;
    PipelineLayout* PrefilterEnvMapPipelineLayout;
    Buffer* PrefilterEnvMapBuffer;

    IPipeline* BRDFIntegrationPipeline;
    Shader* BRDFIntegrationVertexShader;
    Shader* BRDFIntegrationFragmentShader;
    IDescriptorSets* BRDFIntegrationDescSet;

    TextureHandle BRDFLutTexture;
    TextureHandle PrefilterEnvMapTexture;

    // Position | Normals -> Data stored 
    Buffer* SphereBuffer;
    Buffer* SphereIndexBuffer;
    uint32 NormalOffset; // offset from the start of the buffer to the normals 
    uint32 SphereTexCoordOffset;
    uint32 NumSphereVerts;
    uint32 NumSphereIndices;
    uint32 NumSphereNormals;

    Buffer* LocalConstantsBuffer;
    Buffer* IBLDataBuffer;
    Buffer* HDRConstantsBuffer;
    Buffer* MaterialConstantsBuffer;
    UniformBufferLocalConstants LocalConstants;

    std::vector<CStaticMesh*> StaticMeshes;
    std::vector<CStaticMesh*> OpaqueStaticMeshes;
    std::vector<CStaticMesh*> TransparentStaticMeshes;
    std::vector<CStaticMesh*> LightStaticMeshes;

    float MouseDeltaX = 0.0f;
    float MouseDeltaY = 0.0f;

    uint16 LastMouseX = 0u;
    uint16 LastMouseY = 0u;

    bool TranslationKeyDowns[4];

    float CameraMoveSpeed = 0.25f;
    const float CameraTurnSpeed = 50.0f;
    Vector3D CameraTranslation;

    Vector3D CameraRotation;

    Matrix4D ViewMatrixWorld;

    bool bIsCameraRotationControlled;
    bool bLeftButtonPressed;

    Vector3D EulerRotation;
    Vector4D LightPosition;

    Matrix4D GlobalMatrix;

    Vector3D CameraEye, CameraTarget, CameraUp;

    PipelineLayout* ImGuiTexturePipelineLayout;
    IDescriptorSets* ImGuiTextureDescSet;

    uint32 DebugFlags;

    int32 SelectedStaticMesh = -1;
    int32 SelectedMaterial = -1;

    // Bindless Texturing
    static const uint32 BINDLESS_TEXTURE_BINDING = 10;
    static const uint32 MAX_BINDLESS_TEXTURES = 1024;
    static const uint32 INVALID_TEXTURE_INDEX = -1;

    IDescriptorSets* BindlessDescriptorSet;

    TextureHandle TexturesToUpdate[128];
    uint32 NumTexturesToUpdate;
    std::mutex TextureUpdateMutex;

    // Command Buffer ready to be updated
    std::vector<ICommandBuffer*> QueuedCommandBuffers;
};

/**
* Metallic roughness workflow material data
*/
struct VRIXIC_API alignas(16) FMaterialData
{
    Vector4D BaseColorFactor;   ////////
    Matrix4D Model;             ////////
    Matrix4D ModelInv;          ////////

    Vector3D EmissiveFactor;    ////////
    float MetallicFactor;     ////////

    float RoughnessFactor;    ////////
    float OcclusionFactor;    ////////
    float AlphaMask;
    float AlphaMaskCutoff;

    uint32 AlbedoIndex;
    uint32 RoughnessIndex;
    uint32 NormalIndex;
    uint32 OcclusionIndex;

    uint32 EmissiveIndex;
    uint32 BRDFLutIndex;
    uint32 IrradianceIndex;
    uint32 PrefilterMapIndex;

    uint32 Flags;              ////////
    uint32 Padding[3];

    bool IsValid()
    {
        if (Flags & MaterialFeatures_ColorTexture && Renderer::Get().GetTextureResource(AlbedoIndex) == nullptr)
        {
            return false;
        }

        if (Flags & MaterialFeatures_NormalTexture && Renderer::Get().GetTextureResource(NormalIndex) == nullptr)
        {
            return false;
        }

        if (Flags & MaterialFeatures_RoughnessTexture && Renderer::Get().GetTextureResource(RoughnessIndex) == nullptr)
        {
            return false;
        }

        if (Flags & MaterialFeatures_OcclusionTexture && Renderer::Get().GetTextureResource(OcclusionIndex) == nullptr)
        {
            return false;
        }

        if (Flags & MaterialFeatures_EmissiveTexture && Renderer::Get().GetTextureResource(EmissiveIndex) == nullptr)
        {
            return false;
        }

        return true;
    }
};

struct VRIXIC_API FMeshDraw
{
public:
    Buffer* IndexBuffer;
    Buffer* PositionBuffer;
    Buffer* TangentBuffer;
    Buffer* NormalBuffer;
    Buffer* TexCoordBuffer;

    Buffer* MaterialBuffer;
    FMaterialData MaterialData;

    uint32 IndexOffset;
    uint32 PositionOffset;
    uint32 TangentOffset;
    uint32 NormalOffset;
    uint32 TexCoordOffset;

    uint32 Count;

    EPixelFormat IndexType;

    IDescriptorSets* DescriptorSet;
};

/**
* Render Asset Section indicates the part of a mesh to be rendered with a material to go along with it
*/
struct VRIXIC_API FRenderAssetSection
{
public:
    /** Index of the material that will be used to render this section */
    int32 MaterialIndex;

    uint32 IndexOffset;
    uint32 PositionOffset;
    uint32 TangentOffset;
    uint32 NormalOffset;
    uint32 TexCoordOffset;

    Buffer* MaterialBuffer;

    // Count of vertices or indicies to render 
    uint32 Count;
    EPixelFormat IndexType;

    IDescriptorSets* RenderAssetDescriptorSet;

public:
    FRenderAssetSection() : MaterialIndex(0), PositionOffset(0), IndexOffset(0), TangentOffset(0),
        NormalOffset(0), TexCoordOffset(0), Count(0)
    {

    }
};

struct VRIXIC_API FRenderAssetData
{
public:
    /** All Buffers Necessary for (PB) Rendering*/
    Buffer* IndexBuffer;
    Buffer* PositionBuffer;
    Buffer* TangentBuffer;
    Buffer* NormalBuffer;
    Buffer* TexCoordBuffer;

    /** Sections for this render asset */
    std::vector<FRenderAssetSection> RenderAssetSections;

public:
    FRenderAssetData() : IndexBuffer(nullptr), PositionBuffer(nullptr), TangentBuffer(nullptr),
        NormalBuffer(nullptr), TexCoordBuffer(nullptr)
    {

    }

    ~FRenderAssetData()
    {

    }
};

/**
* Base class for anything that can be rendered
*/
class VRIXIC_API CRenderAsset
{
public:
    virtual ~CRenderAsset() { }

protected:
    CRenderAsset()
    {

    }
};

class VRIXIC_API CStaticMesh : public CRenderAsset
{
public:
    CStaticMesh()
    {
        WorldTransform = Matrix4D::Identity();
        bIsTransparent = false;
    }

    virtual ~CStaticMesh()
    {

    }

    void AddNewMaterial(FMaterialData& inMaterialData, FRenderAssetSection* inRenderAssetSection = nullptr)
    {
        MaterialDatas.push_back(inMaterialData);

        // Add render asset section if it is valid and assign the material to it as well
        if (inRenderAssetSection != nullptr)
        {
            inRenderAssetSection->MaterialIndex = MaterialDatas.size();
            RenderAssetData.RenderAssetSections.push_back(*inRenderAssetSection);
        }
    }

    uint32 GetNumMaterials() const
    {
        return MaterialDatas.size();
    }

    FMaterialData& GetMaterial(uint32 inIndex)
    {
        return MaterialDatas[inIndex];
    }

    const FRenderAssetData& GetRenderAssetData() const
    {
        return RenderAssetData;
    }

    const Matrix4D& GetWorldTransform() const
    {
        return WorldTransform;
    }

    void SetIsTransparent(bool bInIsTransparent)
    {
        bIsTransparent = bInIsTransparent;
    }

    bool GetIsTransparent()
    {
        return bIsTransparent;
    }

    void SetName(std::string& inName)
    {
        Name = inName;
    }

    const std::string& GetName()
    {
        return Name;
    }

private:
    FRenderAssetData RenderAssetData;
    std::vector<FMaterialData> MaterialDatas;
    Matrix4D WorldTransform;

    std::string Name;

    bool bIsTransparent;

    friend class Renderer;
};

/**
*
*/
class VRIXIC_API CSkybox : public CRenderAsset
{
public:
    CSkybox(IRenderPass* inRenderPass, Buffer* inCubeVertexBuffer, Buffer* inLocalConstantsBuffer, const std::string& inCubemapFilePath)
    {
        RenderPass = inRenderPass;
        CubeVertexBuffer = inCubeVertexBuffer;
        LocalConstantsBuffer = inLocalConstantsBuffer;

        bIsCubemapDirty = false;

        // Create Pipeline
        {
            FGraphicsPipelineConfig GPConfig;
            {
                // Create a generic vertex shader as it is mandatory to have a vertex shader 
                FShaderConfig VSConfig = { };
                VSConfig.Flags |= FShaderFlags::GLSL;
                VSConfig.EntryPoint = "main";
                VSConfig.SourceCode = Renderer::Get().MakePathToResource("Skybox/Skybox.vert", 's');
                VSConfig.SourceType = EShaderSourceType::Filepath;
                VSConfig.Type = EShaderType::Vertex;

                // Add a vertex bindings
                VSConfig.VertexBindings.resize(1);

                // Add a vertex attribute to the vertex binding 
                FVertexInputAttribute Attribute = { };

                // Position
                VSConfig.VertexBindings[0].BindingNum = 0;
                VSConfig.VertexBindings[0].Stride = 12;
                VSConfig.VertexBindings[0].InputRate = EInputRate::Vertex;
                Attribute.Location = 0;
                Attribute.BindingNum = 0;
                Attribute.Offset = 0;
                Attribute.Format = EPixelFormat::RGB32Float;
                VSConfig.VertexBindings[0].AddVertexAttribute(Attribute);

                VertexShader = Renderer::Get().LoadShader(VSConfig);

                // Create a fragment shader as well
                FShaderConfig FragmentSConfig = { };
                FragmentSConfig.Flags |= FShaderFlags::GLSL;
                FragmentSConfig.EntryPoint = "main";
                FragmentSConfig.SourceCode = Renderer::Get().MakePathToResource("Skybox/Skybox.frag", 's');
                FragmentSConfig.SourceType = EShaderSourceType::Filepath;
                FragmentSConfig.Type = EShaderType::Fragment;

                FragmentShader = Renderer::Get().LoadShader(FragmentSConfig);
            }

            PipelineLayoutHandle = Renderer::Get().CreatePipelineLayoutFromShaders(VertexShader, FragmentShader);;

            GPConfig.RenderPassPtr = RenderPass;
            GPConfig.PipelineLayoutPtr = PipelineLayoutHandle;
            GPConfig.FragmentShader = FragmentShader;
            GPConfig.VertexShader = VertexShader;
            GPConfig.PrimitiveTopology = EPrimitiveTopology::TriangleList;

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
            GPConfig.DepthState.CompareOp = ECompareOp::LessOrEqual;

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

            Pipeline = Renderer::Get().GetRenderInterface().Get()->CreatePipelineWithCache(GPConfig, Renderer::Get().MakePathToResource("CubemapPipelineCache", 'p'));
        }

        // Create the Sampler and Texture for cubemap 
        {
            Name = inCubemapFilePath;
            CubemapTexture = Renderer::Get().CreateTextureCubemap(inCubemapFilePath.c_str(), CubemapTextureBuffer);

            FSamplerConfig SamplerConfig = { };
            SamplerConfig.SetDefault();

            SamplerConfig.AddressModeU = ESamplerAddressMode::ClampToEdge;
            SamplerConfig.AddressModeV = ESamplerAddressMode::ClampToEdge;
            SamplerConfig.AddressModeW = ESamplerAddressMode::ClampToEdge;

            CubemapSampler = Renderer::Get().GetRenderInterface().Get()->CreateSampler(SamplerConfig);
        }

        // Create and Update descriptr set 
        {
            FDescriptorSetsConfig Config = { };
            Config.NumSets = 1;
            Config.PipelineLayoutPtr = PipelineLayoutHandle;

            DescriptorSet = Renderer::Get().GetRenderInterface().Get()->CreateDescriptorSet(Config);

            FDescriptorSetsLinkInfo LinkInfo = { };
            LinkInfo.ArrayElementStart = 0;
            LinkInfo.DescriptorCount = 1;

            LinkInfo.BindingStart = 0;
            LinkInfo.ResourceHandle.BufferHandle = LocalConstantsBuffer;

            DescriptorSet->LinkToBuffer(0, LinkInfo);

            LinkInfo.BindingStart = 1;
            LinkInfo.TextureSampler = CubemapSampler;
            LinkInfo.ResourceHandle.TextureHandle = Renderer::Get().GetTextureResource(CubemapTexture);

            DescriptorSet->LinkToTexture(0, LinkInfo);
        }
    }

    virtual ~CSkybox()
    {
        Renderer::Get().GetRenderInterface().Get()->Free(PipelineLayoutHandle);
        Renderer::Get().GetRenderInterface().Get()->Free(Pipeline);
        Renderer::Get().GetRenderInterface().Get()->Free(DescriptorSet);
        Renderer::Get().GetRenderInterface().Get()->Free(CubemapSampler);
        Renderer::Get().GetRenderInterface().Get()->Free(VertexShader);
        Renderer::Get().GetRenderInterface().Get()->Free(FragmentShader);
    }

public:
    void Update()
    {
        if (bIsCubemapDirty) // if it is dirty we have to update it 
        {
            bIsCubemapDirty = false;

            FDescriptorSetsLinkInfo LinkInfo = { };
            LinkInfo.ArrayElementStart = 0;
            LinkInfo.DescriptorCount = 1;

            LinkInfo.BindingStart = 1;
            LinkInfo.TextureSampler = CubemapSampler;
            LinkInfo.ResourceHandle.TextureHandle = Renderer::Get().GetTextureResource(CubemapTexture);

            DescriptorSet->LinkToTexture(0, LinkInfo);
        }
    }

    void Render(ICommandBuffer* inCurrentCommandBuffer)
    {
        inCurrentCommandBuffer->BindPipeline(Pipeline);

        inCurrentCommandBuffer->SetVertexBuffer(*CubeVertexBuffer);

        FDescriptorSetsBindInfo DescriptorSetsBindInfo = { };
        DescriptorSetsBindInfo.DescriptorSets = DescriptorSet;
        DescriptorSetsBindInfo.NumSets = 1;
        DescriptorSetsBindInfo.PipelineBindPoint = EPipelineBindPoint::Graphics;
        DescriptorSetsBindInfo.PipelineLayoutPtr = PipelineLayoutHandle;
        inCurrentCommandBuffer->BindDescriptorSets(DescriptorSetsBindInfo);

        inCurrentCommandBuffer->Draw(36);
    }

    void UpdateCubemapTexture(const std::string& inNewCubemapTexture)
    {
        CubemapTexture = Renderer::Get().CreateTextureCubemap(inNewCubemapTexture.c_str(), CubemapTextureBuffer);
        bIsCubemapDirty = true;
    }

    void SetName(std::string& inName)
    {
        Name = inName;
    }

public:
    TextureResource* GetCubemapTexture() const
    {
        return Renderer::Get().GetTextureResource(CubemapTexture);
    }

    const std::string& GetName() const
    {
        return Name;
    }

private:
    IRenderPass* RenderPass;

    PipelineLayout* PipelineLayoutHandle;
    IPipeline* Pipeline;

    Shader* VertexShader;
    Shader* FragmentShader;
    IDescriptorSets* DescriptorSet;

    Buffer* CubeVertexBuffer;
    Buffer* LocalConstantsBuffer;

    Buffer* CubemapTextureBuffer;
    Sampler* CubemapSampler;
    TextureHandle CubemapTexture;

    std::string Name;

    bool bIsCubemapDirty;
};

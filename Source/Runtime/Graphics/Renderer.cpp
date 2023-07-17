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
#include <Core/MouseCodes.h>
#include <Core/KeyCodes.h>
#include <Runtime/Memory/Core/MemoryManager.h>
#include <Runtime/Memory/ResourceManager.h>
#include <Runtime/File/GLTFLoader.h>

#include <External/glfw/Includes/GLFW/glfw3.h>
#include <Runtime/Core/Math/Quat.h>
#include <stack>
#include <Runtime/Core/Math/ProjectionMatrix4D.h>
#include <Runtime/Core/Math/Vector2D.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <External/stb/Includes/stb_image_write.h>

#include <shobjidl.h> 

//static constexpr const char FilePathToResources[] = ""; 
static constexpr const char FilePathToResources[] = "../Assets/";
static constexpr const char FilePathToTextures[] = "../Assets/Textures/";
static constexpr const char FilePathToModels[] = "../Assets/Models/";
static constexpr const char FilePathToShaders[] = "../Assets/Shaders/";

std::string Renderer::MakePathToResource(const std::string& inResourceName, char inResourceType)
{
    switch (inResourceType)
    {
    case 't':
        return FilePathToTextures + inResourceName;
    case 'm':
        return FilePathToModels + inResourceName;
    case 's':
        return FilePathToShaders + inResourceName;
    }

    return "not_valud_resource_type.error";
}

int32 GetAttributeAccessorIndex(std::vector<GLTF::FMeshPrimitiveAttribute>& attributes, const std::string& attribute_name) {
    for (uint32 index = 0; index < attributes.size(); ++index) {
        GLTF::FMeshPrimitiveAttribute& attribute = attributes[index];
        if (strcmp(attribute.Key.data(), attribute_name.data()) == 0) {
            return attribute.AccessorIndex;
        }
    }

    return -1;
}

static uint8* GetBufferData(GLTF::FBufferView* buffer_views, uint32 buffer_index, std::vector<uint8*>& buffers_data, uint32* buffer_size = nullptr)
{
    GLTF::FBufferView& BufferView = buffer_views[buffer_index];

    int32 Byteoffset = BufferView.ByteOffset;
    if (Byteoffset == UINT32_MAX) {
        Byteoffset = 0;
    }

    if (buffer_size != nullptr) {
        *buffer_size = BufferView.ByteLength;
    }

    uint8* Data = ((uint8*)buffers_data[BufferView.BufferIndex]) + Byteoffset;
    return Data;
}

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

    //CameraTranslation.Z = -25.0f;
    ViewMatrixWorld = Matrix4D::Identity();
    ViewMatrixWorld.SetTranslation(Vector3D(0.0f, 0.0f, 0.0f));
    GlobalMatrix = Matrix4D::Identity();
    LightPosition = Vector4D(-10.0f, 10.0f, 10.0f, 1.0f);
    DebugFlags = 0;
}

void Renderer::Shutdown()
{
    if (RenderInterface.IsValid())
    {
        delete SkyboxAsset;

        RenderInterface.Get()->Free(TextureSet);
        RenderInterface.Get()->Free(SamplerHandle);
        RenderInterface.Get()->Free(BRDFSamplerHandle);
        RenderInterface.Get()->Free(LODSamplerHandle);

        for (uint32 i = 0; i < Textures.size(); ++i)
        {
            RenderInterface.Get()->Free(Textures[i]);
        }

        for (uint32 i = 0; i < TexturesArray.size(); ++i)
        {
            RenderInterface.Get()->Free(TexturesArray[i]);
        }

        for (uint32 i = 0; i < Samplers.size(); ++i)
        {
            RenderInterface.Get()->Free(Samplers[i]);
        }

        for (uint32 i = 0; i < BufferDatas.size(); ++i)
        {
            delete[] BufferDatas[i];
        }

        for (uint32 i = 0; i < StaticMeshes.size(); ++i)
        {
            delete StaticMeshes[i];
        }

        RenderInterface.Get()->Free(PBRVertexShader);
        RenderInterface.Get()->Free(PBRTexturePipelineLayout);
        RenderInterface.Get()->Free(PBRTexturePipelineStencil);
        RenderInterface.Get()->Free(PBRTexturePipelineOutline);
        RenderInterface.Get()->Free(PBRTexturePipeline);
        RenderInterface.Get()->Free(PBRTextureFragmentShader);

        RenderInterface.Get()->Free(PrefilterEnvMapPipelineLayout);
        RenderInterface.Get()->Free(PrefilterEnvMapPipeline);
        RenderInterface.Get()->Free(PrefilterEnvMapVertexShader);
        RenderInterface.Get()->Free(PrefilterEnvMapFragmentShader);
        RenderInterface.Get()->Free(PrefilterEnvMapDescSet);

        RenderInterface.Get()->Free(BRDFIntegrationPipeline);
        RenderInterface.Get()->Free(BRDFIntegrationVertexShader);
        RenderInterface.Get()->Free(BRDFIntegrationFragmentShader);
        RenderInterface.Get()->Free(BRDFIntegrationDescSet);
        RenderInterface.Get()->Free(BRDFIntegrationRenderPass);

        RenderInterface.Get()->Free(ImGuiTexturePipelineLayout);

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

        RenderInterface.Get()->Free(HDRPipelineLayout);
        RenderInterface.Get()->Free(HDRPipeline);
        RenderInterface.Get()->Free(HDRVertexShader);
        RenderInterface.Get()->Free(HDRFragmentShader);
        RenderInterface.Get()->Free(HDRDescSet);

        RenderInterface.Get()->Free(IrridiancePipeline);
        RenderInterface.Get()->Free(IrridianceVertexShader);
        RenderInterface.Get()->Free(IrridianceFragmentShader);
        RenderInterface.Get()->Free(IrridianceDescSet);

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

void Renderer::RenderStaticMesh(ICommandBuffer* inCurrentCommandBuffer, CStaticMesh* inStaticMesh)
{
    const FRenderAssetData& RenderData = inStaticMesh->GetRenderAssetData();
    for (uint32 SectionIndex = 0; SectionIndex < RenderData.RenderAssetSections.size(); ++SectionIndex)
    {
        const FRenderAssetSection& Section = RenderData.RenderAssetSections[SectionIndex];
        FMaterialData& Material = inStaticMesh->GetMaterial(SectionIndex);

        Material.ModelInv = (inStaticMesh->GetWorldTransform() * Material.Model).Inverse();

        // Update the inverse matrix 
        RenderInterface.Get()->WriteToBuffer(Section.MaterialBuffer, 0, &Material, sizeof(FMaterialData));

        inCurrentCommandBuffer->SetVertexBuffer(*RenderData.PositionBuffer, 0, 1, Section.PositionOffset);
        inCurrentCommandBuffer->SetIndexBuffer(*RenderData.IndexBuffer, Section.IndexOffset, Section.IndexType);
        inCurrentCommandBuffer->SetVertexBuffer(*RenderData.NormalBuffer, 2, 1, Section.NormalOffset);

        if (Material.Flags & MaterialFeatures_TangentVertexAttribute) {
            inCurrentCommandBuffer->SetVertexBuffer(*RenderData.TangentBuffer, 1, 1, Section.TangentOffset);
        }
        else
        {
            inCurrentCommandBuffer->SetVertexBuffer(*RenderData.NormalBuffer, 1, 1, Section.NormalOffset);
        }

        if (Material.Flags & MaterialFeatures_TexcoordVertexAttribute) {
            inCurrentCommandBuffer->SetVertexBuffer(*RenderData.TexCoordBuffer, 3, 1, Section.TexCoordOffset);
        }

        FDescriptorSetsBindInfo BindInfo = { };
        BindInfo.DescriptorSets = Section.RenderAssetDescriptorSet;
        BindInfo.NumSets = 1;
        BindInfo.PipelineBindPoint = EPipelineBindPoint::Graphics;
        BindInfo.PipelineLayoutPtr = PBRTexturePipelineLayout;
        //BindInfo.PipelineLayoutPtr = PBRPipelineLayout;
        inCurrentCommandBuffer->BindDescriptorSets(BindInfo);

        inCurrentCommandBuffer->DrawIndexed(Section.Count);
    }
}

void Renderer::Render()
{
    BeginFrame();

    SkyboxAsset->Update();

    TransparentStaticMeshes.clear();
    OpaqueStaticMeshes.clear();
    for (uint32 i = 0; i < StaticMeshes.size(); ++i)
    {
        if (StaticMeshes[i]->GetIsTransparent())
        {
            TransparentStaticMeshes.push_back(StaticMeshes[i]);
        }
        else
        {
            OpaqueStaticMeshes.push_back(StaticMeshes[i]);
        }
    }

    // IMGUI
    {
        static bool bShowDemoWindow = true;
        RenderInterface.Get()->BeginImGuiFrame();

        DrawEditorTools();

        RenderInterface.Get()->EndImGuiFrame();
    }

    if (bIsCameraRotationControlled)
    {
        CameraRotation += (Vector3D(MouseDeltaY, MouseDeltaX, 0.0f));
    }

    if (bLeftButtonPressed)
    {
        CameraTranslation += Vector3D(MouseDeltaX * 0.005f, MouseDeltaY * -0.005f, 0.0f);
    }

    //VE_CORE_LOG_DISPLAY(VE_TEXT("MouseDeltaY: {0}"), MouseDeltaY);

    Matrix4D Rotation;
    Rotation.SetIdentity();

    Rotation.Rotate(CameraRotation.X, Vector3D(1.0f, 0.0f, 0.0f));
    Rotation.Rotate(CameraRotation.Y, Vector3D(0.0f, 1.0f, 0.0f));
    //Rotation.Rotate(CameraRotation.Z, Vector3D(0.0f, 0.0f, 1.0f));

    Matrix4D Translation;
    Translation.SetIdentity();
    Translation.SetTranslation(CameraTranslation);

    ViewMatrixWorld = Translation * Rotation;

    //float CameraDeltaX = 0.0f, CameraDeltaZ = 0.0f;
    //// Update camera translation vector
    //CameraDeltaZ = bIsCameraControlled * CameraMoveSpeed * (TranslationKeyDowns[0] - TranslationKeyDowns[1]);
    //CameraDeltaX = bIsCameraControlled * CameraMoveSpeed * (TranslationKeyDowns[2] - TranslationKeyDowns[3]);
    //
    //float TotalPitch = bIsCameraControlled * (MathUtils::DegreesToRadians(65.0f) * ((float)MouseDeltaY / (float)Application::Get()->GetWindow().GetHeight())) * CameraTurnSpeed;
    //float TotalYaw = bIsCameraControlled * (MathUtils::DegreesToRadians(65.0f) * ((float)MouseDeltaX / (float)Application::Get()->GetWindow().GetWidth())) * CameraTurnSpeed;
    //
    MouseDeltaX = MouseDeltaY = 0;
    //
    //Matrix4D CameraTranslationMatrix = Matrix4D::Identity();
    //CameraTranslationMatrix.SetTranslation(Vector3D(CameraDeltaX, 0.0f, CameraDeltaZ));
    //ViewMatrixWorld = CameraTranslationMatrix * ViewMatrixWorld;
    //
    //// Rotate camera up/down orientations
    //Matrix4D PitchMatrix = Matrix4D::MakeRotX(TotalPitch);
    //ViewMatrixWorld = PitchMatrix * ViewMatrixWorld;
    //
    //// Rotate camera left/right orientations
    //Matrix4D YawMatrix = Matrix4D::Identity();
    //Vector4D CameraPosition = ViewMatrixWorld[3]; // preserve the cameras current location
    //YawMatrix = Matrix4D::MakeRotY(TotalYaw);
    //ViewMatrixWorld = ViewMatrixWorld * YawMatrix;
    //ViewMatrixWorld.SetTranslation(CameraPosition.ToVector3D()); // set the preserved location back 

    Matrix4D ViewMatrix = ViewMatrixWorld.Inverse();

    float AspectRatio = (float)Application::Get()->GetWindow().GetWidth() / (float)Application::Get()->GetWindow().GetHeight();

    // Using DirectX Left Handed Projection Matrix since we are already flipping the renderviewport by using a negative value which in turn will flip the clip space coordinates of vulkan from y down to y up
    ProjectionMatrix4D Projection = ProjectionMatrix4D::MakeProjectionVulkanLH(AspectRatio, 60.0f, 0.01f, 1000.0f, false);

    UniformBufferLocalConstants UniformData = { };
    UniformData.Eye = ViewMatrixWorld[3];
    UniformData.Matrix = GlobalMatrix;
    UniformData.ViewProjection = (ViewMatrix * (Matrix4D&)Projection);
    UniformData.Light = LightPosition.ToVector3D();
    UniformData.DebugFlags = DebugFlags;

    for (uint32 i = 0; i < 4; ++i)
    {
        UniformData.LightPositions[i] = LightStaticMeshes[i]->GetWorldTransform()[3].ToVector3D();
        UniformData.LightColors[i] = LightStaticMeshes[i]->GetMaterial(0).BaseColorFactor.ToVector3D();
    }

    // Upload to buffer
    RenderInterface.Get()->WriteToBuffer(LocalConstantsBuffer, 0, &UniformData, UniformBufferLocalConstants::GetStaticSize());

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
        ClearValues[0].Stencil = { 0u };

        RPBeginInfo.ClearValues = ClearValues;
        RPBeginInfo.NumClearValues = 2;

        RPBeginInfo.RenderPassPtr = RenderPass;
        RPBeginInfo.FrameBuffer = FrameBuffers[CurrentImageIndex];

        CurrentCommandBuffer->BeginRenderPass(RPBeginInfo);

        //PBR
        {
            // Render Meshes 
            //CurrentCommandBuffer->BindPipeline(PBRPipeline);
            CurrentCommandBuffer->BindPipeline(PBRTexturePipeline);

            for (uint32 i = 0; i < OpaqueStaticMeshes.size(); ++i)
            {
                if (SelectedStaticMesh != -1 && StaticMeshes[SelectedStaticMesh] == OpaqueStaticMeshes[i])
                {
                    continue;
                }

                CStaticMesh* MeshDraw = OpaqueStaticMeshes[i];
                RenderStaticMesh(CurrentCommandBuffer, MeshDraw);
            }
        }

        // Render skybox 
        SkyboxAsset->Render(CurrentCommandBuffer);

        // PBR Again for transparent models 
        // Blend Models 
        CurrentCommandBuffer->BindPipeline(PBRTexturePipeline);
        for (uint32 i = 0; i < TransparentStaticMeshes.size(); ++i)
        {
            if (SelectedStaticMesh != -1 && StaticMeshes[SelectedStaticMesh] == TransparentStaticMeshes[i])
            {
                continue;
            }

            CStaticMesh* MeshDraw = TransparentStaticMeshes[i];
            RenderStaticMesh(CurrentCommandBuffer, MeshDraw);
        }

        for (uint32 i = 0; i < 4; ++i)
        {
            if (SelectedStaticMesh != -1 && StaticMeshes[SelectedStaticMesh] == LightStaticMeshes[i])
            {
                continue;
            }

            CStaticMesh* MeshDraw = LightStaticMeshes[i];
            RenderStaticMesh(CurrentCommandBuffer, MeshDraw);
        }

        if (SelectedStaticMesh != -1)
        {
            CurrentCommandBuffer->BindPipeline(PBRTexturePipelineStencil);
            RenderStaticMesh(CurrentCommandBuffer, StaticMeshes[SelectedStaticMesh]);

            CurrentCommandBuffer->BindPipeline(PBRTexturePipelineOutline);
            RenderStaticMesh(CurrentCommandBuffer, StaticMeshes[SelectedStaticMesh]);
        }

        // End the render pas
        CurrentCommandBuffer->EndRenderPass();

        RenderInterface.Get()->RenderImGui(CurrentCommandBuffer, CurrentImageIndex);

        // Stop encoding commands to the command buffer
        CurrentCommandBuffer->End();
    }

    Present();
}

Texture* Renderer::CreateTexture2D(const std::string& inTexturePath, Buffer*& outTextureBuffer, EPixelFormat inFormat)
{
    std::string Extension = inTexturePath.substr(inTexturePath.length() - 4);
    if (std::strcmp(Extension.c_str(), ".ktx") == 0)
    {
        return CreateTexture2DKtx(inTexturePath, outTextureBuffer);
    }

    FTextureConfig Config = FTextureConfig();
    Config.BindFlags |= FResourceBindFlags::Sampled | FResourceBindFlags::DstTransfer;
    Config.Extent.Depth = 1;
    Config.MipLevels = 1;
    Config.NumArrayLayers = 1;
    Config.NumSamples = 1;
    Config.Type = ETextureType::Texture2D;
    //Config.Layout = ETextureLayout::Undefined;

    TextureHandle& TexHandle = ResourceManager::Get().LoadTexture(inTexturePath);

    Config.Extent.Width = TexHandle.Width;
    Config.Extent.Height = TexHandle.Height;

    Config.Format = inFormat;

    Texture* NewTextureHandle = RenderInterface.Get()->CreateTexture(Config);

    NewTextureHandle->SetPath(inTexturePath);

    // Create Buffer for Image Memory
    FBufferConfig BufferConfig = { };
    BufferConfig.InitialData = TexHandle.GetMemoryHandle();
    BufferConfig.MemoryFlags |= FMemoryFlags::HostCoherent | FMemoryFlags::HostVisible;
    BufferConfig.Size = TexHandle.SizeInBytes;
    BufferConfig.UsageFlags |= FResourceBindFlags::UniformBuffer | FResourceBindFlags::SrcTransfer;;

    outTextureBuffer = RenderInterface.Get()->CreateBuffer(BufferConfig);

    // Copy Buffer Memory Into Image 
    FTextureWriteInfo TextureWriteInfo = FTextureWriteInfo();
    TextureWriteInfo.BufferHandle = outTextureBuffer;
    TextureWriteInfo.Subresource.BaseArrayLayer = 0;
    TextureWriteInfo.Subresource.NumArrayLayers = 1;
    TextureWriteInfo.Subresource.BaseMipLevel = 0;
    TextureWriteInfo.Subresource.NumMipLevels = 1;

    TextureWriteInfo.Extent = { (uint32)TexHandle.Width, (uint32)TexHandle.Height, 1u };

    RenderInterface.Get()->WriteToTexture(NewTextureHandle, TextureWriteInfo);

    Buffers.push_back(outTextureBuffer);
    TexturesArray.push_back(NewTextureHandle);

    return NewTextureHandle;
}

Texture* Renderer::CreateTexture2DKtx(const std::string& inTexturePath, Buffer*& outTextureBuffer)
{
    ktxResult Result;
    ktxTexture* KtxTextureHandle;

    Result = ktxTexture_CreateFromNamedFile(inTexturePath.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &KtxTextureHandle);
    VE_ASSERT(Result == KTX_SUCCESS, VE_TEXT("Could not create a texture from the KTX file passed in: %s"), inTexturePath);

    uint32 TexWidth, TexHeight, TexMipLevels;

    // Get properties required for using and upload texture data from the ktx texture object
    TexWidth = KtxTextureHandle->baseWidth;
    TexHeight = KtxTextureHandle->baseHeight;
    TexMipLevels = KtxTextureHandle->numLevels;

    FVulkanTextureConfig Config = { };
    Config.BindFlags |= FResourceBindFlags::Sampled | FResourceBindFlags::DstTransfer;
    Config.CreationFlags = FResourceCreationFlags::KTX;
    Config.KtxTextureHandle = KtxTextureHandle;
    Config.Extent.Depth = 1;
    Config.MipLevels = TexMipLevels;
    Config.NumArrayLayers = KtxTextureHandle->numLayers;
    Config.NumSamples = 1;
    Config.Type = ETextureType::Texture2D;
    //Config.Layout = ETextureLayout::Undefined;

    Config.Extent.Width = TexWidth;
    Config.Extent.Height = TexHeight;

    Config.Format = (EPixelFormat)((ktxTexture2*)(KtxTextureHandle))->vkFormat;

    Texture* NewTextureHandle = RenderInterface.Get()->CreateTexture(Config);
    NewTextureHandle->SetPath(inTexturePath);

    uint8* KtxTextureData = ktxTexture_GetData(KtxTextureHandle);
    uint64 KtxTextureSize = ktxTexture_GetDataSize(KtxTextureHandle);

    // Create Buffer for Image Memory
    FBufferConfig BufferConfig = { };
    BufferConfig.InitialData = KtxTextureData;
    BufferConfig.MemoryFlags |= FMemoryFlags::HostCoherent | FMemoryFlags::HostVisible;
    BufferConfig.Size = KtxTextureSize;
    BufferConfig.UsageFlags |= FResourceBindFlags::UniformBuffer | FResourceBindFlags::SrcTransfer;;

    outTextureBuffer = RenderInterface.Get()->CreateBuffer(BufferConfig);

    // Copy Buffer Memory Into Image 
    FTextureWriteInfo TextureWriteInfo = { };
    TextureWriteInfo.BufferHandle = outTextureBuffer;
    TextureWriteInfo.Subresource.BaseArrayLayer = 0;
    TextureWriteInfo.Subresource.NumArrayLayers = KtxTextureHandle->numLayers;
    TextureWriteInfo.Subresource.BaseMipLevel = 0;
    TextureWriteInfo.Subresource.NumMipLevels = KtxTextureHandle->numLevels;

    TextureWriteInfo.Extent = { (uint32)TexWidth, (uint32)TexHeight, 1u };

    RenderInterface.Get()->WriteToTexture(NewTextureHandle, TextureWriteInfo);

    Buffers.push_back(outTextureBuffer);
    TexturesArray.push_back(NewTextureHandle);

    // Clean up staging resources
    ktxTexture_Destroy(KtxTextureHandle);

    return NewTextureHandle;
}

Texture* Renderer::CreateTextureCubemap(const std::string& inTexturePath, Buffer*& outTextureBuffer, EPixelFormat inFormat)
{
    std::string Extension = inTexturePath.substr(inTexturePath.length() - 4);
    if (std::strcmp(Extension.c_str(), ".ktx") == 0)
    {
        return CreateTextureCubemapKtx(inTexturePath, outTextureBuffer);
    }

    /**
        Cube Map Order:
        - Face +X
        - Face -X
        - Face +Y
        - Face -Y
        - Face +Z
        - Face -Z
    */
    const std::string TextureNames[] = { "PositiveX.png", "NegativeX.png", "PositiveY.png", "NegativeY.png", "PositiveZ.png", "NegativeZ.png" };

    TextureHandle CubemapTextureHandles[6];

    for (uint32 i = 0; i < 6; ++i)
    {
        CubemapTextureHandles[i] = ResourceManager::Get().LoadTexture(inTexturePath + TextureNames[i]);
    }

    const uint32 TextureWidth = CubemapTextureHandles[0].Width;
    const uint32 TextureHeight = CubemapTextureHandles[0].Height;

    FTextureConfig TextureConfig = FTextureConfig();
    TextureConfig.Type = ETextureType::TextureCube;
    TextureConfig.BindFlags |= FResourceBindFlags::Sampled | FResourceBindFlags::DstTransfer;
    TextureConfig.CreationFlags = FResourceCreationFlags::Cube;
    TextureConfig.Extent.Width = TextureWidth;
    TextureConfig.Extent.Height = TextureHeight;
    TextureConfig.Extent.Depth = 1;
    TextureConfig.Format = inFormat;
    TextureConfig.MipLevels = 1;
    TextureConfig.NumArrayLayers = 6;

    Texture* CubemapTexture = RenderInterface.Get()->CreateTexture(TextureConfig);
    CubemapTexture->SetPath(inTexturePath);

    FBufferConfig BufferConfig;
    BufferConfig.InitialData = CubemapTextureHandles[0].GetMemoryHandle();
    BufferConfig.Size = ((TextureWidth * 6) * TextureHeight) * 4;
    BufferConfig.UsageFlags = FResourceBindFlags::UniformBuffer | FResourceBindFlags::SrcTransfer;
    BufferConfig.MemoryFlags |= FMemoryFlags::HostCoherent | FMemoryFlags::HostVisible;

    outTextureBuffer = RenderInterface.Get()->CreateBuffer(BufferConfig);

    // Copy Buffer Memory Into Image 
    FTextureWriteInfo TextureWriteInfo = FTextureWriteInfo();
    TextureWriteInfo.BufferHandle = outTextureBuffer;
    TextureWriteInfo.Subresource.BaseArrayLayer = 0;
    TextureWriteInfo.Subresource.NumArrayLayers = 6;
    TextureWriteInfo.Subresource.BaseMipLevel = 0;
    TextureWriteInfo.Subresource.NumMipLevels = 1;

    TextureWriteInfo.Offset.Width = TextureWidth;
    TextureWriteInfo.Offset.Height = TextureHeight;

    TextureWriteInfo.Extent = { (uint32)TextureWidth, (uint32)TextureWidth, 1u };
    RenderInterface.Get()->WriteToTexture(CubemapTexture, TextureWriteInfo);

    TexturesArray.push_back(CubemapTexture);
    Buffers.push_back(outTextureBuffer);

    return CubemapTexture;
}

Texture* Renderer::CreateTextureCubemapKtx(const std::string& inTexturePath, Buffer*& outTextureBuffer)
{
    ktxResult Result;
    ktxTexture* KtxTextureHandle;

    Result = ktxTexture_CreateFromNamedFile(inTexturePath.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &KtxTextureHandle);
    VE_ASSERT(Result == KTX_SUCCESS, VE_TEXT("Could not create a texture from the KTX file passed in: ${0}"), inTexturePath);

    uint32 CubemapWidth, CubemapHeight, CubemapMipLevels;

    // Get properties required for using and upload texture data from the ktx texture object
    CubemapWidth = KtxTextureHandle->baseWidth;
    CubemapHeight = KtxTextureHandle->baseHeight;
    CubemapMipLevels = KtxTextureHandle->numLevels;

    uint8* ktxTextureData = ktxTexture_GetData(KtxTextureHandle);
    uint64 ktxTextureSize = ktxTexture_GetDataSize(KtxTextureHandle);

    // Create the host-visible staging buffer that contains the raw image data and also up load it 
    FBufferConfig BufferConfig;
    BufferConfig.InitialData = ktxTextureData;
    BufferConfig.Size = ktxTextureSize;
    BufferConfig.UsageFlags = FResourceBindFlags::UniformBuffer | FResourceBindFlags::SrcTransfer;
    BufferConfig.MemoryFlags |= FMemoryFlags::HostCoherent | FMemoryFlags::HostVisible;
    outTextureBuffer = RenderInterface.Get()->CreateBuffer(BufferConfig);

    // Next Create a Texture and copy the data from buffer into the Texture 
    FVulkanTextureConfig TextureConfig = { };
    TextureConfig.Type = ETextureType::TextureCube;
    TextureConfig.BindFlags |= FResourceBindFlags::Sampled | FResourceBindFlags::DstTransfer;
    TextureConfig.CreationFlags = FResourceCreationFlags::Cube | FResourceCreationFlags::KTX;
    TextureConfig.Extent.Width = CubemapWidth;
    TextureConfig.Extent.Height = CubemapHeight;
    TextureConfig.Extent.Depth = 1;
    TextureConfig.Format = (EPixelFormat)((ktxTexture2*)(KtxTextureHandle))->vkFormat;
    TextureConfig.MipLevels = CubemapMipLevels;
    TextureConfig.NumArrayLayers = 6;
    TextureConfig.KtxTextureHandle = KtxTextureHandle;
    Texture* NewTexture = RenderInterface.Get()->CreateTexture(TextureConfig);
    NewTexture->SetPath(inTexturePath);

    // Copy Buffer Memory Into Image 
    FTextureWriteInfo TextureWriteInfo = { };
    TextureWriteInfo.BufferHandle = outTextureBuffer;
    TextureWriteInfo.Subresource.BaseArrayLayer = 0;
    TextureWriteInfo.Subresource.NumArrayLayers = 6;
    TextureWriteInfo.Subresource.BaseMipLevel = 0;
    TextureWriteInfo.Subresource.NumMipLevels = CubemapMipLevels;

    TextureWriteInfo.Offset.Width = 0;
    TextureWriteInfo.Offset.Height = 0;

    TextureWriteInfo.Extent = { (uint32)CubemapWidth, (uint32)CubemapHeight, 1u };
    RenderInterface.Get()->WriteToTexture(NewTexture, TextureWriteInfo);

    TexturesArray.push_back(NewTexture);
    Buffers.push_back(outTextureBuffer);

    // Clean up staging resources
    ktxTexture_Destroy(KtxTextureHandle);

    VE_CORE_LOG_INFO(VE_TEXT("[Renderer]: Created cubemap '{0}' with {1} width, {2} height. NumMips: {3}"), inTexturePath, CubemapWidth, CubemapHeight, CubemapMipLevels);

    return NewTexture;
}

Shader* Renderer::LoadShader(FShaderConfig& inShaderConfig)
{
    if (inShaderConfig.SourceType != EShaderSourceType::String)
    {
        std::string ShaderSource;

        FileHelper::LoadFileToString(ShaderSource, inShaderConfig.SourceCode);

        for (uint32 i = ShaderSource.length() - 1; i >= 0; --i)
        {
            if (ShaderSource[i] != '\0')
            {
                break;
            }
            ShaderSource.pop_back();
        }

        inShaderConfig.SourceCode = ShaderSource;
        inShaderConfig.SourceType = EShaderSourceType::String;
    }

    return RenderInterface.Get()->CreateShader(inShaderConfig);
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

bool Renderer::OnMouseButtonPressed(MouseButtonPressedEvent& inMouseEvent)
{
    if (inMouseEvent.GetMouseButton() == Mouse::ButtonRight)
    {
        bIsCameraRotationControlled = true;
        return true;
    }

    if (inMouseEvent.GetMouseButton() == Mouse::ButtonLeft)
    {
        bLeftButtonPressed = true;
        return true;
    }

    return false;
}

bool Renderer::OnMouseButtonReleased(MouseButtonReleasedEvent& inMouseEvent)
{
    if (inMouseEvent.GetMouseButton() == Mouse::ButtonRight)
    {
        bIsCameraRotationControlled = false;
        return true;
    }

    if (inMouseEvent.GetMouseButton() == Mouse::ButtonLeft)
    {
        bLeftButtonPressed = false;
        return true;
    }

    return false;
}

bool Renderer::OnMouseMoved(MouseMovedEvent& inMouseEvent)
{
    ImGuiIO IO = ImGui::GetIO();
    if (IO.WantCaptureMouse)
    {
        return true;
    }

    // Update camera yaw and pitch values
    MouseDeltaX = (inMouseEvent.GetMouseX() - LastMouseX);
    MouseDeltaY = (inMouseEvent.GetMouseY() - LastMouseY);

    LastMouseX = inMouseEvent.GetMouseX();
    LastMouseY = inMouseEvent.GetMouseY();

    return true;
}

bool Renderer::OnMouseScrolled(MouseScrolledEvent& inMouseEvent)
{
    if (inMouseEvent.GetOffsetY() > 0)
    {
        CameraMoveSpeed += 0.01f;
        CameraTranslation.Z += 0.25f;
    }
    else
    {
        CameraMoveSpeed -= 0.01f;
        CameraTranslation.Z -= 0.25f;
    }

    CameraMoveSpeed = MathUtils::Clamp(0.0f, 0.5f, CameraMoveSpeed);

    return false;
}

bool Renderer::OnKeyPressed(KeyPressedEvent& inKeyPressedEvent)
{
    // check if the camera is being controlled right now
    switch (inKeyPressedEvent.GetKeyCode())
    {
    case Key::W:
        TranslationKeyDowns[0] = true;
        break;
    case Key::S:
        TranslationKeyDowns[1] = true;
        break;
    case Key::D:
        TranslationKeyDowns[2] = true;
        break;
    case Key::A:
        TranslationKeyDowns[3] = true;
        break;
    }

    return false;
}

bool Renderer::OnKeyReleased(KeyReleasedEvent& inKeyReleasedEvent)
{
    // check if the camera is being controlled right now
    switch (inKeyReleasedEvent.GetKeyCode())
    {
    case Key::W:
        TranslationKeyDowns[0] = false;
        break;
    case Key::S:
        TranslationKeyDowns[1] = false;
        break;
    case Key::D:
        TranslationKeyDowns[2] = false;
        break;
    case Key::A:
        TranslationKeyDowns[3] = false;
        break;
    }
    return false;
}

void Renderer::CreatePBRPipeline()
{
    {
        {
            FPipelineLayoutConfig Config = { };
            FPipelineBindingDescriptor Desc = { };
            FPipelineBindingSlot Slot = { };

            Slot.Index = 0;
            Slot.SetIndex = 0;

            // UNIFORM Buffer for Local Constants 
            Desc.BindingSlot = Slot;
            Desc.ResourceType = EResourceType::Buffer;
            Desc.BindFlags |= FResourceBindFlags::UniformBuffer;
            Desc.NumResources = 1;
            Desc.StageFlags = FShaderStageFlags::DefaultStages;

            Config.Bindings.push_back(Desc);

            Desc.BindingSlot.Index = 1;
            Config.Bindings.push_back(Desc); // Another for MaterialConstants 

            // Diffuse Texture 
            Desc.BindingSlot.Index = 2;
            Desc.NumResources = 1;
            Desc.ResourceType = EResourceType::Texture;
            Desc.BindFlags = 0;
            Desc.BindFlags |= FResourceBindFlags::Sampled;
            Config.Bindings.push_back(Desc);

            Desc.BindingSlot.Index = 3;
            Config.Bindings.push_back(Desc); // Roughness Metallness Texture

            Desc.BindingSlot.Index = 4;
            Config.Bindings.push_back(Desc); // Emissive Texture

            Desc.BindingSlot.Index = 5;
            Config.Bindings.push_back(Desc); // Occlusion Texture

            Desc.BindingSlot.Index = 6;
            Config.Bindings.push_back(Desc); // Normal Texture

            Desc.BindingSlot.Index = 7;
            Config.Bindings.push_back(Desc); // Irridiance Map Texture

            Desc.BindingSlot.Index = 8;
            Config.Bindings.push_back(Desc); // Prefilter Env Map Texture

            Desc.BindingSlot.Index = 9;
            Config.Bindings.push_back(Desc); // BRDF LUT Texture

            PBRTexturePipelineLayout = RenderInterface.Get()->CreatePipelineLayout(Config);
        }
    }

    // Create The PBR Pipelines
    {
        FGraphicsPipelineConfig GPConfig;
        {
            // Create a generic vertex shader as it is mandatory to have a vertex shader 
            FShaderConfig VSConfig = { };
            VSConfig.CompileFlags |= FShaderCompileFlags::GLSL;
            VSConfig.EntryPoint = "main";
            VSConfig.SourceCode = MakePathToResource("PBR/pbr.vert", 's');
            VSConfig.SourceType = EShaderSourceType::Filepath;
            VSConfig.Type = EShaderType::Vertex;

            // Add a vertex bindings
            VSConfig.VertexBindings.resize(4);

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

            // Tangent 
            VSConfig.VertexBindings[1].BindingNum = 1;
            VSConfig.VertexBindings[1].Stride = 16;
            VSConfig.VertexBindings[1].InputRate = EInputRate::Vertex;
            Attribute.Location = 1;
            Attribute.BindingNum = 1;
            Attribute.Format = EPixelFormat::RGBA32Float;
            VSConfig.VertexBindings[1].AddVertexAttribute(Attribute);

            // Normal
            VSConfig.VertexBindings[2].BindingNum = 2;
            VSConfig.VertexBindings[2].Stride = 12;
            VSConfig.VertexBindings[2].InputRate = EInputRate::Vertex;
            Attribute.Location = 2;
            Attribute.BindingNum = 2;
            Attribute.Format = EPixelFormat::RGB32Float;
            VSConfig.VertexBindings[2].AddVertexAttribute(Attribute);

            // Texcoord
            VSConfig.VertexBindings[3].BindingNum = 3;
            VSConfig.VertexBindings[3].Stride = 8;
            VSConfig.VertexBindings[3].InputRate = EInputRate::Vertex;
            Attribute.Location = 3;
            Attribute.BindingNum = 3;
            Attribute.Format = EPixelFormat::RG32Float;
            VSConfig.VertexBindings[3].AddVertexAttribute(Attribute);

            PBRVertexShader = LoadShader(VSConfig);

            // Create a fragment shaders as well
            {
                FShaderConfig FragmentSConfig = { };
                FragmentSConfig.CompileFlags |= FShaderCompileFlags::GLSL;
                FragmentSConfig.EntryPoint = "main";
                FragmentSConfig.SourceCode = MakePathToResource("PBR/pbr_khr_debug.frag", 's');
                FragmentSConfig.SourceType = EShaderSourceType::Filepath;
                FragmentSConfig.Type = EShaderType::Fragment;

                PBRTextureFragmentShader = LoadShader(FragmentSConfig);
            }
        }

        {
            // Create a generic vertex shader as it is mandatory to have a vertex shader 
            FShaderConfig VSConfig = { };
            VSConfig.CompileFlags |= FShaderCompileFlags::GLSL;
            VSConfig.EntryPoint = "main";
            VSConfig.SourceCode = MakePathToResource("EdgeDetection/outline.vert", 's');
            VSConfig.SourceType = EShaderSourceType::Filepath;
            VSConfig.Type = EShaderType::Vertex;

            // Add a vertex bindings
            VSConfig.VertexBindings.resize(4);

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

            // Tangent 
            VSConfig.VertexBindings[1].BindingNum = 1;
            VSConfig.VertexBindings[1].Stride = 16;
            VSConfig.VertexBindings[1].InputRate = EInputRate::Vertex;
            Attribute.Location = 1;
            Attribute.BindingNum = 1;
            Attribute.Format = EPixelFormat::RGBA32Float;
            VSConfig.VertexBindings[1].AddVertexAttribute(Attribute);

            // Normal
            VSConfig.VertexBindings[2].BindingNum = 2;
            VSConfig.VertexBindings[2].Stride = 12;
            VSConfig.VertexBindings[2].InputRate = EInputRate::Vertex;
            Attribute.Location = 2;
            Attribute.BindingNum = 2;
            Attribute.Format = EPixelFormat::RGB32Float;
            VSConfig.VertexBindings[2].AddVertexAttribute(Attribute);

            // Texcoord
            VSConfig.VertexBindings[3].BindingNum = 3;
            VSConfig.VertexBindings[3].Stride = 8;
            VSConfig.VertexBindings[3].InputRate = EInputRate::Vertex;
            Attribute.Location = 3;
            Attribute.BindingNum = 3;
            Attribute.Format = EPixelFormat::RG32Float;
            VSConfig.VertexBindings[3].AddVertexAttribute(Attribute);

            PBRVertexShaderOutline = LoadShader(VSConfig);

            // Create a fragment shaders as well
            {
                FShaderConfig FragmentSConfig = { };
                FragmentSConfig.CompileFlags |= FShaderCompileFlags::GLSL;
                FragmentSConfig.EntryPoint = "main";
                FragmentSConfig.SourceCode = MakePathToResource("EdgeDetection/outline.frag", 's');
                FragmentSConfig.SourceType = EShaderSourceType::Filepath;
                FragmentSConfig.Type = EShaderType::Fragment;

                PBRTextureFragmentShaderOutline = LoadShader(FragmentSConfig);
            }
        }

        {
            GPConfig.RenderPassPtr = RenderPass;
            GPConfig.PipelineLayoutPtr = PBRTexturePipelineLayout;
            GPConfig.FragmentShader = PBRTextureFragmentShader;
            GPConfig.VertexShader = PBRVertexShader;
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

            // Color blend state
            GPConfig.BlendState.LogicOp = ELogicOp::Disabled;

            FBlendOpConfig BOConfig = { };
            BOConfig.ColorWriteMask = 0xF;
            BOConfig.bIsBlendEnabled = true;
            BOConfig.SrcColorBlendFactor = EBlendFactor::SrcAlpha;
            BOConfig.DstColorBlendFactor = EBlendFactor::OneMinusSrcAlpha;
            BOConfig.ColorBlendOp = EBlendOp::Add;
            BOConfig.SrcAlphaBlendFactor = EBlendFactor::OneMinusSrcAlpha;
            BOConfig.DstAlphaBlendFactor = EBlendFactor::Zero;
            BOConfig.AlphaBlendOp = EBlendOp::Add;

            GPConfig.BlendState.BlendOpConfigs.push_back(BOConfig);

            // Depth stencil state
            GPConfig.DepthState.bIsTestingEnabled = true;
            GPConfig.DepthState.bIsWritingEnabled = true;
            GPConfig.DepthState.CompareOp = ECompareOp::Less;

            PBRTexturePipeline = RenderInterface.Get()->CreatePipeline(GPConfig);

            // Depth stencil state
            //GPConfig.DepthState.bIsTestingEnabled = true;
            //GPConfig.DepthState.bIsWritingEnabled = true;
            GPConfig.DepthState.CompareOp = ECompareOp::LessOrEqual;

            GPConfig.StencilState.bIsTestingEnabled = true;
            GPConfig.StencilState.Back.CompareOp = ECompareOp::Always;
            GPConfig.StencilState.Back.StencilFailOp = EStencilOp::Replace;
            GPConfig.StencilState.Back.DepthFailOp = EStencilOp::Replace;
            GPConfig.StencilState.Back.StencilPassOp = EStencilOp::Replace;
            GPConfig.StencilState.Back.CompareMask = 0xFF;
            GPConfig.StencilState.Back.WriteMask = 0xFF;
            GPConfig.StencilState.Back.ReferenceValue = 1;
            GPConfig.StencilState.Front = GPConfig.StencilState.Back;

            PBRTexturePipelineStencil = RenderInterface.Get()->CreatePipeline(GPConfig);

            // Outline Pipeline Pass
            GPConfig.StencilState.Back.CompareOp = ECompareOp::NotEqual;
            GPConfig.StencilState.Back.StencilFailOp = EStencilOp::Keep;
            GPConfig.StencilState.Back.DepthFailOp = EStencilOp::Keep;
            GPConfig.StencilState.Back.StencilPassOp = EStencilOp::Replace;
            GPConfig.StencilState.Front = GPConfig.StencilState.Back;
            GPConfig.DepthState.bIsTestingEnabled = false;

            GPConfig.FragmentShader = PBRTextureFragmentShaderOutline;
            GPConfig.VertexShader = PBRVertexShaderOutline;

            PBRTexturePipelineOutline = RenderInterface.Get()->CreatePipeline(GPConfig);
        }
    }
}

void Renderer::CreateSkyboxPipeline()
{
    std::string HDRPath = MakePathToResource("NewportLoft.hdr", 't');
    std::wstring WHDRPath(HDRPath.begin(), HDRPath.end());

    GetFileAttributes(WHDRPath.c_str()); // from winbase.h
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(WHDRPath.c_str()) && GetLastError() == ERROR_FILE_NOT_FOUND)
    {
        VE_ASSERT(false, VE_TEXT("Could not find NewportLoftCubemap.hdr"));
    }

    CreateHighDynamicImagePipeline(MakePathToResource("NewportLoft.hdr", 't').c_str());

    std::string CubemapPath = MakePathToResource("NewportLoftCubemap.ktx", 't');
    std::wstring WCubemapPath(CubemapPath.begin(), CubemapPath.end());

    GetFileAttributes(WCubemapPath.c_str()); // from winbase.h
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(WCubemapPath.c_str()) && GetLastError() == ERROR_FILE_NOT_FOUND)
    {
        CreateCubemapFromHighDynamicImage("NewportLoftCubemap", 512);
    }

    SkyboxAsset = new CSkybox(RenderPass, CubeVertexBuffer, LocalConstantsBuffer, MakePathToResource("NewportLoftCubemap.ktx", 't'));

    // Create Vertex and Normal Data for a sphere 
    {
        {
            float Radius = 1.0f;
            uint32 NumSectors = 36;
            uint32 NumStacks = 18;

            std::vector<float> Vertices;
            std::vector<float> Normals;
            std::vector<float> TexCoords;
            std::vector<uint32> Indices;

            CreateSphereMeshData(Radius, NumStacks, NumSectors, Vertices, Normals, Indices, TexCoords);

            float* BufferData = new float[Vertices.size() + Normals.size() + TexCoords.size()];
            uint32 SizeInBytes = Vertices.size() * 4;

            memcpy(BufferData, Vertices.data(), SizeInBytes);
            memcpy(BufferData + Vertices.size(), Normals.data(), SizeInBytes);
            memcpy(BufferData + (Vertices.size() + Normals.size()), TexCoords.data(), TexCoords.size() * 4);

            FBufferConfig Config = { };
            Config.UsageFlags |= FResourceBindFlags::VertexBuffer;
            Config.MemoryFlags |= FMemoryFlags::HostCached;
            Config.InitialData = BufferData;
            Config.Size = (Vertices.size() + Normals.size() + TexCoords.size()) * 4;
            SphereBuffer = RenderInterface.Get()->CreateBuffer(Config);

            Config.UsageFlags |= FResourceBindFlags::IndexBuffer;
            Config.MemoryFlags |= FMemoryFlags::HostCached;
            Config.InitialData = Indices.data();
            Config.Size = (Indices.size()) * 4;
            SphereIndexBuffer = RenderInterface.Get()->CreateBuffer(Config);

            Buffers.push_back(SphereBuffer);
            Buffers.push_back(SphereIndexBuffer);

            NumSphereVerts = Vertices.size();
            NumSphereNormals = Normals.size();
            NumSphereIndices = Indices.size();

            SphereTexCoordOffset = (Vertices.size() + Normals.size()) * 4;
        }
    }

    // Create Irridiance pipeline 
    {
        {
            if (IrridiancePipeline == nullptr)
            {
                FGraphicsPipelineConfig GPConfig;
                {
                    // Create a generic vertex shader as it is mandatory to have a vertex shader 
                    FShaderConfig VSConfig = { };
                    VSConfig.CompileFlags |= FShaderCompileFlags::GLSL;
                    VSConfig.EntryPoint = "main";
                    VSConfig.SourceCode = MakePathToResource("Skybox/irridiance.vert", 's');
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

                    IrridianceVertexShader = LoadShader(VSConfig);

                    // Create a fragment shader as well
                    FShaderConfig FragmentSConfig = { };
                    FragmentSConfig.CompileFlags |= FShaderCompileFlags::GLSL;
                    FragmentSConfig.EntryPoint = "main";
                    FragmentSConfig.SourceCode = MakePathToResource("Skybox/irridiance.frag", 's');
                    FragmentSConfig.SourceType = EShaderSourceType::Filename;
                    FragmentSConfig.Type = EShaderType::Fragment;

                    IrridianceFragmentShader = LoadShader(FragmentSConfig);
                }

                GPConfig.RenderPassPtr = RenderPass;
                GPConfig.PipelineLayoutPtr = HDRPipelineLayout;
                GPConfig.FragmentShader = IrridianceFragmentShader;
                GPConfig.VertexShader = IrridianceVertexShader;
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

                IrridiancePipeline = RenderInterface.Get()->CreatePipeline(GPConfig);
            }
        }

        // Create Descriptor Set 
        {
            if (IrridianceDescSet == nullptr)
            {
                FDescriptorSetsConfig DescriptorSetsConfig = { };
                DescriptorSetsConfig.NumSets = 1;
                DescriptorSetsConfig.PipelineLayoutPtr = HDRPipelineLayout;
                IrridianceDescSet = RenderInterface.Get()->CreateDescriptorSet(DescriptorSetsConfig);

                FDescriptorSetsLinkInfo BufferLinkInfo = { };
                BufferLinkInfo.DescriptorCount = 1;
                BufferLinkInfo.ResourceHandle.BufferHandle = LocalConstantsBuffer;
                BufferLinkInfo.BindingStart = 0;
                BufferLinkInfo.ArrayElementStart = 0;

                IrridianceDescSet->LinkToBuffer(0, BufferLinkInfo);

                BufferLinkInfo.DescriptorCount = 1;
                BufferLinkInfo.ResourceHandle.TextureHandle = SkyboxAsset->GetCubemapTexture();
                BufferLinkInfo.BindingStart = 1;
                BufferLinkInfo.ArrayElementStart = 0;
                BufferLinkInfo.TextureSampler = SamplerHandle;
                IrridianceDescSet->LinkToTexture(0, BufferLinkInfo);
            }
        }
    }

    // Create PrefilterEnvMap pipeline 
    {
        {
            if (PrefilterEnvMapPipeline == nullptr)
            {
                {
                    FPipelineLayoutConfig Config = { };
                    FPipelineBindingDescriptor Desc = { };
                    FPipelineBindingSlot Slot = { };

                    Slot.Index = 0;
                    Slot.SetIndex = 0;

                    // UNIFORM Buffer for Local Constants 
                    Desc.BindingSlot = Slot;
                    Desc.ResourceType = EResourceType::Buffer;
                    Desc.BindFlags |= FResourceBindFlags::UniformBuffer;
                    Desc.NumResources = 1;
                    Desc.StageFlags = FShaderStageFlags::VertexStage;

                    Config.Bindings.push_back(Desc);

                    // cubemap Texture 
                    Desc.BindingSlot.Index = 1;
                    Desc.NumResources = 1;
                    Desc.ResourceType = EResourceType::Texture;
                    Desc.BindFlags = 0;
                    Desc.BindFlags |= FResourceBindFlags::Sampled;
                    Desc.StageFlags = 0;
                    Desc.StageFlags |= FShaderStageFlags::FragmentStage;
                    Config.Bindings.push_back(Desc);

                    // prefiler env map struct  
                    Desc.BindingSlot.Index = 2;
                    Desc.ResourceType = EResourceType::Buffer;
                    Desc.BindFlags = 0;
                    Desc.BindFlags |= FResourceBindFlags::UniformBuffer;
                    Desc.NumResources = 1;
                    Desc.StageFlags = 0;
                    Desc.StageFlags = FShaderStageFlags::FragmentStage;

                    Config.Bindings.push_back(Desc);

                    PrefilterEnvMapPipelineLayout = RenderInterface.Get()->CreatePipelineLayout(Config);
                }

                FGraphicsPipelineConfig GPConfig;
                {
                    // Create a generic vertex shader as it is mandatory to have a vertex shader 
                    FShaderConfig VSConfig = { };
                    VSConfig.CompileFlags |= FShaderCompileFlags::GLSL;
                    VSConfig.EntryPoint = "main";
                    VSConfig.SourceCode = MakePathToResource("Skybox/prefilter_envmap.vert", 's');
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

                    PrefilterEnvMapVertexShader = LoadShader(VSConfig);

                    // Create a fragment shader as well
                    FShaderConfig FragmentSConfig = { };
                    FragmentSConfig.CompileFlags |= FShaderCompileFlags::GLSL;
                    FragmentSConfig.EntryPoint = "main";
                    FragmentSConfig.SourceCode = MakePathToResource("Skybox/prefilter_envmap.frag", 's');
                    FragmentSConfig.SourceType = EShaderSourceType::Filename;
                    FragmentSConfig.Type = EShaderType::Fragment;

                    PrefilterEnvMapFragmentShader = LoadShader(FragmentSConfig);
                }

                GPConfig.RenderPassPtr = RenderPass;
                GPConfig.PipelineLayoutPtr = PrefilterEnvMapPipelineLayout;
                GPConfig.FragmentShader = PrefilterEnvMapFragmentShader;
                GPConfig.VertexShader = PrefilterEnvMapVertexShader;
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

                PrefilterEnvMapPipeline = RenderInterface.Get()->CreatePipeline(GPConfig);
            }
        }

        // Create Descriptor Set 
        {
            if (PrefilterEnvMapDescSet == nullptr)
            {
                FDescriptorSetsConfig DescriptorSetsConfig = { };
                DescriptorSetsConfig.NumSets = 1;
                DescriptorSetsConfig.PipelineLayoutPtr = PrefilterEnvMapPipelineLayout;
                PrefilterEnvMapDescSet = RenderInterface.Get()->CreateDescriptorSet(DescriptorSetsConfig);

                FDescriptorSetsLinkInfo BufferLinkInfo = { };
                BufferLinkInfo.DescriptorCount = 1;
                BufferLinkInfo.ArrayElementStart = 0;
                BufferLinkInfo.TextureSampler = SamplerHandle;

                BufferLinkInfo.ResourceHandle.BufferHandle = LocalConstantsBuffer;
                BufferLinkInfo.BindingStart = 0;
                PrefilterEnvMapDescSet->LinkToBuffer(0, BufferLinkInfo);

                BufferLinkInfo.ResourceHandle.TextureHandle = SkyboxAsset->GetCubemapTexture();
                BufferLinkInfo.BindingStart = 1;
                PrefilterEnvMapDescSet->LinkToTexture(0, BufferLinkInfo);

                BufferLinkInfo.ResourceHandle.BufferHandle = IBLDataBuffer;
                BufferLinkInfo.BindingStart = 2;
                PrefilterEnvMapDescSet->LinkToBuffer(0, BufferLinkInfo);
            }
        }
    }

    // Create BRDFIntegration pipeline 
    {
        {
            if (BRDFIntegrationPipeline == nullptr)
            {
                FGraphicsPipelineConfig GPConfig;
                {
                    // Create a generic vertex shader as it is mandatory to have a vertex shader 
                    FShaderConfig VSConfig = { };
                    VSConfig.CompileFlags |= FShaderCompileFlags::GLSL;
                    VSConfig.EntryPoint = "main";
                    VSConfig.SourceCode = MakePathToResource("Skybox/brdf_integration.vert", 's');
                    VSConfig.SourceType = EShaderSourceType::Filepath;
                    VSConfig.Type = EShaderType::Vertex;

                    BRDFIntegrationVertexShader = LoadShader(VSConfig);

                    // Create a fragment shader as well
                    FShaderConfig FragmentSConfig = { };
                    FragmentSConfig.CompileFlags |= FShaderCompileFlags::GLSL;
                    FragmentSConfig.EntryPoint = "main";
                    FragmentSConfig.SourceCode = MakePathToResource("Skybox/brdf_integration.frag", 's');
                    FragmentSConfig.SourceType = EShaderSourceType::Filename;
                    FragmentSConfig.Type = EShaderType::Fragment;

                    BRDFIntegrationFragmentShader = LoadShader(FragmentSConfig);
                }

                GPConfig.RenderPassPtr = BRDFIntegrationRenderPass;
                GPConfig.PipelineLayoutPtr = PrefilterEnvMapPipelineLayout;
                GPConfig.FragmentShader = BRDFIntegrationFragmentShader;
                GPConfig.VertexShader = BRDFIntegrationVertexShader;
                GPConfig.PrimitiveTopology = EPrimitiveTopology::TriangleStrip;

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

                BRDFIntegrationPipeline = RenderInterface.Get()->CreatePipeline(GPConfig);
            }
        }

        // Create Descriptor Set 
        {
            if (BRDFIntegrationDescSet == nullptr)
            {
                FDescriptorSetsConfig DescriptorSetsConfig = { };
                DescriptorSetsConfig.NumSets = 1;
                DescriptorSetsConfig.PipelineLayoutPtr = PrefilterEnvMapPipelineLayout;
                BRDFIntegrationDescSet = RenderInterface.Get()->CreateDescriptorSet(DescriptorSetsConfig);

                FDescriptorSetsLinkInfo BufferLinkInfo = { };
                BufferLinkInfo.ArrayElementStart = 0;
                BufferLinkInfo.DescriptorCount = 1;
                BufferLinkInfo.TextureSampler = BRDFSamplerHandle;

                BufferLinkInfo.ResourceHandle.BufferHandle = LocalConstantsBuffer;
                BufferLinkInfo.BindingStart = 0;
                BRDFIntegrationDescSet->LinkToBuffer(0, BufferLinkInfo);

                BufferLinkInfo.ResourceHandle.TextureHandle = SkyboxAsset->GetCubemapTexture();
                BufferLinkInfo.BindingStart = 1;
                BRDFIntegrationDescSet->LinkToTexture(0, BufferLinkInfo);

                BufferLinkInfo.ResourceHandle.BufferHandle = IBLDataBuffer;
                BufferLinkInfo.BindingStart = 2;
                BRDFIntegrationDescSet->LinkToBuffer(0, BufferLinkInfo);
            }
        }
    }

    //CreateSpecularIBL("Skybox1", 128);

    std::string Path = MakePathToResource("NewportLoft_BRDFIntegration.ktx", 't');
    std::wstring WPath(Path.begin(), Path.end());

    GetFileAttributes(WPath.c_str()); // from winbase.h
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(WPath.c_str()) && GetLastError() == ERROR_FILE_NOT_FOUND)
    {
        CreateBrdfIntegration("NewportLoft", 512); // Create BRDF Lut
    }

    Path = MakePathToResource("NewportLoftPrefilteredEnvMap.ktx", 't');
    WPath = std::wstring(Path.begin(), Path.end());

    GetFileAttributes(WPath.c_str()); // from winbase.h
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(WPath.c_str()) && GetLastError() == ERROR_FILE_NOT_FOUND)
    {
        CreatePrefilterEnvMap("NewportLoft", 512); // Create Prefiltered env map
    }

    Path = MakePathToResource("NewportLoftIrradianceMap.ktx", 't');
    WPath = std::wstring(Path.begin(), Path.end());

    GetFileAttributes(WPath.c_str()); // from winbase.h
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(WPath.c_str()) && GetLastError() == ERROR_FILE_NOT_FOUND)
    {
        CreateIrradianceMap("NewportLoft", 32); // Create Irradiance Map 
    }

    {
        //FPipelineLayoutConfig Config = { };
        //FPipelineBindingDescriptor Desc = { };
        //FPipelineBindingSlot Slot = { };

        //Slot.Index = 0;
        //Slot.SetIndex = 0;

        //// UNIFORM Buffer for Local Constants 
        //Desc.BindingSlot = Slot;
        //Desc.ResourceType = EResourceType::Texture;
        //Desc.BindFlags |= FResourceBindFlags::Sampled;
        //Desc.NumResources = 1;
        //Desc.StageFlags |= FShaderStageFlags::FragmentStage;

        //Config.Bindings.push_back(Desc);

        //ImGuiTexturePipelineLayout = RenderInterface.Get()->CreatePipelineLayout(Config);

        //// Create Descriptor Sets 
        //{
        //    FDescriptorSetsConfig DescriptorSetsConfig = { };
        //    DescriptorSetsConfig.NumSets = 1;
        //    DescriptorSetsConfig.PipelineLayoutPtr = ImGuiTexturePipelineLayout;
        //    ImGuiTextureDescSet = RenderInterface.Get()->CreateDescriptorSet(DescriptorSetsConfig);
        //    DescriptorSets.push_back(ImGuiTextureDescSet);

        //    FDescriptorSetsLinkInfo BufferLinkInfo = { };
        //    SkyboxAsset->GetCubemapTexture() = CreateTexture2D(SkyboxToUse, SkyboxBuffer);

        //    BufferLinkInfo.ResourceHandle.TextureHandle = SkyboxAsset->GetCubemapTexture();
        //    BufferLinkInfo.BindingStart = 0;
        //    BufferLinkInfo.ArrayElementStart = 0;
        //    BufferLinkInfo.TextureSampler = SamplerHandle;

        //    ImGuiTextureDescSet->LinkToTexture(0, BufferLinkInfo);
        //}
    }

    Buffer* BrdfLutBuffer = nullptr;
    BRDFLutTexture = CreateTexture2D(MakePathToResource("NewportLoft_BRDFIntegration.ktx", 't'), BrdfLutBuffer, EPixelFormat::RG16UNorm);

    Buffer* PrefilterEnvMapBuff = nullptr;
    PrefilterEnvMapTexture = CreateTextureCubemap(MakePathToResource("NewportLoftPrefilteredEnvMap.ktx", 't'), PrefilterEnvMapBuff);

    Buffer* IrridianceBuffer = nullptr;
    IrridianceTexture = CreateTextureCubemap(MakePathToResource("NewportLoftIrradianceMap.ktx", 't'), IrridianceBuffer, EPixelFormat::BGRA8UNorm);

    Buffers.push_back(PrefilterEnvMapBuff);
    Buffers.push_back(BrdfLutBuffer);
    Buffers.push_back(IrridianceBuffer);
}

void Renderer::LoadModels()
{
    {
        {
            const std::string NameOfModel = "scene";
            {
                using namespace GLTF;

                std::string BusterDroneFolderPath = FilePathToModels;
                BusterDroneFolderPath += "buster_drone/";

                std::string BusterDroneModelPath = BusterDroneFolderPath;
                BusterDroneModelPath += NameOfModel + ".gltf";
                FWorld World = FGLTFLoader::LoadFromFile(BusterDroneModelPath.data());

                // Parsing the World Data 
                {
                    // Load All Textures
                    Textures.resize(World.Images.size());
                    TextureBuffers.resize(World.Images.size());

                    for (uint32 i = 0; i < Textures.size(); ++i)
                    {
                        FImage Image = World.Images[i];
                        Textures[i] = CreateTexture2D(MakePathToResource("buster_drone/" + Image.Uri, 't').c_str(), TextureBuffers[i]);
                        TexturesArray.pop_back();
                        Buffers.pop_back();
                    }
                }

                Samplers.resize(World.Samplers.size());

                // Samplers
                {
                    FSamplerConfig Config;
                    Config.SetDefault();
                    for (uint32 i = 0; i < Samplers.size(); ++i)
                    {
                        Config.MagFilter = World.Samplers[i].MagFilter == GLTF::FSampler::EFilter::Linear ? ESamplerFilter::Linear : ESamplerFilter::Nearest;
                        Config.MinFilter = World.Samplers[i].MinFilter == GLTF::FSampler::EFilter::Linear ? ESamplerFilter::Linear : ESamplerFilter::Nearest;

                        Samplers[i] = RenderInterface.Get()->CreateSampler(Config);
                    }
                }

                // All buffers data vertex/index
                BufferDatas.resize(World.Buffers.size());
                {
                    for (uint32 i = 0; i < BufferDatas.size(); ++i)
                    {
                        if (!World.Buffers[i].bIsUriBuffer)
                        {
                            std::string PathToBuffer = BusterDroneFolderPath + World.Buffers[i].Uri;
                            // Firstly Read the buffer data from binary file 
                            std::ifstream FileHandle(PathToBuffer, std::ios_base::binary | std::ios_base::ate);
                            if (!FileHandle.is_open())
                            {
                                VE_ASSERT(false, VE_TEXT("The URI for buffer index {0} which is {1} is invalid..."), i, PathToBuffer);
                            }

                            uint64 Size = FileHandle.tellg();
                            if (Size == 0)
                            {
                                VE_ASSERT(false, VE_TEXT("Buffer index {0} has no data (byte length 0)..."), i);
                            }

                            FileHandle.seekg(std::ios::beg);

                            BufferDatas[i] = new uint8[Size];
                            FileHandle.read((char*)BufferDatas[i], Size);

                            FileHandle.close();
                        }
                        else
                        {
                            BufferDatas[i] = new uint8[World.Buffers[i].Uri.size()];
                            memcpy(BufferDatas[i], World.Buffers[i].Uri.data(), World.Buffers[i].Uri.size());
                        }
                    }
                }

                // Create all the buffer resources
                Buffers.resize(World.BufferViews.size());
                {
                    std::unordered_map<uint32, uint32> Map;

                    FBufferConfig Config = { };
                    for (uint32 i = 0; i < Buffers.size(); ++i)
                    {
                        FBufferView BufferView = World.BufferViews[i];/*
                        Config.InitialData = (BufferDatas[BufferView.BufferIndex]) + (BufferView.ByteOffset);
                        Config.Size = BufferView.ByteLength;
                        Config.UsageFlags = BufferView.Target == FBufferView::ETarget::IndexData ? FResourceBindFlags::IndexBuffer : FResourceBindFlags::VertexBuffer;
                        Config.MemoryFlags = FMemoryFlags::HostCached;

                        Buffers[i] = RenderInterface.Get()->CreateBuffer(Config);*/

                        //if (BufferView.Target == GLTF::FBufferView::ETarget::Invalid)
                        //{
                        //    break;
                        //}

                        if (Map.find(BufferView.BufferIndex) != Map.end())
                        {
                            Map[BufferView.BufferIndex] += BufferView.ByteLength;
                        }
                        else
                        {
                            Map.insert(std::make_pair(BufferView.BufferIndex, BufferView.ByteLength));
                        }
                    }

                    Config.UsageFlags = FResourceBindFlags::IndexBuffer | FResourceBindFlags::VertexBuffer;
                    Config.MemoryFlags = FMemoryFlags::HostCached;

                    for (auto It = Map.begin(); It != Map.end(); ++It)
                    {
                        Config.InitialData = BufferDatas[It->first];
                        Config.Size = It->second;

                        Buffers[It->first] = RenderInterface.Get()->CreateBuffer(Config);
                    }
                }

                // Create the drawables 
                {
                    FScene Scene = World.Scenes[0];
                    std::vector<int32> NodeParents(World.Nodes.size());
                    std::vector<uint32> NodeStack;
                    std::vector<Matrix4D> NodeMatrices(World.Nodes.size());

                    for (uint32 i = 0; i < Scene.Nodes.size(); ++i)
                    {
                        uint32 RootNode = Scene.Nodes[i];
                        NodeParents[RootNode] = -1;
                        NodeStack.push_back(RootNode);
                    }

                    while (!NodeStack.empty())
                    {
                        uint32 NodeIndex = NodeStack.back();
                        NodeStack.pop_back();
                        FNode Node = World.Nodes[NodeIndex];

                        Matrix4D LocalMatrix = { };

                        if (Node.Matrix(0, 0) != EPSILON)
                        {
                            LocalMatrix = Matrix4D::Transpose(Node.Matrix);
                        }
                        else
                        {
                            Quat Rotation(
                                Node.Rotation.X,
                                Node.Rotation.Y,
                                Node.Rotation.Z,
                                Node.Rotation.W);

                            Matrix4D ScaleMatrix;
                            ScaleMatrix.SetIdentity();
                            ScaleMatrix.ScaleMatrix(Node.Scale);

                            Matrix4D TranslationMatrix;
                            TranslationMatrix.SetIdentity();
                            TranslationMatrix.SetTranslation(Node.Translation);

                            LocalMatrix = ScaleMatrix * Rotation.ToMatrix4D() * TranslationMatrix;
                        }

                        NodeMatrices[NodeIndex] = LocalMatrix;

                        // Add current nodes children
                        for (uint32 i = 0; i < Node.Children.size(); ++i)
                        {
                            uint32 ChildrenNodeIndex = Node.Children[i];
                            NodeParents[ChildrenNodeIndex] = NodeIndex;
                            NodeStack.push_back(ChildrenNodeIndex);
                        }

                        // Mesh doesn't exist for this node 
                        if (Node.MeshIndex == -1)
                        {
                            continue;
                        }

                        FMesh Mesh = World.Meshes[Node.MeshIndex];

                        // Get the final matrix through many affine transformation of parents 
                        Matrix4D FinalMatrix = LocalMatrix;
                        uint32 NodeParent = NodeParents[NodeIndex];
                        while (NodeParent != -1)
                        {
                            FinalMatrix = FinalMatrix * NodeMatrices[NodeParent];
                            NodeParent = NodeParents[NodeParent];
                        }

                        // Create an array for render asset sections
                        std::vector<FRenderAssetSection> RenderAssetSections;
                        RenderAssetSections.resize(Mesh.Primitives.size());

                        // Create an array for all material data 
                        std::vector<FMaterialData> MaterialDatas;
                        MaterialDatas.resize(Mesh.Primitives.size());

                        Buffer* IndexBuffer = nullptr;
                        Buffer* PositionBuffer = nullptr;
                        Buffer* TangentBuffer = nullptr;
                        Buffer* NormalBuffer = nullptr;
                        Buffer* TexCoordBuffer = nullptr;

                        bool bIsStaticMeshBlendable = false;

                        for (uint32 i = 0; i < Mesh.Primitives.size(); ++i)
                        {
                            FRenderAssetSection Section = { };
                            FMaterialData MaterialData = { };

                            MaterialData.Model = FinalMatrix;

                            FMeshPrimitive& MeshPrimitive = Mesh.Primitives[i];

                            FAccessor& IndicesAccessor = World.Accessors[MeshPrimitive.IndiciesIndex];
                            Section.IndexType = IndicesAccessor.ComponentType == FAccessor::EComponentType::UnsignedInt ? EPixelFormat::R32UInt : EPixelFormat::R16UInt;

                            FBufferView& IndicesBufferView = World.BufferViews[IndicesAccessor.BufferView];
                            Buffer* IndicesBufferGpu = Buffers[IndicesBufferView.BufferIndex];

                            IndexBuffer = IndicesBufferGpu;
                            Section.IndexOffset = (IndicesAccessor.ByteOffset) + (IndicesBufferView.ByteOffset);
                            Section.Count = IndicesAccessor.Count;

                            int32 PositionAccessorIndex = GetAttributeAccessorIndex(MeshPrimitive.Attributes, "POSITION");
                            int32 tangent_accessor_index = GetAttributeAccessorIndex(MeshPrimitive.Attributes, "TANGENT");
                            int32 normal_accessor_index = GetAttributeAccessorIndex(MeshPrimitive.Attributes, "NORMAL");
                            int32 texcoord_accessor_index = GetAttributeAccessorIndex(MeshPrimitive.Attributes, "TEXCOORD_0");

                            Vector3D* position_data = nullptr;
                            uint8* index_data_8 = GetBufferData(World.BufferViews.data(), IndicesAccessor.BufferView, BufferDatas);
                            uint32 vertex_count = 0;

                            uint16* NewIndexData = 0;
                            if (IndicesAccessor.ComponentType == FAccessor::EComponentType::Byte)// || IndicesAccessor.ComponentType == FAccessor::EComponentType::UnsignedByte)
                            {
                                int8* IndexDataS8 = (int8*)index_data_8;
                                NewIndexData = new uint16[Section.Count];
                                for (uint32 i = 0; i < Section.Count; ++i)
                                {
                                    int8 Byte = IndexDataS8[i];
                                    NewIndexData[i] = Byte;
                                }

                                FBufferConfig Config;
                                Config.InitialData = NewIndexData;
                                Config.Size = Section.Count * sizeof(uint16);
                                Config.MemoryFlags |= FMemoryFlags::HostCached;
                                Config.UsageFlags |= FResourceBindFlags::IndexBuffer;

                                Buffer* NewIndexBuffer = RenderInterface.Get()->CreateBuffer(Config);
                                Buffers.push_back(NewIndexBuffer);

                                IndexBuffer = NewIndexBuffer;
                                Section.IndexOffset = 0;

                                delete[] NewIndexData;
                            }
                            if (IndicesAccessor.ComponentType == FAccessor::EComponentType::UnsignedByte)
                            {
                                NewIndexData = new uint16[Section.Count];

                                for (uint32 i = 0; i < Section.Count; ++i)
                                {
                                    uint8 Byte = index_data_8[i];
                                    NewIndexData[i] = Byte;
                                }

                                FBufferConfig Config;
                                Config.InitialData = NewIndexData;
                                Config.Size = Section.Count * sizeof(uint16);
                                Config.MemoryFlags |= FMemoryFlags::HostCached;
                                Config.UsageFlags |= FResourceBindFlags::IndexBuffer;

                                Buffer* NewIndexBuffer = RenderInterface.Get()->CreateBuffer(Config);
                                Buffers.push_back(NewIndexBuffer);

                                IndexBuffer = NewIndexBuffer;
                                Section.IndexOffset = 0;

                                delete[] NewIndexData;
                            }

                            if (PositionAccessorIndex != -1) {
                                FAccessor& position_accessor = World.Accessors[PositionAccessorIndex];
                                FBufferView& position_buffer_view = World.BufferViews[position_accessor.BufferView];
                                Buffer* position_buffer_gpu = Buffers[position_buffer_view.BufferIndex];

                                vertex_count = position_accessor.Count;

                                PositionBuffer = position_buffer_gpu;
                                Section.PositionOffset = (position_accessor.ByteOffset) + (position_buffer_view.ByteOffset);

                                position_data = (Vector3D*)GetBufferData(World.BufferViews.data(), position_accessor.BufferView, BufferDatas);
                            }
                            else {
                                VE_ASSERT(false, "No position data found!");
                                continue;
                            }

                            if (normal_accessor_index != -1) {
                                FAccessor& normal_accessor = World.Accessors[normal_accessor_index];
                                FBufferView& normal_buffer_view = World.BufferViews[normal_accessor.BufferView];
                                Buffer* normal_buffer_gpu = Buffers[normal_buffer_view.BufferIndex];

                                NormalBuffer = normal_buffer_gpu;
                                Section.NormalOffset = (normal_accessor.ByteOffset) + (normal_buffer_view.ByteOffset);
                            }
                            else {
                                VE_ASSERT(false, VE_TEXT("Normals computed at runtime not supported anymore..."))
                                    //    // NOTE(marco): we could compute this at runtime
                                    //    std::vector<Vector3D> normals_array(vertex_count);
                                    //memset(normals_array.data(), 0, normals_array.size() * sizeof(Vector3D));

                                    //uint32 index_count = Section.Count;
                                    //for (uint32 index = 0; index < index_count; index += 3) {
                                    //    uint32 i0 = IndicesAccessor.ComponentType == GLTF::FAccessor::EComponentType::UnsignedInt ? index_data_32[index] : index_data_16[index];
                                    //    uint32 i1 = IndicesAccessor.ComponentType == GLTF::FAccessor::EComponentType::UnsignedInt ? index_data_32[index + 1] : index_data_16[index + 1];
                                    //    uint32 i2 = IndicesAccessor.ComponentType == GLTF::FAccessor::EComponentType::UnsignedInt ? index_data_32[index + 2] : index_data_16[index + 2];

                                    //    Vector3D p0 = position_data[i0];
                                    //    Vector3D p1 = position_data[i1];
                                    //    Vector3D p2 = position_data[i2];

                                    //    Vector3D a = p1 - p0;
                                    //    Vector3D b = p2 - p0;

                                    //    Vector3D normal = Vector3D::CrossProduct(a, b);

                                    //    normals_array[i0] = normals_array[i0] + normal;
                                    //    normals_array[i1] = normals_array[i1] + normal;
                                    //    normals_array[i2] = normals_array[i2] + normal;
                                    //}

                                    //for (uint32 vertex = 0; vertex < vertex_count; ++vertex) {
                                    //    normals_array[vertex] = normals_array[vertex].Normalize();
                                    //}

                                    //FBufferConfig Config;
                                    //Config.InitialData = normals_array.data();
                                    //Config.Size = normals_array.size() * sizeof(Vector3D);
                                    //Config.MemoryFlags |= FMemoryFlags::HostCached;
                                    //Config.UsageFlags |= FResourceBindFlags::VertexBuffer;

                                    /*Buffer* NormalsBuffer = RenderInterface.Get()->CreateBuffer(Config);
                                    MeshDraw.NormalBuffer = NormalsBuffer;
                                    MeshDraw.NormalOffset = 0;

                                    Buffers.push_back(NormalsBuffer);*/
                            }

                            if (tangent_accessor_index != -1) {
                                FAccessor& tangent_accessor = World.Accessors[tangent_accessor_index];
                                FBufferView& tangent_buffer_view = World.BufferViews[tangent_accessor.BufferView];
                                Buffer* tangent_buffer_gpu = Buffers[tangent_buffer_view.BufferIndex];

                                TangentBuffer = tangent_buffer_gpu;
                                Section.TangentOffset = (tangent_accessor.ByteOffset) + (tangent_buffer_view.ByteOffset);

                                MaterialData.Flags |= MaterialFeatures_TangentVertexAttribute;
                            }

                            if (texcoord_accessor_index != -1) {
                                FAccessor& texcoord_accessor = World.Accessors[texcoord_accessor_index];
                                FBufferView& texcoord_buffer_view = World.BufferViews[texcoord_accessor.BufferView];
                                Buffer* texcoord_buffer_gpu = Buffers[texcoord_buffer_view.BufferIndex];

                                TexCoordBuffer = texcoord_buffer_gpu;
                                Section.TexCoordOffset = (texcoord_accessor.ByteOffset) + (texcoord_buffer_view.ByteOffset);

                                MaterialData.Flags |= MaterialFeatures_TexcoordVertexAttribute;
                            }

                            VE_ASSERT(MeshPrimitive.MaterialIndex != -1, "Mesh with no material is not supported!");
                            FMaterial& material = World.Materials[MeshPrimitive.MaterialIndex];

                            FDescriptorSetsConfig DescriptorSetsConfig = { };
                            DescriptorSetsConfig.NumSets = 1;
                            DescriptorSetsConfig.PipelineLayoutPtr = PBRTexturePipelineLayout;
                            IDescriptorSets* DescriptorSet = RenderInterface.Get()->CreateDescriptorSet(DescriptorSetsConfig);
                            Section.RenderAssetDescriptorSet = DescriptorSet;
                            DescriptorSets.push_back(DescriptorSet);

                            // For Texture Linkage 
                            FDescriptorSetsLinkInfo LinkInfo = { };
                            LinkInfo.ArrayElementStart = 0;
                            LinkInfo.DescriptorCount = 1;

                            {
                                // Bind for Diffuse texture 
                                LinkInfo.BindingStart = 2;

                                // Set to Default value first 
                                LinkInfo.TextureSampler = SamplerHandle;
                                LinkInfo.ResourceHandle.TextureHandle = CP2077TextureHandle;

                                if (material.PBRMetallicRoughnessInfo.BaseColorTexture.Index != -1) {
                                    FTexture& diffuse_texture = World.Textures[material.PBRMetallicRoughnessInfo.BaseColorTexture.Index];
                                    Texture* diffuse_texture_gpu = Textures[diffuse_texture.ImageIndex];

                                    Sampler* SamplerDef = SamplerHandle;
                                    if (diffuse_texture.SamplerIndex != -1) {
                                        SamplerDef = Samplers[diffuse_texture.SamplerIndex];
                                    }

                                    MaterialData.Flags |= MaterialFeatures_ColorTexture;

                                    LinkInfo.TextureSampler = SamplerDef;
                                    LinkInfo.ResourceHandle.TextureHandle = diffuse_texture_gpu;

                                    DescriptorSet->LinkToTexture(0, LinkInfo);
                                }

                                // Update the diffuse texture set 
                                DescriptorSet->LinkToTexture(0, LinkInfo);
                            }

                            {
                                // Bind for MetallicRoughness texture 
                                LinkInfo.BindingStart = 3;

                                // Set to Default value first 
                                LinkInfo.TextureSampler = SamplerHandle;
                                LinkInfo.ResourceHandle.TextureHandle = CP2077TextureHandle;

                                if (material.PBRMetallicRoughnessInfo.MetallicRoughnessTexture.Index != -1) {
                                    FTexture& roughness_texture = World.Textures[material.PBRMetallicRoughnessInfo.MetallicRoughnessTexture.Index];
                                    Texture* roughness_texture_gpu = Textures[roughness_texture.ImageIndex];

                                    Sampler* SamplerDef = SamplerHandle;
                                    if (roughness_texture.SamplerIndex != -1) {
                                        SamplerDef = Samplers[roughness_texture.SamplerIndex];
                                    }

                                    // Link to textue
                                    LinkInfo.TextureSampler = SamplerDef;
                                    LinkInfo.ResourceHandle.TextureHandle = roughness_texture_gpu;

                                    MaterialData.Flags |= MaterialFeatures_RoughnessTexture;
                                }

                                // Update MetallicRoughness texture
                                DescriptorSet->LinkToTexture(0, LinkInfo);
                            }

                            {
                                // Bind for OcclusionTexture texture 
                                LinkInfo.BindingStart = 4;

                                // Set to Default value first 
                                LinkInfo.TextureSampler = SamplerHandle;
                                LinkInfo.ResourceHandle.TextureHandle = CP2077TextureHandle;

                                if (material.OcclusionTexture.Index != -1) {
                                    FTexture& occlusion_texture = World.Textures[material.OcclusionTexture.Index];

                                    // NOTE(marco): this could be the same as the roughness texture, but for now we treat it as a separate
                                    // texture
                                    Texture* occlusion_texture_gpu = Textures[occlusion_texture.ImageIndex];

                                    Sampler* SamplerDef = SamplerHandle;
                                    if (occlusion_texture.SamplerIndex != -1) {
                                        SamplerDef = Samplers[occlusion_texture.SamplerIndex];
                                    }

                                    // Link to textue
                                    LinkInfo.TextureSampler = SamplerDef;
                                    LinkInfo.ResourceHandle.TextureHandle = occlusion_texture_gpu;

                                    MaterialData.Flags |= MaterialFeatures_OcclusionTexture;
                                }

                                // Update Occlusion Texture
                                DescriptorSet->LinkToTexture(0, LinkInfo);
                            }

                            {
                                // Bind for EmissiveTexture texture 
                                LinkInfo.BindingStart = 5;

                                // Set to Default value first 
                                LinkInfo.TextureSampler = SamplerHandle;
                                LinkInfo.ResourceHandle.TextureHandle = CP2077TextureHandle;

                                if (material.EmissiveTexture.Index != -1) {
                                    FTexture& emissive_texture = World.Textures[material.EmissiveTexture.Index];

                                    // NOTE(marco): this could be the same as the roughness texture, but for now we treat it as a separate
                                    // texture
                                    Texture* emissive_texture_gpu = Textures[emissive_texture.ImageIndex];

                                    Sampler* SamplerDef = SamplerHandle;
                                    if (emissive_texture.SamplerIndex != -1) {
                                        SamplerDef = Samplers[emissive_texture.SamplerIndex];
                                    }

                                    // Link to textue
                                    LinkInfo.TextureSampler = SamplerDef;
                                    LinkInfo.ResourceHandle.TextureHandle = emissive_texture_gpu;

                                    MaterialData.Flags |= MaterialFeatures_EmissiveTexture;
                                }

                                DescriptorSet->LinkToTexture(0, LinkInfo);
                            }

                            {
                                // Bind for NormalTexture texture 
                                LinkInfo.BindingStart = 6;

                                // Set to Default value first 
                                LinkInfo.TextureSampler = SamplerHandle;
                                LinkInfo.ResourceHandle.TextureHandle = CP2077TextureHandle;

                                if (material.NormalTexture.Index != -1) {
                                    FTexture& normal_texture = World.Textures[material.NormalTexture.Index];
                                    Texture* normal_texture_gpu = Textures[normal_texture.ImageIndex];

                                    Sampler* SamplerDef = SamplerHandle;
                                    if (normal_texture.SamplerIndex != -1) {
                                        SamplerDef = Samplers[normal_texture.SamplerIndex];
                                    }

                                    // Link to textue
                                    LinkInfo.TextureSampler = SamplerDef;
                                    LinkInfo.ResourceHandle.TextureHandle = normal_texture_gpu;

                                    DescriptorSet->LinkToTexture(0, LinkInfo);

                                    MaterialData.Flags |= MaterialFeatures_NormalTexture;
                                }

                                DescriptorSet->LinkToTexture(0, LinkInfo);
                            }

                            MaterialData.EmissiveFactor = material.EmissiveFactor;
                            MaterialData.OcclusionFactor = material.OcclusionTexture.Strength;

                            // Metallic Roughness workflow data 
                            MaterialData.BaseColorFactor = material.PBRMetallicRoughnessInfo.BaseColorFactor;
                            MaterialData.MetallicFactor = material.PBRMetallicRoughnessInfo.MetallicFactor;
                            MaterialData.RoughnessFactor = material.PBRMetallicRoughnessInfo.RoughnessFactor;

                            // Blending 
                            MaterialData.AlphaMaskCutoff = material.AlphaCutoff;
                            MaterialData.AlphaMask = material.AlphaMode == GLTF::FMaterial::EAlphaMode::Mask;

                            if (material.AlphaMode == GLTF::FMaterial::EAlphaMode::Blend)
                            {
                                bIsStaticMeshBlendable = true;
                            }

                            // Link to Irridiance cube map
                            LinkInfo.BindingStart = 7;
                            LinkInfo.TextureSampler = SamplerHandle;
                            LinkInfo.ResourceHandle.TextureHandle = IrridianceTexture;

                            DescriptorSet->LinkToTexture(0, LinkInfo);

                            // Prefilter env map
                            LinkInfo.BindingStart = 8;
                            LinkInfo.TextureSampler = SamplerHandle;
                            LinkInfo.ResourceHandle.TextureHandle = PrefilterEnvMapTexture;

                            DescriptorSet->LinkToTexture(0, LinkInfo);

                            // brdr lut 
                            LinkInfo.BindingStart = 9;
                            LinkInfo.TextureSampler = SamplerHandle;
                            LinkInfo.ResourceHandle.TextureHandle = BRDFLutTexture;

                            DescriptorSet->LinkToTexture(0, LinkInfo);

                            FBufferConfig BufferConfig = { };
                            BufferConfig.InitialData = &MaterialData;
                            BufferConfig.Size = sizeof(FMaterialData);
                            BufferConfig.MemoryFlags |= FMemoryFlags::HostCached;
                            BufferConfig.UsageFlags |= FResourceBindFlags::UniformBuffer;

                            Section.MaterialBuffer = RenderInterface.Get()->CreateBuffer(BufferConfig);
                            Buffers.push_back(Section.MaterialBuffer);

                            // Link to Material Buffer
                            LinkInfo.BindingStart = 1;
                            LinkInfo.ResourceHandle.BufferHandle = Section.MaterialBuffer;
                            DescriptorSet->LinkToBuffer(0, LinkInfo);

                            // Link to Local Constants Buffer
                            LinkInfo.BindingStart = 0;
                            LinkInfo.ResourceHandle.BufferHandle = LocalConstantsBuffer;
                            DescriptorSet->LinkToBuffer(0, LinkInfo);

                            RenderAssetSections[i] = Section;
                            MaterialDatas[i] = MaterialData;
                        }

                        CStaticMesh* StaticMesh = new CStaticMesh();
                        for (uint32 i = 0; i < RenderAssetSections.size(); ++i)
                        {
                            StaticMesh->AddNewMaterial(MaterialDatas[i], &RenderAssetSections[i]);
                        }

                        StaticMesh->RenderAssetData.IndexBuffer = IndexBuffer;
                        StaticMesh->RenderAssetData.PositionBuffer = PositionBuffer;
                        StaticMesh->RenderAssetData.TangentBuffer = TangentBuffer;
                        StaticMesh->RenderAssetData.NormalBuffer = NormalBuffer;
                        StaticMesh->RenderAssetData.TexCoordBuffer = TexCoordBuffer;

                        StaticMesh->SetName(Mesh.Name);
                        StaticMesh->SetIsTransparent(bIsStaticMeshBlendable);
                        StaticMeshes.push_back(StaticMesh);
                    }
                }
            }
        }
    }

    CreateSphereModels();
}

void Renderer::CreateSphereMeshData(float inRadius, uint32 inNumStacks, uint32 inNumSectors, std::vector<float>& outVerts, std::vector<float>& outNormals, std::vector<uint32>& outIndices, std::vector<float>& outTexCoords)
{
    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.0f / inRadius;
    float s, t;

    float sectorStep = 2 * PI / inNumSectors;
    float stackStep = PI / inNumStacks;
    float sectorAngle, stackAngle;

    // Create vertices and normals 
    {
        for (uint32 i = 0; i <= inNumStacks; ++i)
        {
            stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            xy = inRadius * cosf(stackAngle);             // r * cos(u)
            z = inRadius * sinf(stackAngle);              // r * sin(u)

            // add (sectorCount+1) vertices per stack
            // first and last vertices have same position and normal, but different tex coords
            for (int j = 0; j <= inNumSectors; ++j)
            {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                // vertex position (x, y, z)
                x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
                outVerts.push_back(x);
                outVerts.push_back(y);
                outVerts.push_back(z);

                // normalized vertex normal (nx, ny, nz)
                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;
                outNormals.push_back(nx);
                outNormals.push_back(ny);
                outNormals.push_back(nz);

                // vertex tex coord (s, t) range between [0, 1]
                s = (float)j / inNumSectors;
                t = (float)i / inNumStacks;
                outTexCoords.push_back(s);
                outTexCoords.push_back(t);
            }
        }
    }

    // generate CCW index list of sphere triangles
       // k1--k1+1
       // |  / |
       // | /  |
       // k2--k2+1
    //std::vector<int> lineIndices;
    {
        int k1, k2;
        for (int i = 0; i < inNumStacks; ++i)
        {
            k1 = i * (inNumSectors + 1);     // beginning of current stack
            k2 = k1 + inNumSectors + 1;      // beginning of next stack

            for (int j = 0; j < inNumSectors; ++j, ++k1, ++k2)
            {
                // 2 triangles per sector excluding first and last stacks
                // k1 => k2 => k1+1
                if (i != 0)
                {
                    outIndices.push_back(k1);
                    outIndices.push_back(k2);
                    outIndices.push_back(k1 + 1);
                }

                // k1+1 => k2 => k2+1
                if (i != (inNumStacks - 1))
                {
                    outIndices.push_back(k1 + 1);
                    outIndices.push_back(k2);
                    outIndices.push_back(k2 + 1);
                }

                // store indices for lines
                // vertical lines for all stacks, k1 => k2
                //lineIndices.push_back(k1);
                //lineIndices.push_back(k2);
                //if (i != 0)  // horizontal lines except 1st stack, k1 => k+1
                //{
                //    lineIndices.push_back(k1);
                //    lineIndices.push_back(k1 + 1);
                //}
            }
        }
    }
}

void Renderer::CreateSphereModels()
{
    Matrix4D model = Matrix4D::Identity();

    int32 nrRows = 7;
    int32 nrCols = 7;
    float spacing = 2.5f;

    Vector3D Translation;

    std::string Name = "Sphere ";

    Vector3D LightPositions[4];
    LightPositions[0] = Vector3D(-10.0f, 10.0f, -10.0f);
    LightPositions[1] = Vector3D(10.0f, 10.0f, -10.0f);
    LightPositions[2] = Vector3D(-10.0f, -10.0f, -10.0f);
    LightPositions[3] = Vector3D(10.0f, -10.0f, -10.0f);

    //for (int32 row = 0; row <= nrRows; ++row)
    for (uint32 i = 0; i < 4; ++i)
    {
        float MetallicFactor = 0;// (float)row / (float)nrRows;
        //for (int32 col = 0; col <= nrCols; ++col)
        {
            // we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
            // on direct lighting.
            float RoughnessFactor = 1;// MathUtils::Clamp(0.05f, 1.0f, (float)col / (float)nrCols);

            CStaticMesh* StaticMesh = new CStaticMesh();

            FMaterialData Material = { };
            Material.Flags = MaterialFeatures::MaterialFeatures_TexcoordVertexAttribute;
            Material.BaseColorFactor = Vector4D(0.5f, 0.0f, 0.0f, 1.0f);
            Material.EmissiveFactor = 0;
            Material.MetallicFactor = MetallicFactor;

            model = Matrix4D::Identity();
            //Translation = Vector3D(
            //    (float)(col - (nrCols / 2)) * spacing,
            //    (float)(row - (nrRows / 2)) * spacing,
            //    -2.0f
            //);
            //model.SetTranslation(Translation);
            model.SetTranslation(LightPositions[i]);

            Material.Model = model;
            Material.ModelInv = model.Inverse();
            Material.OcclusionFactor = 1;
            Material.RoughnessFactor = RoughnessFactor;
            Material.AlphaMask = 0;
            Material.AlphaMaskCutoff = 0.5f;

            FRenderAssetSection Section = { };
            Section.Count = NumSphereIndices;
            Section.IndexOffset = 0;
            Section.IndexType = EPixelFormat::R32UInt;
            Section.MaterialIndex = 0;
            Section.NormalOffset = NumSphereVerts * 4;
            Section.PositionOffset = 0;
            Section.TangentOffset = 0;
            Section.TexCoordOffset = SphereTexCoordOffset;

            FDescriptorSetsConfig Config = { };
            Config.NumSets = 1;
            Config.PipelineLayoutPtr = PBRTexturePipelineLayout;
            IDescriptorSets* Set = RenderInterface.Get()->CreateDescriptorSet(Config);
            DescriptorSets.push_back(Set);

            FBufferConfig BufferConfig = { };
            BufferConfig.InitialData = &Material;
            BufferConfig.Size = sizeof(FMaterialData);
            BufferConfig.MemoryFlags |= FMemoryFlags::HostCached;
            BufferConfig.UsageFlags |= FResourceBindFlags::UniformBuffer;

            Section.MaterialBuffer = RenderInterface.Get()->CreateBuffer(BufferConfig);
            Buffers.push_back(Section.MaterialBuffer);

            Section.RenderAssetDescriptorSet = Set;
            StaticMesh->AddNewMaterial(Material, &Section);

            FDescriptorSetsLinkInfo LinkInfo = { };
            LinkInfo.ArrayElementStart = 0;
            LinkInfo.DescriptorCount = 1;
            LinkInfo.TextureSampler = SamplerHandle;

            LinkInfo.BindingStart = 0;
            LinkInfo.ResourceHandle.BufferHandle = LocalConstantsBuffer;
            Set->LinkToBuffer(0, LinkInfo);

            LinkInfo.BindingStart = 1;
            LinkInfo.ResourceHandle.BufferHandle = Section.MaterialBuffer;
            Set->LinkToBuffer(0, LinkInfo);

            for (uint32 i = 2; i < 7; ++i)
            {
                LinkInfo.BindingStart = i;
                LinkInfo.ResourceHandle.TextureHandle = CP2077TextureHandle;
                Set->LinkToTexture(0, LinkInfo);
            }

            LinkInfo.BindingStart = 7;
            LinkInfo.ResourceHandle.TextureHandle = IrridianceTexture;
            Set->LinkToTexture(0, LinkInfo);

            LinkInfo.BindingStart = 9;
            LinkInfo.ResourceHandle.TextureHandle = BRDFLutTexture;
            Set->LinkToTexture(0, LinkInfo);

            LinkInfo.TextureSampler = LODSamplerHandle;
            LinkInfo.BindingStart = 8;
            LinkInfo.ResourceHandle.TextureHandle = PrefilterEnvMapTexture;
            Set->LinkToTexture(0, LinkInfo);

            StaticMesh->RenderAssetData.IndexBuffer = SphereIndexBuffer;
            StaticMesh->RenderAssetData.PositionBuffer = SphereBuffer;
            StaticMesh->RenderAssetData.TangentBuffer = nullptr;
            StaticMesh->RenderAssetData.NormalBuffer = SphereBuffer;
            StaticMesh->RenderAssetData.TexCoordBuffer = SphereBuffer;
            StaticMesh->SetName(Name + std::to_string(i));//std::to_string(row) + std::to_string(col));
            StaticMeshes.push_back(StaticMesh);
            LightStaticMeshes.push_back(StaticMesh);
        }
    }
}

void Renderer::CreateHighDynamicImagePipeline(const char* inFilePath)
{
    // Create HDRI pipeline 
    // the goal of this pipeline is to be able to convert hrdi images to cubemaps 
    // ...
    // it will use the same layout as the skybox pipeline 
    {
        if (HDRPipeline == nullptr)
        {
            {
                FPipelineLayoutConfig Config = { };
                FPipelineBindingDescriptor Desc = { };
                FPipelineBindingSlot Slot = { };

                Slot.Index = 0;
                Slot.SetIndex = 0;

                // UNIFORM Buffer for Local Constants 
                Desc.BindingSlot = Slot;
                Desc.ResourceType = EResourceType::Buffer;
                Desc.BindFlags |= FResourceBindFlags::UniformBuffer;
                Desc.NumResources = 1;
                Desc.StageFlags = FShaderStageFlags::FragmentStage | FShaderStageFlags::VertexStage;

                Config.Bindings.push_back(Desc);

                // cubemap Texture 
                Desc.BindingSlot.Index = 1;
                Desc.NumResources = 1;
                Desc.ResourceType = EResourceType::Texture;
                Desc.BindFlags = 0;
                Desc.BindFlags |= FResourceBindFlags::Sampled;
                Desc.StageFlags = 0;
                Desc.StageFlags |= FShaderStageFlags::FragmentStage;
                Config.Bindings.push_back(Desc);

                HDRPipelineLayout = RenderInterface.Get()->CreatePipelineLayout(Config);
            }

            FGraphicsPipelineConfig GPConfig;
            {
                // Create a generic vertex shader as it is mandatory to have a vertex shader 
                FShaderConfig VSConfig = { };
                VSConfig.CompileFlags |= FShaderCompileFlags::GLSL | FShaderCompileFlags::InvertY;
                VSConfig.EntryPoint = "main";
                VSConfig.SourceCode = MakePathToResource("Skybox/hdr_khronos.vert", 's');
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

                HDRVertexShader = LoadShader(VSConfig);

                // Create a fragment shader as well
                FShaderConfig FragmentSConfig = { };
                FragmentSConfig.CompileFlags |= FShaderCompileFlags::GLSL;
                FragmentSConfig.EntryPoint = "main";
                FragmentSConfig.SourceCode = MakePathToResource("Skybox/hdr_khronos.frag", 's');
                FragmentSConfig.SourceType = EShaderSourceType::Filename;
                FragmentSConfig.Type = EShaderType::Fragment;

                HDRFragmentShader = LoadShader(FragmentSConfig);
            }

            GPConfig.RenderPassPtr = RenderPass;
            GPConfig.PipelineLayoutPtr = HDRPipelineLayout;
            GPConfig.FragmentShader = HDRFragmentShader;
            GPConfig.VertexShader = HDRVertexShader;
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

            HDRPipeline = RenderInterface.Get()->CreatePipeline(GPConfig);
        }
    }

    // Create Descriptor Set 
    {
        if (HDRDescSet == nullptr)
        {
            FDescriptorSetsConfig DescriptorSetsConfig = { };
            DescriptorSetsConfig.NumSets = 1;
            DescriptorSetsConfig.PipelineLayoutPtr = HDRPipelineLayout;
            HDRDescSet = RenderInterface.Get()->CreateDescriptorSet(DescriptorSetsConfig);

            FBufferConfig Config;
            Config.InitialData = nullptr;
            Config.Size = sizeof(HDRBufferData);
            Config.MemoryFlags |= FMemoryFlags::HostVisible;
            Config.UsageFlags |= FResourceBindFlags::UniformBuffer;

            HDRConstantsBuffer = RenderInterface.Get()->CreateBuffer(Config);

            FDescriptorSetsLinkInfo BufferLinkInfo = { };
            BufferLinkInfo.DescriptorCount = 1;
            BufferLinkInfo.ResourceHandle.BufferHandle = HDRConstantsBuffer;
            BufferLinkInfo.BindingStart = 0;
            BufferLinkInfo.ArrayElementStart = 0;

            HDRDescSet->LinkToBuffer(0, BufferLinkInfo);
        }

        //"../Assets/Textures/Skybox.hdr"
        Buffer* TextureBuffer = nullptr;
        Texture* HDRTex = CreateTexture2D(inFilePath, TextureBuffer);

        FDescriptorSetsLinkInfo BufferLinkInfo = { };
        BufferLinkInfo.DescriptorCount = 1;
        BufferLinkInfo.ResourceHandle.TextureHandle = HDRTex;
        BufferLinkInfo.BindingStart = 1;
        BufferLinkInfo.ArrayElementStart = 0;
        BufferLinkInfo.TextureSampler = SamplerHandle;
        HDRDescSet->LinkToTexture(0, BufferLinkInfo);
    }
}

uint32 Renderer::ConvertBGRA_To_RGBA(uint32 inBGRA)
{
    uint32 B = (inBGRA & 0x000000ff);
    uint32 G = (inBGRA & 0x0000ff00) >> 8;
    uint32 R = (inBGRA & 0x00ff0000) >> 16;
    uint32 A = (inBGRA & 0xff000000) >> 24;

    uint32 NewPixel = R;
    NewPixel |= (G << 8);
    NewPixel |= (B << 16);
    NewPixel |= (A << 24);

    return NewPixel;
}

void Renderer::CreateCubemap(const char* inCubeMapName, const IPipeline* inPipeline, PipelineLayout* inPipelineLayout, IDescriptorSets* inDescriptorSet, float inViewportSize)
{
    {
        // to do that we have to render the scene 6 times at different 90 degrees angles 
        // capturing all of the cubemap faces x, -x, y, -y, z, -z
        // Let do it 

        ProjectionMatrix4D CaptureProjection = ProjectionMatrix4D::MakeProjectionVulkanLH(1, 90.0f, 0.01f, 10.0f, false);
        Matrix4D ViewMatrices[6] =
        {
            //Matrix4D::LookAt(Vector3D(0.0f), Vector3D(1.0f,  0.0f,  0.0f),  Vector3D(0.0f, -1.0f,  0.0f)),
            //Matrix4D::LookAt(Vector3D(0.0f), Vector3D(-1.0f,  0.0f,  0.0f), Vector3D(0.0f, -1.0f,  0.0f)),
            //Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  -1.0f,  0.0f), Vector3D(0.0f,  0.0f,  -1.0f)), // Y-
            //Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  1.0f,  0.0f),  Vector3D(0.0f,  0.0f,  1.0f)),
            //Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  0.0f,  1.0f),  Vector3D(0.0f, -1.0f,  0.0f)),
            //Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  0.0f,  -1.0f), Vector3D(0.0f, -1.0f,  0.0f)),

            Matrix4D::LookAt(Vector3D(0), Vector3D(0.0f, 0.0f, 1.0f),  Vector3D(0.0f, 1.0f, 0.0f)),   // right
            Matrix4D::LookAt(Vector3D(0), Vector3D(0.0f, 0.0f, -1.0f), Vector3D(0.0f, 1.0f, 0.0f)),  // left
            Matrix4D::LookAt(Vector3D(0), Vector3D(1.0f, 0.0f, 0.0f),  Vector3D(0.0f, 0.0f, 1.0f)),   // up
            Matrix4D::LookAt(Vector3D(0), Vector3D(1.0f, 0.0f, 0.0f),  Vector3D(0.0f, 0.0f, -1.0f)),   // down
            Matrix4D::LookAt(Vector3D(0), Vector3D(1.0f, 0.0f, 0.0f),  Vector3D(0.0f, 1.0f, 0.0f)),  // front
            Matrix4D::LookAt(Vector3D(0), Vector3D(-1.0f, 0.0f, 0.0f), Vector3D(0.0f, 1.0f, 0.0f)),   // back 
        };

        // Create the texture that we will be attching to the framebuffer
        FTextureConfig TextureConfig = { };
        TextureConfig.Type = ETextureType::Texture2D;
        TextureConfig.BindFlags = FResourceBindFlags::ColorAttachment | FResourceBindFlags::SrcTransfer | FResourceBindFlags::Sampled;
        TextureConfig.CreationFlags = FResourceCreationFlags::Mutable;
        TextureConfig.Format = EPixelFormat::BGRA8UNorm;
        TextureConfig.Extent = { Application::Get()->GetWindow().GetWidth(), Application::Get()->GetWindow().GetHeight(), 1 };
        TextureConfig.MipLevels = 1;
        TextureConfig.NumArrayLayers = 1;
        TextureConfig.NumSamples = 1;

        Texture* FrameBufferTexture = RenderInterface.Get()->CreateTexture(TextureConfig);

        // Create the frame buffer with the texture we would like to use 
        FFrameBufferConfig FrameBufferConfig = { };
        FrameBufferConfig.RenderPass = RenderPass;
        FrameBufferConfig.Resolution = { SwapChainMain->GetScreenWidth(), SwapChainMain->GetScreenHeight() };

        FFrameBufferAttachment FrameBufferAttachment = { };
        FrameBufferAttachment.Attachment = FrameBufferTexture;
        FrameBufferConfig.Attachments.push_back(FrameBufferAttachment);
        FrameBufferAttachment.Attachment = DepthStencilView;
        FrameBufferConfig.Attachments.push_back(FrameBufferAttachment);

        IFrameBuffer* FrameBuffer = RenderInterface.Get()->CreateFrameBuffer(FrameBufferConfig);

        // Create a command buffer that we will use to encode commands with 
        FCommandBufferConfig CommandBufferConfig = { };
        CommandBufferConfig.CommandQueue = RenderInterface.Get()->GetCommandQueue();
        CommandBufferConfig.NumBuffersToAllocate = 1;
        CommandBufferConfig.Flags = FCommandBufferLevelFlags::Primary;

        ICommandBuffer* CommandBuffer = RenderInterface.Get()->CreateCommandBuffer(CommandBufferConfig);

        // Pre defined clear values 
        FRenderPassBeginInfo RPBeginInfo = { };
        FRenderClearValues ClearValues[2];
        ClearValues[0].Color = { 0.0f, 0.0f, 0.2f,  1.0f };
        ClearValues[0].Depth = { 1.0f };
        ClearValues[0].Stencil = { 0 };

        RPBeginInfo.ClearValues = ClearValues;
        RPBeginInfo.NumClearValues = 2;

        RPBeginInfo.RenderPassPtr = RenderPass;
        RPBeginInfo.FrameBuffer = FrameBuffer;

        FRenderViewport Viewport[1];
        // Set the main render viewport
        Viewport[0].X = 0.0f;

        // Since we flipped the viewport height, we now have to move up to the screen or else
        // we will be seeing a screen that is not being renderered 
        Viewport[0].Y = inViewportSize;

        Viewport[0].MinDepth = 0.0f;
        Viewport[0].MaxDepth = 1.0f;

        Viewport[0].Width = inViewportSize;
        // Since vulkan has a coordinate system where Y points down we have to flip the frame or viewports around the center
        Viewport[0].Height = -inViewportSize;

        FRenderScissor Scissor[1] = { };
        Scissor[0].OffsetX = 0;
        Scissor[0].OffsetY = 0;
        Scissor[0].Width = inViewportSize;
        Scissor[0].Height = inViewportSize;

        ktxTexture2* KtxTextureHandle = nullptr;
        ktxTextureCreateInfo KtxTextureCreateInfo = { };
        KTX_error_code KtxResult;

        uint32 NumLevels = std::log2(inViewportSize) + 1;
        uint32 NumFaceSlices = 6;

        uint32 BaseWidth = inViewportSize;
        uint32 BaseHeight = inViewportSize;

        KtxTextureCreateInfo.vkFormat = VK_FORMAT_B8G8R8A8_UNORM;
        KtxTextureCreateInfo.baseWidth = inViewportSize;
        KtxTextureCreateInfo.baseHeight = inViewportSize;
        KtxTextureCreateInfo.baseDepth = 1;
        KtxTextureCreateInfo.numDimensions = 2;
        // Note: it is not necessary to provide a full mipmap pyramid.
        KtxTextureCreateInfo.numLevels = NumLevels;
        KtxTextureCreateInfo.numLayers = 1;
        KtxTextureCreateInfo.numFaces = NumFaceSlices;
        KtxTextureCreateInfo.isArray = KTX_FALSE;
        KtxTextureCreateInfo.generateMipmaps = KTX_FALSE;

        KtxResult = ktxTexture2_Create(&KtxTextureCreateInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &KtxTextureHandle);

        // Capture all 6 sides of the cubemap 
        for (uint32 faceSlice = 0; faceSlice < NumFaceSlices; ++faceSlice)
        {
            for (uint32 mipLevel = 0; mipLevel < NumLevels; ++mipLevel)
            {
                int32 TextureWidth = BaseWidth >> mipLevel;
                int32 TextureHeight = BaseHeight >> mipLevel;

                Viewport[0].Y = TextureHeight;
                Viewport[0].Width = TextureWidth;
                // Since vulkan has a coordinate system where Y points down we have to flip the frame or viewports around the center
                Viewport[0].Height = -TextureHeight;

                Scissor[0].Width = TextureWidth;
                Scissor[0].Height = TextureHeight;

                RenderInterface.Get()->GetCommandQueue()->SetWaitFence(CommandBuffer->GetWaitFence(), UINT64_MAX);

                CommandBuffer->Begin();
                CommandBuffer->BeginRenderPass(RPBeginInfo);

                CommandBuffer->SetRenderViewports(Viewport, 1);
                CommandBuffer->SetRenderScissors(Scissor, 1);

                CommandBuffer->BindPipeline(inPipeline);

                {
                    UniformBufferLocalConstants UniformData = { };
                    UniformData.Eye = Vector4D(CameraTranslation, 1.0f);
                    UniformData.Matrix = GlobalMatrix;
                    UniformData.ViewProjection = (ViewMatrices[faceSlice] * (Matrix4D&)CaptureProjection);
                    UniformData.Light = LightPosition.ToVector3D();

                    RenderInterface.Get()->WriteToBuffer(LocalConstantsBuffer, 0, &UniformData, UniformBufferLocalConstants::GetStaticSize());
                }

                CommandBuffer->SetVertexBuffer(*CubeVertexBuffer);

                FDescriptorSetsBindInfo DescriptorSetsBindInfo = { };
                DescriptorSetsBindInfo.DescriptorSets = inDescriptorSet;
                DescriptorSetsBindInfo.NumSets = 1;
                DescriptorSetsBindInfo.PipelineBindPoint = EPipelineBindPoint::Graphics;
                DescriptorSetsBindInfo.PipelineLayoutPtr = inPipelineLayout;
                CommandBuffer->BindDescriptorSets(DescriptorSetsBindInfo);

                CommandBuffer->Draw(36);

                CommandBuffer->EndRenderPass();
                CommandBuffer->End();

                RenderInterface.Get()->GetCommandQueue()->ResetWaitFence(CommandBuffer->GetWaitFence());
                RenderInterface.Get()->GetCommandQueue()->Submit(CommandBuffer, 0, nullptr);
                RenderInterface.Get()->GetCommandQueue()->SetWaitIdle();

                // Read from texture 
                FTextureReadInfo TextureReadInfo = { };
                {
                    FTextureSection TextureSection = { };
                    TextureSection.Extent = { (uint32)TextureWidth, (uint32)TextureHeight, 1u };
                    TextureSection.Offset = { 0, 0, 0 };

                    TextureSection.Subresource.BaseArrayLayer = 0;
                    TextureSection.Subresource.NumArrayLayers = 1;
                    TextureSection.Subresource.NumMipLevels = 1;
                    TextureSection.Subresource.BaseMipLevel = 0;

                    RenderInterface.Get()->ReadFromTexture(FrameBufferTexture, TextureSection, ETextureLayout::ColorAttachment, TextureReadInfo);

                    uint32* TextureData = (uint32*)TextureReadInfo.Data;
                    uint32 TextureDataSize = TextureReadInfo.SizeInByte / 4;

                    // Convert from BGRA to RGBA
                    // also flip images horizontally
                    //for (int32 j = 0; j < inViewportSize; ++j)
                    //{
                    //    // mirror the line at row j
                    //    for (int32 i = 1; i <= TextureWidth / 2; i++)
                    //    {
                    //        // swap the left side pixel with the right side pixel
                    //        int32 MirrorPixelIndex = ((TextureWidth * j) + (TextureWidth - i));
                    //        int32 CurrentPixelIndex = (TextureWidth * j) + i - 1;
                    //
                    //        uint32 Temp = TextureData[MirrorPixelIndex];  // temp = right side pixel
                    //        TextureData[MirrorPixelIndex] = (TextureData[CurrentPixelIndex]);     // right side pixel = left side pixel
                    //        //TextureData[MirrorPixelIndex] = ConvertBGRA_To_RGBA(TextureData[CurrentPixelIndex]);     // right side pixel = left side pixel
                    //        TextureData[CurrentPixelIndex] = (Temp);                    // left side pixel gets original right pixel value
                    //        //TextureData[CurrentPixelIndex] = ConvertBGRA_To_RGBA(Temp);                    // left side pixel gets original right pixel value
                    //    }
                    //}
                }

                ktxTexture_SetImageFromMemory(ktxTexture(KtxTextureHandle), mipLevel, 0, faceSlice, (const uint8*)TextureReadInfo.Data, TextureReadInfo.SizeInByte);
            }
        }

        std::string FilePath = "../Assets/Textures/";
        FilePath += inCubeMapName;
        FilePath += ".ktx";

        ktxTexture_WriteToNamedFile(ktxTexture(KtxTextureHandle), FilePath.c_str());
        ktxTexture_Destroy(ktxTexture(KtxTextureHandle));

        // Clean up memory
        {
            delete FrameBuffer;
            delete FrameBufferTexture;
            //delete[] CubeMapData;
        }
    }
}

void Renderer::CreateIrradianceMap(const char* inCubeMapName, float inViewportSize)
{
    {
        // to do that we have to render the scene 6 times at different 90 degrees angles 
        // capturing all of the cubemap faces x, -x, y, -y, z, -z
        // Let do it 

        ProjectionMatrix4D CaptureProjection = ProjectionMatrix4D::MakeProjectionVulkanLH(1, 90.0f, 0.01f, 10.0f, false);
        Matrix4D ViewMatrices[6] =
        {
            //Matrix4D::LookAt(Vector3D(0.0f), Vector3D(1.0f,  0.0f,  0.0f),  Vector3D(0.0f, -1.0f,  0.0f)),
            //Matrix4D::LookAt(Vector3D(0.0f), Vector3D(-1.0f,  0.0f,  0.0f), Vector3D(0.0f, -1.0f,  0.0f)),
            //Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  -1.0f,  0.0f), Vector3D(0.0f,  0.0f,  -1.0f)), // Y-
            //Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  1.0f,  0.0f),  Vector3D(0.0f,  0.0f,  1.0f)),
            //Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  0.0f,  1.0f),  Vector3D(0.0f, -1.0f,  0.0f)),
            //Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  0.0f,  -1.0f), Vector3D(0.0f, -1.0f,  0.0f)),

            Matrix4D::LookAt(Vector3D(0), Vector3D(0.0f, 0.0f, 1.0f),  Vector3D(0.0f, 1.0f, 0.0f)),   // right
            Matrix4D::LookAt(Vector3D(0), Vector3D(0.0f, 0.0f, -1.0f), Vector3D(0.0f, 1.0f, 0.0f)),  // left
            Matrix4D::LookAt(Vector3D(0), Vector3D(1.0f, 0.0f, 0.0f),  Vector3D(0.0f, 0.0f, 1.0f)),   // up
            Matrix4D::LookAt(Vector3D(0), Vector3D(1.0f, 0.0f, 0.0f),  Vector3D(0.0f, 0.0f, -1.0f)),   // down
            Matrix4D::LookAt(Vector3D(0), Vector3D(1.0f, 0.0f, 0.0f),  Vector3D(0.0f, 1.0f, 0.0f)),  // front
            Matrix4D::LookAt(Vector3D(0), Vector3D(-1.0f, 0.0f, 0.0f), Vector3D(0.0f, 1.0f, 0.0f)),   // back 
        };

        // Create the texture that we will be attching to the framebuffer
        FTextureConfig TextureConfig = { };
        TextureConfig.Type = ETextureType::Texture2D;
        TextureConfig.BindFlags = FResourceBindFlags::ColorAttachment | FResourceBindFlags::SrcTransfer | FResourceBindFlags::Sampled;
        TextureConfig.CreationFlags = FResourceCreationFlags::Mutable;
        TextureConfig.Format = EPixelFormat::BGRA8UNorm;
        TextureConfig.Extent = { Application::Get()->GetWindow().GetWidth(), Application::Get()->GetWindow().GetHeight(), 1 };
        TextureConfig.MipLevels = 1;
        TextureConfig.NumArrayLayers = 1;
        TextureConfig.NumSamples = 1;

        Texture* FrameBufferTexture = RenderInterface.Get()->CreateTexture(TextureConfig);

        // Create the frame buffer with the texture we would like to use 
        FFrameBufferConfig FrameBufferConfig = { };
        FrameBufferConfig.RenderPass = RenderPass;
        FrameBufferConfig.Resolution = { SwapChainMain->GetScreenWidth(), SwapChainMain->GetScreenHeight() };

        FFrameBufferAttachment FrameBufferAttachment = { };
        FrameBufferAttachment.Attachment = FrameBufferTexture;
        FrameBufferConfig.Attachments.push_back(FrameBufferAttachment);
        FrameBufferAttachment.Attachment = DepthStencilView;
        FrameBufferConfig.Attachments.push_back(FrameBufferAttachment);

        IFrameBuffer* FrameBuffer = RenderInterface.Get()->CreateFrameBuffer(FrameBufferConfig);

        // Create a command buffer that we will use to encode commands with 
        FCommandBufferConfig CommandBufferConfig = { };
        CommandBufferConfig.CommandQueue = RenderInterface.Get()->GetCommandQueue();
        CommandBufferConfig.NumBuffersToAllocate = 1;
        CommandBufferConfig.Flags = FCommandBufferLevelFlags::Primary;

        ICommandBuffer* CommandBuffer = RenderInterface.Get()->CreateCommandBuffer(CommandBufferConfig);

        // Pre defined clear values 
        FRenderPassBeginInfo RPBeginInfo = { };
        FRenderClearValues ClearValues[2];
        ClearValues[0].Color = { 0.0f, 0.0f, 0.2f,  1.0f };
        ClearValues[0].Depth = { 1.0f };
        ClearValues[0].Stencil = { 0 };

        RPBeginInfo.ClearValues = ClearValues;
        RPBeginInfo.NumClearValues = 2;

        RPBeginInfo.RenderPassPtr = RenderPass;
        RPBeginInfo.FrameBuffer = FrameBuffer;

        FRenderViewport Viewport[1];
        // Set the main render viewport
        Viewport[0].X = 0.0f;

        // Since we flipped the viewport height, we now have to move up to the screen or else
        // we will be seeing a screen that is not being renderered 
        Viewport[0].Y = inViewportSize;

        Viewport[0].MinDepth = 0.0f;
        Viewport[0].MaxDepth = 1.0f;

        Viewport[0].Width = inViewportSize;
        // Since vulkan has a coordinate system where Y points down we have to flip the frame or viewports around the center
        Viewport[0].Height = -inViewportSize;

        FRenderScissor Scissor[1] = { };
        Scissor[0].OffsetX = 0;
        Scissor[0].OffsetY = 0;
        Scissor[0].Width = inViewportSize;
        Scissor[0].Height = inViewportSize;

        ktxTexture2* KtxTextureHandle = nullptr;
        ktxTextureCreateInfo KtxTextureCreateInfo = { };
        KTX_error_code KtxResult;

        uint32 NumLevels = 1;// std::log2(inViewportSize) + 1;
        uint32 NumFaceSlices = 6;

        uint32 BaseWidth = inViewportSize;
        uint32 BaseHeight = inViewportSize;

        KtxTextureCreateInfo.vkFormat = VK_FORMAT_B8G8R8A8_UNORM;
        KtxTextureCreateInfo.baseWidth = inViewportSize;
        KtxTextureCreateInfo.baseHeight = inViewportSize;
        KtxTextureCreateInfo.baseDepth = 1;
        KtxTextureCreateInfo.numDimensions = 2;
        // Note: it is not necessary to provide a full mipmap pyramid.
        KtxTextureCreateInfo.numLevels = NumLevels;
        KtxTextureCreateInfo.numLayers = 1;
        KtxTextureCreateInfo.numFaces = NumFaceSlices;
        KtxTextureCreateInfo.isArray = KTX_FALSE;
        KtxTextureCreateInfo.generateMipmaps = KTX_FALSE;

        KtxResult = ktxTexture2_Create(&KtxTextureCreateInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &KtxTextureHandle);

        // Capture all 6 sides of the cubemap 
        for (uint32 faceSlice = 0; faceSlice < NumFaceSlices; ++faceSlice)
        {
            for (uint32 mipLevel = 0; mipLevel < NumLevels; ++mipLevel)
            {
                int32 TextureWidth = BaseWidth >> mipLevel;
                int32 TextureHeight = BaseHeight >> mipLevel;

                Viewport[0].Y = TextureHeight;
                Viewport[0].Width = TextureWidth;
                // Since vulkan has a coordinate system where Y points down we have to flip the frame or viewports around the center
                Viewport[0].Height = -TextureHeight;

                Scissor[0].Width = TextureWidth;
                Scissor[0].Height = TextureHeight;

                RenderInterface.Get()->GetCommandQueue()->SetWaitFence(CommandBuffer->GetWaitFence(), UINT64_MAX);

                CommandBuffer->Begin();
                CommandBuffer->BeginRenderPass(RPBeginInfo);

                CommandBuffer->SetRenderViewports(Viewport, 1);
                CommandBuffer->SetRenderScissors(Scissor, 1);

                CommandBuffer->BindPipeline(IrridiancePipeline);

                {
                    UniformBufferLocalConstants UniformData = { };
                    UniformData.Eye = Vector4D(CameraTranslation, 1.0f);
                    UniformData.Matrix = GlobalMatrix;
                    UniformData.ViewProjection = (ViewMatrices[faceSlice] * (Matrix4D&)CaptureProjection);
                    UniformData.Light = LightPosition.ToVector3D();

                    RenderInterface.Get()->WriteToBuffer(LocalConstantsBuffer, 0, &UniformData, UniformBufferLocalConstants::GetStaticSize());
                }

                CommandBuffer->SetVertexBuffer(*CubeVertexBuffer);

                FDescriptorSetsBindInfo DescriptorSetsBindInfo = { };
                DescriptorSetsBindInfo.DescriptorSets = IrridianceDescSet;
                DescriptorSetsBindInfo.NumSets = 1;
                DescriptorSetsBindInfo.PipelineBindPoint = EPipelineBindPoint::Graphics;
                DescriptorSetsBindInfo.PipelineLayoutPtr = HDRPipelineLayout;
                CommandBuffer->BindDescriptorSets(DescriptorSetsBindInfo);

                CommandBuffer->Draw(36);

                CommandBuffer->EndRenderPass();
                CommandBuffer->End();

                RenderInterface.Get()->GetCommandQueue()->ResetWaitFence(CommandBuffer->GetWaitFence());
                RenderInterface.Get()->GetCommandQueue()->Submit(CommandBuffer, 0, nullptr);
                RenderInterface.Get()->GetCommandQueue()->SetWaitIdle();

                // Read from texture 
                FTextureReadInfo TextureReadInfo = { };
                {
                    FTextureSection TextureSection = { };
                    TextureSection.Extent = { (uint32)TextureWidth, (uint32)TextureHeight, 1u };
                    TextureSection.Offset = { 0, 0, 0 };

                    TextureSection.Subresource.BaseArrayLayer = 0;
                    TextureSection.Subresource.NumArrayLayers = 1;
                    TextureSection.Subresource.NumMipLevels = 1;
                    TextureSection.Subresource.BaseMipLevel = 0;

                    RenderInterface.Get()->ReadFromTexture(FrameBufferTexture, TextureSection, ETextureLayout::ColorAttachment, TextureReadInfo);
                }

                ktxTexture_SetImageFromMemory(ktxTexture(KtxTextureHandle), mipLevel, 0, faceSlice, (const uint8*)TextureReadInfo.Data, TextureReadInfo.SizeInByte);
            }
        }

        std::string FilePath = FilePathToResources;
        FilePath += "Textures/";
        FilePath += inCubeMapName;
        FilePath += "IrradianceMap.ktx";

        ktxTexture_WriteToNamedFile(ktxTexture(KtxTextureHandle), FilePath.c_str());
        ktxTexture_Destroy(ktxTexture(KtxTextureHandle));

        // Clean up memory
        {
            delete FrameBuffer;
            delete FrameBufferTexture;
        }
    }
}

void Renderer::CreateIrradianceMap(CSkybox* inSkyboxAsset, float inViewportSize)
{
    FDescriptorSetsLinkInfo LinkInfo = { };
    LinkInfo.ArrayElementStart = 0;
    LinkInfo.DescriptorCount = 1;
    LinkInfo.BindingStart = 1;
    LinkInfo.ResourceHandle.TextureHandle = inSkyboxAsset->GetCubemapTexture();
    LinkInfo.TextureSampler = SamplerHandle;

    IrridianceDescSet->LinkToTexture(0, LinkInfo);
    CreateIrradianceMap(inSkyboxAsset->GetName().c_str(), 512.0f);
}

void Renderer::CreateCubemapFromHighDynamicImage(const char* inCubeMapName, float inViewportSize)
{
    {
        // to do that we have to render the scene 6 times at different 90 degrees angles 
        // capturing all of the cubemap faces x, -x, y, -y, z, -z
        // Let do it 

        ProjectionMatrix4D CaptureProjection = ProjectionMatrix4D::MakeProjectionVulkanLH(1, 90.0f, 0.01f, 10.0f, true);
        Matrix4D ViewMatrices[6] =
        {
            Matrix4D::LookAt(Vector3D(0), Vector3D(0.0f, 0.0f, 1.0f),  Vector3D(0.0f, 1.0f, 0.0f)),   // right
            Matrix4D::LookAt(Vector3D(0), Vector3D(0.0f, 0.0f, -1.0f), Vector3D(0.0f, 1.0f, 0.0f)),  // left
            Matrix4D::LookAt(Vector3D(0), Vector3D(1.0f, 0.0f, 0.0f),  Vector3D(0.0f, 0.0f, 1.0f)),   // up
            Matrix4D::LookAt(Vector3D(0), Vector3D(1.0f, 0.0f, 0.0f),  Vector3D(0.0f, 0.0f, -1.0f)),   // down
            Matrix4D::LookAt(Vector3D(0), Vector3D(1.0f, 0.0f, 0.0f),  Vector3D(0.0f, 1.0f, 0.0f)),  // front
            Matrix4D::LookAt(Vector3D(0), Vector3D(-1.0f, 0.0f, 0.0f), Vector3D(0.0f, 1.0f, 0.0f)),   // back 
        };

        // Create the texture that we will be attching to the framebuffer
        FTextureConfig TextureConfig = { };
        TextureConfig.Type = ETextureType::Texture2D;
        TextureConfig.BindFlags = FResourceBindFlags::ColorAttachment | FResourceBindFlags::SrcTransfer | FResourceBindFlags::Sampled;
        TextureConfig.CreationFlags = FResourceCreationFlags::Mutable;
        TextureConfig.Format = EPixelFormat::BGRA8UNorm;
        TextureConfig.Extent = { Application::Get()->GetWindow().GetWidth(), Application::Get()->GetWindow().GetHeight(), 1 };
        TextureConfig.MipLevels = 1;
        TextureConfig.NumArrayLayers = 1;
        TextureConfig.NumSamples = 1;

        Texture* FrameBufferTexture = RenderInterface.Get()->CreateTexture(TextureConfig);

        // Create the frame buffer with the texture we would like to use 
        FFrameBufferConfig FrameBufferConfig = { };
        FrameBufferConfig.RenderPass = RenderPass;
        FrameBufferConfig.Resolution = { SwapChainMain->GetScreenWidth(), SwapChainMain->GetScreenHeight() };

        FFrameBufferAttachment FrameBufferAttachment = { };
        FrameBufferAttachment.Attachment = FrameBufferTexture;
        FrameBufferConfig.Attachments.push_back(FrameBufferAttachment);
        FrameBufferAttachment.Attachment = DepthStencilView;
        FrameBufferConfig.Attachments.push_back(FrameBufferAttachment);

        IFrameBuffer* FrameBuffer = RenderInterface.Get()->CreateFrameBuffer(FrameBufferConfig);

        // Create a command buffer that we will use to encode commands with 
        FCommandBufferConfig CommandBufferConfig = { };
        CommandBufferConfig.CommandQueue = RenderInterface.Get()->GetCommandQueue();
        CommandBufferConfig.NumBuffersToAllocate = 1;
        CommandBufferConfig.Flags = FCommandBufferLevelFlags::Primary;

        ICommandBuffer* CommandBuffer = RenderInterface.Get()->CreateCommandBuffer(CommandBufferConfig);

        // Pre defined clear values 
        FRenderPassBeginInfo RPBeginInfo = { };
        FRenderClearValues ClearValues[2];
        ClearValues[0].Color = { 0.0f, 0.0f, 0.2f,  1.0f };
        ClearValues[0].Depth = { 1.0f };
        ClearValues[0].Stencil = { 0 };

        RPBeginInfo.ClearValues = ClearValues;
        RPBeginInfo.NumClearValues = 2;

        RPBeginInfo.RenderPassPtr = RenderPass;
        RPBeginInfo.FrameBuffer = FrameBuffer;

        FRenderViewport Viewport[1];
        // Set the main render viewport
        Viewport[0].X = 0.0f;

        // Since we flipped the viewport height, we now have to move up to the screen or else
        // we will be seeing a screen that is not being renderered 
        Viewport[0].Y = inViewportSize;

        Viewport[0].MinDepth = 0.0f;
        Viewport[0].MaxDepth = 1.0f;

        Viewport[0].Width = inViewportSize;
        // Since vulkan has a coordinate system where Y points down we have to flip the frame or viewports around the center
        Viewport[0].Height = -inViewportSize;

        FRenderScissor Scissor[1] = { };
        Scissor[0].OffsetX = 0;
        Scissor[0].OffsetY = 0;
        Scissor[0].Width = inViewportSize;
        Scissor[0].Height = inViewportSize;

        ktxTexture2* KtxTextureHandle = nullptr;
        ktxTextureCreateInfo KtxTextureCreateInfo = { };
        KTX_error_code KtxResult;

        uint32 NumLevels = std::log2(inViewportSize) + 1;
        uint32 NumFaceSlices = 6;

        uint32 BaseWidth = inViewportSize;
        uint32 BaseHeight = inViewportSize;

        KtxTextureCreateInfo.vkFormat = VK_FORMAT_B8G8R8A8_UNORM;
        KtxTextureCreateInfo.baseWidth = inViewportSize;
        KtxTextureCreateInfo.baseHeight = inViewportSize;
        KtxTextureCreateInfo.baseDepth = 1;
        KtxTextureCreateInfo.numDimensions = 2;
        // Note: it is not necessary to provide a full mipmap pyramid.
        KtxTextureCreateInfo.numLevels = NumLevels;
        KtxTextureCreateInfo.numLayers = 1;
        KtxTextureCreateInfo.numFaces = NumFaceSlices;
        KtxTextureCreateInfo.isArray = KTX_FALSE;
        KtxTextureCreateInfo.generateMipmaps = KTX_FALSE;

        KtxResult = ktxTexture2_Create(&KtxTextureCreateInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &KtxTextureHandle);

        // Capture all 6 sides of the cubemap 
        for (uint32 faceSlice = 0; faceSlice < NumFaceSlices; ++faceSlice)
        {
            int32 CurrentFaceSlice = faceSlice;
            if (faceSlice == 2)
            {
                CurrentFaceSlice = 3;
            }
            else if (faceSlice == 3)
            {
                CurrentFaceSlice = 2;
            }

            for (uint32 mipLevel = 0; mipLevel < NumLevels; ++mipLevel)
            {
                int32 TextureWidth = BaseWidth >> mipLevel;
                int32 TextureHeight = BaseHeight >> mipLevel;

                Viewport[0].Y = TextureHeight;
                Viewport[0].Width = TextureWidth;
                // Since vulkan has a coordinate system where Y points down we have to flip the frame or viewports around the center
                Viewport[0].Height = -TextureHeight;

                Scissor[0].Width = TextureWidth;
                Scissor[0].Height = TextureHeight;

                RenderInterface.Get()->GetCommandQueue()->SetWaitFence(CommandBuffer->GetWaitFence(), UINT64_MAX);

                CommandBuffer->Begin();
                CommandBuffer->BeginRenderPass(RPBeginInfo);

                CommandBuffer->SetRenderViewports(Viewport, 1);
                CommandBuffer->SetRenderScissors(Scissor, 1);

                CommandBuffer->BindPipeline(HDRPipeline);

                {
                    UniformBufferLocalConstants UniformData = { };
                    UniformData.Eye = Vector4D(CameraTranslation, 1.0f);
                    UniformData.Matrix = GlobalMatrix;
                    UniformData.ViewProjection = (ViewMatrices[CurrentFaceSlice] * (Matrix4D&)CaptureProjection);
                    UniformData.Light = LightPosition.ToVector3D();

                    HDRBufferData Data = { CurrentFaceSlice };

                    RenderInterface.Get()->WriteToBuffer(HDRConstantsBuffer, 0, &Data, sizeof(HDRBufferData));
                }

                CommandBuffer->SetVertexBuffer(*CubeVertexBuffer);

                FDescriptorSetsBindInfo DescriptorSetsBindInfo = { };
                DescriptorSetsBindInfo.DescriptorSets = HDRDescSet;
                DescriptorSetsBindInfo.NumSets = 1;
                DescriptorSetsBindInfo.PipelineBindPoint = EPipelineBindPoint::Graphics;
                DescriptorSetsBindInfo.PipelineLayoutPtr = HDRPipelineLayout;
                CommandBuffer->BindDescriptorSets(DescriptorSetsBindInfo);

                CommandBuffer->Draw(36);

                CommandBuffer->EndRenderPass();
                CommandBuffer->End();

                RenderInterface.Get()->GetCommandQueue()->ResetWaitFence(CommandBuffer->GetWaitFence());
                RenderInterface.Get()->GetCommandQueue()->Submit(CommandBuffer, 0, nullptr);
                RenderInterface.Get()->GetCommandQueue()->SetWaitIdle();

                // Read from texture 
                FTextureReadInfo TextureReadInfo = { };
                {
                    FTextureSection TextureSection = { };
                    TextureSection.Extent = { (uint32)TextureWidth, (uint32)TextureHeight, 1u };
                    TextureSection.Offset = { 0, 0, 0 };

                    TextureSection.Subresource.BaseArrayLayer = 0;
                    TextureSection.Subresource.NumArrayLayers = 1;
                    TextureSection.Subresource.NumMipLevels = 1;
                    TextureSection.Subresource.BaseMipLevel = 0;

                    RenderInterface.Get()->ReadFromTexture(FrameBufferTexture, TextureSection, ETextureLayout::ColorAttachment, TextureReadInfo);
                }

                ktxTexture_SetImageFromMemory(ktxTexture(KtxTextureHandle), mipLevel, 0, CurrentFaceSlice, (const uint8*)TextureReadInfo.Data, TextureReadInfo.SizeInByte);
            }
        }

        std::string FilePath = "../Assets/Textures/";
        FilePath += inCubeMapName;
        FilePath += ".ktx";

        ktxTexture_WriteToNamedFile(ktxTexture(KtxTextureHandle), FilePath.c_str());
        ktxTexture_Destroy(ktxTexture(KtxTextureHandle));

        // Clean up memory
        {
            delete FrameBuffer;
            delete FrameBufferTexture;
        }
    }
}

void Renderer::CreatePrefilterEnvMap(const char* inCubeMapName, float inViewportSize)
{
    {
        // to do that we have to render the scene 6 times at different 90 degrees angles 
        // capturing all of the cubemap faces x, -x, y, -y, z, -z
        // Let do it 

        ProjectionMatrix4D CaptureProjection = ProjectionMatrix4D::MakeProjectionVulkanLH(1, 90.0f, 0.01f, 10.0f, false);
        Matrix4D ViewMatrices[6] =
        {
            Matrix4D::LookAt(Vector3D(0.0f), Vector3D(1.0f,  0.0f,  0.0f),  Vector3D(0.0f, -1.0f,  0.0f)),
            Matrix4D::LookAt(Vector3D(0.0f), Vector3D(-1.0f,  0.0f,  0.0f), Vector3D(0.0f, -1.0f,  0.0f)),
            Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  1.0f,  0.0f),  Vector3D(0.0f,  0.0f,  1.0f)),
            Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  -1.0f,  0.0f), Vector3D(0.0f,  0.0f,  -1.0f)), // Y-
            Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  0.0f,  1.0f),  Vector3D(0.0f, -1.0f,  0.0f)),
            Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  0.0f,  -1.0f), Vector3D(0.0f, -1.0f,  0.0f)),
        };

        // Create the texture that we will be attching to the framebuffer
        FTextureConfig TextureConfig = { };
        TextureConfig.Type = ETextureType::Texture2D;
        TextureConfig.BindFlags = FResourceBindFlags::ColorAttachment | FResourceBindFlags::SrcTransfer | FResourceBindFlags::Sampled;
        TextureConfig.CreationFlags = FResourceCreationFlags::Mutable;
        TextureConfig.Format = EPixelFormat::BGRA8UNorm;
        TextureConfig.Extent = { Application::Get()->GetWindow().GetWidth(), Application::Get()->GetWindow().GetHeight(), 1 };
        TextureConfig.MipLevels = 1;
        TextureConfig.NumArrayLayers = 1;
        TextureConfig.NumSamples = 1;

        Texture* FrameBufferTexture = RenderInterface.Get()->CreateTexture(TextureConfig);

        // Create the frame buffer with the texture we would like to use 
        FFrameBufferConfig FrameBufferConfig = { };
        FrameBufferConfig.RenderPass = RenderPass;
        FrameBufferConfig.Resolution = { SwapChainMain->GetScreenWidth(), SwapChainMain->GetScreenHeight() };

        FFrameBufferAttachment FrameBufferAttachment = { };
        FrameBufferAttachment.Attachment = FrameBufferTexture;
        FrameBufferConfig.Attachments.push_back(FrameBufferAttachment);
        FrameBufferAttachment.Attachment = DepthStencilView;
        FrameBufferConfig.Attachments.push_back(FrameBufferAttachment);

        IFrameBuffer* FrameBuffer = RenderInterface.Get()->CreateFrameBuffer(FrameBufferConfig);

        // Create a command buffer that we will use to encode commands with 
        FCommandBufferConfig CommandBufferConfig = { };
        CommandBufferConfig.CommandQueue = RenderInterface.Get()->GetCommandQueue();
        CommandBufferConfig.NumBuffersToAllocate = 1;
        CommandBufferConfig.Flags = FCommandBufferLevelFlags::Primary;

        ICommandBuffer* CommandBuffer = RenderInterface.Get()->CreateCommandBuffer(CommandBufferConfig);

        // Pre defined clear values 
        FRenderPassBeginInfo RPBeginInfo = { };
        FRenderClearValues ClearValues[2];
        ClearValues[0].Color = { 0.0f, 0.0f, 0.2f,  1.0f };
        ClearValues[0].Depth = { 1.0f };
        ClearValues[0].Stencil = { 0 };

        RPBeginInfo.ClearValues = ClearValues;
        RPBeginInfo.NumClearValues = 2;

        RPBeginInfo.RenderPassPtr = RenderPass;
        RPBeginInfo.FrameBuffer = FrameBuffer;

        FRenderViewport Viewport[1];
        // Set the main render viewport
        Viewport[0].X = 0.0f;

        // Since we flipped the viewport height, we now have to move up to the screen or else
        // we will be seeing a screen that is not being renderered 
        Viewport[0].Y = inViewportSize;

        Viewport[0].MinDepth = 0.0f;
        Viewport[0].MaxDepth = 1.0f;

        Viewport[0].Width = inViewportSize;
        // Since vulkan has a coordinate system where Y points down we have to flip the frame or viewports around the center
        Viewport[0].Height = -inViewportSize;

        FRenderScissor Scissor[1] = { };
        Scissor[0].OffsetX = 0;
        Scissor[0].OffsetY = 0;
        Scissor[0].Width = inViewportSize;
        Scissor[0].Height = inViewportSize;

        ktxTexture2* KtxTextureHandle = nullptr;
        ktxTextureCreateInfo KtxTextureCreateInfo = { };
        KTX_error_code KtxResult;

        uint32 NumLevels = std::log2(inViewportSize) + 1;
        uint32 NumFaceSlices = 6;
        std::string Mode = "W";
        //FILE* FileSource = fopen(inCubeMapName, Mode.c_str());
        //uint64 FileSourceSize = 0;

        uint32 BaseWidth = inViewportSize;
        uint32 BaseHeight = inViewportSize;

        KtxTextureCreateInfo.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
        KtxTextureCreateInfo.baseWidth = inViewportSize;
        KtxTextureCreateInfo.baseHeight = inViewportSize;
        KtxTextureCreateInfo.baseDepth = 1;
        KtxTextureCreateInfo.numDimensions = 2;
        // Note: it is not necessary to provide a full mipmap pyramid.
        KtxTextureCreateInfo.numLevels = NumLevels;
        KtxTextureCreateInfo.numLayers = 1;
        KtxTextureCreateInfo.numFaces = NumFaceSlices;
        KtxTextureCreateInfo.isArray = KTX_FALSE;
        KtxTextureCreateInfo.generateMipmaps = KTX_FALSE;

        KtxResult = ktxTexture2_Create(&KtxTextureCreateInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &KtxTextureHandle);

        std::vector<FTextureReadInfo> CubemapFacesInfo;
        CubemapFacesInfo.resize(6 * NumLevels);

        std::string FilePath = FilePathToResources;
        FilePath += "Textures/";
        FilePath += inCubeMapName;
        FilePath += "PrefilteredEnvMap.ktx";

        float RoughnessDelta = 1.0f / ((float)NumLevels - 1);

        for (uint32 faceSlice = 0; faceSlice < NumFaceSlices; ++faceSlice)
        {
            for (uint32 mipLevel = 0; mipLevel < NumLevels; ++mipLevel)
            {
                int32 TextureWidth = BaseWidth >> mipLevel;
                int32 TextureHeight = BaseHeight >> mipLevel;

                Viewport[0].Y = TextureHeight;
                Viewport[0].Width = TextureWidth;
                // Since vulkan has a coordinate system where Y points down we have to flip the frame or viewports around the center
                Viewport[0].Height = -TextureHeight;

                Scissor[0].Width = TextureWidth;
                Scissor[0].Height = TextureHeight;

                // Render First 
                {
                    RenderInterface.Get()->GetCommandQueue()->SetWaitFence(CommandBuffer->GetWaitFence(), UINT64_MAX);

                    CommandBuffer->Begin();
                    CommandBuffer->BeginRenderPass(RPBeginInfo);

                    CommandBuffer->SetRenderViewports(Viewport, 1);
                    CommandBuffer->SetRenderScissors(Scissor, 1);

                    CommandBuffer->BindPipeline(PrefilterEnvMapPipeline);

                    {
                        UniformBufferLocalConstants UniformData = { };
                        UniformData.Eye = Vector4D(CameraTranslation, 1.0f);
                        UniformData.Matrix = GlobalMatrix;
                        UniformData.ViewProjection = (ViewMatrices[faceSlice] * (Matrix4D&)CaptureProjection);
                        UniformData.Light = LightPosition.ToVector3D();

                        RenderInterface.Get()->WriteToBuffer(LocalConstantsBuffer, 0, &UniformData, UniformBufferLocalConstants::GetStaticSize());

                        IBLData Data = { };
                        Data.Roughness = (float)mipLevel / (float)(NumLevels - 1);

                        RenderInterface.Get()->WriteToBuffer(IBLDataBuffer, 0, &Data, sizeof(IBLData));
                    }

                    CommandBuffer->SetVertexBuffer(*CubeVertexBuffer);

                    FDescriptorSetsBindInfo DescriptorSetsBindInfo = { };
                    DescriptorSetsBindInfo.DescriptorSets = PrefilterEnvMapDescSet;
                    DescriptorSetsBindInfo.NumSets = 1;
                    DescriptorSetsBindInfo.PipelineBindPoint = EPipelineBindPoint::Graphics;
                    DescriptorSetsBindInfo.PipelineLayoutPtr = PrefilterEnvMapPipelineLayout;
                    CommandBuffer->BindDescriptorSets(DescriptorSetsBindInfo);

                    CommandBuffer->Draw(36);

                    CommandBuffer->EndRenderPass();
                    CommandBuffer->End();

                    RenderInterface.Get()->GetCommandQueue()->ResetWaitFence(CommandBuffer->GetWaitFence());
                    RenderInterface.Get()->GetCommandQueue()->Submit(CommandBuffer, 0, nullptr);
                    RenderInterface.Get()->GetCommandQueue()->SetWaitIdle();
                }

                // Read from texture 
                FTextureReadInfo TextureReadInfo = { };
                {
                    FTextureSection TextureSection = { };
                    TextureSection.Extent = { (uint32)TextureWidth, (uint32)TextureHeight, 1u };
                    TextureSection.Offset = { 0, 0, 0 };

                    TextureSection.Subresource.BaseArrayLayer = 0;
                    TextureSection.Subresource.NumArrayLayers = 1;
                    TextureSection.Subresource.NumMipLevels = 1;
                    TextureSection.Subresource.BaseMipLevel = 0;

                    RenderInterface.Get()->ReadFromTexture(FrameBufferTexture, TextureSection, ETextureLayout::ColorAttachment, TextureReadInfo);

                    uint32* TextureData = (uint32*)TextureReadInfo.Data;
                    uint32 TextureDataSize = TextureReadInfo.SizeInByte / 4;

                    // Convert from BGRA to RGBA
                    // also flip images horizontally
                    for (int32 j = 0; j < TextureHeight; ++j)
                    {
                        // mirror the line at row j
                        for (int32 i = 1; i <= TextureWidth / 2; i++)
                        {
                            // swap the left side pixel with the right side pixel
                            int32 MirrorPixelIndex = ((TextureWidth * j) + (TextureWidth - i));
                            int32 CurrentPixelIndex = (TextureWidth * j) + i - 1;

                            uint32 Temp = TextureData[MirrorPixelIndex];  // temp = right side pixel
                            TextureData[MirrorPixelIndex] = ConvertBGRA_To_RGBA(TextureData[CurrentPixelIndex]);     // right side pixel = left side pixel
                            TextureData[CurrentPixelIndex] = ConvertBGRA_To_RGBA(Temp);                    // left side pixel gets original right pixel value
                        }
                    }

                    // Next apply a vertical flip
                    for (int32 j = 0; j < TextureHeight; ++j) // col
                    {
                        // mirror the line at col j
                        for (int32 i = 0; i <= TextureWidth / 2; i++) // row
                        {
                            // swap the left side pixel with the right side pixel
                            int32 MirrorPixelIndex = ((TextureWidth * (TextureWidth - i - 1)) + j);
                            int32 CurrentPixelIndex = (TextureWidth * i) + j;

                            uint32 Temp = TextureData[MirrorPixelIndex];  // temp = right side pixel
                            TextureData[MirrorPixelIndex] = TextureData[CurrentPixelIndex];     // right side pixel = left side pixel
                            TextureData[CurrentPixelIndex] = Temp;                    // left side pixel gets original right pixel value
                        }
                    }
                }

                //fwrite(TextureReadInfo.Data, sizeof(uint32), TextureWidth * TextureHeight, FileSource);
                ktxTexture_SetImageFromMemory(ktxTexture(KtxTextureHandle), mipLevel, 0, faceSlice, (const uint8*)TextureReadInfo.Data, TextureReadInfo.SizeInByte);
            }
        }

        ktxTexture_WriteToNamedFile(ktxTexture(KtxTextureHandle), FilePath.c_str());
        ktxTexture_Destroy(ktxTexture(KtxTextureHandle));

        // Clean up memory
        {
            delete FrameBuffer;
            delete FrameBufferTexture;
        }
    }
}

void Renderer::CreatePrefilterEnvMap(CSkybox* inSkyboxAsset, float inViewportSize)
{
    FDescriptorSetsLinkInfo LinkInfo = { };
    LinkInfo.ArrayElementStart = 0;
    LinkInfo.DescriptorCount = 1;
    LinkInfo.BindingStart = 1;
    LinkInfo.ResourceHandle.TextureHandle = inSkyboxAsset->GetCubemapTexture();
    LinkInfo.TextureSampler = SamplerHandle;

    PrefilterEnvMapDescSet->LinkToTexture(0, LinkInfo);
    CreatePrefilterEnvMap(inSkyboxAsset->GetName().c_str(), 512.0f);
}

void Renderer::CreateBrdfIntegration(CSkybox* inSkyboxAsset, float inViewportSize)
{
    FDescriptorSetsLinkInfo LinkInfo = { };
    LinkInfo.ArrayElementStart = 0;
    LinkInfo.DescriptorCount = 1;
    LinkInfo.BindingStart = 1;
    LinkInfo.ResourceHandle.TextureHandle = inSkyboxAsset->GetCubemapTexture();
    LinkInfo.TextureSampler = SamplerHandle;

    BRDFIntegrationDescSet->LinkToTexture(0, LinkInfo);
    CreateBrdfIntegration(inSkyboxAsset->GetName().c_str(), 512.0f);
}

void Renderer::CreateBrdfIntegration(const char* inCubeMapName, float inViewportSize)
{
    {
        // to do that we have to render the scene 6 times at different 90 degrees angles 
        // capturing all of the cubemap faces x, -x, y, -y, z, -z
        // Let do it 

        ProjectionMatrix4D CaptureProjection = ProjectionMatrix4D::MakeProjectionVulkanLH(1, 90.0f, 0.01f, 10.0f, false);
        Matrix4D ViewMatrices[6] =
        {
            Matrix4D::LookAt(Vector3D(0.0f), Vector3D(1.0f,  0.0f,  0.0f),  Vector3D(0.0f, -1.0f,  0.0f)),
            Matrix4D::LookAt(Vector3D(0.0f), Vector3D(-1.0f,  0.0f,  0.0f), Vector3D(0.0f, -1.0f,  0.0f)),
            Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  1.0f,  0.0f),  Vector3D(0.0f,  0.0f,  1.0f)),
            Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  -1.0f,  0.0f), Vector3D(0.0f,  0.0f,  -1.0f)), // Y-
            Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  0.0f,  1.0f),  Vector3D(0.0f, -1.0f,  0.0f)),
            Matrix4D::LookAt(Vector3D(0.0f), Vector3D(0.0f,  0.0f,  -1.0f), Vector3D(0.0f, -1.0f,  0.0f)),
        };

        // Create the texture that we will be attching to the framebuffer
        FTextureConfig TextureConfig = { };
        TextureConfig.Type = ETextureType::Texture2D;
        TextureConfig.BindFlags = FResourceBindFlags::ColorAttachment | FResourceBindFlags::SrcTransfer | FResourceBindFlags::Sampled;
        TextureConfig.CreationFlags = FResourceCreationFlags::Mutable;
        TextureConfig.Format = EPixelFormat::RG16UNorm;
        TextureConfig.Extent = { Application::Get()->GetWindow().GetWidth(), Application::Get()->GetWindow().GetHeight(), 1 };
        TextureConfig.MipLevels = 1;
        TextureConfig.NumArrayLayers = 1;
        TextureConfig.NumSamples = 1;

        Texture* FrameBufferTexture = RenderInterface.Get()->CreateTexture(TextureConfig);

        // Create the frame buffer with the texture we would like to use 
        FFrameBufferConfig FrameBufferConfig = { };
        FrameBufferConfig.RenderPass = BRDFIntegrationRenderPass;
        FrameBufferConfig.Resolution = { SwapChainMain->GetScreenWidth(), SwapChainMain->GetScreenHeight() };

        FFrameBufferAttachment FrameBufferAttachment = { };
        FrameBufferAttachment.Attachment = FrameBufferTexture;
        FrameBufferConfig.Attachments.push_back(FrameBufferAttachment);
        FrameBufferAttachment.Attachment = DepthStencilView;
        FrameBufferConfig.Attachments.push_back(FrameBufferAttachment);

        IFrameBuffer* FrameBuffer = RenderInterface.Get()->CreateFrameBuffer(FrameBufferConfig);

        // Create a command buffer that we will use to encode commands with 
        FCommandBufferConfig CommandBufferConfig = { };
        CommandBufferConfig.CommandQueue = RenderInterface.Get()->GetCommandQueue();
        CommandBufferConfig.NumBuffersToAllocate = 1;
        CommandBufferConfig.Flags = FCommandBufferLevelFlags::Primary;

        ICommandBuffer* CommandBuffer = RenderInterface.Get()->CreateCommandBuffer(CommandBufferConfig);

        // Pre defined clear values 
        FRenderPassBeginInfo RPBeginInfo = { };
        FRenderClearValues ClearValues[2];
        ClearValues[0].Color = { 0.0f, 0.0f, 0.2f,  1.0f };
        ClearValues[0].Depth = { 1.0f };
        ClearValues[0].Stencil = { 0 };

        RPBeginInfo.ClearValues = ClearValues;
        RPBeginInfo.NumClearValues = 2;

        RPBeginInfo.RenderPassPtr = BRDFIntegrationRenderPass;
        RPBeginInfo.FrameBuffer = FrameBuffer;

        FRenderViewport Viewport[1];
        // Set the main render viewport
        Viewport[0].X = 0.0f;

        // Since we flipped the viewport height, we now have to move up to the screen or else
        // we will be seeing a screen that is not being renderered 
        Viewport[0].Y = inViewportSize;

        Viewport[0].MinDepth = 0.0f;
        Viewport[0].MaxDepth = 1.0f;

        Viewport[0].Width = inViewportSize;
        // Since vulkan has a coordinate system where Y points down we have to flip the frame or viewports around the center
        Viewport[0].Height = -inViewportSize;

        FRenderScissor Scissor[1] = { };
        Scissor[0].OffsetX = 0;
        Scissor[0].OffsetY = 0;
        Scissor[0].Width = inViewportSize;
        Scissor[0].Height = inViewportSize;

        ktxTexture2* KtxTextureHandle = nullptr;
        ktxTextureCreateInfo KtxTextureCreateInfo = { };
        KTX_error_code KtxResult;

        uint32 NumLevels = std::log2(inViewportSize) + 1;

        uint32 BaseWidth = inViewportSize;
        uint32 BaseHeight = inViewportSize;

        KtxTextureCreateInfo.vkFormat = VK_FORMAT_R16G16_UNORM;
        KtxTextureCreateInfo.baseWidth = inViewportSize;
        KtxTextureCreateInfo.baseHeight = inViewportSize;
        KtxTextureCreateInfo.baseDepth = 1;
        KtxTextureCreateInfo.numDimensions = 2;
        // Note: it is not necessary to provide a full mipmap pyramid.
        KtxTextureCreateInfo.numLevels = NumLevels;
        KtxTextureCreateInfo.numLayers = 1;
        KtxTextureCreateInfo.numFaces = 1;
        KtxTextureCreateInfo.isArray = KTX_FALSE;
        KtxTextureCreateInfo.generateMipmaps = KTX_FALSE;

        KtxResult = ktxTexture2_Create(&KtxTextureCreateInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &KtxTextureHandle);

        std::vector<FTextureReadInfo> CubemapFacesInfo;
        CubemapFacesInfo.resize(NumLevels);

        std::string FilePath = FilePathToResources;
        FilePath += "Textures/";
        FilePath += inCubeMapName;
        FilePath += "_BRDFIntegration.ktx";

        for (uint32 mipLevel = 0; mipLevel < NumLevels; ++mipLevel)
        {
            int32 TextureWidth = BaseWidth >> mipLevel;
            int32 TextureHeight = BaseHeight >> mipLevel;

            Viewport[0].Y = TextureHeight;
            Viewport[0].Width = TextureWidth;
            // Since vulkan has a coordinate system where Y points down we have to flip the frame or viewports around the center
            Viewport[0].Height = -TextureHeight;

            Scissor[0].Width = TextureWidth;
            Scissor[0].Height = TextureHeight;

            // Render First 
            {
                RenderInterface.Get()->GetCommandQueue()->SetWaitFence(CommandBuffer->GetWaitFence(), UINT64_MAX);

                CommandBuffer->Begin();
                CommandBuffer->BeginRenderPass(RPBeginInfo);

                CommandBuffer->SetRenderViewports(Viewport, 1);
                CommandBuffer->SetRenderScissors(Scissor, 1);

                CommandBuffer->BindPipeline(BRDFIntegrationPipeline);

                {
                    UniformBufferLocalConstants UniformData = { };
                    UniformData.Eye = Vector4D(CameraTranslation, 1.0f);
                    UniformData.Matrix = GlobalMatrix;
                    UniformData.ViewProjection = ((Matrix4D&)CaptureProjection);
                    UniformData.Light = LightPosition.ToVector3D();

                    RenderInterface.Get()->WriteToBuffer(LocalConstantsBuffer, 0, &UniformData, UniformBufferLocalConstants::GetStaticSize());

                    IBLData Data = { };
                    Data.Roughness = (float)mipLevel / (float)(NumLevels - 1);

                    RenderInterface.Get()->WriteToBuffer(IBLDataBuffer, 0, &Data, sizeof(IBLData));
                }

                CommandBuffer->SetVertexBuffer(*QuadVertexBuffer);
                CommandBuffer->SetVertexBuffer(*QuadVertexTexcoordBuffer, 1);

                FDescriptorSetsBindInfo DescriptorSetsBindInfo = { };
                DescriptorSetsBindInfo.DescriptorSets = BRDFIntegrationDescSet;
                DescriptorSetsBindInfo.NumSets = 1;
                DescriptorSetsBindInfo.PipelineBindPoint = EPipelineBindPoint::Graphics;
                DescriptorSetsBindInfo.PipelineLayoutPtr = PrefilterEnvMapPipelineLayout;
                CommandBuffer->BindDescriptorSets(DescriptorSetsBindInfo);

                CommandBuffer->Draw(4);

                CommandBuffer->EndRenderPass();
                CommandBuffer->End();

                RenderInterface.Get()->GetCommandQueue()->ResetWaitFence(CommandBuffer->GetWaitFence());
                RenderInterface.Get()->GetCommandQueue()->Submit(CommandBuffer, 0, nullptr);
                RenderInterface.Get()->GetCommandQueue()->SetWaitIdle();
            }

            // Read from texture 
            FTextureReadInfo TextureReadInfo = { };
            {
                FTextureSection TextureSection = { };
                TextureSection.Extent = { (uint32)TextureWidth, (uint32)TextureHeight, 1u };
                TextureSection.Offset = { 0, 0, 0 };

                TextureSection.Subresource.BaseArrayLayer = 0;
                TextureSection.Subresource.NumArrayLayers = 1;
                TextureSection.Subresource.NumMipLevels = 1;
                TextureSection.Subresource.BaseMipLevel = 0;

                RenderInterface.Get()->ReadFromTexture(FrameBufferTexture, TextureSection, ETextureLayout::ColorAttachment, TextureReadInfo);
            }
            ktxTexture_SetImageFromMemory(ktxTexture(KtxTextureHandle), mipLevel, 0, 0, (const uint8*)TextureReadInfo.Data, TextureReadInfo.SizeInByte);
        }

        ktxTexture_WriteToNamedFile(ktxTexture(KtxTextureHandle), FilePath.c_str());
        ktxTexture_Destroy(ktxTexture(KtxTextureHandle));

        // Clean up memory
        {
            delete FrameBuffer;
            delete FrameBufferTexture;
        }
    }
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

static bool OpenFileDialog(std::string& outSelectedFile, std::string& outFilePath)
{
    //  CREATE FILE OBJECT INSTANCE
    HRESULT f_SysHr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(f_SysHr))
        return false;

    // CREATE FileOpenDialog OBJECT
    IFileOpenDialog* f_FileSystem;
    f_SysHr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&f_FileSystem));
    if (FAILED(f_SysHr)) {
        CoUninitialize();
        return false;
    }

    //  SHOW OPEN FILE DIALOG WINDOW
    f_SysHr = f_FileSystem->Show(NULL);
    if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return false;
    }

    //  RETRIEVE FILE NAME FROM THE SELECTED ITEM
    IShellItem* f_Files;
    f_SysHr = f_FileSystem->GetResult(&f_Files);
    if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return false;
    }

    //  STORE AND CONVERT THE FILE NAME
    PWSTR f_Path;
    f_SysHr = f_Files->GetDisplayName(SIGDN_FILESYSPATH, &f_Path);
    if (FAILED(f_SysHr)) {
        f_Files->Release();
        f_FileSystem->Release();
        CoUninitialize();
        return false;
    }

    //  FORMAT AND STORE THE FILE PATH
    std::wstring path(f_Path);
    std::string c(path.begin(), path.end());
    outFilePath = c;

    //  FORMAT STRING FOR EXECUTABLE NAME
    const size_t slash = outFilePath.find_last_of("/\\");
    outSelectedFile = outFilePath.substr(slash + 1);

    //  SUCCESS, CLEAN UP
    CoTaskMemFree(f_Path);
    f_Files->Release();
    f_FileSystem->Release();
    CoUninitialize();

    return true;
}

void Renderer::DrawEditorTools()
{
    //static bool bShowStaticMeshWindow = false;
    static bool bShowMaterialWindow = false;

    static bool bShowWindow = 1;

    {
        static bool bShowOverlayWindow = true;
        ImGuiWindowFlags OverlayWindowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

        const float PAD = 10.0f;
        const ImGuiViewport* Viewport = ImGui::GetMainViewport();
        ImVec2 WorkPosition = Viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 WorkSize = Viewport->WorkSize;
        ImVec2 WindowPosition, WindowPositionPivot;
        WindowPosition.x = (WorkPosition.x + PAD);// (WorkPosition.x + PAD);
        WindowPosition.y = (WorkPosition.y + PAD);// (WorkPosition.y + PAD);
        WindowPositionPivot.x = 0.0f;// 0.0f;
        WindowPositionPivot.y = 0.0f;// 0.0f;
        ImGui::SetNextWindowPos(WindowPosition, ImGuiCond_Always, WindowPositionPivot);
        ImGui::SetNextWindowViewport(Viewport->ID);
        OverlayWindowFlags |= ImGuiWindowFlags_NoMove;

        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (ImGui::Begin("Timer", &bShowOverlayWindow, OverlayWindowFlags))
        {
            auto TextWidth = ImGui::CalcTextSize("Timer").x;
            ImVec2 WindowSize = ImGui::GetWindowSize();

            ImGui::SameLine((WindowSize.x * 0.5f) - (TextWidth * 0.5f));
            ImGui::Text("Timer");
            ImGui::Separator();

            ImGui::Text("Frame Rate: %u", VGameEngine::Get()->GetFrameRate());
            ImGui::Text("Render Time: %.0001f ms", VGameEngine::Get()->GetRenderTime());
            ImGui::Text("Tick Time: %.0001f ms", VGameEngine::Get()->GetTickTime());
        }
        ImGui::End();
    }

    if (ImGui::Begin("Shader Debug Tool", &bShowWindow))
    {
        if (ImGui::BeginMenu("Windows"))
        {
            //ImGui::MenuItem("Static Meshes", NULL, &bShowStaticMeshWindow);
            ImGui::MenuItem("Material", NULL, &bShowMaterialWindow);
            ImGui::EndMenu();
        }

        {
            //if (ImGui::Button("Select Cubemap", ImVec2(128, 128)))
            //{
            //    std::string sSelectedFile;
            //    std::string sFilePath;
            //    if (OpenFileDialog(sSelectedFile, sFilePath))
            //    {
            //        if (strcmp(sSelectedFile.substr(sSelectedFile.length() - 4).c_str(), ".ktx") == 0)
            //        {
            //            // Load this cubemap 
            //            SkyboxAsset->UpdateCubemapTexture(sFilePath);
            //            SkyboxAsset->SetName(sSelectedFile.substr(0, sSelectedFile.length() - 4));

            //            CreatePrefilterEnvMap(SkyboxAsset, 512.0f);
            //            CreateIrradianceMap(SkyboxAsset, 32.0f);
            //            CreateBrdfIntegration(SkyboxAsset, 512.0f);

            //            std::string Path = FilePathToTextures;
            //            Path += SkyboxAsset->GetName();

            //            std::string Prefilter = Path + "PrefilteredEnvMap.ktx";
            //            std::string Irradiance = Path + "IrradianceMap.ktx";
            //            std::string BRDFPath = Path + "_BRDFIntegration.ktx";

            //            PrefilterEnvMapTexture = CreateTextureCubemapKtx(Prefilter, PrefilterEnvMapBuffer);

            //            Buffer* IrridianceBuffer = nullptr;
            //            IrridianceTexture = CreateTextureCubemapKtx(Irradiance, IrridianceBuffer);

            //            Buffer* BRDFLutBuffer = nullptr;
            //            BRDFLutTexture = CreateTexture2DKtx(BRDFPath, BRDFLutBuffer);

            //            FDescriptorSetsLinkInfo LinkInfo = { };
            //            LinkInfo.ArrayElementStart = 0;
            //            LinkInfo.DescriptorCount = 1;

            //            for (uint32 i = 0; i < StaticMeshes.size(); ++i)
            //            {
            //                CStaticMesh* StaticMesh = StaticMeshes[i];
            //                const FRenderAssetData& RenderData = StaticMesh->GetRenderAssetData();
            //                for (uint32 SectionIndex = 0; SectionIndex < RenderData.RenderAssetSections.size(); ++SectionIndex)
            //                {
            //                    const FRenderAssetSection& Section = RenderData.RenderAssetSections[SectionIndex];

            //                    LinkInfo.TextureSampler = SamplerHandle;

            //                    LinkInfo.BindingStart = 8;
            //                    LinkInfo.ResourceHandle.TextureHandle = PrefilterEnvMapTexture;
            //                    Section.RenderAssetDescriptorSet->LinkToTexture(0, LinkInfo);

            //                    LinkInfo.BindingStart = 7;
            //                    LinkInfo.ResourceHandle.TextureHandle = IrridianceTexture;
            //                    Section.RenderAssetDescriptorSet->LinkToTexture(0, LinkInfo);

            //                    LinkInfo.BindingStart = 9;
            //                    LinkInfo.ResourceHandle.TextureHandle = BRDFLutTexture;
            //                    Section.RenderAssetDescriptorSet->LinkToTexture(0, LinkInfo);
            //                }
            //            }

            //            {
            //                /*for (uint32 i = 0; i < TransparentMeshDraws.size(); ++i)
            //                {
            //                    CStaticMesh* StaticMesh = TransparentMeshDraws[i];
            //                    const FRenderAssetData& RenderData = StaticMesh->GetRenderAssetData();
            //                    for (uint32 SectionIndex = 0; SectionIndex < RenderData.RenderAssetSections.size(); ++SectionIndex)
            //                    {
            //                        const FRenderAssetSection& Section = RenderData.RenderAssetSections[SectionIndex];

            //                        LinkInfo.TextureSampler = SamplerHandle;

            //                        LinkInfo.BindingStart = 8;
            //                        LinkInfo.ResourceHandle.TextureHandle = PrefilterEnvMapTexture;
            //                        Section.RenderAssetDescriptorSet->LinkToTexture(0, LinkInfo);

            //                        LinkInfo.BindingStart = 7;
            //                        LinkInfo.ResourceHandle.TextureHandle = IrridianceTexture;
            //                        Section.RenderAssetDescriptorSet->LinkToTexture(0, LinkInfo);

            //                        LinkInfo.BindingStart = 9;
            //                        LinkInfo.ResourceHandle.TextureHandle = BRDFLutTexture;
            //                        Section.RenderAssetDescriptorSet->LinkToTexture(0, LinkInfo);
            //                    }
            //                }*/
            //            }
            //        }
            //    }
            //}
        }

        bool bEnableSRGB = DebugFlags & DebugFlags_DisableSRGBConversion;
        bool bOnlyShowDiffuseContrib = DebugFlags & DebugFlags_OnlyDiffuseContribution;
        bool bOnlyShowDiffuseLightContrib = DebugFlags & DebugFlags_OnlyDiffuseLightContribution;
        bool bOnlyShowSpecularContrib = DebugFlags & DebugFlags_OnlySpecularContribution;
        bool bOnlyShowSpecularLightContrib = DebugFlags & DebugFlags_OnlySpecularLightContribution;
        bool bOnlyShowLightContrib = DebugFlags & DebugFlags_OnlyLightContribution;

        ImGui::Checkbox("Enable SRGB", &bEnableSRGB);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Converts SRGB to linear when turned on..");
        }

        if (ImGui::Checkbox("Show Diffuse Contribution", &bOnlyShowDiffuseContrib))
        {
            bOnlyShowSpecularContrib = bOnlyShowSpecularLightContrib = bOnlyShowDiffuseLightContrib = bOnlyShowLightContrib = false;
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Onlys shows the diffuse contribution that gets calculated...");
        }

        if (ImGui::Checkbox("Show Diffuse Light Contribution", &bOnlyShowDiffuseLightContrib))
        {
            bOnlyShowSpecularContrib = bOnlyShowSpecularLightContrib = bOnlyShowLightContrib = bOnlyShowDiffuseContrib = false;
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Onlys shows the diffuse light contribution that gets calculated...");
        }

        if (ImGui::Checkbox("Show Specular Contribution", &bOnlyShowSpecularContrib))
        {
            bOnlyShowDiffuseContrib = bOnlyShowDiffuseLightContrib = bOnlyShowLightContrib = bOnlyShowSpecularLightContrib = false;
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Onlys shows the specular contribution that gets calculated...");
        }

        if (ImGui::Checkbox("Show Specular Light Contribution", &bOnlyShowSpecularLightContrib))
        {
            bOnlyShowDiffuseContrib = bOnlyShowDiffuseLightContrib = bOnlyShowLightContrib = bOnlyShowSpecularContrib = false;
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Onlys shows the specular light contribution that gets calculated...");
        }

        if (ImGui::Checkbox("Show Light Contribution", &bOnlyShowLightContrib))
        {
            bOnlyShowDiffuseContrib = bOnlyShowDiffuseLightContrib = bOnlyShowSpecularContrib = bOnlyShowSpecularContrib = false;
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Onlys shows the light contribution that gets calculated...");
        }

        DebugFlags = 0;
        if (bEnableSRGB)
        {
            DebugFlags |= DebugFlags_DisableSRGBConversion;
        }
        if (bOnlyShowDiffuseContrib)
        {
            DebugFlags |= DebugFlags_OnlyDiffuseContribution;
        }
        if (bOnlyShowDiffuseLightContrib)
        {
            DebugFlags |= DebugFlags_OnlyDiffuseLightContribution;
        }
        if (bOnlyShowSpecularContrib)
        {
            DebugFlags |= DebugFlags_OnlySpecularContribution;
        }
        if (bOnlyShowSpecularLightContrib)
        {
            DebugFlags |= DebugFlags_OnlySpecularLightContribution;
        }
        if (bOnlyShowLightContrib)
        {
            DebugFlags |= DebugFlags_OnlyLightContribution;
        }
    }

    ImGui::End();

    //if (bShowStaticMeshWindow)
    //{
    //    if (ImGui::Begin("Static Meshes", &bShowStaticMeshWindow))
    //    {
    //        for (uint32 i = 0; i < StaticMeshes.size(); ++i)
    //        {
    //            CStaticMesh* StaticMesh = StaticMeshes[i];
    //            ImGui::Button(StaticMesh->GetName().c_str());
    //        }
    //    }
    //
    //    ImGui::End();
    //}

    if (bShowMaterialWindow)
    {
        if (ImGui::Begin("Material Panel", &bShowMaterialWindow))
        {
            if (ImGui::Button("Select Mesh"))
                ImGui::OpenPopup("Static_Meshes");
            ImGui::SameLine();
            ImGui::TextUnformatted(SelectedStaticMesh == -1 ? "None" : StaticMeshes[SelectedStaticMesh]->GetName().c_str());

            ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoResize;
            ImGui::SetNextWindowSize(ImVec2(200, 300));

            if (ImGui::BeginPopup("Static_Meshes", WindowFlags))
            {
                ImGui::SeparatorText("Static Meshes");
                for (int i = 0; i < StaticMeshes.size(); i++)
                {
                    if (ImGui::Selectable(StaticMeshes[i]->GetName().c_str()))
                    {
                        SelectedStaticMesh = i;
                        SelectedMaterial = -1;
                        break;
                    }
                }
                ImGui::EndPopup();
            }
        }

        if (SelectedStaticMesh != -1)
        {
            CStaticMesh* StaticMesh = StaticMeshes[SelectedStaticMesh];
            std::string MaterialText = "Material ";

            ImGui::NewLine();

            if (ImGui::Button("Select Material"))
                ImGui::OpenPopup("Material_Selection");
            ImGui::SameLine();
            ImGui::TextUnformatted(SelectedMaterial == -1 ? "None" : (MaterialText + std::to_string(SelectedMaterial)).c_str());

            ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoResize;
            ImGui::SetNextWindowSize(ImVec2(200, 300));

            if (ImGui::BeginPopup("Material_Selection", WindowFlags))
            {
                ImGui::SeparatorText("Material Slots");
                for (int i = 0; i < StaticMesh->GetNumMaterials(); i++)
                {
                    if (ImGui::Selectable((MaterialText + std::to_string(i)).c_str()))
                    {
                        SelectedMaterial = i;
                        break;
                    }
                }
                ImGui::EndPopup();
            }

            ImGui::NewLine();

            if (SelectedMaterial != -1)
            {
                FMaterialData& Data = StaticMesh->GetMaterial(SelectedMaterial);

                bool bIsTransparent = StaticMesh->GetIsTransparent();
                if (ImGui::Checkbox("Is Transparent", &bIsTransparent))
                {
                    StaticMesh->SetIsTransparent(bIsTransparent);
                }

                ImGui::SliderFloat4("Base Color Factor", &Data.BaseColorFactor.X, 0.0f, 1.0f);
                ImGui::SliderFloat3("Emissive Factor", &Data.EmissiveFactor.X, 0.0f, 1.0f);
                ImGui::SliderFloat("Metallic Factor", &Data.MetallicFactor, 0.0f, 1.0f);
                ImGui::SliderFloat("Roughness Factor", &Data.RoughnessFactor, 0.0f, 1.0f);
                ImGui::SliderFloat("Occlusion Factor", &Data.OcclusionFactor, 0.0f, 1.0f);
                //ImGui::SliderFloat("Alpha Mask", &AlphaMask, 0.0f, 1.0f);
                //ImGui::SliderFloat("Alpha Mask Cutoff", &AlphaMaskCutoff, 0.0f, 1.0f);
            }
        }

        ImGui::End();
    }

    //ImGui::ShowDemoWindow();
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
    SwapChainConfiguration.bEnableVSync = true;
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

    // Setting up default render pass
    {
        FRenderPassConfig Config = { };
        Config.RenderArea = SwapChainConfiguration.ScreenResolution;
        Config.NumSamples = 1;

        // Depth Stencil Attachment
        FAttachmentDescription DepthStencil = { };
        DepthStencil.Format = EPixelFormat::D32FloatS8X24UInt;
        DepthStencil.LoadOp = EAttachmentLoadOp::Clear;
        DepthStencil.StoreOp = EAttachmentStoreOp::Store;
        DepthStencil.StencilLoadOp = EAttachmentLoadOp::Clear;
        DepthStencil.StencilStoreOp = EAttachmentStoreOp::Undefined;
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
            Color.Format = EPixelFormat::RG16UNorm;
            Color.LoadOp = EAttachmentLoadOp::Clear;
            Color.StoreOp = EAttachmentStoreOp::Store;
            Color.InitialLayout = ETextureLayout::Undefined;
            Color.FinalLayout = ETextureLayout::PresentSrc;

            Config.ColorAttachments.push_back(Color);

            FSubpassDependencyDescription SBDesc = { };
            SBDesc.SrcAccessMaskFlags = 0;
            SBDesc.DstAccessMaskFlags = FSubpassAssessFlags::ColorAttachmentRead | FSubpassAssessFlags::ColorAttachmentWrite;

            Config.SubpassDependencies.push_back(SBDesc);
            BRDFIntegrationRenderPass = RenderInterface.Get()->CreateRenderPass(Config);
        }
    }
    {

        // Setting up pbr render pass
        FrameBuffers.resize(SwapChainMain->GetImageCount());

        FFrameBufferAttachment DepthStencilAttachment = { };
        DepthStencilAttachment.Attachment = (Texture*)DepthStencilView;

        FFrameBufferAttachment ColorAttachment = { };

        FFrameBufferConfig Config = { };
        Config.RenderPass = RenderPass;
        Config.Resolution = { SwapChainMain->GetScreenWidth(), SwapChainMain->GetScreenHeight() };
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
    }

    // Create Vertex/TextureCoord/Index Buffers for cube 
    {
        FBufferConfig Config = { };
        Config.UsageFlags |= FResourceBindFlags::VertexBuffer;
        Config.MemoryFlags |= FMemoryFlags::HostCached;

        float Vertices[] =
        {
            // back face
           -1.0f, -1.0f, -1.0f, // bottom-left
            1.0f,  1.0f, -1.0f, // top-right
            1.0f, -1.0f, -1.0f, // bottom-right         
            1.0f,  1.0f, -1.0f, // top-right
           -1.0f, -1.0f, -1.0f, // bottom-left
           -1.0f,  1.0f, -1.0f, // top-left
           // front face
           -1.0f, -1.0f,  1.0f, // bottom-left
            1.0f, -1.0f,  1.0f, // bottom-right
            1.0f,  1.0f,  1.0f, // top-right
            1.0f,  1.0f,  1.0f, // top-right
           -1.0f,  1.0f,  1.0f, // top-left
           -1.0f, -1.0f,  1.0f, // bottom-left
           // left face
           -1.0f,  1.0f,  1.0f, // top-right
           -1.0f,  1.0f, -1.0f, // top-left
           -1.0f, -1.0f, -1.0f, // bottom-left
           -1.0f, -1.0f, -1.0f, // bottom-left
           -1.0f, -1.0f,  1.0f, // bottom-right
           -1.0f,  1.0f,  1.0f, // top-right
           // right face
            1.0f,  1.0f,  1.0f, // top-left
            1.0f, -1.0f, -1.0f, // bottom-right
            1.0f,  1.0f, -1.0f, // top-right         
            1.0f, -1.0f, -1.0f, // bottom-right
            1.0f,  1.0f,  1.0f, // top-left
            1.0f, -1.0f,  1.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f, // top-right
             1.0f, -1.0f, -1.0f, // top-left
             1.0f, -1.0f,  1.0f, // bottom-left
             1.0f, -1.0f,  1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, // bottom-right
            -1.0f, -1.0f, -1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f, // top-left
             1.0f,  1.0f , 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f, // top-right     
             1.0f,  1.0f,  1.0f, // bottom-right
            -1.0f,  1.0f, -1.0f, // top-left
            -1.0f,  1.0f,  1.0f  // bottom-left   
        };

        const float CubeTexcoords[] =
        {
            0.0f,  0.0f,  //-1.0f, 0.0f, 0.0f,
            0.0f,  0.0f,  //-1.0f, 1.0f, 1.0f,
            0.0f,  0.0f,  //-1.0f, 1.0f, 0.0f,
            0.0f,  0.0f,  //-1.0f, 1.0f, 1.0f,
            0.0f,  0.0f,  //-1.0f, 0.0f, 0.0f,
            0.0f,  0.0f,  //-1.0f, 0.0f, 1.0f,
            //
    0.0f,  0.0f,  // 1.0f, 0.0f, 0.0f,
    0.0f,  0.0f,  // 1.0f, 1.0f, 0.0f,
    0.0f,  0.0f,  // 1.0f, 1.0f, 1.0f,
    0.0f,  0.0f,  // 1.0f, 1.0f, 1.0f,
    0.0f,  0.0f,  // 1.0f, 0.0f, 1.0f,
    0.0f,  0.0f,  // 1.0f, 0.0f, 0.0f,
    //
    -1.0f,  0.0f,  // 0.0f, 1.0f, 0.0f,
    -1.0f,  0.0f,  // 0.0f, 1.0f, 1.0f,
    -1.0f,  0.0f,  // 0.0f, 0.0f, 1.0f,
    -1.0f,  0.0f,  // 0.0f, 0.0f, 1.0f,
    -1.0f,  0.0f,  // 0.0f, 0.0f, 0.0f,
    -1.0f,  0.0f,  // 0.0f, 1.0f, 0.0f,
    //
    1.0f,  0.0f,  // 0.0f, 1.0f, 0.0f,
    1.0f,  0.0f,  // 0.0f, 0.0f, 1.0f,
    1.0f,  0.0f,  // 0.0f, 1.0f, 1.0f,
    1.0f,  0.0f,  // 0.0f, 0.0f, 1.0f,
    1.0f,  0.0f,  // 0.0f, 1.0f, 0.0f,
    1.0f,  0.0f,  // 0.0f, 0.0f, 0.0f,
    //
    0.0f, -1.0f, //  0.0f, 0.0f, 1.0f,
    0.0f, -1.0f, //  0.0f, 1.0f, 1.0f,
    0.0f, -1.0f, //  0.0f, 1.0f, 0.0f,
    0.0f, -1.0f, //  0.0f, 1.0f, 0.0f,
    0.0f, -1.0f, //  0.0f, 0.0f, 0.0f,
    0.0f, -1.0f, //  0.0f, 0.0f, 1.0f,
    //
    0.0f,  1.0f, //  0.0f, 0.0f, 1.0f,
    0.0f,  1.0f, //  0.0f, 1.0f, 0.0f,
    0.0f,  1.0f, //  0.0f, 1.0f, 1.0f,
    0.0f,  1.0f, //  0.0f, 1.0f, 0.0f,
    0.0f,  1.0f, //  0.0f, 0.0f, 1.0f,
    0.0f,  1.0f  //  0.0f, 0.0f, 0.0f
        };

        Config.InitialData = Vertices;
        Config.Size = sizeof(Vertices);
        CubeVertexBuffer = RenderInterface.Get()->CreateBuffer(Config);

        Config.InitialData = CubeTexcoords;
        Config.Size = sizeof(CubeTexcoords);
        CubeVertexTexcoordBuffer = RenderInterface.Get()->CreateBuffer(Config);

        float QuadVertices[] = {
            // positions        
            -1.0f,  1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
             1.0f, -1.0f, 0.0f
        };

        float QuadVerticesTexcoord[] =
        {
            // texture Coords
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 0.0f
        };

        Config.InitialData = QuadVertices;
        Config.Size = sizeof(QuadVertices);
        QuadVertexBuffer = RenderInterface.Get()->CreateBuffer(Config);

        Config.InitialData = QuadVerticesTexcoord;
        Config.Size = sizeof(QuadVerticesTexcoord);
        QuadVertexTexcoordBuffer = RenderInterface.Get()->CreateBuffer(Config);

        //Config.InitialData = Indices;
        //Config.Size = 36 * 4;
        //Config.UsageFlags |= FResourceBindFlags::IndexBuffer;
        //CubeIndexBuffer = RenderInterface.Get()->CreateBuffer(Config);
    }

    // Default Sampler And Textures
    {
        FSamplerConfig SamplerConfig = { };
        SamplerConfig.SetDefault();

        SamplerConfig.AddressModeU = ESamplerAddressMode::ClampToEdge;
        SamplerConfig.AddressModeV = ESamplerAddressMode::ClampToEdge;
        SamplerConfig.AddressModeW = ESamplerAddressMode::ClampToEdge;

        SamplerHandle = RenderInterface.Get()->CreateSampler(SamplerConfig);

        BRDFSamplerHandle = RenderInterface.Get()->CreateSampler(SamplerConfig);

        SamplerConfig.MinLod = 0;
        SamplerConfig.MaxLod = 9;
        LODSamplerHandle = RenderInterface.Get()->CreateSampler(SamplerConfig);

        CP2077TextureHandle = CreateTexture2D(MakePathToResource("Cybepunk2077.jpg", 't'), CP2077BufferHandle);
        VELogoTextureHandle = CreateTexture2D(MakePathToResource("VrixicEngineLogo.png", 't'), VELogoBufferHandle);
    }

    // Create Local Constants Buffer
    {
        LocalConstants.Matrix.SetIdentity();
        LocalConstants.ViewProjection.SetIdentity();

        FBufferConfig Config;
        Config.InitialData = &LocalConstants;
        Config.Size = UniformBufferLocalConstants::GetStaticSize();
        Config.MemoryFlags |= FMemoryFlags::HostVisible;
        Config.UsageFlags |= FResourceBindFlags::UniformBuffer;

        LocalConstantsBuffer = RenderInterface.Get()->CreateBuffer(Config);

        Config.InitialData = nullptr;
        Config.Size = sizeof(IBLData);

        IBLDataBuffer = RenderInterface.Get()->CreateBuffer(Config);
    }

    CreateSkyboxPipeline();
    CreatePBRPipeline();

    LoadModels();

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
    RenderPass->UpdateRenderArea(NewRenderArea); // Since we flipped the viewport height, we now have to move up to the screen or else
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

        // Tell the render interface that window just resized 
        RenderInterface.Get()->OnRenderViewportResized(SwapChainMain, inNewRenderViewport);

        return true;
    }
}

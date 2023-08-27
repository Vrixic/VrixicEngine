#include "AsynchronousLoader.h"

#include <Runtime/Graphics/Renderer.h>

void AsynchronousLoader::Init(enki::TaskScheduler* inTaskScheduler)
{
    TaskScheduler = inTaskScheduler;

    TextureReady = InvalidTextureHandle;

    FCommandBufferConfig CommandBufferConfig = { };
    CommandBufferConfig.CommandQueue = Renderer::Get().GetRenderInterface().Get()->GetTransferQueue();
    CommandBufferConfig.NumBuffersToAllocate = 1;
    CommandBufferConfig.Flags = FCommandBufferLevelFlags::Primary;
    CommandBuffers.resize(Renderer::Get().GetSwapchain()->GetImageCount());

    for (uint32 i = 0; i < CommandBuffers.size(); ++i)
    {
        ICommandBuffer* Buffer = Renderer::Get().GetRenderInterface().Get()->CreateCommandBuffer(CommandBufferConfig);
        CommandBuffers[i] = Buffer;
    }

    FSemaphoreConfig SemaphoreConfig = { 1 };
    TransferCompleteSemaphore = Renderer::Get().GetRenderInterface().Get()->CreateRenderSemaphore(SemaphoreConfig);

    TransferFence = Renderer::Get().GetRenderInterface().Get()->CreateFence();

    FBufferConfig BufferConfig = { };
    BufferConfig.InitialData = nullptr;
    BufferConfig.MemoryFlags |= FMemoryFlags::HostCoherent | FMemoryFlags::HostVisible;
    StagingBufferSize = static_cast<uint64>(512u * 1024u) * 1024u;
    BufferConfig.Size = StagingBufferSize;
    BufferConfig.UsageFlags |= FResourceBindFlags::StagingBuffer | FResourceBindFlags::SrcTransfer;

    StagingBuffer = Renderer::Get().GetRenderInterface().Get()->CreateBuffer(BufferConfig);
    StagingBufferOffset = 0u;
}

void AsynchronousLoader::Update()
{
    // If we have a texture that has been processed and is ready, signal the renderer 
    if (TextureReady != InvalidTextureHandle)
    {
        Renderer::Get().AddTextureToUpdate(TextureReady);
    }

    TextureReady = InvalidTextureHandle;

    if (TextureUploadRequests.size())
    {
        ICommandBuffer* CommandBuffer = CommandBuffers[Renderer::Get().GetCurrentFrame()];

        // Wait for fence 
        if (!Renderer::Get().GetRenderInterface().Get()->GetTransferQueue()->GetWaitFenceStatus(TransferFence))
        {
            return;
        }

        // Reset if file requests are present 
        Renderer::Get().GetRenderInterface().Get()->GetTransferQueue()->ResetWaitFence(TransferFence);

        TextureUploadRequest Request = TextureUploadRequests.back();
        TextureUploadRequests.pop_back();

        CommandBuffer->Begin();

        if (Request.Texture != InvalidTextureHandle)
        {
            TextureResource*& TextureHandle = Renderer::Get().GetTextureResource(Request.Texture);

            uint64 TextureSize = Request.CpuHandle.SizeInBytes;
            // Align Memory
            const uint64 AlignmentMask = 3;
            uint64 AlignedImageSize = (TextureSize + AlignmentMask) & ~AlignmentMask;
            const uint64 CurrentOffset = std::atomic_fetch_add(&StagingBufferOffset, AlignedImageSize);

            VE_ASSERT(StagingBufferOffset < StagingBufferSize, VE_TEXT("[AsynchronousLoader]: Staging Buffers Size overflow...!"));

            // Create the texture resource
            FTextureConfig Config = FTextureConfig();
            Config.BindFlags |= FResourceBindFlags::Sampled | FResourceBindFlags::DstTransfer | FResourceBindFlags::SrcTransfer;
            Config.Extent.Depth = 1;
            Config.MipLevels = 1;
            Config.NumArrayLayers = 1;
            Config.NumSamples = 1;
            Config.Type = ETextureType::Texture2D;

            Config.Extent.Width = Request.CpuHandle.Width;
            Config.Extent.Height = Request.CpuHandle.Height;

            Config.Format = Request.Format;

            TextureHandle = Renderer::Get().GetRenderInterface().Get()->CreateTexture(Config);

            // Copy Texture Data to buffer
            Renderer::Get().GetRenderInterface().Get()->WriteToBuffer(StagingBuffer, CurrentOffset, Request.CpuHandle.GetMemoryHandle(), AlignedImageSize);

            // Copy Buffer Memory Into Image 
            FTextureWriteInfo TextureWriteInfo = FTextureWriteInfo();
            TextureWriteInfo.BufferHandle = StagingBuffer;
            TextureWriteInfo.Subresource.BaseArrayLayer = 0;
            TextureWriteInfo.Subresource.NumArrayLayers = 1;
            TextureWriteInfo.Subresource.BaseMipLevel = 0;
            TextureWriteInfo.Subresource.NumMipLevels = 1;
            TextureWriteInfo.InitialBufferOffset = CurrentOffset;

            TextureWriteInfo.Extent = { (uint32)Request.CpuHandle.Width, (uint32)Request.CpuHandle.Height, 1u };

            CommandBuffer->UploadTextureData(TextureHandle, TextureWriteInfo);

            CommandBuffer->End();

            Renderer::Get().GetRenderInterface().Get()->GetTransferQueue()->Submit(CommandBuffer, TransferFence);

            if (Request.Texture != InvalidTextureHandle) {
                VE_ASSERT(TextureReady == InvalidTextureHandle, VE_TEXT("[AsynchronousLoader]: 'Bug' - If there is a texture already ready then it should have been already passed to the renderer for update...."));
                TextureReady = Request.Texture;
            }
        }
    }

    if (TextureLoadRequests.size())
    {
        TextureLoadRequest LoadRequest = TextureLoadRequests.back();
        TextureLoadRequests.pop_back();

        TextureResourceHandle Handle = ResourceManager::Get().LoadTexture(LoadRequest.Path);
        if (Handle.GetMemoryHandle() != nullptr)
        {
            TextureUploadRequest URequest = { };

            URequest.Texture = LoadRequest.Texture;
            URequest.CpuHandle = Handle;
            URequest.Format = LoadRequest.Format;

            TextureUploadRequests.push_back(URequest);
        }
    }
}

void AsynchronousLoader::Shutdown()
{
    delete TransferCompleteSemaphore;
    delete TransferFence;
    //delete StagingBuffer;
}

void AsynchronousLoader::RequestTextureData(const std::string& inFilePath, TextureHandle inTexture, EPixelFormat inTextureFormat)
{
    TextureLoadRequest LoadRequest = { };
    strcpy(LoadRequest.Path, inFilePath.c_str());
    LoadRequest.Texture = inTexture;
    LoadRequest.Format = inTextureFormat;

    TextureLoadRequests.push_back(LoadRequest);
}

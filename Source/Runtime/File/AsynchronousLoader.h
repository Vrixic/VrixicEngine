#pragma once
#include <Runtime/Graphics/Renderer.h>
#include <Runtime/Memory/ResourceManager.h>

#include <TaskScheduler.h>

struct TextureLoadRequest
{
    char Path[512];
    TextureHandle Texture = InvalidTextureHandle;
    EPixelFormat Format = EPixelFormat::Undefined;
};

struct TextureUploadRequest
{
    TextureHandle Texture = InvalidTextureHandle;
    EPixelFormat Format = EPixelFormat::Undefined;
    TextureResourceHandle CpuHandle = { };
};

class AsynchronousLoader
{
public:
    /**
    * Call Destructor ONLY when you allocate this class dynamically...
    */
    ~AsynchronousLoader()
    {
        Shutdown();
    }
    void Init(enki::TaskScheduler* inTaskScheduler);
    void Update();
    void Shutdown();

    void RequestTextureData(const std::string& inFilePath, TextureHandle inTexture, EPixelFormat inTextureFormat);

private:
    enki::TaskScheduler* TaskScheduler;

    std::vector<TextureLoadRequest> TextureLoadRequests;
    std::vector<TextureUploadRequest> TextureUploadRequests;

    std::vector<ICommandBuffer*>  CommandBuffers;

    ISemaphore* TransferCompleteSemaphore;
    IFence* TransferFence;

    Buffer* StagingBuffer;
    std::atomic_size_t StagingBufferOffset;
    uint64 StagingBufferSize;

    /** Ready Resources */
    TextureHandle TextureReady;
};

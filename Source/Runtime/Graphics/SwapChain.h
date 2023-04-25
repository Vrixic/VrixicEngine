/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "CommandQueue.h"
#include <Core/Misc/Interface.h>
#include "Semaphore.h"
#include "Surface.h"
#include "SwapChainGenerics.h"
#include "Texture.h"

/**
* Defines a swapchian which has a surface and buffers (Images) it can write into with capabilities of presenting
* Stripped of I as it has definitions
*/
class VRIXIC_API SwapChain : public Interface
{
public:
    /**
    * Presents the current buffer to the screen
    *
    * @param inSubmissionQueue the presentation queue used for presenting an image
    * @param inWaitSemaphore (Optional) Semaphore that is waited on before the image gets presented (only used if its != to nullptr
    * @param inImageIndex the index of the swapchain image to queue for presentation
    */
    virtual void Present(ICommandQueue* inSubmissionQueue, ISemaphore* inWaitSemaphore, uint32 inImageIndex) = 0;

    /**
    * Resizes all buffers/images within the swapchain, essentially recreating the swapchain
    * @returns bool true if it resized, false otherwise
    * @remarks use cases is on window resize or what ever render target its rendering to resized
    */
    virtual bool ResizeSwapChain(const FExtent2D& inNewResolution) = 0;

    /**
    * Sets the vsync interval for this swapchain (vertical synchronization), 0 to disable, and 1 or more halfs the refresh rate
    *
    * @returns bool true if successfully set to new vsync interval, false otherwise making it invalid
    */
    virtual bool SetVSyncInterval(uint32 inVSyncInterval) = 0;

public:

    /**
    * Acquires the next swapchain image index 
    * 
    * @param inWaitSemaphore the presentation complete semaphore to wait on 
    * @param outIndex the new image index to use 
    */
    virtual void AcquireNextImageIndex(ISemaphore* inWaitSemaphore, uint32* outIndex) const = 0;

    /**
    * @returns EFormat the color format of this swapchain
    */
    virtual EPixelFormat GetColorFormat() const = 0;

    /**
    * @returns EFormat the depth stencil format of this swapchain
    */
    virtual EPixelFormat GetDepthStencilFormat() const = 0;

    /**
    * @returns uint32 the count of images used by this swapchain
    */
    virtual uint32 GetImageCount() const = 0;

    /**
    * Returns the texture that is requested from the texture this swapchain is using
    *
    * @returns Texture* the texture at the index specified
    */
    virtual Texture* GetTextureAt(uint32 inTextureIndex) const = 0;

    /**
    * @return Surface& the handle to the surface that is associated with the swapchain
    */
    inline Surface& GetSurfaceHandle() const
    {
        return *SurfaceHandle;
    }

protected:
    /** The swap chain configuration */
    FSwapChainConfig Configuration;

    /** The surface handle that the sawp chain is associated with */
    Surface* SurfaceHandle;
};

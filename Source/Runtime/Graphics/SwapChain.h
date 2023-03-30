/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Misc/Interface.h>
#include "Surface.h"

/**
* Defines a swapchian which has a surface and buffers (Images) it can write into with capabilities of presenting
* Stripped of I as it has definitions
*/
class VRIXIC_API SwapChain : public Interface
{
private:
    Surface* SurfaceHandle;

public:
    /**
    * Presents the current buffer to the screen
    */
    virtual void Present() = 0;

    /**
    * Resizes all buffers/images within the swapchain, essentially recreating the swapchain
    * @remarks use cases is on window resize or what ever render target its rendering to resized
    */
    virtual void ResizeBuffers(uint32* outImageWidth, uint32* outImageHeight) = 0;

    /**
    * Sets the vsync interval for this swapchain (vertical synchronization), 0 to disable, and 1 or more halfs the refresh rate
    *
    * @returns bool true if successfully set to new vsync interval, false otherwise making it invalid
    */
    virtual bool SetVSyncInterval(uint32 inVSyncInterval) = 0;

    /**
    * @returns EFormat the color format of this swapchain
    */
    virtual EFormat GetColorFormat() const = 0;

    /**
    * @returns EFormat the depth stencil format of this swapchain
    */
    virtual EFormat GetDepthStencilFormat() const = 0;

    Surface& GetSurfaceHandle() const
    {
        return *SurfaceHandle;
    }
};
